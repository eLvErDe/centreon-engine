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

#include <cmath>
#include <cstddef>
#include <cstring>
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;
using namespace com::centreon::logging;

static applier::scheduler* _instance(NULL);

/**
 *  Apply new configuration.
 *
 *  @param[in] config        The new configuration.
 *  @param[in] diff_hosts    The difference between old and the
 *                           new host configuration.
 *  @param[in] diff_services The difference between old and the
 *                           new service configuration.
 */
void applier::scheduler::apply(
       configuration::state& config,
       difference<set_host> const& diff_hosts,
       difference<set_service> const& diff_services) {
  (void)config;

  // Remove and create misc event.
  _apply_misc_event();

  // Objects set.
  set_host hst_to_unschedule;
  set_service svc_to_unschedule;
  set_host hst_to_schedule;
  set_service svc_to_schedule;
  hst_to_unschedule = diff_hosts.deleted();
  svc_to_unschedule = diff_services.deleted();
  hst_to_schedule = diff_hosts.added();
  svc_to_schedule = diff_services.added();
  for (set_host::iterator
         it(diff_hosts.modified().begin()),
         end(diff_hosts.modified().end());
       it != end;
       ++it) {
    umap<std::string, shared_ptr<host_struct> > const&
      hosts(applier::state::instance().hosts());
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      hst(hosts.find((*it)->host_name()));
    if (hst != hosts.end()) {
      bool has_event(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::host_check,
                                         hst->second.get()));
      bool should_schedule((*it)->checks_active()
                           && ((*it)->check_interval() > 0));
      if (has_event && should_schedule) {
        hst_to_unschedule.insert(*it);
        hst_to_schedule.insert(*it);
      }
      else if (!has_event && should_schedule)
        hst_to_schedule.insert(*it);
      else if (has_event && !should_schedule)
        hst_to_unschedule.insert(*it);
      // Else it has no event and should not be scheduled, so do nothing.
    }
  }
  for (set_service::iterator
         it(diff_services.modified().begin()),
         end(diff_services.modified().end());
       it != end;
       ++it) {
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
      services(applier::state::instance().services());
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
      svc(services.find(std::make_pair(
                               (*it)->hosts().front(),
                               (*it)->service_description())));
    if (svc != services.end()) {
      bool has_event(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::service_check,
                                         svc->second.get()));
      bool should_schedule((*it)->checks_active()
                           && ((*it)->check_interval() > 0));
      if (has_event && should_schedule) {
        svc_to_unschedule.insert(*it);
        svc_to_schedule.insert(*it);
      }
      else if (!has_event && should_schedule)
        svc_to_schedule.insert(*it);
      else if (has_event && !should_schedule)
        svc_to_unschedule.insert(*it);
      // Else it has no event and should not be scheduled, so do nothing.
    }
  }

  // Remove deleted host check from the scheduler.
  {
    std::vector<host_struct*> old_hosts;
    _get_hosts(hst_to_unschedule, old_hosts, false);
    _unschedule_host_checks(old_hosts);
  }

  // Remove deleted service check from the scheduler.
  {
    std::vector<service_struct*> old_services;
    _get_services(svc_to_unschedule, old_services, false);
    _unschedule_service_checks(old_services);
  }

  // Check if we need to add or modify objects into the scheduler.
  if (!hst_to_schedule.empty() || !svc_to_schedule.empty()) {
    // Reset scheduling info.
    memset(&scheduling_info, 0, sizeof(scheduling_info));

    // Calculate scheduling parameters.
    _calculate_host_scheduling_params();
    _calculate_service_scheduling_params();

    // Get and schedule new hosts.
    {
      std::vector<host_struct*> new_hosts;
      _get_hosts(hst_to_schedule, new_hosts, true);
      _schedule_host_checks(new_hosts);
    }

    // Get and schedule new services.
    {
      std::vector<service_struct*> new_services;
      _get_services(svc_to_schedule, new_services, true);
      _schedule_service_checks(new_services);
    }
  }
}

