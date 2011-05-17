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

#include <limits.h>
#include <sstream>
#include <string.h>
#include "error.hh"

using namespace com::centreon::engine;

/**
 *  Check that unsigned long long insertion works properly in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert unsigned long longs.
  error e;
  e << 42ull << 1ull << 7896ull << ULLONG_MAX - 789;
  e << ULLONG_MAX << 0ull;

  // Conversion reference.
  std::ostringstream oss;
  oss << 42ull << 1ull << 7896ull << ULLONG_MAX - 789
      << ULLONG_MAX << 0ull;

  // Check.
  return (strcmp(oss.str().c_str(), e.what()));
}