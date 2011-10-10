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
#include "test/testing.hh"
#include "logging/engine.hh"
#include "error.hh"
#include "commands.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Cleanup host memory.
 */
static void _release_host(host* hst) {
  if (hst->parent_hosts) {
    delete[] hst->parent_hosts->host_name;
    delete hst->parent_hosts;
  }

  if (hst->child_hosts) {
    delete[] hst->child_hosts->host_name;
    delete hst->child_hosts;
  }

  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete hst;
}

/**
 *  Run schedule_and_propagate_triggered_host_downtime test.
 */
static void check_schedule_and_propagate_triggered_host_downtime() {
  init_object_skiplists();

  host* hst_parent = add_host("parent", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                              0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                              0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                              0.0, 0, 0, 0, 0, 0);
  host* hst_child = add_host("child", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                             0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                             0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                             NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                             0.0, 0, 0, 0, 0, 0);

  if (!hst_parent || !hst_child)
    throw (engine_error() << "create host failed.");

  add_parent_host_to_host(hst_child, "parent");
  add_child_link_to_host(hst_parent, hst_child);

  scheduled_downtime_list = NULL;
  char const* cmd("[1317196300] SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME;parent;1317196300;2000000000;0;0;7200;user;comment");
  process_external_command(cmd);

  if (!scheduled_downtime_list)
    throw (engine_error() << "schedule_and_propagate_triggered_host_downtime failed.");

  _release_host(hst_parent);
  _release_host(hst_child);

  free_object_skiplists();
}

/**
 *  Check processing of schedule_and_propagate_triggered_host_downtime works.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    testing init;

    logging::engine& engine = logging::engine::instance();
    check_schedule_and_propagate_triggered_host_downtime();
    engine.cleanup();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}