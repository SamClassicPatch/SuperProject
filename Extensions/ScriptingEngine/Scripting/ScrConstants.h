/* Copyright (c) 2025-2026 Dreamy Cecil
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
    // After adding all constants, the enum table needs to be added to the Squirrel VM using ConstTable::AddEnum()
    inline Enumeration(HSQUIRRELVM v, bool bCreateTable = true) : Object(v, false) {
      if (bCreateTable) {
        sq_newtable(m_vm);
        sq_getstackobj(m_vm, -1, &m_obj);
        sq_addref(m_vm, &m_obj);
        sq_poptop(m_vm);
      }
    };

  public:
    // Add an integer constant
    #define SET_INT_VALUE(_Type) \
      inline void SetValue(const SQChar *strName, const _Type iValue) { BindValue(strName, iValue, false); };

    SET_INT_VALUE(SBYTE);
    SET_INT_VALUE(UBYTE);
    SET_INT_VALUE(SWORD);
    SET_INT_VALUE(UWORD);
    SET_INT_VALUE(SLONG);
    SET_INT_VALUE(ULONG);
    SET_INT_VALUE(int);
    SET_INT_VALUE(__int64);

    #undef SET_INT_VALUE

    // Add a float constant
    inline void SetValue(const SQChar *strName, const FLOAT fValue) {
      BindValue(strName, fValue, false);
    };

    inline void SetValue(const SQChar *strName, const DOUBLE fValue) {
      BindValue(strName, fValue, false);
    };

    // Add a string constant
    inline void SetValue(const SQChar *strName, const SQChar *strValue) {
      BindValue(strName, strValue, false);
    };
};

// Class that references a VM's const table with all the constant values for Squirrel scripts
class ConstTable : public Enumeration {
  public:
    inline ConstTable(HSQUIRRELVM v) : Enumeration(v, false) {
      // Store object reference to the const table immediately
      sq_pushconsttable(m_vm);
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_poptop(m_vm);
    };

  public:
    // Define an enumeration table under some name
    void AddEnum(const SQChar *strName, const Enumeration &en) {
      ASSERT(GetVM() == en.GetVM());

      HSQUIRRELVM v = GetVM();
      sq_pushconsttable(v); // Push const table

      // Create enum table in the const table
      sq_pushstring(v, strName, -1);
      sq_pushobject(v, en.GetObj());
      sq_newslot(v, -3, SQFalse);

      sq_poptop(v); // Pop const table
    };
};

}; // namespace

#endif
