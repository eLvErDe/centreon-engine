/*
** Copyright 2011-2015 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::host::host() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::host::host(applier::host const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::host::~host() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::host& applier::host::operator=(applier::host const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new host.
 *
 *  @param[in] obj The new host to add into the monitoring engine.
 */
void applier::host::add_object(
                      shared_ptr<configuration::host> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new host '" << obj->host_name() << "'.";

  // Add host to the global configuration set.
  config->hosts().insert(obj);

  // Create host.
  host_struct*
    h(add_host(
        obj->host_id(),
        obj->host_name().c_str(),
        NULL_IF_EMPTY(obj->alias()),
        NULL_IF_EMPTY(obj->address()),
        NULL_IF_EMPTY(obj->check_period()),
        obj->initial_state(),
        obj->check_interval(),
        obj->retry_interval(),
        obj->max_check_attempts(),
        (obj->check_timeout() >= 0) ? obj->check_timeout().get() : 0,
        NULL_IF_EMPTY(obj->check_command()),
        obj->checks_active(),
        NULL_IF_EMPTY(obj->event_handler()),
        obj->event_handler_enabled(),
        obj->flap_detection_enabled(),
        obj->low_flap_threshold(),
        obj->high_flap_threshold(),
        static_cast<bool>(obj->flap_detection_options()
                          & configuration::host::up),
        static_cast<bool>(obj->flap_detection_options()
                          & configuration::host::down),
        static_cast<bool>(obj->flap_detection_options()
                          & configuration::host::unreachable),
        obj->check_freshness(),
        obj->freshness_threshold(),
        true, // should_be_drawn, enabled by Nagios
        obj->obsess_over_host(),
        NULL_IF_EMPTY(obj->timezone())));
  if (!h)
    throw (engine_error() << "Could not register host '"
           << obj->host_name() << "'");
  host_other_props[obj->host_name()].should_reschedule_current_check = false;

  // Custom variables.
  for (map_customvar::const_iterator
         it(obj->customvariables().begin()),
         end(obj->customvariables().end());
       it != end;
       ++it)
    if (!add_custom_variable_to_host(
           h,
           it->first.c_str(),
           it->second.c_str()))
      throw (engine_error() << "Could not add custom variable '"
             << it->first << "' to host '" << obj->host_name() << "'");

  // Parents.
  for (list_string::const_iterator
         it(obj->parents().begin()),
         end(obj->parents().end());
       it != end;
       ++it)
    if (!add_parent_host_to_host(h, it->c_str()))
      throw (engine_error() << "Could not add parent '"
             << *it << "' to host '" << obj->host_name() << "'");

  return ;
}

/**
 *  @brief Expand a host.
 *
 *  Host configuration objects do not need expansion. Therefore this
 *  method does nothing.
 *
 *  @param[in] obj  Unused.
 *  @param[in] s    Unused.
 */
void applier::host::expand_object(
                      shared_ptr<configuration::host> obj,
                      configuration::state& s) {
  (void)obj;
  (void)s;
  return ;
}

/**
 *  Modified host.
 *
 *  @param[in] obj The new host to modify into the monitoring engine.
 */
