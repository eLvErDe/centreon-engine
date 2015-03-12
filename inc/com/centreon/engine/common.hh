/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2011 Nagios Core Development Team and Community Contributors
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

#ifndef CCE_COMMON_HH
#  define CCE_COMMON_HH

#  include "com/centreon/engine/checks/stats.hh"
#  include "com/centreon/engine/shared.hh"

/* Daemon is thread safe. */
#  ifndef _REENTRANT
#    define _REENTRANT
#  endif /* !_REENTRANT */
#  ifndef _THREAD_SAFE
#    define _THREAD_SAFE
#  endif /* !_THREAD_SAFE */

/* Max number of old states to keep track of for flap detection. */
#  define MAX_STATE_HISTORY_ENTRIES 21

/* Commands. */
#  define CMD_NONE                                             0
#  define CMD_ADD_HOST_COMMENT                                 1
#  define CMD_DEL_HOST_COMMENT                                 2
#  define CMD_ADD_SVC_COMMENT                                  3
#  define CMD_DEL_SVC_COMMENT                                  4
#  define CMD_ENABLE_SVC_CHECK                                 5
#  define CMD_DISABLE_SVC_CHECK                                6
#  define CMD_SCHEDULE_SVC_CHECK                               7
#  define CMD_DELAY_SVC_NOTIFICATION                           9
#  define CMD_DELAY_HOST_NOTIFICATION                         10
#  define CMD_DISABLE_NOTIFICATIONS                           11
#  define CMD_ENABLE_NOTIFICATIONS                            12
#  define CMD_RESTART_PROCESS                                 13
#  define CMD_SHUTDOWN_PROCESS                                14
#  define CMD_ENABLE_HOST_SVC_CHECKS                          15
#  define CMD_DISABLE_HOST_SVC_CHECKS                         16
#  define CMD_SCHEDULE_HOST_SVC_CHECKS                        17
#  define CMD_DELAY_HOST_SVC_NOTIFICATIONS                    19  /* Currently unimplemented. */
#  define CMD_DEL_ALL_HOST_COMMENTS                           20
#  define CMD_DEL_ALL_SVC_COMMENTS                            21
#  define CMD_ENABLE_SVC_NOTIFICATIONS                        22
#  define CMD_DISABLE_SVC_NOTIFICATIONS                       23
#  define CMD_ENABLE_HOST_NOTIFICATIONS                       24
#  define CMD_DISABLE_HOST_NOTIFICATIONS                      25
#  define CMD_ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST            26
#  define CMD_DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST           27
#  define CMD_ENABLE_HOST_SVC_NOTIFICATIONS                   28
#  define CMD_DISABLE_HOST_SVC_NOTIFICATIONS                  29
#  define CMD_PROCESS_SERVICE_CHECK_RESULT                    30
#  define CMD_SAVE_STATE_INFORMATION                          31
#  define CMD_READ_STATE_INFORMATION                          32
#  define CMD_ACKNOWLEDGE_HOST_PROBLEM                        33
#  define CMD_ACKNOWLEDGE_SVC_PROBLEM                         34
#  define CMD_START_EXECUTING_SVC_CHECKS                      35
#  define CMD_STOP_EXECUTING_SVC_CHECKS                       36
#  define CMD_START_ACCEPTING_PASSIVE_SVC_CHECKS              37
#  define CMD_STOP_ACCEPTING_PASSIVE_SVC_CHECKS               38
#  define CMD_ENABLE_PASSIVE_SVC_CHECKS                       39
#  define CMD_DISABLE_PASSIVE_SVC_CHECKS                      40
#  define CMD_ENABLE_EVENT_HANDLERS                           41
#  define CMD_DISABLE_EVENT_HANDLERS                          42
#  define CMD_ENABLE_HOST_EVENT_HANDLER                       43
#  define CMD_DISABLE_HOST_EVENT_HANDLER                      44
#  define CMD_ENABLE_SVC_EVENT_HANDLER                        45
#  define CMD_DISABLE_SVC_EVENT_HANDLER                       46
#  define CMD_ENABLE_HOST_CHECK                               47
#  define CMD_DISABLE_HOST_CHECK                              48
#  define CMD_START_OBSESSING_OVER_SVC_CHECKS                 49
#  define CMD_STOP_OBSESSING_OVER_SVC_CHECKS                  50
#  define CMD_REMOVE_HOST_ACKNOWLEDGEMENT                     51
#  define CMD_REMOVE_SVC_ACKNOWLEDGEMENT                      52
#  define CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS                 53
#  define CMD_SCHEDULE_FORCED_SVC_CHECK                       54
#  define CMD_SCHEDULE_HOST_DOWNTIME                          55
#  define CMD_SCHEDULE_SVC_DOWNTIME                           56
#  define CMD_ENABLE_HOST_FLAP_DETECTION                      57
#  define CMD_DISABLE_HOST_FLAP_DETECTION                     58
#  define CMD_ENABLE_SVC_FLAP_DETECTION                       59
#  define CMD_DISABLE_SVC_FLAP_DETECTION                      60
#  define CMD_ENABLE_FLAP_DETECTION                           61
#  define CMD_DISABLE_FLAP_DETECTION                          62
#  define CMD_ENABLE_HOSTGROUP_SVC_NOTIFICATIONS              63
#  define CMD_DISABLE_HOSTGROUP_SVC_NOTIFICATIONS             64
#  define CMD_ENABLE_HOSTGROUP_HOST_NOTIFICATIONS             65
#  define CMD_DISABLE_HOSTGROUP_HOST_NOTIFICATIONS            66
#  define CMD_ENABLE_HOSTGROUP_SVC_CHECKS                     67
#  define CMD_DISABLE_HOSTGROUP_SVC_CHECKS                    68
#  define CMD_CANCEL_HOST_DOWNTIME                            69 /* Not internally implemented. */
#  define CMD_CANCEL_SVC_DOWNTIME                             70 /* Not internally implemented. */
#  define CMD_CANCEL_ACTIVE_HOST_DOWNTIME                     71 /* Old - no longer used. */
#  define CMD_CANCEL_PENDING_HOST_DOWNTIME                    72 /* Old - no longer used. */
#  define CMD_CANCEL_ACTIVE_SVC_DOWNTIME                      73 /* Old - no longer used. */
#  define CMD_CANCEL_PENDING_SVC_DOWNTIME                     74 /* Old - no longer used. */
#  define CMD_CANCEL_ACTIVE_HOST_SVC_DOWNTIME                 75 /* Unimplemented. */
#  define CMD_CANCEL_PENDING_HOST_SVC_DOWNTIME                76 /* Unimplemented. */
#  define CMD_FLUSH_PENDING_COMMANDS                          77
#  define CMD_DEL_HOST_DOWNTIME                               78
#  define CMD_DEL_SVC_DOWNTIME                                79
#  define CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME                84
#  define CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME                 85
#  define CMD_SCHEDULE_HOST_SVC_DOWNTIME                      86
#  define CMD_PROCESS_HOST_CHECK_RESULT                       87
#  define CMD_START_EXECUTING_HOST_CHECKS                     88
#  define CMD_STOP_EXECUTING_HOST_CHECKS                      89
#  define CMD_START_ACCEPTING_PASSIVE_HOST_CHECKS             90
#  define CMD_STOP_ACCEPTING_PASSIVE_HOST_CHECKS              91
#  define CMD_ENABLE_PASSIVE_HOST_CHECKS                      92
#  define CMD_DISABLE_PASSIVE_HOST_CHECKS                     93
#  define CMD_START_OBSESSING_OVER_HOST_CHECKS                94
#  define CMD_STOP_OBSESSING_OVER_HOST_CHECKS                 95
#  define CMD_SCHEDULE_HOST_CHECK                             96
#  define CMD_SCHEDULE_FORCED_HOST_CHECK                      98
#  define CMD_START_OBSESSING_OVER_SVC                        99
#  define CMD_STOP_OBSESSING_OVER_SVC                        100
#  define CMD_START_OBSESSING_OVER_HOST                      101
#  define CMD_STOP_OBSESSING_OVER_HOST                       102
#  define CMD_ENABLE_HOSTGROUP_HOST_CHECKS                   103
#  define CMD_DISABLE_HOSTGROUP_HOST_CHECKS                  104
#  define CMD_ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS            105
#  define CMD_DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS           106
#  define CMD_ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS           107
#  define CMD_DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS          108
#  define CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS          109
#  define CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS         110
#  define CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS         111
#  define CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS        112
#  define CMD_ENABLE_SERVICEGROUP_SVC_CHECKS                 113
#  define CMD_DISABLE_SERVICEGROUP_SVC_CHECKS                114
#  define CMD_ENABLE_SERVICEGROUP_HOST_CHECKS                115
#  define CMD_DISABLE_SERVICEGROUP_HOST_CHECKS               116
#  define CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS         117
#  define CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS        118
#  define CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS        119
#  define CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS       120
#  define CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME            121
#  define CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME             122
#  define CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER               123
#  define CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER                124
#  define CMD_CHANGE_HOST_EVENT_HANDLER                      125
#  define CMD_CHANGE_SVC_EVENT_HANDLER                       126
#  define CMD_CHANGE_HOST_CHECK_COMMAND                      127
#  define CMD_CHANGE_SVC_CHECK_COMMAND                       128
#  define CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL              129
#  define CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL               130
#  define CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL                131
#  define CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS                 132
#  define CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS                  133
#  define CMD_SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME 134
#  define CMD_ENABLE_HOST_AND_CHILD_NOTIFICATIONS            135
#  define CMD_DISABLE_HOST_AND_CHILD_NOTIFICATIONS           136
#  define CMD_SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME           137
#  define CMD_ENABLE_SERVICE_FRESHNESS_CHECKS                138
#  define CMD_DISABLE_SERVICE_FRESHNESS_CHECKS               139
#  define CMD_ENABLE_HOST_FRESHNESS_CHECKS                   140
#  define CMD_DISABLE_HOST_FRESHNESS_CHECKS                  141
#  define CMD_SET_HOST_NOTIFICATION_NUMBER                   142
#  define CMD_SET_SVC_NOTIFICATION_NUMBER                    143
#  define CMD_CHANGE_HOST_CHECK_TIMEPERIOD                   144
#  define CMD_CHANGE_SVC_CHECK_TIMEPERIOD                    145
#  define CMD_PROCESS_FILE                                   146
#  define CMD_CHANGE_CUSTOM_HOST_VAR                         147
#  define CMD_CHANGE_CUSTOM_SVC_VAR                          148
#  define CMD_CHANGE_CUSTOM_CONTACT_VAR                      149
#  define CMD_ENABLE_CONTACT_HOST_NOTIFICATIONS              150
#  define CMD_DISABLE_CONTACT_HOST_NOTIFICATIONS             151
#  define CMD_ENABLE_CONTACT_SVC_NOTIFICATIONS               152
#  define CMD_DISABLE_CONTACT_SVC_NOTIFICATIONS              153
#  define CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS         154
#  define CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS        155
#  define CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS          156
#  define CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS         157
#  define CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL               158
#  define CMD_SEND_CUSTOM_HOST_NOTIFICATION                  159
#  define CMD_SEND_CUSTOM_SVC_NOTIFICATION                   160
#  define CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD            161
#  define CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD             162
#  define CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD    163
#  define CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD     164
#  define CMD_CHANGE_HOST_MODATTR                            165
#  define CMD_CHANGE_SVC_MODATTR                             166
#  define CMD_CHANGE_CONTACT_MODATTR                         167
#  define CMD_CHANGE_CONTACT_MODHATTR                        168
#  define CMD_CHANGE_CONTACT_MODSATTR                        169
#  define CMD_DEL_DOWNTIME_BY_HOST_NAME                      170
#  define CMD_DEL_DOWNTIME_BY_HOSTGROUP_NAME                 171
#  define CMD_DEL_DOWNTIME_BY_START_TIME_COMMENT             172
#  define CMD_CUSTOM_COMMAND                                 999

