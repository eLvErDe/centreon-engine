/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <QTemporaryFile>
#include <QDir>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <fstream>
#include <map>

#include "engine.hh"
#include "globals.hh"
#include "macros.hh"
#include "error.hh"

using namespace com::centreon::engine;

/**
 *  Create a random integer between min and max.
 *
 *  @param[in] min The minimum.
 *  @param[in] max The maximum.
 */
static int my_rand(int min = INT_MIN + 1, int max = INT_MAX - 1) throw() {
  return ((random() % (max - min + 1)) + min);
}

/**
 *  Create a random float between min and max.
 *
 *  @param[in] min The minimum.
 *  @param[in] max The maximum.
 */
static float my_rand(float min, float max) throw() {
  return ((max - min) * ((float)random() / (float)RAND_MAX) + min);
}

/**
 *  Get value into string.
 *
 *  @param[in] value The value to translate on string.
 *
 *  @return The value into string.
 */
template<class T> static QString obj2str(T const& value) {
  std::ostringstream oss;
  oss << value;
  return (oss.str().c_str());
}

/**
 *  Build the resource file.
 *
 *  @param[in] resource The resource file name.
 */
static void build_resource(QString const& resource) {
  std::ofstream ofs(resource.toStdString().c_str());
  for (unsigned int i = 0; i < MAX_USER_MACROS; ++i) {
    if (my_rand(1, 5) == 5) {
      ofs << "# comment !" << std::endl;
    }

    ofs << std::string(my_rand(0, 10), ' ')
    	<< "$USER" + obj2str(i).toStdString() + "$"
    	<< std::string(my_rand(0, 10), ' ')
    	<< "="
    	<< std::string(my_rand(0, 10), ' ')
    	<< "USER" + obj2str(i).toStdString()
    	<< std::string(my_rand(0, 10), ' ')
    	<< std::endl;
  }
  ofs.close();
}

/**
 *  Build the main configuration file.
 *
 *  @param[in] mainconf The main configuration file name.
 *  @param[in] resource The resource file name.
 *
 *  @return The configuration value.
 */
