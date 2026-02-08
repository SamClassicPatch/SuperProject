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

#ifndef CECIL_INCL_OLD_SQOBJECT_H
#define CECIL_INCL_OLD_SQOBJECT_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ScrValue.h"

namespace sq {

// Class that either owns or references an object in some Squirrel VM
// May be derived to provide extra interactions with the object, e.g. a table or a class
class Object {
  protected:
    HSQUIRRELVM m_vm; // Squirrel VM this object belongs to
    HSQOBJECT m_obj;  // Reference to the Squirrel object
    bool m_bOwner;    // Whether this class currently owns any object

    // Constructor for creating empty objects of derived classes
    inline Object(HSQUIRRELVM vmSet, bool bObjectOwner) : m_vm(vmSet), m_bOwner(bObjectOwner) {
      sq_resetobject(&m_obj);
    };

  public:
    // Constructor with no object
    inline Object() : m_vm(NULL), m_bOwner(false) {
      sq_resetobject(&m_obj);
    };

    // Take ownership of some Squirrel object
    inline Object(HSQUIRRELVM vmSet, HSQOBJECT objSet) : m_vm(vmSet), m_obj(objSet), m_bOwner(true) {
      // This class references the object now
      sq_addref(m_vm, &m_obj);
    };

    // Take ownership of some Squirrel object from the stack
    inline Object(HSQUIRRELVM vmSet, SQInteger idx) : m_vm(vmSet)
    {
      if (SQ_SUCCEEDED(sq_getstackobj(m_vm, idx, &m_obj))) {
        // This class references the object now
        m_bOwner = true;
        sq_addref(m_vm, &m_obj);

      } else {
        m_bOwner = false;
      }
    };

    // Copy constructor
    inline Object(const Object &other) : m_vm(other.m_vm), m_obj(other.m_obj), m_bOwner(other.m_bOwner) {
      // This class references the object now
      sq_addref(m_vm, &m_obj);
    };

    // Destructor
    virtual ~Object() {
      Release();
    };

    // Get object's VM
    __forceinline HSQUIRRELVM GetVM(void) const { return m_vm; };

    // Get object handler
    __forceinline HSQOBJECT GetObj(void) const { return m_obj; };

  public:
    // Assignment operator
    Object &operator=(const Object &other);

    // Clear the class from the object
    void Release(void);

    // Check if a slot exists under some key
    bool Contains(const SQChar *strName) const;

    // Get value of a slot under some key from the object, if it exists
    Object GetValue(const SQChar *strKey) const;

    // Get size of the Squirrel object
    SQInteger GetSize(void) const;

    // Register function using Squirrel declaration
    void RegisterFunc(const SQRegFunction &regfunc);

  // Bindings for various values
  protected:

    // Add a value to the object
    template<class Type> inline
    void BindValue(const SQChar *strName, const Type &val, bool bStaticVar) {
      sq_pushobject(m_vm, m_obj); // Push object

      // Create a new slot with the value under a name
      sq_pushstring(m_vm, strName, -1);
      Value<Type>(val).Push(m_vm);
      sq_newslot(m_vm, -3, bStaticVar);

      sq_poptop(m_vm); // Pop object
    };

    // Add an indexed value to the object
    template<class Type> inline
    void BindValue(SQInteger i, const Type &val, bool bStaticVar) {
      sq_pushobject(m_vm, m_obj); // Push object

      // Create a new slot with the value under an index
      sq_pushinteger(m_vm, i);
      Value<Type>(val).Push(m_vm);
      sq_newslot(m_vm, -3, bStaticVar);

      sq_poptop(m_vm); // Pop object
    };

    // Special cases to make the compiler shut it
    inline void BindValue(const SQChar *strName, const char *val, bool bStaticVar) {
      BindValue(strName, CTString(val), bStaticVar);
    };

    inline void BindValue(SQInteger i, const char *val, bool bStaticVar) {
      BindValue(i, CTString(val), bStaticVar);
    };

    // Add a closure to the object
    void BindFunc(const SQChar *strName, SQFUNCTION pFunc, bool bStaticVar);
};

}; // namespace

#endif