/**
 *  Get the singleton instance of scheduler applier.
 *
 *  @return Singleton instance.
 */
applier::scheduler& applier::scheduler::instance() {
  return (*_instance);
}

/**
 *  Load scheduler applier singleton.
 */
void applier::scheduler::load() {
  if (!_instance)
    _instance = new applier::scheduler;
}

/**
 *  Remove some host from scheduling.
 *
 *  @param[in] h  Host configuration.
 */
void applier::scheduler::remove_host(configuration::host const& h) {
  umap<std::string, shared_ptr<host_struct> > const&
    hosts(applier::state::instance().hosts());
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    hst(hosts.find(h.host_name()));
  if (hst != hosts.end()) {
    std::vector<host_struct*> hvec;
    hvec.push_back(hst->second.get());
    _unschedule_host_checks(hvec);
  }
  return ;
}

/**
 *  Remove some service from scheduling.
 *
 *  @param[in] s  Service configuration.
 */
void applier::scheduler::remove_service(
                           configuration::service const& s) {
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    services(applier::state::instance().services());
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    svc(services.find(std::make_pair(
                             s.hosts().front(),
                             s.service_description())));
  if (svc != services.end()) {
    std::vector<service_struct*> svec;
    svec.push_back(svc->second.get());
    _unschedule_service_checks(svec);
  }
  return ;
}

/**
 *  Unload scheduler applier singleton.
 */
void applier::scheduler::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::scheduler::scheduler()
  : _evt_check_reaper(NULL),
    _evt_command_check(NULL),
    _evt_hfreshness_check(NULL),
    _evt_reschedule_checks(NULL),
    _evt_retention_save(NULL),
    _evt_sfreshness_check(NULL),
    _evt_status_save(NULL),
    _old_check_reaper_interval(0),
    _old_command_check_interval(0),
    _old_host_freshness_check_interval(0),
    _old_retention_update_interval(0),
    _old_service_freshness_check_interval(0) {
  memset(&scheduling_info, 0, sizeof(scheduling_info));
}

/**
 *  Default destructor.
 */
applier::scheduler::~scheduler() throw () {}

/**
 *  Remove and create misc event if necessary.
 */
void applier::scheduler::_apply_misc_event() {
  // Get current time.
  time_t const now(time(NULL));

  // Remove and add check result reaper event.
  if (!_evt_check_reaper
      || (_old_check_reaper_interval
          != config->check_reaper_interval())) {
    _remove_misc_event(_evt_check_reaper);
    _evt_check_reaper = _create_misc_event(
                          EVENT_CHECK_REAPER,
                          now + config->check_reaper_interval(),
                          config->check_reaper_interval());
    _old_check_reaper_interval = config->check_reaper_interval();
  }

  // Remove and add an external command check event.
  if (_old_command_check_interval != config->command_check_interval()) {
    _remove_misc_event(_evt_command_check);
    unsigned long interval;
    if (config->command_check_interval() != -1)
      interval = (unsigned long)config->command_check_interval();
    else
      interval = 5;
    _evt_command_check = _create_misc_event(
                           EVENT_COMMAND_CHECK,
                           now + interval,
                           interval);
    _old_command_check_interval = config->command_check_interval();
  }

  // Remove and add a host result "freshness" check event.
  if ((!_evt_hfreshness_check && config->check_host_freshness())
      || (_evt_hfreshness_check && !config->check_host_freshness())
      || (_old_host_freshness_check_interval
          != config->host_freshness_check_interval())) {
    _remove_misc_event(_evt_hfreshness_check);
    if (config->check_host_freshness())
      _evt_hfreshness_check
        = _create_misc_event(
            EVENT_HFRESHNESS_CHECK,
            now + config->host_freshness_check_interval(),
            config->host_freshness_check_interval());
    _old_host_freshness_check_interval
      = config->host_freshness_check_interval();
  }

  // Add a host and service check rescheduling event if necessary.
  if (!_evt_reschedule_checks) {
    _evt_reschedule_checks
      = _create_misc_event(
          EVENT_RESCHEDULE_CHECKS,
          now + auto_rescheduling_interval,
          auto_rescheduling_interval);
  }

  // Remove and add a retention data save event if needed.
  if (!_evt_retention_save
      || (_old_retention_update_interval
          != config->retention_update_interval())) {
    _remove_misc_event(_evt_retention_save);
    unsigned long interval(config->retention_update_interval() * 60);
    _evt_retention_save = _create_misc_event(
                            EVENT_RETENTION_SAVE,
                            now + interval,
                            interval);
    _old_retention_update_interval
      = config->retention_update_interval();
  }

  // Remove add a service result "freshness" check event.
  if ((!_evt_sfreshness_check && config->check_service_freshness())
      || (!_evt_sfreshness_check && !config->check_service_freshness())
      || (_old_service_freshness_check_interval
          != config->service_freshness_check_interval())) {
    _remove_misc_event(_evt_sfreshness_check);
    if (config->check_service_freshness())
      _evt_sfreshness_check
        = _create_misc_event(
            EVENT_SFRESHNESS_CHECK,
            now + config->service_freshness_check_interval(),
            config->service_freshness_check_interval());
    _old_service_freshness_check_interval
      = config->service_freshness_check_interval();
  }

  return ;
}

