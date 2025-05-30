/* Copyright (c) 2025 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef CECIL_INCL_OLD_SQCONST_H
#define CECIL_INCL_OLD_SQCONST_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ScrObject.h"

namespace sq {

// Class for adding values to an enumerator tables of some Squirrel VM
class Enumeration : public Object {
  public:
    // Constructor for an optional enumerator table
    // If no table is created, the constants are added to the current table at the top of the stack
    Enumeration(HSQUIRRELVM v, bool bCreateTable = true) : Object(v, false) {
      if (bCreateTable) {
        sq_newtable(m_vm);
        sq_getstackobj(m_vm, -1, &m_obj);
        sq_addref(m_vm, &m_obj);
        sq_poptop(m_vm);
      }
    };

  public:
    // Add an integer constant
    inline void SetValue(const SQChar *strName, const SQInteger iValue) {
      BindValue(strName, iValue, false);
    };

    // Add a float constant
    inline void SetValue(const SQChar *strName, const SQFloat fValue) {
      BindValue(strName, fValue, false);
    };

    // Add a string constant
    inline void SetValue(const SQChar *strName, const SQChar *strValue) {
      BindValue(strName, strValue, false);
    };
};

}; // namespace

#endif
