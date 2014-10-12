/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef TYPES_EXT_CONSTR_H
#define TYPES_EXT_CONSTR_H

namespace types {

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
