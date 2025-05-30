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

#ifndef CECIL_INCL_OLD_SQTABLE_H
#define CECIL_INCL_OLD_SQTABLE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ScrObject.h"

namespace sq {

class Table;

// Abstract class for interacting with some Squirrel table
class TableBase : public Object {
  protected:
    // Default constructor
    TableBase() : Object()
    {
    };

    // Constructor with a Squirrel VM
    TableBase(HSQUIRRELVM vmSet) : Object(vmSet, true)
    {
    };

    // Take ownership of some Squirrel table
    TableBase(HSQUIRRELVM vmSet, HSQOBJECT objSet) : Object(vmSet, objSet)
    {
    };

    // Constructor from an existing object
    TableBase(const Object &other) : Object(other)
    {
    };

  public:
    // Add a value or a data pointer
    inline void SetValue(const SQChar *strName, const Value &val) {
      BindValue(strName, val, false);
    };

    // Add a closure
    inline void SetFunc(const SQChar *strName, SQFUNCTION pFunc) {
      BindFunc(strName, pFunc, false);
    };

    // Add a table (can be used to facilitate namespaces)
    void SetTable(const SQChar *strName, const TableBase &objTable);

    // Bind an empty table
    Table AddTable(const SQChar *strName, bool bStatic = false);
};

// Class that represents or references a Squirrel table
class Table : public TableBase {
  public:
    // Default constructor
    Table() : TableBase()
    {
    };

    // Construct a new Squirrel table and take ownership of it
    Table(HSQUIRRELVM vmSet) : TableBase(vmSet) {
      sq_newtable(m_vm);
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_addref(m_vm, &m_obj);
      sq_poptop(m_vm);
    };

    // Take ownership of some Squirrel table
    Table(HSQUIRRELVM vmSet, HSQOBJECT objSet) : TableBase(vmSet, objSet)
    {
    };

    // Constructor from an existing object
    Table(const Object &other) : TableBase(other)
    {
    };
};

// Class that references a VM's root table with all the globals for Squirrel scripts
class RootTable : public TableBase {
  public:
    RootTable(HSQUIRRELVM vmSet) : TableBase(vmSet) {
      sq_pushroottable(m_vm); // Push table
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_addref(m_vm, &m_obj);
      sq_poptop(m_vm); // Pop table
    };
};

// Class that references a VM's registry table, which is similar to the root table
// except that Squirrel scripts themselves cannot directly interact with it
class RegistryTable : public TableBase {
  public:
    RegistryTable(HSQUIRRELVM vmSet) : TableBase(vmSet) {
      sq_pushregistrytable(m_vm); // Push table
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_addref(m_vm, &m_obj);
      sq_poptop(m_vm); // Pop table
    };
};

}; // namespace

#endif