/* Service check types. */
#  define SERVICE_CHECK_ACTIVE  0 /* Engine performed the service check. */
#  define SERVICE_CHECK_PASSIVE 1 /* The service check result was submitted by an external source. */

/* Host check types. */
#  define HOST_CHECK_ACTIVE  0 /* Engine performed the host check. */
#  define HOST_CHECK_PASSIVE 1 /* The host check result was submitted by an external source. */

/* Service state types. */
#  define SOFT_STATE 0
#  define HARD_STATE 1

/* Scheduled downtime types. */
#  define SERVICE_DOWNTIME 1 /* Service downtime. */
#  define HOST_DOWNTIME    2 /* Host downtime. */
#  define ANY_DOWNTIME     3 /* Host or service downtime. */

/* Notification options. */
#  define NOTIFICATION_OPTION_NONE      0
#  define NOTIFICATION_OPTION_BROADCAST 1
#  define NOTIFICATION_OPTION_FORCED    2
#  define NOTIFICATION_OPTION_INCREMENT 4

/* Acknowledgement types. */
#  define HOST_ACKNOWLEDGEMENT    0
#  define SERVICE_ACKNOWLEDGEMENT 1

#  define ACKNOWLEDGEMENT_NONE    0
#  define ACKNOWLEDGEMENT_NORMAL  1
#  define ACKNOWLEDGEMENT_STICKY  2