/**
 *  Compute the host inter-check delay to use.
 */
void applier::scheduler::_calculate_host_inter_check_delay() {
  // Be smart and calculate the best delay that minimize local load
  // while scheduling host checks within a limited time frame.
  if (scheduling_info.total_scheduled_hosts > 0) {
    // Calculate inter-check delay.
    scheduling_info.host_inter_check_delay
      = static_cast<double>(scheduling_info.host_check_spread)
        / scheduling_info.total_scheduled_hosts;

    logger(dbg_events, more)
      << "Total scheduled host checks:  "
      << scheduling_info.total_scheduled_hosts;
    logger(dbg_events, more)
      << setprecision(2) << "Average host check interval:    "
      << scheduling_info.average_host_check_interval << " sec";
    logger(dbg_events, more)
      << setprecision(2) << "Host inter-check delay:       "
      << scheduling_info.host_inter_check_delay << " sec";
  }
  else
    scheduling_info.host_inter_check_delay = 0.0;

  return ;
}

/**
 *  Compute host scheduling parameters (check spread, average check
 *  interval, ...).
 */
void applier::scheduler::_calculate_host_scheduling_params() {
  logger(dbg_events, most)
    << "Determining host scheduling parameters...";

  // Counter.
  double host_check_interval_total(0.0);

  // Get current time.
  time_t const now(time(NULL));

  // Get total hosts, total scheduled hosts and host check spread.
  double host_check_spread(std::numeric_limits<double>::max());
  for (umap<std::string, shared_ptr<host_struct> >::const_iterator
         it(applier::state::instance().hosts().begin()),
         end(applier::state::instance().hosts().end());
         it != end;
         ++it) {
    host_struct& hst(*it->second);

    bool schedule_check(true);
    if ((hst.check_interval <= 0) || !hst.checks_enabled)
      schedule_check = false;
    else {
      if (check_time_against_period(
            now,
            hst.check_period_ptr,
            hst.timezone) == ERROR) {
        time_t next_valid_time(0);
        get_next_valid_time(
          now,
          &next_valid_time,
          hst.check_period_ptr,
          hst.timezone);
        if (now == next_valid_time)
          schedule_check = false;
      }
    }

    ++scheduling_info.total_hosts;

    if (schedule_check) {
      ++scheduling_info.total_scheduled_hosts;
      host_check_interval_total += hst.check_interval;
      if (hst.check_interval < host_check_spread)
        host_check_spread = hst.check_interval;
      if ((hst.retry_interval < host_check_spread)
          && (hst.retry_interval > 0))
        host_check_spread = hst.retry_interval;
    }
    else {
      hst.should_be_scheduled = false;
      logger(dbg_events, more)
        << "Host " << hst.name << " should not be scheduled.";
    }
  }

  // Compute statistics.
  if (scheduling_info.total_scheduled_hosts) {
    scheduling_info.average_host_check_interval
      = host_check_interval_total
      / scheduling_info.total_scheduled_hosts;
  }
  else
    scheduling_info.average_host_check_interval = 0.0;

  // Compute additional scheduling parameters.
  if ((host_check_spread < 0.0)
      || (host_check_spread > 366.0 * 24 * 60 * 60))
    scheduling_info.host_check_spread = 0;
  else
    scheduling_info.host_check_spread
      = static_cast<int>(host_check_spread);
  _calculate_host_inter_check_delay();

  return ;
}

