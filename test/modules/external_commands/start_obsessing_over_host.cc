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

#include <exception>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run start_obsessing_over_host test.
 */
static int check_start_obsessing_over_host(int argc, char** argv) {
  (void)argc;
  (void)argv;

  host* hst = unittest::add_generic_host();
  if (!hst)
    throw (engine_error() << "create host failed.");

  hst->obsess_over_host = false;
  char const* cmd("[1317196300] START_OBSESSING_OVER_HOST;name");
  process_external_command(cmd);

  if (!hst->obsess_over_host)
    throw (engine_error() << "start_obsessing_over_host failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_start_obsessing_over_host);
  return (utest.run());
}