/* Dependency types. */
#  define NOTIFICATION_DEPENDENCY 1
#  define EXECUTION_DEPENDENCY    2

/* Host/service check options. */
#  define CHECK_OPTION_NONE            0 /* No check options. */
#  define CHECK_OPTION_FORCE_EXECUTION 1 /* Force execution of a check (ignores disabled services/hosts, invalid timeperiods). */
#  define CHECK_OPTION_FRESHNESS_CHECK 2 /* This is a freshness check. */

/* Program modes. */
#  define STANDBY_MODE 0
#  define ACTIVE_MODE  1

/* Log versions. */
#  define LOG_VERSION_1 "1.0"
#  define LOG_VERSION_2 "2.0"

/* General definitions. */
#  define OK     0
#  define ERROR  -2 /* Value was changed from -1 so as to not interfere with STATUS_UNKNOWN plugin result. */

#  ifndef TRUE
#    define TRUE  1
#  elif (TRUE!=1)
#    define TRUE  1
#   endif /* !TRUE */
#  ifndef FALSE
#    define FALSE 0
#  elif (FALSE!=0)
#    define FALSE 0
#  endif /* !FALSE */

/* Date range types. */
#  define DATERANGE_CALENDAR_DATE  0  /* 2008-12-25 */
#  define DATERANGE_MONTH_DATE     1  /* july 4 (specific month) */
#  define DATERANGE_MONTH_DAY      2  /* day 21 (generic month) */
#  define DATERANGE_MONTH_WEEK_DAY 3  /* 3rd thursday (specific month) */
#  define DATERANGE_WEEK_DAY       4  /* 3rd thursday (generic month) */
#  define DATERANGE_TYPES          5

