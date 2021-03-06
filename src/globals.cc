/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;

configuration::state* config(NULL);
events::hash_timed_event quick_timed_event;
std::map<std::string, host_other_properties> host_other_props;

FILE*               debug_file_fp(NULL);
char*               command_file(NULL);
char*               config_file(NULL);
char*               debug_file(NULL);
char*               global_host_event_handler(NULL);
char*               global_service_event_handler(NULL);
char*               illegal_object_chars(NULL);
char*               illegal_output_chars(NULL);
char*               log_file(NULL);
char*               macro_user[MAX_USER_MACROS];  // $USERx$ macros
char*               macro_x_names[MACRO_X_COUNT]; // The macro names
char*               ochp_command(NULL);
char*               ocsp_command(NULL);
char*               use_timezone(NULL);
char**              macro_x(NULL);
check_stats         check_statistics[MAX_CHECK_STATS_TYPES];
command*            command_list(NULL);
command*            command_list_tail(NULL);
command*            global_host_event_handler_ptr(NULL);
command*            global_service_event_handler_ptr(NULL);
command*            ochp_command_ptr(NULL);
command*            ocsp_command_ptr(NULL);
float               high_host_flap_threshold(30.0);
float               high_service_flap_threshold(30.0);
float               low_host_flap_threshold(20.0);
float               low_service_flap_threshold(20.0);
float               sleep_time(0.1);
host*               host_list(NULL);
host*               host_list_tail(NULL);
hostdependency*     hostdependency_list(NULL);
hostdependency*     hostdependency_list_tail(NULL);
int                 __nagios_object_structure_version(CURRENT_OBJECT_STRUCTURE_VERSION);
int                 additional_freshness_latency(15);
int                 command_check_interval(-1);
int                 config_errors(0);
int                 config_warnings(0);
int                 external_command_buffer_slots(4096);
int                 log_host_retries(false);
bool                sighup(true);
bool                sigrestart(false);
bool                sigshutdown(false);
int                 test_scheduling(false);
int                 verify_circular_paths(true);
int                 verify_config(false);
nebcallback*        neb_callback_list[NEBCALLBACK_NUMITEMS];
sched_info          scheduling_info;
service*            service_list(NULL);
service*            service_list_tail(NULL);
servicedependency*  servicedependency_list(NULL);
servicedependency*  servicedependency_list_tail(NULL);
time_t              event_start(0L);
time_t              last_command_check(0L);
time_t              last_command_status_update(0L);
time_t              program_start(0L);
timed_event*        event_list_high(NULL);
timed_event*        event_list_high_tail(NULL);
timed_event*        event_list_low(NULL);
timed_event*        event_list_low_tail(NULL);
timeperiod*         timeperiod_list(NULL);
timeperiod*         timeperiod_list_tail(NULL);
unsigned int        check_host_freshness(false);
unsigned int        check_reaper_interval(10);
unsigned int        check_service_freshness(true);
unsigned int        currently_running_host_checks(0);
unsigned int        currently_running_service_checks(0);
unsigned int        debug_level(0);
unsigned int        debug_verbosity(1);
unsigned int        enable_event_handlers(true);
unsigned int        enable_flap_detection(false);
unsigned int        enable_predictive_host_dependency_checks(true);
unsigned int        enable_predictive_service_dependency_checks(true);
unsigned int        event_handler_timeout(30);
unsigned int        host_check_timeout(30);
unsigned int        host_freshness_check_interval(60);
unsigned int        log_event_handlers(true);
unsigned int        log_external_commands(true);
unsigned int        log_initial_states(false);
unsigned int        log_passive_checks(true);
unsigned int        log_service_retries(false);
unsigned int        max_parallel_service_checks(0);
unsigned int        obsess_over_hosts(false);
unsigned int        obsess_over_services(false);
unsigned int        ochp_timeout(15);
unsigned int        ocsp_timeout(15);
unsigned int        retention_update_interval(3600);
unsigned int        service_check_timeout(60);
unsigned int        service_freshness_check_interval(60);
unsigned int        soft_state_dependencies(false);
unsigned int        time_change_threshold(900);
unsigned int        use_syslog(true);
unsigned long       cached_host_check_horizon(15);
unsigned long       cached_service_check_horizon(15);
unsigned long       event_broker_options(~0);
unsigned long       logging_options(
                      logging::log_runtime_error
                      | logging::log_runtime_warning
                      | logging::log_verification_error
                      | logging::log_verification_warning
                      | logging::log_config_error
                      | logging::log_config_warning
                      | logging::log_process_info
                      | logging::log_event_handler
                      | logging::log_external_command
                      | logging::log_passive_check
                      | logging::log_host_up
                      | logging::log_host_down
                      | logging::log_host_unreachable
                      | logging::log_service_ok
                      | logging::log_service_warning
                      | logging::log_service_unknown
                      | logging::log_service_critical
                      | logging::log_info_message);
unsigned long       max_debug_file_size(1000000);
unsigned long       modified_host_process_attributes(MODATTR_NONE);
unsigned long       modified_service_process_attributes(MODATTR_NONE);
unsigned long       next_event_id(1);
unsigned long       next_problem_id(0L);
unsigned long       syslog_options(
                      logging::log_runtime_error
                      | logging::log_runtime_warning
                      | logging::log_verification_error
                      | logging::log_verification_warning
                      | logging::log_config_error
                      | logging::log_config_warning
                      | logging::log_process_info
                      | logging::log_event_handler
                      | logging::log_external_command
                      | logging::log_passive_check
                      | logging::log_host_up
                      | logging::log_host_down
                      | logging::log_host_unreachable
                      | logging::log_service_ok
                      | logging::log_service_warning
                      | logging::log_service_unknown
                      | logging::log_service_critical
                      | logging::log_info_message);
