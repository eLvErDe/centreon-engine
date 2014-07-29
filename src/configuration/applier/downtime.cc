/*
** Copyright 2011-2014 Merethis
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
#include "com/centreon/engine/configuration/applier/downtime.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/configuration/downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;


/**
 *  Default constructor.
 */
applier::downtime::downtime() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::downtime::downtime(applier::downtime const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::downtime::~downtime() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::downtime& applier::downtime::operator=(applier::downtime const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new downtime.
 *
 *  @param[in] obj The new downtime to add into the monitoring engine.
 */
void applier::downtime::add_object(
               shared_ptr<configuration::downtime> obj) {
  // If no recurring period, do nothing.
  if (obj->recurring_period())
    return ;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new downtime.";

  // Add downtime to the global configuration set.
  config->downtimes().insert(obj);

  // Create downtime.
  int ret = OK;
  unsigned long id;
  time_t new_start_time, new_end_time;
  get_new_recurring_times(obj->start_time(), obj->end_time(),
                          obj->recurring_interval(),
                          obj->recurring_period(),
                          &new_start_time, &new_end_time);
  ret = add_new_downtime(obj->downtime_type(),
                         obj->host_name().c_str(),
                         obj->service_description().c_str(),
                         obj->entry_time(),
                         obj->author().c_str(),
                         obj->comment_data().c_str(),
                         new_start_time,
                         new_end_time,
                         obj->fixed(),
                         obj->triggered_by(),
                         obj->duration(),
                         obj->recurring_interval(),
                         obj->recurring_period(),
                         &id);
  if (ret == ERROR)
    throw (engine_error() << "Could not register downtime '"
         << obj->host_name() << "'");

  register_downtime(obj->downtime_type(), id);
}

void applier::downtime::expand_object(
    shared_ptr<configuration::downtime> obj,
    configuration::state& s) {
  s.downtimes().insert(obj);
}

/**
 *  Modified downtime.
 *
 *  @param[in] obj The new downtime to modify into the monitoring engine.
 */
void applier::downtime::modify_object(
    shared_ptr<configuration::downtime> obj) {
  /*logger(logging::dbg_config, logging::more)
    << "Modifying downtime '" << obj->downtime_id() << "'.";

  // Find the configuration object.
  std::set<shared_ptr<configuration::downtime> >::iterator it_cfg(config->downtimes_find(obj->key()));
  if (it_cfg == config->downtimes().end())
    throw (engine_error() << "Cannot modify non-existing downtime '" << (long)obj->downtime_id() << "'");

  // Find downtime object.
  umap<unsigned long, shared_ptr<scheduled_downtime_struct> >::iterator
    it_obj(applier::state::instance().downtimes_find(obj->key()));
  if (it_obj == applier::state::instance().downtimes().end())
    throw (engine_error() << "Could not modify non-existing "
           << "downtime object '" << (long)obj->downtime_id() << "'");
  scheduled_downtime_struct* sd(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::downtime> obj_old(*it_cfg);
  config->downtimes().erase(it_cfg);
  config->downtimes().insert(obj);

  // Modify properties.
  modify_if_different(sd->author, obj->author().c_str());
  modify_if_different(sd->comment, obj->comment_data().c_str());
  modify_if_different(sd->type, (int)obj->downtime_id());
  modify_if_different(sd->duration, obj->duration());
  modify_if_different(sd->end_time, obj->end_time());
  modify_if_different(sd->start_time, obj->start_time());
  modify_if_different(sd->fixed, (int)obj->fixed());
  modify_if_different(sd->host_name, obj->host_name().c_str());
  modify_if_different(sd->service_description, obj->service_description().c_str());
  modify_if_different(sd->triggered_by, obj->triggered_by());
  modify_if_different(sd->recurring_interval, obj->recurring_interval());

  // Notify event broker.
  /*
    broker_downtime_data(
    NEBTYPE_DOWNTIME_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    downtime_type,
    host_name,
    svc_description,
    entry_time,
    author,
    comment_data,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    recurring_interval,
    recurring_period,
    downtime_id,
    NULL);
   */
  /*timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);*/

  return ;
}

void applier::downtime::remove_object(
    shared_ptr<configuration::downtime> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing downtime '" << obj->downtime_id() << "'.";

  // Find downtime.
  umap<unsigned long, shared_ptr<scheduled_downtime> >::iterator
    it(applier::state::instance().downtimes_find(obj->key()));
  if (it != applier::state::instance().downtimes().end()) {
    scheduled_downtime* sd(it->second.get());

    // Remove downtime from its list.
    //unregister_object<scheduled_downtime>(&downtime_list, sd);

    // Notify event broker.
    /*timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);*/

    // Erase downtime object (will effectively delete the object).
    applier::state::instance().downtimes().erase(it);
  }

  // Remove host from the global configuration set.
  config->downtimes().erase(obj);

  return ;
}

void applier::downtime::resolve_object(
    shared_ptr<configuration::downtime> obj) {
  if (obj->resolve_recurring_period() == false)
    throw (engine_error() << "Could not register downtime '"
         << obj->host_name() << "'");
}