/**
 *  Compute the service inter-check delay to use.
 */
void applier::scheduler::_calculate_service_inter_check_delay() {
  // Be smart and calculate the best delay that will minimize local load
  // while scheduling service checks within a limited time frame.
  if (scheduling_info.total_scheduled_services > 0) {
    // Calculate inter-check delay.
    scheduling_info.service_inter_check_delay
      = static_cast<double>(scheduling_info.service_check_spread)
        / scheduling_info.total_scheduled_services;

    logger(dbg_events, more)
      << "Total scheduled service checks:  "
      << scheduling_info.total_scheduled_services;
    logger(dbg_events, more)
      << setprecision(2) << "Average service check interval:  "
      << scheduling_info.average_service_check_interval << " sec";
    logger(dbg_events, more)
      << setprecision(2) << "Service inter-check delay:       "
      << scheduling_info.service_inter_check_delay << " sec";
  }
  else
    scheduling_info.service_inter_check_delay = 0.0;

  return ;
}

/**
 *  Compute the service interleave factor.
 */
void applier::scheduler::_calculate_service_interleave_factor() {
  scheduling_info.service_interleave_factor
    = (int)(ceil(scheduling_info.average_scheduled_services_per_host));

  logger(dbg_events, more)
    << "Service Interleave factor:      "
    << scheduling_info.service_interleave_factor;

  return ;
}

/**
 *  Compute service scheduling parameters (check spread, average check
 *  interval, interleave factor, ...).
 */
void applier::scheduler::_calculate_service_scheduling_params() {
  logger(dbg_events, most)
    << "Determining service scheduling parameters...";

  // Counter.
  double service_check_interval_total(0.0);

  // Get current time.
  time_t const now(time(NULL));

  // Get total services, total scheduled services
  // and service check spread.
  double service_check_spread(std::numeric_limits<double>::max());
  for (umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
         it(applier::state::instance().services().begin()),
         end(applier::state::instance().services().end());
       it != end;
       ++it) {
    service_struct& svc(*it->second);

    bool schedule_check(true);
    if ((svc.check_interval <= 0.0) || !svc.checks_enabled)
      schedule_check = false;
    else {
      if (check_time_against_period(
            now,
            svc.check_period_ptr,
            svc.timezone) == ERROR) {
        time_t next_valid_time(0);
        get_next_valid_time(
          now,
          &next_valid_time,
          svc.check_period_ptr,
          svc.timezone);
        if (now == next_valid_time)
          schedule_check = false;
      }
    }

    ++scheduling_info.total_services;

    if (schedule_check) {
      ++scheduling_info.total_scheduled_services;
      service_check_interval_total += svc.check_interval;
      if (svc.check_interval < service_check_spread)
        service_check_spread = svc.check_interval;
      if ((svc.retry_interval < service_check_spread)
          && (svc.retry_interval > 0))
        service_check_spread = svc.retry_interval;
    }
    else {
      svc.should_be_scheduled = false;
      logger(dbg_events, more)
        << "Service " << svc.description << " on host " << svc.host_name
        << " should not be scheduled.";
    }
  }

  // Compute statistics.
  if (scheduling_info.total_hosts) {
    scheduling_info.average_services_per_host
      = scheduling_info.total_services
      / (double)scheduling_info.total_hosts;
    scheduling_info.average_scheduled_services_per_host
      = scheduling_info.total_scheduled_services
      / (double)scheduling_info.total_hosts;
  }
  else {
    scheduling_info.average_services_per_host = 0.0;
    scheduling_info.average_scheduled_services_per_host = 0.0;
  }
  if (scheduling_info.total_scheduled_services)
    scheduling_info.average_service_check_interval
      = service_check_interval_total
        / scheduling_info.total_scheduled_services;
  else
    scheduling_info.average_service_check_interval = 0.0;

  // Compute additional scheduling parameters.
  if ((service_check_spread < 0.0)
      || (service_check_spread > 366.0 * 24 * 60 * 60))
    scheduling_info.service_check_spread = 0;
  else
    scheduling_info.service_check_spread
      = static_cast<int>(service_check_spread);
  _calculate_service_inter_check_delay();
  _calculate_service_interleave_factor();

  return ;
}

