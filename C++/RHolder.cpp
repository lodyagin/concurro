/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "RHolder.hpp"

namespace curr {

DEFINE_AXIS(
  NReaders1WriterAxis,
  {
    "free",
    "reader_entering",
    "readers_entered", // read operations on an object in
                       // progress
    "reader_exiting",
    "writer_entered"   // write operation on an object
                       // in progress
  },
  { 
    { "free", "reader_entering" },
    { "reader_entering", "readers_entered" },
      
    // more readers
    { "readers_entered", "reader_entering" }, 
 
    { "readers_entered", "reader_exiting" },
    { "reader_exiting", "readers_entered" },
    { "reader_exiting", "free" },

    // <NB> only one writer
    { "free", "writer_entered" },
    { "writer_entered", "free" }
  });

DEFINE_AXIS(
  HolderAxis,
  {
    "not_initialized", // created but not bound to an
                       // object
    "charged",         // bound to an object
    "busy",            // there is some access to an object
                       // through this holder now
    "discharged",      // unbound
//    "holey_array"      // for arrays with some elements
                       // being unbound
  },
  { 
    { "not_initialized", "charged" },
    { "charged", "busy" },
    { "busy", "busy" },
    { "busy", "charged" },
    { "charged", "discharged" },
    { "discharged", "charged" },
//    { "charged", "holey_array" },
//    { "holey_array", "discharged" }
  });
}
