/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/deleter/serviceescalation.hh"
#include "com/centreon/engine/objects.hh"

/**
 *  Delete serviceescalation.
 *
 *  @param[in] ptr The serviceescalation to delete.
 */
void deleter::serviceescalation(void* ptr) throw () {
  serviceescalation_struct* obj(static_cast<serviceescalation_struct*>(ptr));

  delete obj;
}
