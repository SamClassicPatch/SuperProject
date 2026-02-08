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
    inline TableBase() : Object()
    {
    };

    // Constructor with a Squirrel VM
    inline TableBase(HSQUIRRELVM vmSet) : Object(vmSet, true)
    {
    };

    // Take ownership of some Squirrel table
    inline TableBase(HSQUIRRELVM vmSet, HSQOBJECT objSet) : Object(vmSet, objSet)
    {
    };

    // Take ownership of some Squirrel table from the stack
    inline TableBase(HSQUIRRELVM vmSet, SQInteger idx) : Object(vmSet, idx)
    {
    };

    // Constructor from an existing object
    inline TableBase(const Object &other) : Object(other)
    {
    };

  public:
    // Add a value as a variable
    template<class Type> inline
    void SetValue(const SQChar *strName, const Type &val) {
      BindValue(strName, val, false);
    };

    // Add an indexed value
    template<class Type> inline
    void SetIndexedValue(SQInteger i, const Type &val) {
      BindValue(i, val, false);
    };

    // Add a closure
    inline void SetFunc(const SQChar *strName, SQFUNCTION pFunc) {
      BindFunc(strName, pFunc, false);
    };

    // Add a table (can be used to facilitate namespaces)
    void SetTable(const SQChar *strName, const TableBase &objTable);

    // Add a class
    void SetClass(const class AbstractClass &objClass);

    // Bind an empty table
    Table AddTable(const SQChar *strName, bool bStatic = false);

    // Find some object inside the table by name and push it on top of the stack
    bool PushObject(const SQChar *strName);

    // Create an instance of some class defined in the table and push it on top of the stack
    bool CreateInstance(const SQChar *strClassName, InstanceAny **ppInstance = NULL);

    // Create an instance of a specific class and retrieve its value
    template<class Type> inline
    bool CreateInstanceOf(const SQChar *strClassName, Type **ppVal) {
      Instance<Type> *pInstance = NULL;
      if (!CreateInstance(strClassName, (InstanceAny **)&pInstance)) return false;

      *ppVal = &pInstance->val;
      return true;
    };
};

// Class that represents or references a Squirrel table
class Table : public TableBase {
  public:
    // Default constructor
    inline Table() : TableBase()
    {
    };

    // Construct a new Squirrel table and take ownership of it
    inline Table(HSQUIRRELVM vmSet) : TableBase(vmSet) {
      sq_newtable(m_vm);
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_addref(m_vm, &m_obj);
      sq_poptop(m_vm);
    };

    // Take ownership of some Squirrel table
    inline Table(HSQUIRRELVM vmSet, HSQOBJECT objSet) : TableBase(vmSet, objSet)
    {
    };

    // Take ownership of some Squirrel table from the stack
    inline Table(HSQUIRRELVM vmSet, SQInteger idx) : TableBase(vmSet, idx)
    {
    };

    // Constructor from an existing object
    inline Table(const Object &other) : TableBase(other)
    {
    };
};

// Class that references a VM's root table with all the globals for Squirrel scripts
class RootTable : public TableBase {
  public:
    inline RootTable(HSQUIRRELVM vmSet) : TableBase(vmSet) {
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
    inline RegistryTable(HSQUIRRELVM vmSet) : TableBase(vmSet) {
      sq_pushregistrytable(m_vm); // Push table
      sq_getstackobj(m_vm, -1, &m_obj);
      sq_addref(m_vm, &m_obj);
      sq_poptop(m_vm); // Pop table
    };
};

}; // namespace

#endif