static std::map<QString, QString> build_configuration(QString const& mainconf, QString const& resource) {
  std::map<QString, QString> var;

  QString date_format[] = { "us", "euro", "iso8601", "strict-iso8601" };
  QString log_rotation[] = { "n", "h", "d", "w", "m" };
  QString check_delay[] = { "n", "d", "s", "" };

  int cmd_check_interval = my_rand(-1, 10000);
  cmd_check_interval = (cmd_check_interval == 0 ? -1 : cmd_check_interval);

  QString scd = check_delay[my_rand(0, 3)];
  if (scd == "")
    scd = obj2str(my_rand(0.1f, 10000.0f));

  QString hcd = check_delay[my_rand(0, 3)];
  if (hcd == "")
    hcd = obj2str(my_rand(0.1f, 10000.0f));

  var["command_check_interval"] = obj2str(cmd_check_interval) + (my_rand(0, 1) ? "s" : "");
  var["service_inter_check_delay_method"] = scd;
  var["host_inter_check_delay_method"] = hcd;
  var["service_interleave_factor"] = (my_rand(0, 1) == 0 ? "s" : obj2str(my_rand(1)));
  var["resource_file"] = resource;
  var["log_file"] = "log_file.tmp";
  var["debug_level"] = obj2str(my_rand(0));
  var["debug_verbosity"] = obj2str(my_rand(0));
  var["debug_file"] = "debug_file.tmp";
  var["max_debug_file_size"] = obj2str(my_rand(0));
  var["command_file"] = "command_file.tmp";
  var["temp_file"] = "temp_file.tmp";
  var["temp_path"] = "./";
  var["check_result_path"] = "./";
  var["max_check_result_file_age"] = obj2str(my_rand(0));
  var["global_host_event_handler"] = "host-event-handler";
  var["global_service_event_handler"] = "service-event-handler";
  var["ocsp_command"] = "ocsp-command";
  var["ochp_command"] = "ochp-command";
  var["admin_email"] = "admin@localhost";
  var["admin_pager"] = "pageadmin@localhost";
  var["use_syslog"] = obj2str(my_rand(0, 1));
  var["log_notifications"] = obj2str(my_rand(0, 1));
  var["log_service_retries"] = obj2str(my_rand(0, 1));
  var["log_host_retries"] = obj2str(my_rand(0, 1));
  var["log_event_handlers"] = obj2str(my_rand(0, 1));
  var["log_external_commands"] = obj2str(my_rand(0, 1));
  var["log_passive_checks"] = obj2str(my_rand(0, 1));
  var["log_initial_states"] = obj2str(my_rand(0, 1));
  var["retain_state_information"] = obj2str(my_rand(0, 1));
  var["retention_update_interval"] = obj2str(my_rand(0));
  var["use_retained_program_state"] = obj2str(my_rand(0, 1));
  var["use_retained_scheduling_info"] = obj2str(my_rand(0, 1));
  var["retention_scheduling_horizon"] = obj2str(my_rand(0));
  var["additional_freshness_latency"] = obj2str(my_rand());
  var["retained_host_attribute_mask"] = obj2str(my_rand(0));
  var["retained_service_attribute_mask"] = obj2str(my_rand(0));
  var["retained_process_host_attribute_mask"] = obj2str(my_rand(0));
  var["retained_process_service_attribute_mask"] = obj2str(my_rand(0));
  var["retained_contact_host_attribute_mask"] = obj2str(my_rand(0));
  var["retained_contact_service_attribute_mask"] = obj2str(my_rand(0));
  var["obsess_over_services"] = obj2str(my_rand(0, 1));
  var["obsess_over_hosts"] = obj2str(my_rand(0, 1));
  var["translate_passive_host_checks"] = obj2str(my_rand(0, 1));
  var["passive_host_checks_are_soft"] = obj2str(my_rand(0, 1));
  var["service_check_timeout"] = obj2str(my_rand(1));
  var["host_check_timeout"] = obj2str(my_rand(1));
  var["event_handler_timeout"] = obj2str(my_rand(1));
  var["notification_timeout"] = obj2str(my_rand(1));
  var["ocsp_timeout"] = obj2str(my_rand(1));
  var["ochp_timeout"] = obj2str(my_rand(1));
  var["use_agressive_host_checking"] = obj2str(my_rand(0, 1));
  var["use_aggressive_host_checking"] = var["use_agressive_host_checking"];
  var["cached_host_check_horizon"] = obj2str(my_rand(0));
  var["enable_predictive_host_dependency_checks"] = obj2str(my_rand(0, 1));
  var["cached_service_check_horizon"] = obj2str(my_rand(0));
  var["enable_predictive_service_dependency_checks"] = obj2str(my_rand(0, 1));
  var["soft_state_dependencies"] = obj2str(my_rand(0, 1));
  var["log_rotation_method"] = log_rotation[my_rand(0, 4)];
  var["log_archive_path"] = "./";
  var["enable_event_handlers"] = obj2str(my_rand(0, 1));
  var["enable_notifications"] = obj2str(my_rand(0, 1));
  var["execute_service_checks"] = obj2str(my_rand(0, 1));
  var["accept_passive_service_checks"] = obj2str(my_rand(0, 1));
  var["execute_host_checks"] = obj2str(my_rand(0, 1));
  var["accept_passive_host_checks"] = obj2str(my_rand(0, 1));
  var["max_service_check_spread"] = obj2str(my_rand(1));
  var["max_host_check_spread"] = obj2str(my_rand(1));
  var["max_concurrent_checks"] = obj2str(my_rand(0));
  var["check_result_reaper_frequency"] = obj2str(my_rand(1));
  var["service_reaper_frequency"] = var["check_result_reaper_frequency"];
  var["max_check_result_reaper_time"] = obj2str(my_rand(1));
  var["sleep_time"] = obj2str(my_rand(0.1f, 10000.0f));
  var["interval_length"] = obj2str(my_rand(1, 10000));
  var["check_external_commands"] = obj2str(my_rand(0, 1));
  var["check_for_orphaned_services"] = obj2str(my_rand(0, 1));
  var["check_for_orphaned_hosts"] = obj2str(my_rand(0, 1));
  var["check_service_freshness"] = obj2str(my_rand(0, 1));
  var["check_host_freshness"] = obj2str(my_rand(0, 1));
  var["service_freshness_check_interval"] = obj2str(my_rand(1));
  var["host_freshness_check_interval"] = obj2str(my_rand(1));
  var["auto_reschedule_checks"] = obj2str(my_rand(0, 1));
  var["auto_rescheduling_interval"] = obj2str(my_rand(1));
  var["auto_rescheduling_window"] = obj2str(my_rand(1));
  var["aggregate_status_updates"] = "DEPRECATED";
  var["status_update_interval"] = obj2str(my_rand(2));
  var["time_change_threshold"] = obj2str(my_rand(6));
  var["process_performance_data"] = obj2str(my_rand(0, 1));
  var["enable_flap_detection"] = obj2str(my_rand(0, 1));
  var["enable_failure_prediction"] = obj2str(my_rand(0, 1));
  var["low_service_flap_threshold"] = obj2str(my_rand(0.1f, 99.0f));
  var["high_service_flap_threshold"] = obj2str(my_rand(0.1f, 99.0f));
  var["low_host_flap_threshold"] = obj2str(my_rand(0.1f, 99.0f));
  var["high_host_flap_threshold"] = obj2str(my_rand(0.1f, 99.0f));
  var["date_format"] = date_format[my_rand(0, 3)];
  var["use_timezone"] = "US/Mountain";
  var["p1_file"] = "p1_file.tmp";
  var["event_broker_options"] = obj2str(my_rand(1));
  var["illegal_object_name_chars"] = "`~!$%^&*|'\"<>?,()";
  var["illegal_macro_output_chars"] = "`~$&|'\"<>";
  var["broker_module"] = "module argument";
  var["use_regexp_matching"] = obj2str(my_rand(0, 1)) ;
  var["use_true_regexp_matching"] = obj2str(my_rand(0, 1));
  var["use_large_installation_tweaks"] = obj2str(my_rand(0, 1));
  var["enable_environment_macros"] = obj2str(my_rand(0, 1));
  var["free_child_process_memory"] = obj2str(my_rand(0, 1));
  var["child_processes_fork_twice"] = obj2str(my_rand(0, 1));
  var["enable_embedded_perl"] = obj2str(my_rand(0, 1));
  var["use_embedded_perl_implicitly"] = obj2str(my_rand(0, 1));
  var["external_command_buffer_slots"] = obj2str(my_rand());
  var["auth_file"] = "auth_file.tmp";
  var["comment_file"] = "comment_file.tmp";
  var["xcddefault_comment_file"] = "comment_file.tmp";
  var["downtime_file"] = "downtime_file.tmp";
  var["xdddefault_downtime_file"] = "downtime_file.tmp";
  var["allow_empty_hostgroup_assignment"] = obj2str(my_rand(0, 1));

  std::ofstream ofs(mainconf.toStdString().c_str());
  for (std::map<QString, QString>::const_iterator it = var.begin(), end = var.end();
       it != end; ++it) {
    if (my_rand(1, 5) == 5) {
      ofs << "# comment !" << std::endl;
    }
    ofs << std::string(my_rand(0, 10), ' ')
	<< it->first.toStdString()
	<< std::string(my_rand(0, 10), ' ')
	<< "="
	<< std::string(my_rand(0, 10), ' ')
	<< it->second.toStdString()
	<< std::string(my_rand(0, 10), ' ')
	<< std::endl;
  }
  ofs.close();

  build_resource(resource);

  return (var);
}

