/*
** Copyright 1999-2010 Ethan Galstad
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

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/******************* MACRO GENERATION FUNCTIONS *******************/
/******************************************************************/

/* calculates the value of a custom macro */
int grab_custom_macro_value_r(
      nagios_macros* mac,
      char* macro_name,
      char const* host_name,
      char const* service_description,
      char** output) {
  int result = OK;

  if (macro_name == NULL || output == NULL)
    return (ERROR);

  /***** CUSTOM HOST MACRO *****/
  if (strstr(macro_name, "_HOST") == macro_name) {
    host* temp_host(NULL);
    /* find the host for on-demand macros */
    if (host_name) {
      if ((temp_host = find_host(host_name)) == NULL)
        return (ERROR);
    }
    /* else use saved host pointer */
    else if ((temp_host = mac->host_ptr) == NULL)
      return (ERROR);

    /* get the host macro value */
    result = grab_custom_object_macro_r(
               mac,
               macro_name + 5,
               temp_host->custom_variables,
               output);
  }
  /***** CUSTOM SERVICE MACRO *****/
  else if (strstr(macro_name, "_SERVICE") == macro_name) {
    service* temp_service(NULL);

    /* use saved service pointer */
    if (host_name == NULL && service_description == NULL) {
      if ((temp_service = mac->service_ptr) == NULL)
        return (ERROR);

      /* get the service macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name + 8,
                 temp_service->custom_variables,
                 output);
    }
    /* else and ondemand macro... */
    else {
      /* if host name is blank, it means use the current host name */
      if (mac->host_ptr == NULL)
        return (ERROR);
      if ((temp_service = find_service(
                            mac->host_ptr ? mac->host_ptr->name : NULL,
                            service_description))
          == NULL)
        return (ERROR);

      /* get the service macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name + 8,
                 temp_service->custom_variables,
                 output);
    }
  }
  else
    return (ERROR);
  return (result);
}

/* calculates a date/time macro */
int grab_datetime_macro_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output) {
  time_t current_time = 0L;
  timeperiod* temp_timeperiod = NULL;
  time_t test_time = 0L;
  time_t next_valid_time = 0L;

  (void)mac;

  if (output == NULL)
    return (ERROR);

  /* get the current time */
  time(&current_time);

  /* parse args, do prep work */
  switch (macro_type) {
  case MACRO_ISVALIDTIME:
  case MACRO_NEXTVALIDTIME:
    /* find the timeperiod */
    if ((temp_timeperiod = find_timeperiod(arg1)) == NULL)
      return (ERROR);
    /* what timestamp should we use? */
    if (arg2)
      test_time = (time_t)strtoul(arg2, NULL, 0);
    else
      test_time = current_time;
    break;

  default:
    break;
  }

  /*
  ** Calculate the value.
  **
  ** Default timezone is likely not valid but fetching it would require
  ** a heavy API refactoring for what I believe to be very little gain.
  */
  switch (macro_type) {
  case MACRO_TIMET:
    string::setstr(*output, current_time);
    break;

  case MACRO_ISVALIDTIME:
    string::setstr(
      *output,
      !check_time_against_period(test_time, temp_timeperiod, NULL));
    break;

  case MACRO_NEXTVALIDTIME:
    get_next_valid_time(
      test_time,
      &next_valid_time,
      temp_timeperiod,
      NULL);
    if (next_valid_time == test_time
        && check_time_against_period(
             test_time,
             temp_timeperiod,
             NULL) == ERROR)
      next_valid_time = (time_t)0L;
    string::setstr(*output, next_valid_time);
    break;

  default:
    return (ERROR);
  }
  return (OK);
}

/* computes a custom object macro */
int grab_custom_object_macro_r(
      nagios_macros* mac,
      char* macro_name,
      customvariablesmember* vars,
      char** output) {
  customvariablesmember* temp_customvariablesmember = NULL;
  int result = ERROR;

  (void)mac;

  if (macro_name == NULL || vars == NULL || output == NULL)
    return (ERROR);

  /* get the custom variable */
  for (temp_customvariablesmember = vars;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {

    if (temp_customvariablesmember->variable_name == NULL)
      continue;

    if (!strcmp(macro_name, temp_customvariablesmember->variable_name)) {
      if (temp_customvariablesmember->variable_value)
        *output = string::dup(temp_customvariablesmember->variable_value);
      result = OK;
      break;
    }
  }
  return (result);
}

/******************************************************************/
/********************* MACRO STRING FUNCTIONS *********************/
/******************************************************************/

/* cleans illegal characters in macros before output */
char const* clean_macro_chars(char* macro, int options) {
  if (macro == NULL)
    return ("");

  int len((int)strlen(macro));

  /* strip illegal characters out of macro */
  if (options & STRIP_ILLEGAL_MACRO_CHARS) {
    int y(0);
    for (int x(0); x < len; x++) {
      /*ch=(int)macro[x]; */
      /* allow non-ASCII characters (Japanese, etc) */
      int ch(macro[x] & 0xff);

      /* illegal ASCII characters */
      if (ch < 32 || ch == 127)
        continue;

      /* illegal user-specified characters */
      if (config->illegal_output_chars().find(ch) == std::string::npos)
        macro[y++] = macro[x];
    }

    macro[y++] = '\x0';
  }
  return (macro);
}

/* encodes a string in proper URL format */
char* get_url_encoded_string(char* input) {
  int x = 0;
  int y = 0;
  char* encoded_url_string = NULL;
  char temp_expansion[6] = "";

  /* bail if no input */
  if (input == NULL)
    return (NULL);

  /* allocate enough memory to escape all characters if necessary */
  encoded_url_string = new char[strlen(input) * 3 + 1];

  /* check/encode all characters */
  for (x = 0, y = 0; input[x] != (char)'\x0'; x++) {
    /* alpha-numeric characters and a few other characters don't get encoded */
    if (((char)input[x] >= '0' && (char)input[x] <= '9')
        || ((char)input[x] >= 'A' && (char)input[x] <= 'Z')
        || ((char)input[x] >= (char)'a' && (char)input[x] <= (char)'z')
        || (char)input[x] == (char)'.' || (char)input[x] == (char)'-'
        || (char)input[x] == (char)'_' || (char)input[x] == (char)':'
        || (char)input[x] == (char)'/' || (char)input[x] == (char)'?'
        || (char)input[x] == (char)'=' || (char)input[x] == (char)'&')
      encoded_url_string[y++] = input[x];
    /* spaces are pluses */
    else if (input[x] == ' ')
      encoded_url_string[y++] = '+';
    /* anything else gets represented by its hex value */
    else {
      encoded_url_string[y] = '\x0';
      sprintf(temp_expansion, "%%%02X", (unsigned int)(input[x] & 0xFF));
      strcat(encoded_url_string, temp_expansion);
      y += 3;
    }
  }

  /* terminate encoded string */
  encoded_url_string[y] = '\x0';
  return (encoded_url_string);
}

/******************************************************************/
/***************** MACRO INITIALIZATION FUNCTIONS *****************/
/******************************************************************/

/* initializes global macros */
int init_macros() {
  init_macrox_names();

  /*
   * non-volatile macros are free()'d when they're set.
   * We must do this in order to not lose the constant
   * ones when we get SIGHUP or a RESTART_PROGRAM event
   * from the command fifo. Otherwise a memset() would
   * have been better.
   */
  clear_volatile_macros_r(get_global_macros());

  /* backwards compatibility hack */
  macro_x = get_global_macros()->x;
  return (OK);
}

/*
 * initializes the names of macros, using this nifty little macro
 * which ensures we never add any typos to the list
 */
#define add_macrox_name(name) macro_x_names[MACRO_##name] = string::dup(#name)
int init_macrox_names() {
  unsigned int x = 0;

  /* initialize macro names */
  for (x = 0; x < MACRO_X_COUNT; x++)
    macro_x_names[x] = NULL;

  /* initialize each macro name */
  add_macrox_name(HOSTNAME);
  add_macrox_name(HOSTALIAS);
  add_macrox_name(HOSTADDRESS);
  add_macrox_name(SERVICEDESC);
  add_macrox_name(SERVICESTATE);
  add_macrox_name(SERVICESTATEID);
  add_macrox_name(SERVICEATTEMPT);
  add_macrox_name(SERVICEISVOLATILE);
  add_macrox_name(TIMET);
  add_macrox_name(LASTHOSTCHECK);
  add_macrox_name(LASTSERVICECHECK);
  add_macrox_name(LASTHOSTSTATECHANGE);
  add_macrox_name(LASTSERVICESTATECHANGE);
  add_macrox_name(HOSTOUTPUT);
  add_macrox_name(SERVICEOUTPUT);
  add_macrox_name(HOSTPERFDATA);
  add_macrox_name(SERVICEPERFDATA);
  add_macrox_name(HOSTSTATE);
  add_macrox_name(HOSTSTATEID);
  add_macrox_name(HOSTATTEMPT);
  add_macrox_name(HOSTEXECUTIONTIME);
  add_macrox_name(SERVICEEXECUTIONTIME);
  add_macrox_name(HOSTLATENCY);
  add_macrox_name(SERVICELATENCY);
  add_macrox_name(HOSTDURATION);
  add_macrox_name(SERVICEDURATION);
  add_macrox_name(HOSTDURATIONSEC);
  add_macrox_name(SERVICEDURATIONSEC);
  add_macrox_name(HOSTSTATETYPE);
  add_macrox_name(SERVICESTATETYPE);
  add_macrox_name(HOSTPERCENTCHANGE);
  add_macrox_name(SERVICEPERCENTCHANGE);
  add_macrox_name(LASTSERVICEOK);
  add_macrox_name(LASTSERVICEWARNING);
  add_macrox_name(LASTSERVICEUNKNOWN);
  add_macrox_name(LASTSERVICECRITICAL);
  add_macrox_name(LASTHOSTUP);
  add_macrox_name(LASTHOSTDOWN);
  add_macrox_name(LASTHOSTUNREACHABLE);
  add_macrox_name(SERVICECHECKCOMMAND);
  add_macrox_name(HOSTCHECKCOMMAND);
  add_macrox_name(MAINCONFIGFILE);
  add_macrox_name(STATUSDATAFILE);
  add_macrox_name(RETENTIONDATAFILE);
  add_macrox_name(LOGFILE);
  add_macrox_name(COMMANDFILE);
  add_macrox_name(TOTALHOSTSUP);
  add_macrox_name(TOTALHOSTSDOWN);
  add_macrox_name(TOTALHOSTSUNREACHABLE);
  add_macrox_name(TOTALHOSTPROBLEMS);
  add_macrox_name(TOTALSERVICESOK);
  add_macrox_name(TOTALSERVICESWARNING);
  add_macrox_name(TOTALSERVICESCRITICAL);
  add_macrox_name(TOTALSERVICESUNKNOWN);
  add_macrox_name(TOTALSERVICEPROBLEMS);
  add_macrox_name(PROCESSSTARTTIME);
  add_macrox_name(HOSTCHECKTYPE);
  add_macrox_name(SERVICECHECKTYPE);
  add_macrox_name(LONGHOSTOUTPUT);
  add_macrox_name(LONGSERVICEOUTPUT);
  add_macrox_name(HOSTEVENTID);
  add_macrox_name(LASTHOSTEVENTID);
  add_macrox_name(SERVICEEVENTID);
  add_macrox_name(LASTSERVICEEVENTID);
  add_macrox_name(MAXHOSTATTEMPTS);
  add_macrox_name(MAXSERVICEATTEMPTS);
  add_macrox_name(TOTALHOSTSERVICES);
  add_macrox_name(TOTALHOSTSERVICESOK);
  add_macrox_name(TOTALHOSTSERVICESWARNING);
  add_macrox_name(TOTALHOSTSERVICESUNKNOWN);
  add_macrox_name(TOTALHOSTSERVICESCRITICAL);
  add_macrox_name(EVENTSTARTTIME);
  add_macrox_name(HOSTPROBLEMID);
  add_macrox_name(LASTHOSTPROBLEMID);
  add_macrox_name(SERVICEPROBLEMID);
  add_macrox_name(LASTSERVICEPROBLEMID);
  add_macrox_name(ISVALIDTIME);
  add_macrox_name(NEXTVALIDTIME);
  add_macrox_name(LASTHOSTSTATE);
  add_macrox_name(LASTHOSTSTATEID);
  add_macrox_name(LASTSERVICESTATE);
  add_macrox_name(LASTSERVICESTATEID);

  return (OK);
}

/******************************************************************/
/********************* MACRO CLEANUP FUNCTIONS ********************/
/******************************************************************/

/* free memory associated with the macrox names */
int free_macrox_names() {
  unsigned int x = 0;

  /* free each macro name */
  for (x = 0; x < MACRO_X_COUNT; x++) {
    delete[] macro_x_names[x];
    macro_x_names[x] = NULL;
  }
  return (OK);
}

/* clear argv macros - used in commands */
int clear_argv_macros_r(nagios_macros* mac) {
  unsigned int x = 0;

  /* command argument macros */
  for (x = 0; x < MAX_COMMAND_ARGUMENTS; x++) {
    delete[] mac->argv[x];
    mac->argv[x] = NULL;
  }
  return (OK);
}

/*
 * copies non-volatile macros from global macro_x to **dest, which
 * must be large enough to hold at least MACRO_X_COUNT entries.
 * We use a shortlived macro to save up on typing
 */
#define cp_macro(name) dest[MACRO_##name] = get_global_macros()->x[MACRO_##name]
void copy_constant_macros(char** dest) {
  cp_macro(MAINCONFIGFILE);
  cp_macro(STATUSDATAFILE);
  cp_macro(RETENTIONDATAFILE);
  cp_macro(LOGFILE);
  cp_macro(COMMANDFILE);
  cp_macro(PROCESSSTARTTIME);
  cp_macro(EVENTSTARTTIME);
  return;
}
#undef cp_macro

/* clear all macros that are not "constant" (i.e. they change throughout the course of monitoring) */
int clear_volatile_macros_r(nagios_macros* mac) {
  customvariablesmember* this_customvariablesmember = NULL;
  customvariablesmember* next_customvariablesmember = NULL;
  unsigned int x = 0;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {

    case MACRO_MAINCONFIGFILE:
    case MACRO_STATUSDATAFILE:
    case MACRO_RETENTIONDATAFILE:
    case MACRO_LOGFILE:
    case MACRO_COMMANDFILE:
    case MACRO_PROCESSSTARTTIME:
    case MACRO_EVENTSTARTTIME:
      /* these don't change during the course of monitoring, so no need to free them */
      break;

    default:
      delete[] mac->x[x];
      mac->x[x] = NULL;
      break;
    }
  }

  /* clear macro pointers */
  mac->host_ptr = NULL;
  mac->service_ptr = NULL;

  /* clear on-demand macro */
  delete[] mac->ondemand;
  mac->ondemand = NULL;

  /* clear ARGx macros */
  clear_argv_macros_r(mac);

  /* clear custom host variables */
  for (this_customvariablesmember = mac->custom_host_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_host_vars = NULL;

  /* clear custom service variables */
  for (this_customvariablesmember = mac->custom_service_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_service_vars = NULL;

  return (OK);
}

/* clear summary macros */
int clear_summary_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = MACRO_TOTALHOSTSUP;
       x <= MACRO_TOTALSERVICEPROBLEMS;
       x++) {
    delete[] mac->x[x];
    mac->x[x] = NULL;
  }

  return (OK);
}
