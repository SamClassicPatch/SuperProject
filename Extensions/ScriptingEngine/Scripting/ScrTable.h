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

#ifndef CECIL_INCL_SQTABLE_H
#define CECIL_INCL_SQTABLE_H

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

  // Bindings for various values (public interface)
  public:

    // Add a value as a variable
    template<class KeyType, class ValueType> inline
    void RegisterValue(const KeyType &valKey, const ValueType &valValue, bool bStatic = false) {
      BindValue(valKey, valValue, bStatic);
    };

    // Add a table (can be used to facilitate namespaces)
    void AddTable(const SQChar *strName, const TableBase &objTable);

    // Add an individual internal class
    void AddClass(const class AbstractClass &objClass);

    // Add a class using a registrar
    void AddClass(const class AbstractClassRegistrar &objClass);

    // Bind an empty table
    Table RegisterTable(const SQChar *strName, bool bStatic = false);

    // Bind a full clone of another table
    Table CloneTable(const SQChar *strName, const TableBase &objOther, bool bStatic = false);

  // Dynamic management
  public:

    // Find some object inside the table by name and push it on top of the stack
    bool PushObject(const SQChar *strName);

    // Create an instance of some class defined in the table and push it on top of the stack
    bool CreateInstance(const SQChar *strClassName, InstanceAny **ppInstance = NULL);

    // Create an instance of a specific class and retrieve its value
    template<class Type> inline
    bool CreateInstanceOf(const SQChar *strClassName, Type **ppVal, InstanceCopy<Type> **ppInstance = NULL) {
      InstanceCopy<Type> *pInstance;
      if (!CreateInstance(strClassName, (InstanceAny **)&pInstance)) return false;

      *ppVal = &pInstance->val;
      if (ppInstance != NULL) *ppInstance = pInstance;
      return true;
    };

    // Create a pointer instance of some class defined in the table and push it on top of the stack
    bool CreatePointerInstance(const SQChar *strClassName, SQUserPointer pData, InstanceAny **ppInstance = NULL);

    // Create a pointer instance of a specific class by passing some value into it
    template<class Type> inline
    bool CreatePointerInstanceOf(const SQChar *strClassName, Type *pData, InstancePtr<Type> **ppInstance = NULL) {
      InstancePtr<Type> *pInstance;
      if (!CreatePointerInstance(strClassName, pData, (InstanceAny **)&pInstance)) return false;

      if (ppInstance != NULL) *ppInstance = pInstance;
      return true;
    };

    // Shortcut for creating a new instance, pushing it on top of the stack and retrieving a pointer to its value
    #define PushNewInstance(_PointerType, _PointerVariable, _Table, _ClassName) \
      _PointerType *_PointerVariable; \
      { if (!(_Table).CreateInstanceOf(_ClassName, &_PointerVariable)) return SQ_ERROR; }

    // Shortcut for creating a new pointer instance with some data and pushing it on top of the stack
    #define PushNewPointer(_Table, _ClassName, _Data) \
      { if (!(_Table).CreatePointerInstanceOf(_ClassName, &(_Data))) return SQ_ERROR; }
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