/**
 *  Test the configuration file.
 *
 *  @param[in] filename The configuration file name.
 *  @param[in] my_conf  The configuration value.
 */
void test_configuration(QString const& filename, std::map<QString, QString>& my_conf) {
  configuration::state config;
  config.parse(filename);

  QString date_format[] = { "us", "euro", "iso8601", "strict-iso8601" };
  QString log_rotation[] = { "n", "h", "d", "w", "m" };
  QString check_delay[] = { "n", "d", "s", "" };

  if (my_conf["date_format"] != date_format[config.get_date_format()]) {
    throw (engine_error() << "date_format: init with '" << my_conf["date_format"] << "'");
  }
  if (my_conf["debug_level"] != obj2str(config.get_debug_level())) {
    throw (engine_error() << "debug_level: init with '" << my_conf["debug_level"] << "'");
  }
  if (my_conf["debug_verbosity"] != obj2str(config.get_debug_verbosity())) {
    throw (engine_error() << "debug_verbosity: init with '" << my_conf["debug_verbosity"] << "'");
  }
  if (my_conf["max_debug_file_size"] != obj2str(config.get_max_debug_file_size())) {
    throw (engine_error() << "max_debug_file_size: init with '" << my_conf["max_debug_file_size"] << "'");
  }
  if (my_conf["max_check_result_file_age"] != obj2str(config.get_max_check_result_file_age())) {
    throw (engine_error() << "max_check_result_file_age: init with '" << my_conf["max_check_result_file_age"] << "'");
  }
  if (my_conf["use_syslog"] != obj2str(config.get_use_syslog())) {
    throw (engine_error() << "use_syslog: init with '" << my_conf["use_syslog"] << "'");
  }
  if (my_conf["log_notifications"] != obj2str(config.get_log_notifications())) {
    throw (engine_error() << "log_notifications: init with '" << my_conf["log_notifications"] << "'");
  }
  if (my_conf["log_service_retries"] != obj2str(config.get_log_service_retries())) {
    throw (engine_error() << "log_service_retries: init with '" << my_conf["log_service_retries"] << "'");
  }
  if (my_conf["log_host_retries"] != obj2str(config.get_log_host_retries())) {
    throw (engine_error() << "log_host_retries: init with '" << my_conf["log_host_retries"] << "'");
  }
  if (my_conf["log_event_handlers"] != obj2str(config.get_log_event_handlers())) {
    throw (engine_error() << "log_event_handlers: init with '" << my_conf["log_event_handlers"] << "'");
  }
  if (my_conf["log_external_commands"] != obj2str(config.get_log_external_commands())) {
    throw (engine_error() << "log_external_commands: init with '" << my_conf["log_external_commands"] << "'");
  }
  if (my_conf["log_passive_checks"] != obj2str(config.get_log_passive_checks())) {
    throw (engine_error() << "log_passive_checks: init with '" << my_conf["log_passive_checks"] << "'");
  }
  if (my_conf["log_initial_states"] != obj2str(config.get_log_initial_state())) {
    throw (engine_error() << "log_initial_states: init with '" << my_conf["log_initial_states"] << "'");
  }
  if (my_conf["retain_state_information"] != obj2str(config.get_retain_state_information())) {
    throw (engine_error() << "retain_state_information: init with '" << my_conf["retain_state_information"] << "'");
  }
  if (my_conf["retention_update_interval"] != obj2str(config.get_retention_update_interval())) {
    throw (engine_error() << "retention_update_interval: init with '" << my_conf["retention_update_interval"] << "'");
  }
  if (my_conf["use_retained_program_state"] != obj2str(config.get_use_retained_program_state())) {
    throw (engine_error() << "use_retained_program_state: init with '" << my_conf["use_retained_program_state"] << "'");
  }
  if (my_conf["use_retained_scheduling_info"] != obj2str(config.get_use_retained_scheduling_info())) {
    throw (engine_error() << "use_retained_scheduling_info: init with '" << my_conf["use_retained_scheduling_info"] << "'");
  }
  if (my_conf["retention_scheduling_horizon"] != obj2str(config.get_retention_scheduling_horizon())) {
    throw (engine_error() << "retention_scheduling_horizon: init with '" << my_conf["retention_scheduling_horizon"] << "'");
  }
  if (my_conf["additional_freshness_latency"] != obj2str(config.get_additional_freshness_latency())) {
    throw (engine_error() << "additional_freshness_latency: init with '" << my_conf["additional_freshness_latency"] << "'");
  }
  if (my_conf["retained_host_attribute_mask"] != obj2str(config.get_retained_host_attribute_mask())) {
    throw (engine_error() << "retained_host_attribute_mask: init with '" << my_conf["retained_host_attribute_mask"] << "'");
  }
  if (my_conf["retained_process_host_attribute_mask"] != obj2str(config.get_retained_process_host_attribute_mask())) {
    throw (engine_error() << "retained_process_host_attribute_mask: init with '" << my_conf["retained_process_host_attribute_mask"] << "'");
  }
  if (my_conf["retained_contact_host_attribute_mask"] != obj2str(config.get_retained_contact_host_attribute_mask())) {
    throw (engine_error() << "retained_contact_host_attribute_mask: init with '" << my_conf["retained_contact_host_attribute_mask"] << "'");
  }
  if (my_conf["retained_contact_service_attribute_mask"] != obj2str(config.get_retained_contact_service_attribute_mask())) {
    throw (engine_error() << "retained_contact_service_attribute_mask: init with '" << my_conf["retained_contact_service_attribute_mask"] << "'");
  }
  if (my_conf["obsess_over_services"] != obj2str(config.get_obsess_over_services())) {
    throw (engine_error() << "obsess_over_services: init with '" << my_conf["obsess_over_services"] << "'");
  }
  if (my_conf["obsess_over_hosts"] != obj2str(config.get_obsess_over_hosts())) {
    throw (engine_error() << "obsess_over_hosts: init with '" << my_conf["obsess_over_hosts"] << "'");
  }
  if (my_conf["translate_passive_host_checks"] != obj2str(config.get_translate_passive_host_checks())) {
    throw (engine_error() << "translate_passive_host_checks: init with '" << my_conf["translate_passive_host_checks"] << "'");
  }
  if (my_conf["passive_host_checks_are_soft"] != obj2str(config.get_passive_host_checks_are_soft())) {
    throw (engine_error() << "passive_host_checks_are_soft: init with '" << my_conf["passive_host_checks_are_soft"] << "'");
  }
  if (my_conf["service_check_timeout"] != obj2str(config.get_service_check_timeout())) {
    throw (engine_error() << "service_check_timeout: init with '" << my_conf["service_check_timeout"] << "'");
  }
  if (my_conf["host_check_timeout"] != obj2str(config.get_host_check_timeout())) {
    throw (engine_error() << "host_check_timeout: init with '" << my_conf["host_check_timeout"] << "'");
  }
  if (my_conf["event_handler_timeout"] != obj2str(config.get_event_handler_timeout())) {
    throw (engine_error() << "event_handler_timeout: init with '" << my_conf["event_handler_timeout"] << "'");
  }
  if (my_conf["notification_timeout"] != obj2str(config.get_notification_timeout())) {
    throw (engine_error() << "notification_timeout: init with '" << my_conf["notification_timeout"] << "'");
  }
  if (my_conf["ocsp_timeout"] != obj2str(config.get_ocsp_timeout())) {
    throw (engine_error() << "ocsp_timeout: init with '" << my_conf["ocsp_timeout"] << "'");
  }
  if (my_conf["ochp_timeout"] != obj2str(config.get_ochp_timeout())) {
    throw (engine_error() << "ochp_timeout: init with '" << my_conf["ochp_timeout"] << "'");
  }
  if (my_conf["use_agressive_host_checking"] != obj2str(config.get_use_aggressive_host_checking())) {
    throw (engine_error() << "use_agressive_host_checking: init with '" << my_conf["use_agressive_host_checking"] << "'");
  }
  if (my_conf["use_aggressive_host_checking"] != obj2str(config.get_use_aggressive_host_checking())) {
    throw (engine_error() << "use_aggressive_host_checking: init with '" << my_conf["use_aggressive_host_checking"] << "'");
  }
  if (my_conf["cached_host_check_horizon"] != obj2str(config.get_cached_host_check_horizon())) {
    throw (engine_error() << "cached_host_check_horizon: init with '" << my_conf["cached_host_check_horizon"] << "'");
  }
  if (my_conf["enable_predictive_host_dependency_checks"] != obj2str(config.get_enable_predictive_host_dependency_checks())) {
    throw (engine_error() << "enable_predictive_host_dependency_checks: init with '" << my_conf["enable_predictive_host_dependency_checks"] << "'");
  }
  if (my_conf["cached_service_check_horizon"] != obj2str(config.get_cached_service_check_horizon())) {
    throw (engine_error() << "cached_service_check_horizon: init with '" << my_conf["cached_service_check_horizon"] << "'");
  }
  if (my_conf["enable_predictive_service_dependency_checks"] != obj2str(config.get_enable_predictive_service_dependency_checks())) {
    throw (engine_error() << "enable_predictive_service_dependency_checks: init with '" << my_conf["enable_predictive_service_dependency_checks"] << "'");
  }
  if (my_conf["soft_state_dependencies"] != obj2str(config.get_soft_state_dependencies())) {
    throw (engine_error() << "soft_state_dependencies: init with '" << my_conf["soft_state_dependencies"] << "'");
  }
  if (my_conf["enable_event_handlers"] != obj2str(config.get_enable_event_handlers())) {
    throw (engine_error() << "enable_event_handlers: init with '" << my_conf["enable_event_handlers"] << "'");
  }
  if (my_conf["enable_notifications"] != obj2str(config.get_enable_notifications())) {
    throw (engine_error() << "enable_notifications: init with '" << my_conf["enable_notifications"] << "'");
  }
  if (my_conf["execute_service_checks"] != obj2str(config.get_execute_service_checks())) {
    throw (engine_error() << "execute_service_checks: init with '" << my_conf["execute_service_checks"] << "'");
  }
  if (my_conf["accept_passive_service_checks"] != obj2str(config.get_accept_passive_service_checks())) {
    throw (engine_error() << "accept_passive_service_checks: init with '" << my_conf["accept_passive_service_checks"] << "'");
  }
  if (my_conf["execute_host_checks"] != obj2str(config.get_execute_host_checks())) {
    throw (engine_error() << "execute_host_checks: init with '" << my_conf["execute_host_checks"] << "'");
  }
  if (my_conf["accept_passive_host_checks"] != obj2str(config.get_accept_passive_host_checks())) {
    throw (engine_error() << "accept_passive_host_checks: init with '" << my_conf["accept_passive_host_checks"] << "'");
  }
  if (my_conf["max_service_check_spread"] != obj2str(config.get_max_service_check_spread())) {
    throw (engine_error() << "max_service_check_spread: init with '" << my_conf["max_service_check_spread"] << "'");
  }
  if (my_conf["max_host_check_spread"] != obj2str(config.get_max_host_check_spread())) {
    throw (engine_error() << "max_host_check_spread: init with '" << my_conf["max_host_check_spread"] << "'");
  }
  if (my_conf["max_concurrent_checks"] != obj2str(config.get_max_parallel_service_checks())) {
    throw (engine_error() << "max_concurrent_checks: init with '" << my_conf["max_concurrent_checks"] << "'");
  }
  if (my_conf["check_result_reaper_frequency"] != obj2str(config.get_check_reaper_interval())) {
    throw (engine_error() << "check_result_reaper_frequency: init with '" << my_conf["check_result_reaper_frequency"] << "'");
  }
  if (my_conf["service_reaper_frequency"] != obj2str(config.get_check_reaper_interval())) {
    throw (engine_error() << "service_reaper_frequency: init with '" << my_conf["service_reaper_frequency"] << "'");
  }
  if (my_conf["max_check_result_reaper_time"] != obj2str(config.get_max_check_reaper_time())) {
    throw (engine_error() << "max_check_result_reaper_time: init with '" << my_conf["max_check_result_reaper_time"] << "'");
  }
  if (my_conf["sleep_time"] != obj2str(config.get_sleep_time())) {
    throw (engine_error() << "sleep_time: init with '" << my_conf["sleep_time"] << "'");
  }
  if (my_conf["interval_length"] != obj2str(config.get_interval_length())) {
    throw (engine_error() << "interval_length: init with '" << my_conf["interval_length"] << "'");
  }
  if (my_conf["check_external_commands"] != obj2str(config.get_check_external_commands())) {
    throw (engine_error() << "check_external_commands: init with '" << my_conf["check_external_commands"] << "'");
  }
  if (my_conf["check_for_orphaned_services"] != obj2str(config.get_check_orphaned_services())) {
    throw (engine_error() << "check_for_orphaned_services: init with '" << my_conf["check_for_orphaned_services"] << "'");
  }
  if (my_conf["check_for_orphaned_hosts"] != obj2str(config.get_check_orphaned_hosts())) {
    throw (engine_error() << "check_for_orphaned_hosts: init with '" << my_conf["check_for_orphaned_hosts"] << "'");
  }
  if (my_conf["check_service_freshness"] != obj2str(config.get_check_service_freshness())) {
    throw (engine_error() << "check_service_freshness: init with '" << my_conf["check_service_freshness"] << "'");
  }
  if (my_conf["check_host_freshness"] != obj2str(config.get_check_host_freshness())) {
    throw (engine_error() << "check_host_freshness: init with '" << my_conf["check_host_freshness"] << "'");
  }
  if (my_conf["service_freshness_check_interval"] != obj2str(config.get_service_freshness_check_interval())) {
    throw (engine_error() << "service_freshness_check_interval: init with '" << my_conf["service_freshness_check_interval"] << "'");
  }
  if (my_conf["host_freshness_check_interval"] != obj2str(config.get_host_freshness_check_interval())) {
    throw (engine_error() << "host_freshness_check_interval: init with '" << my_conf["host_freshness_check_interval"] << "'");
  }
  if (my_conf["auto_reschedule_checks"] != obj2str(config.get_auto_reschedule_checks())) {
    throw (engine_error() << "auto_reschedule_checks: init with '" << my_conf["auto_reschedule_checks"] << "'");
  }
  if (my_conf["auto_rescheduling_interval"] != obj2str(config.get_auto_rescheduling_interval())) {
    throw (engine_error() << "auto_rescheduling_interval: init with '" << my_conf["auto_rescheduling_interval"] << "'");
  }
  if (my_conf["auto_rescheduling_window"] != obj2str(config.get_auto_rescheduling_window())) {
    throw (engine_error() << "auto_rescheduling_window: init with '" << my_conf["auto_rescheduling_window"] << "'");
  }
  if (my_conf["status_update_interval"] != obj2str(config.get_status_update_interval())) {
    throw (engine_error() << "status_update_interval: init with '" << my_conf["status_update_interval"] << "'");
  }
  if (my_conf["time_change_threshold"] != obj2str(config.get_time_change_threshold())) {
    throw (engine_error() << "time_change_threshold: init with '" << my_conf["time_change_threshold"] << "'");
  }
  if (my_conf["process_performance_data"] != obj2str(config.get_process_performance_data())) {
    throw (engine_error() << "process_performance_data: init with '" << my_conf["process_performance_data"] << "'");
  }
  if (my_conf["enable_flap_detection"] != obj2str(config.get_enable_flap_detection())) {
    throw (engine_error() << "enable_flap_detection: init with '" << my_conf["enable_flap_detection"] << "'");
  }
  if (my_conf["enable_failure_prediction"] != obj2str(config.get_enable_failure_prediction())) {
    throw (engine_error() << "enable_failure_prediction: init with '" << my_conf["enable_failure_prediction"] << "'");
  }
  if (my_conf["low_service_flap_threshold"] != obj2str(config.get_low_service_flap_threshold())) {
    throw (engine_error() << "low_service_flap_threshold: init with '" << my_conf["low_service_flap_threshold"] << "'");
  }
  if (my_conf["high_service_flap_threshold"] != obj2str(config.get_high_service_flap_threshold())) {
    throw (engine_error() << "high_service_flap_threshold: init with '" << my_conf["high_service_flap_threshold"] << "'");
  }
  if (my_conf["low_host_flap_threshold"] != obj2str(config.get_low_host_flap_threshold())) {
    throw (engine_error() << "low_host_flap_threshold: init with '" << my_conf["low_host_flap_threshold"] << "'");
  }
  if (my_conf["high_host_flap_threshold"] != obj2str(config.get_high_host_flap_threshold())) {
    throw (engine_error() << "high_host_flap_threshold: init with '" << my_conf["high_host_flap_threshold"] << "'");
  }
  if (my_conf["event_broker_options"] != obj2str(config.get_event_broker_options())) {
    throw (engine_error() << "event_broker_options: init with '" << my_conf["event_broker_options"] << "'");
  }
  if (my_conf["use_true_regexp_matching"] != obj2str(config.get_use_true_regexp_matching())) {
    throw (engine_error() << "use_true_regexp_matching: init with '" << my_conf["use_true_regexp_matching"] << "'");
  }
  if (my_conf["use_large_installation_tweaks"] != obj2str(config.get_use_large_installation_tweaks())) {
    throw (engine_error() << "use_large_installation_tweaks: init with '" << my_conf["use_large_installation_tweaks"] << "'");
  }
  if (my_conf["enable_environment_macros"] != obj2str(config.get_enable_environment_macros())) {
    throw (engine_error() << "enable_environment_macros: init with '" << my_conf["enable_environment_macros"] << "'");
  }
  if (my_conf["free_child_process_memory"] != obj2str(config.get_free_child_process_memory())) {
    throw (engine_error() << "free_child_process_memory: init with '" << my_conf["free_child_process_memory"] << "'");
  }
  if (my_conf["child_processes_fork_twice"] != obj2str(config.get_child_processes_fork_twice())) {
    throw (engine_error() << "child_processes_fork_twice: init with '" << my_conf["child_processes_fork_twice"] << "'");
  }
  if (my_conf["enable_embedded_perl"] != obj2str(config.get_enable_embedded_perl())) {
    throw (engine_error() << "enable_embedded_perl: init with '" << my_conf["enable_embedded_perl"] << "'");
  }
  if (my_conf["use_embedded_perl_implicitly"] != obj2str(config.get_use_embedded_perl_implicitly())) {
    throw (engine_error() << "use_embedded_perl_implicitly: init with '" << my_conf["use_embedded_perl_implicitly"] << "'");
  }
  if (my_conf["external_command_buffer_slots"] != obj2str(config.get_external_command_buffer_slots())) {
    throw (engine_error() << "external_command_buffer_slots: init with '" << my_conf["external_command_buffer_slots"] << "'");
  }
  if (my_conf["allow_empty_hostgroup_assignment"] != obj2str(config.get_allow_empty_hostgroup_assignment())) {
    throw (engine_error() << "allow_empty_hostgroup_assignment: init with '" << my_conf["allow_empty_hostgroup_assignment"] << "'");
  }
  if (my_conf["debug_file"] != config.get_debug_file()) {
    throw (engine_error() << "debug_file: init with '" << my_conf["debug_file"] << "'");
  }
  if (my_conf["command_file"] != config.get_command_file()) {
    throw (engine_error() << "command_file: init with '" << my_conf["command_file"] << "'");
  }
  if (my_conf["temp_file"] != config.get_temp_file()) {
    throw (engine_error() << "temp_file: init with '" << my_conf["temp_file"] << "'");
  }
  if (my_conf["temp_path"] != config.get_temp_path()) {
    throw (engine_error() << "temp_path: init with '" << my_conf["temp_path"] << "'");
  }
  if (my_conf["check_result_path"] != config.get_check_result_path()) {
    throw (engine_error() << "check_result_path: init with '" << my_conf["check_result_path"] << "'");
  }
  if (my_conf["global_host_event_handler"] != config.get_global_host_event_handler()) {
    throw (engine_error() << "global_host_event_handler: init with '" << my_conf["global_host_event_handler"] << "'");
  }
  if (my_conf["global_service_event_handler"] != config.get_global_service_event_handler()) {
    throw (engine_error() << "global_service_event_handler: init with '" << my_conf["global_service_event_handler"] << "'");
  }
  if (my_conf["ocsp_command"] != config.get_ocsp_command()) {
    throw (engine_error() << "ocsp_command: init with '" << my_conf["ocsp_command"] << "'");
  }
  if (my_conf["ochp_command"] != config.get_ochp_command()) {
    throw (engine_error() << "ochp_command: init with '" << my_conf["ochp_command"] << "'");
  }
  if (my_conf["log_archive_path"] != config.get_log_archive_path()) {
    throw (engine_error() << "log_archive_path: init with '" << my_conf["log_archive_path"] << "'");
  }
  if (my_conf["log_file"] != config.get_log_file()) {
    throw (engine_error() << "log_file: init with '" << my_conf["log_file"] << "'");
  }
  if (my_conf["illegal_object_name_chars"] != config.get_illegal_object_chars()) {
    throw (engine_error() << "illegal_object_name_chars: init with '" << my_conf["illegal_object_name_chars"] << "'");
  }
  if (my_conf["illegal_macro_output_chars"] != config.get_illegal_output_chars()) {
    throw (engine_error() << "illegal_macro_output_chars: init with '" << my_conf["illegal_macro_output_chars"] << "'");
  }
  if (my_conf["use_regexp_matching"] != obj2str(config.get_use_regexp_matches())) {
    throw (engine_error() << "use_regexp_matching: init with '" << my_conf["use_regexp_matching"] << "'");
  }
  if (my_conf["log_rotation_method"] != log_rotation[config.get_log_rotation_method()]) {
    throw (engine_error() << "log_rotation_method: init with '" << my_conf["log_rotation_method"] << "'");
  }
  if (my_conf["command_check_interval"] != obj2str(config.get_command_check_interval() / config.get_interval_length()) &&
      my_conf["command_check_interval"] != obj2str(config.get_command_check_interval()) + "s") {
    throw (engine_error() << "command_check_interval: init with '" << my_conf["log_rotation_method"] << "'");
  }
  if (my_conf["service_interleave_factor"] == "s") {
    if (config.get_service_interleave_factor_method() != configuration::state::ilf_smart) {
      throw (engine_error() << "service_interleave_factor: init with '" << my_conf["log_rotation_method"] << "'");
    }
  }
  else {
    if (my_conf["service_interleave_factor"] != obj2str(scheduling_info.service_interleave_factor) &&
	scheduling_info.service_interleave_factor != 1) {
      throw (engine_error() << "service_interleave_factor: init with '" << my_conf["log_rotation_method"] << "'");
    }
  }
  if (my_conf["service_inter_check_delay_method"] != check_delay[config.get_service_inter_check_delay_method()] &&
      my_conf["service_inter_check_delay_method"] != obj2str(scheduling_info.service_inter_check_delay)) {
      throw (engine_error() << "service_inter_check_delay_method: init with '" << my_conf["log_rotation_method"] << "'");
  }
  if (my_conf["host_inter_check_delay_method"] != check_delay[config.get_host_inter_check_delay_method()] &&
      my_conf["host_inter_check_delay_method"] != obj2str(scheduling_info.host_inter_check_delay)) {
      throw (engine_error() << "host_inter_check_delay_method: init with '" << my_conf["log_rotation_method"] << "'");
  }

  nagios_macros* mac = get_global_macros();
  if (mac->x[MACRO_ADMINEMAIL] != NULL &&
      my_conf["admin_email"] != mac->x[MACRO_ADMINEMAIL]) {
    throw (engine_error() << "admin_email: init with '" << my_conf["log_rotation_method"] << "'");
  }
  if (mac->x[MACRO_ADMINPAGER] &&
      my_conf["admin_pager"] != mac->x[MACRO_ADMINPAGER]) {
    throw (engine_error() << "admin_pager: init with '" << my_conf["log_rotation_method"] << "'");
  }

  for (unsigned int i = 0; i < MAX_USER_MACROS; ++i) {
    if (macro_user[i] != "USER" + obj2str(i)) {
      throw (engine_error() << "resource_file: init with '" << my_conf["log_rotation_method"] << "'");
    }
  }
}

/**
 *  Check the configuration working.
 */
int main(void) {
  try {
    srandom(time(NULL));

    QTemporaryFile mainconf("centengine.cfg");
    QTemporaryFile resource("resource.cfg");

    if (mainconf.open() == false || resource.open() == false) {
      throw (engine_error() << "open temporary file failed.");
    }

    mainconf.close();
    resource.close();

    QString mainconf_path = QDir::tempPath() + "/" + mainconf.fileName();
    QString resource_path = QDir::tempPath() + "/" + resource.fileName();

    std::map<QString, QString> my_conf = build_configuration(mainconf_path, resource_path);
    test_configuration(mainconf_path, my_conf);
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