/**
 *  Create and register new misc event.
 *
 *  @param[in] type     The event type.
 *  @param[in] start    The date time to start event.
 *  @param[in] interval The rescheduling interval.
 *  @param[in] data     The timed event data.
 *
 *  @return The new event.
 */
timed_event* applier::scheduler::_create_misc_event(
               int type,
               time_t start,
               unsigned long interval,
               void* data) {
  return (events::schedule(
                    type,
                    true,
                    start,
                    true,
                    interval,
                    NULL,
                    true,
                    data,
                    NULL,
                    0));
}

/**
 *  Get engine hosts struct with configuration hosts objects.
 *
 *  @param[in]  hst_cfg             The list of configuration hosts objects.
 *  @param[out] hst_obj             The list of engine hosts to fill.
 *  @param[in]  throw_if_not_found  Flag to throw if an host is not
 *                                  found.
 */
void applier::scheduler::_get_hosts(
       set_host const& hst_cfg,
       std::vector<host_struct*>& hst_obj,
       bool throw_if_not_found) {
  umap<std::string, shared_ptr<host_struct> > const&
    hosts(applier::state::instance().hosts());
  for (set_host::const_reverse_iterator
         it(hst_cfg.rbegin()), end(hst_cfg.rend());
       it != end;
       ++it) {
    std::string const& host_name((*it)->host_name());
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      hst(hosts.find(host_name));
    if (hst == hosts.end()) {
      if (throw_if_not_found)
        throw (engine_error() << "Could not schedule non-existing host '"
               << host_name << "'");
    }
    else
      hst_obj.push_back(&*hst->second);
  }
  return ;
}

/**
 *  Get engine services struct with configuration services objects.
 *
 *  @param[in]  svc_cfg             The list of configuration services objects.
 *  @param[out] svc_obj             The list of engine services to fill.
 *  @param[in]  throw_if_not_found  Flag to throw if an host is not
 *                                  found.
 */
void applier::scheduler::_get_services(
       set_service const& svc_cfg,
       std::vector<service_struct*>& svc_obj,
       bool throw_if_not_found) {
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    services(applier::state::instance().services());
  for (set_service::const_reverse_iterator
         it(svc_cfg.rbegin()), end(svc_cfg.rend());
       it != end;
       ++it) {
    std::string const& host_name((*it)->hosts().front());
    std::string const& service_description((*it)->service_description());
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
      svc(services.find(std::make_pair(host_name, service_description)));
    if (svc == services.end()) {
      if (throw_if_not_found)
        throw (engine_error() << "Cannot schedule non-existing service '"
               << service_description << "' on host '"
               << host_name << "'");
    }
    else
      svc_obj.push_back(&*svc->second);
  }
  return ;
}

