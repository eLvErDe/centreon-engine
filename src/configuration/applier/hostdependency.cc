/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostdependency::hostdependency() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostdependency::hostdependency(
                           applier::hostdependency const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostdependency::~hostdependency() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostdependency& applier::hostdependency::operator=(
                           applier::hostdependency const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostdependency.
 *
 *  @param[in] obj The new host dependency to add into the monitoring
 *                 engine.
 */
void applier::hostdependency::add_object(
                                shared_ptr<configuration::hostdependency> obj) {
  // Check host dependency.
  if ((obj->hosts().size() != 1)
      || !obj->hostgroups().empty()
      || (obj->dependent_hosts().size() != 1)
      || !obj->dependent_hostgroups().empty())
    throw (engine_error() << "Could not create host dependency "
           "with multiple (dependent) host / host groups");
  if ((obj->dependency_type()
       != configuration::hostdependency::execution_dependency)
      && (obj->dependency_type()
          != configuration::hostdependency::notification_dependency))
    throw (engine_error() << "Could not create unexpanded "
           << "host dependency of '" << obj->dependent_hosts().front()
           << "' on '" << obj->hosts().front() << "'");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new host dependency of host '"
    << obj->dependent_hosts().front() << "' on host '"
    << obj->hosts().front() << "'.";

  // Add dependency to the global configuration set.
  config->hostdependencies().insert(obj);

  // Create execution dependency.
  if (obj->dependency_type()
      == configuration::hostdependency::execution_dependency) {
    if (!add_host_dependency(
           obj->dependent_hosts().front().c_str(),
           obj->hosts().front().c_str(),
           EXECUTION_DEPENDENCY,
           obj->inherits_parent(),
           static_cast<bool>(
             obj->execution_failure_options()
             & configuration::hostdependency::up),
           static_cast<bool>(
             obj->execution_failure_options()
             & configuration::hostdependency::down),
           static_cast<bool>(
             obj->execution_failure_options()
             & configuration::hostdependency::unreachable),
           static_cast<bool>(
             obj->execution_failure_options()
             & configuration::hostdependency::pending),
           NULL_IF_EMPTY(obj->dependency_period())))
      throw (engine_error() << "Could not create host execution "
             << "dependency of '" << obj->dependent_hosts().front()
             << "' on '" << obj->hosts().front() << "'");
  }
  // Create notification dependency.
  else
    if (!add_host_dependency(
           obj->dependent_hosts().front().c_str(),
           obj->hosts().front().c_str(),
           NOTIFICATION_DEPENDENCY,
           obj->inherits_parent(),
           static_cast<bool>(
             obj->notification_failure_options()
             & configuration::hostdependency::up),
           static_cast<bool>(
             obj->notification_failure_options()
             & configuration::hostdependency::down),
           static_cast<bool>(
             obj->notification_failure_options()
             & configuration::hostdependency::unreachable),
           static_cast<bool>(
             obj->notification_failure_options()
             & configuration::hostdependency::pending),
           NULL_IF_EMPTY(obj->dependency_period())))
      throw (engine_error() << "Could not create host "
             << "notification dependency of '"
             << obj->dependent_hosts().front() << "' on '"
             << obj->hosts().front() << "'");

  return ;
}

/**
 *  Expand host dependency.
 *
 *  @param[in]     obj Host dependency object.
 *  @param[in,out] s   Configuration being applied.
 */
void applier::hostdependency::expand_object(
                                shared_ptr<configuration::hostdependency> obj,
                                configuration::state& s) {
  // Expand host dependency instances.
  if ((obj->hosts().size() != 1)
      || !obj->hostgroups().empty()
      || (obj->dependent_hosts().size() != 1)
      || !obj->dependent_hostgroups().empty()
      || (obj->dependency_type()
          == configuration::hostdependency::unknown)) {
    // Expanded depended hosts.
    std::set<std::string> depended_hosts;
    _expand_hosts(
      obj->hosts(),
      obj->hostgroups(),
      s,
      depended_hosts);

    // Expanded dependent hosts.
    std::set<std::string> dependent_hosts;
    _expand_hosts(
      obj->dependent_hosts(),
      obj->dependent_hostgroups(),
      s,
      dependent_hosts);

    // Remove current host dependency.
    s.hostdependencies().erase(obj);

    // Browse all depended and dependent hosts.
    for (std::set<std::string>::const_iterator
           it1(depended_hosts.begin()),
           end1(depended_hosts.end());
         it1 != end1;
         ++it1)
      for (std::set<std::string>::const_iterator
             it2(dependent_hosts.begin()),
             end2(dependent_hosts.end());
           it2 != end2;
           ++it2)
        for (unsigned int i(0); i < 2; ++i) {
          // Create host dependency instance.
          shared_ptr<configuration::hostdependency>
            hdep(new configuration::hostdependency(*obj));
          hdep->hostgroups().clear();
          hdep->hosts().clear();
          hdep->hosts().push_back(*it1);
          hdep->dependent_hostgroups().clear();
          hdep->dependent_hosts().clear();
          hdep->dependent_hosts().push_back(*it2);
          hdep->dependency_type(
           !i
           ? configuration::hostdependency::execution_dependency
           : configuration::hostdependency::notification_dependency);
          if (i)
            hdep->execution_failure_options(0);
          else
            hdep->notification_failure_options(0);

          // Insert new host dependency. We do not need to expand it
          // because no expansion is made on 1->1 dependency.
          s.hostdependencies().insert(hdep);
        }
  }

  return ;
}

/**
 *  @brief Modify host dependency.
 *
 *  Host dependencies cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj Unused.
 */
void applier::hostdependency::modify_object(
                                shared_ptr<configuration::hostdependency> obj) {
  (void)obj;
  throw (engine_error() << "Could not modify a host dependency: "
         << "Host dependency objects can only be added or removed, "
         << "this is likely a software bug that you should report to "
         << "Centreon Engine developers");
  return ;
}

/**
 *  Remove old host dependency.
 *
 *  @param[in] obj The host dependency to remove from the monitoring
 *                 engine.
 */
void applier::hostdependency::remove_object(
                                shared_ptr<configuration::hostdependency> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing a host dependency.";

  // Find host dependency.
  umultimap<std::string, shared_ptr<hostdependency_struct> >::iterator
    it(applier::state::instance().hostdependencies_find(obj->key()));
  if (it != applier::state::instance().hostdependencies().end()) {
    hostdependency_struct* dependency(it->second.get());

    // Remove host dependency from its list.
    unregister_object<hostdependency_struct>(
      &hostdependency_list,
      dependency);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_dependency_data(
      NEBTYPE_HOSTDEPENDENCY_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      dependency,
      &tv);

    // Erase host dependency (will effectively delete the object).
    applier::state::instance().hostdependencies().erase(it);
  }

  // Remove dependency from the global configuration set.
  config->hostdependencies().erase(obj);

  return ;
}

/**
 *  Resolve a hostdependency.
 *
 *  @param[in] obj Hostdependency object.
 */
void applier::hostdependency::resolve_object(
                shared_ptr<configuration::hostdependency> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving a host dependency.";

  // Find host dependency.
  umultimap<std::string, shared_ptr<hostdependency_struct> >::iterator
    it(applier::state::instance().hostdependencies_find(obj->key()));
  if (applier::state::instance().hostdependencies().end() == it)
    throw (engine_error() << "Cannot resolve non-existing "
           << "host dependency");

  // Resolve host dependency.
  if (!check_hostdependency(it->second.get(), NULL, NULL))
    throw (engine_error() << "Cannot resolve host dependency");

  return ;
}

/**
 *  Expand hosts.
 *
 *  @param[in]     hosts      Host list.
 *  @param[in]     hostgroups Host group list.
 *  @param[in,out] s          Configuration being applied.
 *  @param[out]    expanded   Expanded hosts.
 */
void applier::hostdependency::_expand_hosts(
                                std::list<std::string> const& hosts,
                                std::list<std::string> const& hostgroups,
                                configuration::state& s,
                                std::set<std::string>& expanded) {
  // Copy hosts.
  for (std::list<std::string>::const_iterator
         it(hosts.begin()),
         end(hosts.end());
       it != end;
       ++it)
    expanded.insert(*it);

  // Browse host groups.
  for (std::list<std::string>::const_iterator
         it(hostgroups.begin()),
         end(hostgroups.end());
       it != end;
       ++it) {
    // Find host group.
    set_hostgroup::iterator
      it_group(s.hostgroups().begin()),
      end_group(s.hostgroups().end());
    while (it_group != end_group) {
      if ((*it_group)->hostgroup_name() == *it)
        break ;
      ++it_group;
    }
    if (it_group == end_group)
      throw (engine_error()
             << "Could not expand non-existing host group '"
             << *it << "'");

    // Add host group members.
    for (std::set<std::string>::const_iterator
           it_member((*it_group)->resolved_members().begin()),
           end_member((*it_group)->resolved_members().end());
         it_member != end_member;
         ++it_member)
      expanded.insert(*it_member);
  }

  return ;
}