void applier::host::modify_object(
                      shared_ptr<configuration::host> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying host '" << obj->host_name() << "'.";

  // Find the configuration object.
  set_host::iterator it_cfg(config->hosts_find(obj->key()));
  if (it_cfg == config->hosts().end())
    throw (engine_error() << "Cannot modify non-existing host '"
           << obj->host_name() << "'");

  // Find host object.
  umap<std::string, shared_ptr<host_struct> >::iterator
    it_obj(applier::state::instance().hosts_find(obj->key()));
  if (it_obj == applier::state::instance().hosts().end())
    throw (engine_error() << "Could not modify non-existing "
           << "host object '" << obj->host_name() << "'");
  host_struct* h(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::host> obj_old(*it_cfg);
  config->hosts().erase(it_cfg);
  config->hosts().insert(obj);

  // Modify properties.
  h->id = obj->host_id();
  modify_if_different(
    h->alias,
    (obj->alias().empty() ? obj->host_name() : obj-> alias()).c_str());
  modify_if_different(h->address, NULL_IF_EMPTY(obj->address()));
  modify_if_different(
    h->check_period,
    NULL_IF_EMPTY(obj->check_period()));
  modify_if_different(
    h->initial_state,
    static_cast<int>(obj->initial_state()));
  modify_if_different(
    h->check_interval,
    static_cast<double>(obj->check_interval()));
  modify_if_different(
    h->retry_interval,
    static_cast<double>(obj->retry_interval()));
  modify_if_different(
    h->max_attempts,
    static_cast<int>(obj->max_check_attempts()));
  modify_if_different(
    h->check_timeout,
    obj->check_timeout().get());
  modify_if_different(
    h->host_check_command,
    NULL_IF_EMPTY(obj->check_command()));
  modify_if_different(
    h->checks_enabled,
    static_cast<int>(obj->checks_active()));
  modify_if_different(
    h->event_handler,
    NULL_IF_EMPTY(obj->event_handler()));
  modify_if_different(
    h->event_handler_enabled,
    static_cast<int>(obj->event_handler_enabled()));
  modify_if_different(
    h->flap_detection_enabled,
    static_cast<int>(obj->flap_detection_enabled()));
  modify_if_different(
    h->low_flap_threshold,
    static_cast<double>(obj->low_flap_threshold()));
  modify_if_different(
    h->high_flap_threshold,
    static_cast<double>(obj->high_flap_threshold()));
  modify_if_different(
    h->flap_detection_on_up,
    static_cast<int>(static_cast<bool>(
      obj->flap_detection_options() & configuration::host::up)));
  modify_if_different(
    h->flap_detection_on_down,
    static_cast<int>(static_cast<bool>(
      obj->flap_detection_options() & configuration::host::down)));
  modify_if_different(
    h->flap_detection_on_unreachable,
    static_cast<int>(static_cast<bool>(
      obj->flap_detection_options() & configuration::host::unreachable)));
  modify_if_different(
    h->check_freshness,
    static_cast<int>(obj->check_freshness()));
  modify_if_different(
    h->freshness_threshold,
    static_cast<int>(obj->freshness_threshold()));
  modify_if_different(
    h->obsess_over_host,
    static_cast<int>(obj->obsess_over_host()));
  modify_if_different(
    h->timezone,
    NULL_IF_EMPTY(obj->timezone()));

  // Custom variables.
  if (obj->customvariables() != obj_old->customvariables()) {
    // Delete old custom variables.
    remove_all_custom_variables_from_host(h);

    // Add custom variables.
    for (map_customvar::const_iterator
           it(obj->customvariables().begin()),
           end(obj->customvariables().end());
         it != end;
         ++it)
      if (!add_custom_variable_to_host(
             h,
             it->first.c_str(),
             it->second.c_str()))
        throw (engine_error()
               << "Could not add custom variable '" << it->first
               << "' to host '" << obj->host_name() << "'");
  }

  // Parents.
  if (obj->parents() != obj_old->parents()) {
    // Delete old parents.
    deleter::listmember(h->parent_hosts, &deleter::hostsmember);

    // Create parents.
    for (list_string::const_iterator
           it(obj->parents().begin()),
           end(obj->parents().end());
         it != end;
         ++it)
      if (!add_parent_host_to_host(h, it->c_str()))
        throw (engine_error() << "Could not add parent '"
               << *it << "' to host '" << obj->host_name() << "'");
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Remove old host.
 *
 *  @param[in] obj The new host to remove from the monitoring engine.
 */
void applier::host::remove_object(
                      shared_ptr<configuration::host> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing host '" << obj->host_name() << "'.";

  // Find host.
  umap<std::string, shared_ptr<host_struct> >::iterator
    it(applier::state::instance().hosts_find(obj->key()));
  if (it != applier::state::instance().hosts().end()) {
    host_struct* hst(it->second.get());

    // Remove events related to this host.
    applier::scheduler::instance().remove_host(*obj);

    // Remove host from its list.
    unregister_object<host_struct>(&host_list, hst);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Erase host object (will effectively delete the object).
    host_other_props.erase(obj->host_name());
    applier::state::instance().hosts().erase(it);
  }

  // Remove host from the global configuration set.
  config->hosts().erase(obj);

  return ;
}

/**
 *  Resolve a host.
 *
 *  @param[in] obj Host object.
 */
void applier::host::resolve_object(
                      shared_ptr<configuration::host> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving host '" << obj->host_name() << "'.";

  // If it is the very first host to be resolved,
  // remove all the child backlinks of all the hosts.
  // It is necessary to do it only once to prevent the removal
  // of valid child backlinks.
  if (obj == *config->hosts().begin()) {
    for (umap<std::string, shared_ptr<host_struct> >::iterator
         it(applier::state::instance().hosts().begin()),
         end(applier::state::instance().hosts().end()); it != end; ++it)
      deleter::listmember(it->second->child_hosts, &deleter::hostsmember);
  }

  // Find host.
  umap<std::string, shared_ptr<host_struct> >::iterator
    it(applier::state::instance().hosts_find(obj->key()));
  if (applier::state::instance().hosts().end() == it)
    throw (engine_error() << "Cannot resolve non-existing host '"
           << obj->host_name() << "'");

  // Remove service backlinks.
  deleter::listmember(it->second->services, &deleter::servicesmember);

  // Reset host counters.
  it->second->total_services = 0;
  it->second->total_service_check_interval = 0;

  // Resolve host.
  if (!check_host(it->second.get(), &config_warnings, &config_errors))
    throw (engine_error() << "Cannot resolve host '"
           << obj->host_name() << "'");

  return ;
}