/**
 *  Remove misc event.
 *
 *  @param[int,out] evt The event to remove.
 */
void applier::scheduler::_remove_misc_event(timed_event*& evt) {
  if (evt) {
    remove_event(evt, &event_list_high, &event_list_high_tail);
    delete evt;
    evt = NULL;
  }
}

/**
 *  Schedule host checks.
 *
 *  @param[in] hosts The list of hosts to schedule.
 */
void applier::scheduler::_schedule_host_checks(
       std::vector<host_struct*> const& hosts) {
  logger(dbg_events, most)
    << "Scheduling host checks...";

  // get current time.
  time_t const now(time(NULL));

  unsigned int const end(hosts.size());

  // determine check times for host checks.
  int mult_factor(0);
  for (unsigned int i(0); i < end; ++i) {
    host_struct& hst(*hosts[i]);

    logger(dbg_events, most)
      << "Host '" <<  hst.name << "'";

    // skip hosts that shouldn't be scheduled.
    if (!hst.should_be_scheduled) {
      logger(dbg_events, most)
        << "Host check should not be scheduled.";
      continue;
    }

    // calculate preferred host check time.
    hst.next_check
      = (time_t)(now
           + (mult_factor * scheduling_info.host_inter_check_delay));

    logger(dbg_events, most)
      << "Preferred Check Time: " << hst.next_check
      << " --> " << my_ctime(&hst.next_check);

    // Make sure the host can actually be scheduled at this time.
    if (check_time_against_period(
          hst.next_check,
          hst.check_period_ptr,
          hst.timezone) == ERROR) {
      time_t next_valid_time(0);
      get_next_valid_time(
        hst.next_check,
        &next_valid_time,
        hst.check_period_ptr,
        hst.timezone);
      hst.next_check = next_valid_time;
    }

    logger(dbg_events, most)
      << "Actual Check Time: " << hst.next_check
      << " --> " << my_ctime(&hst.next_check);

    if (!scheduling_info.first_host_check
        || (hst.next_check < scheduling_info.first_host_check))
      scheduling_info.first_host_check = hst.next_check;
    if (hst.next_check > scheduling_info.last_host_check)
      scheduling_info.last_host_check = hst.next_check;

    ++mult_factor;
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, host_struct*> hosts_to_schedule;

  // add scheduled host checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    host_struct& hst(*hosts[i]);

    // Update status of all hosts (scheduled or not).
    update_host_status(&hst);

    // skip most hosts that shouldn't be scheduled.
    if (!hst.should_be_scheduled) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(hst.checks_enabled == false
            && hst.next_check
            && (hst.check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    hosts_to_schedule.insert(std::make_pair(hst.next_check, &hst));
  }

  // Schedule events list.
  for (std::multimap<time_t, host_struct*>::const_iterator
         it(hosts_to_schedule.begin()), end(hosts_to_schedule.end());
       it != end;
       ++it) {
    host_struct& hst(*it->second);

    // Schedule a new host check event.
    events::schedule(
              EVENT_HOST_CHECK,
              false,
              hst.next_check,
              false,
              0,
              NULL,
              true,
              (void*)&hst,
              NULL,
              hst.check_options);
  }

  return ;
}

/**
 *  Schedule service checks.
 *
 *  @param[in] services The list of services to schedule.
 */
void applier::scheduler::_schedule_service_checks(
       std::vector<service_struct*> const& services) {
  logger(dbg_events, most)
    << "Scheduling service checks...";

  // get current time.
  time_t const now(time(NULL));

  int total_interleave_blocks(scheduling_info.total_scheduled_services);
  // calculate number of service interleave blocks.
  if (scheduling_info.service_interleave_factor)
    total_interleave_blocks
      = (int)ceil(scheduling_info.total_scheduled_services
                  / (double)scheduling_info.service_interleave_factor);

  // determine check times for service checks (with
  // interleaving to minimize remote load).

  int current_interleave_block(0);
  unsigned int const end(services.size());

  if (scheduling_info.service_interleave_factor > 0) {
    int interleave_block_index(0);
    for (unsigned int i(0); i < end; ++i) {
      service_struct& svc(*services[i]);
      if (interleave_block_index >= scheduling_info.service_interleave_factor) {
        ++current_interleave_block;
        interleave_block_index = 0;
      }

      // skip this service if it shouldn't be scheduled.
      if (!svc.should_be_scheduled)
        continue;

      int const mult_factor(
            current_interleave_block
            + ++interleave_block_index * total_interleave_blocks);

      // set the preferred next check time for the service.
      svc.next_check
        = (time_t)(now
             + mult_factor * scheduling_info.service_inter_check_delay);

      // Make sure the service can actually be scheduled when we want.
      if (check_time_against_period(
            svc.next_check,
            svc.check_period_ptr,
            svc.timezone) == ERROR) {
        time_t next_valid_time(0);
        get_next_valid_time(
          svc.next_check,
          &next_valid_time,
          svc.check_period_ptr,
          svc.timezone);
        svc.next_check = next_valid_time;
      }

      if (!scheduling_info.first_service_check
          || svc.next_check < scheduling_info.first_service_check)
        scheduling_info.first_service_check = svc.next_check;
      if (svc.next_check > scheduling_info.last_service_check)
        scheduling_info.last_service_check = svc.next_check;
    }
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, service_struct*> services_to_schedule;

  // add scheduled service checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    service_struct& svc(*services[i]);

    // Update status of all services (scheduled or not).
    update_service_status(&svc);

    // skip most services that shouldn't be scheduled.
    if (!svc.should_be_scheduled) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(svc.checks_enabled == false
            && svc.next_check
            && (svc.check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    services_to_schedule.insert(std::make_pair(svc.next_check, &svc));
  }

  // Schedule events list.
  for (std::multimap<time_t, service_struct*>::const_iterator
         it(services_to_schedule.begin()),
         end(services_to_schedule.end());
       it != end;
       ++it) {
    service_struct& svc(*it->second);
    // Create a new service check event.
    events::schedule(
              EVENT_SERVICE_CHECK,
              false,
              svc.next_check,
              false,
              0,
              NULL,
              true,
              (void*)&svc,
              NULL,
              svc.check_options);
  }

  return ;
}

/**
 *  Unschedule host checks.
 *
 *  @param[in] hosts The list of hosts to unschedule.
 */
void applier::scheduler::_unschedule_host_checks(
                           std::vector<host_struct*> const& hosts) {
  for (std::vector<host_struct*>::const_iterator
         it(hosts.begin()),
         end(hosts.end());
       it != end;
       ++it) {
    timed_event* evt(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::host_check,
                                         *it));
    while (evt) {
      remove_event(evt, &event_list_low, &event_list_low_tail);
      delete evt;
      evt = quick_timed_event.find(
                                events::hash_timed_event::low,
                                events::hash_timed_event::host_check,
                                *it);
    }
  }
  return ;
}

/**
 *  Schedule service checks.
 *
 *  @param[in] services The list of services to schedule.
 */
void applier::scheduler::_unschedule_service_checks(
                           std::vector<service_struct*> const& services) {
  for (std::vector<service_struct*>::const_iterator
         it(services.begin()),
         end(services.end());
       it != end;
       ++it) {
    timed_event* evt(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::service_check,
                                         *it));
    while (evt) {
      remove_event(evt, &event_list_low, &event_list_low_tail);
      delete evt;
      evt = quick_timed_event.find(
                                events::hash_timed_event::low,
                                events::hash_timed_event::service_check,
                                *it);
    }
  }
  return ;
}
