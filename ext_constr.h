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

#ifndef CONCURRO_TYPES_EXT_CONSTR_H
#define CONCURRO_TYPES_EXT_CONSTR_H

namespace curr {

//! T is a type to be constructed externally by a placement
//! new operator. It will also never call a destructor for
//! the object.
template<class T>
union externally_constructed
{
  typedef T type;

  externally_constructed() {} // it is
  ~externally_constructed() {} // never
  T m;
};

}


#endif
