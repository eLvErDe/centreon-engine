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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "test/unittest.hh"
#include "logging/engine.hh"
#include "error.hh"
#include "commands.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Run change_svc_check_timeperiod test.
 */
static void check_change_svc_check_timeperiod() {
  init_object_skiplists();

  service* svc = add_service("name", "description", NULL,
                             NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
                             0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0,
                             0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
                             0, 0, NULL, NULL, NULL, NULL, NULL,
                             0, 0, 0);
  if (!svc)
    throw (engine_error() << "create service failed.");

  timeperiod* tperiod = add_timeperiod("tperiod", "alias");
  if (!tperiod)
    throw (engine_error() << "create timeperiod failed.");

  svc->check_period_ptr = NULL;
  char const* cmd("[1317196300] CHANGE_SVC_CHECK_TIMEPERIOD;name;description;tperiod");
  process_external_command(cmd);

  if (svc->check_period_ptr != tperiod)
    throw (engine_error() << "change_svc_check_timeperiod failed.");

  delete[] svc->host_name;
  delete[] svc->description;
  delete[] svc->service_check_command;
  delete[] svc->display_name;
  delete[] svc->check_period;
  delete svc;

  delete[] tperiod->name;
  delete[] tperiod->alias;
  delete tperiod;

  free_object_skiplists();
}

/**
 *  Check processing of change_svc_check_timeperiod works.
 */
int main_test() {
  logging::engine& engine = logging::engine::instance();
  check_change_svc_check_timeperiod();
  engine.cleanup();
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}