/* Date/time types. */
#  define LONG_DATE_TIME  0
#  define SHORT_DATE_TIME 1
#  define SHORT_DATE      2
#  define SHORT_TIME      3
#  define HTTP_DATE_TIME  4 /* Time formatted for use in HTTP headers. */

/* Date formats. */
#  define DATE_FORMAT_US             0 /* U.S. (MM-DD-YYYY HH:MM:SS) */
#  define DATE_FORMAT_EURO           1 /* European (DD-MM-YYYY HH:MM:SS) */
#  define DATE_FORMAT_ISO8601        2 /* ISO8601 (YYYY-MM-DD HH:MM:SS) */
#  define DATE_FORMAT_STRICT_ISO8601 3 /* ISO8601 (YYYY-MM-DDTHH:MM:SS) */

/* Misc definitions. */
#  define MAX_FILENAME_LENGTH          256 /* Max length of path/filename that Engine will process. */
#  define MAX_INPUT_BUFFER            1024 /* Size in bytes of max. input buffer (for reading files, misc stuff). */
#  define MAX_COMMAND_BUFFER          8192 /* Max length of raw or processed command line. */
#  define MAX_EXTERNAL_COMMAND_LENGTH 8192 /* Max length of an external command. */
#  define MAX_DATETIME_LENGTH           48

/* Modified attributes. */
#  define MODATTR_NONE                       0
#  define MODATTR_NOTIFICATIONS_ENABLED      (1 << 0)
#  define MODATTR_ACTIVE_CHECKS_ENABLED      (1 << 1)
#  define MODATTR_PASSIVE_CHECKS_ENABLED     (1 << 2)
#  define MODATTR_EVENT_HANDLER_ENABLED      (1 << 3)
#  define MODATTR_FLAP_DETECTION_ENABLED     (1 << 4)
#  define MODATTR_OBSESSIVE_HANDLER_ENABLED  (1 << 7)
#  define MODATTR_EVENT_HANDLER_COMMAND      (1 << 8)
#  define MODATTR_CHECK_COMMAND              (1 << 9)
#  define MODATTR_NORMAL_CHECK_INTERVAL      (1 << 10)
#  define MODATTR_RETRY_CHECK_INTERVAL       (1 << 11)
#  define MODATTR_MAX_CHECK_ATTEMPTS         (1 << 12)
#  define MODATTR_FRESHNESS_CHECKS_ENABLED   (1 << 13)
#  define MODATTR_CHECK_TIMEPERIOD           (1 << 14)
#  define MODATTR_CUSTOM_VARIABLE            (1 << 15)
#  define MODATTR_NOTIFICATION_TIMEPERIOD    (1 << 16)
#  define MODATTR_ALL                        (~0)

/* Host status. */
#  define HOST_UP          0
#  define HOST_DOWN        1
#  define HOST_UNREACHABLE 2

/* Service state. */
#  define STATE_OK       0
#  define STATE_WARNING  1
#  define STATE_CRITICAL 2
#  define STATE_UNKNOWN  3

/* State change types. */
#  define HOST_STATECHANGE    0
#  define SERVICE_STATECHANGE 1

/* Thread stuff. */
#  define TOTAL_WORKER_THREADS 1

#endif /* !CCE_COMMON_HH */
