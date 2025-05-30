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
    Object(HSQUIRRELVM vmSet, bool bObjectOwner) : m_vm(vmSet), m_bOwner(bObjectOwner) {
      sq_resetobject(&m_obj);
    };

  public:
    // Constructor with no object
    Object() : m_vm(NULL), m_bOwner(false) {
      sq_resetobject(&m_obj);
    };

    // Take ownership of some Squirrel object
    Object(HSQUIRRELVM vmSet, HSQOBJECT objSet) : m_vm(vmSet), m_obj(objSet), m_bOwner(true) {
      // This class references the object now
      sq_addref(m_vm, &m_obj);
    };

    // Copy constructor
    Object(const Object &other) : m_vm(other.m_vm), m_obj(other.m_obj), m_bOwner(other.m_bOwner) {
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
    void BindValue(const SQChar *strName, const Value &val, bool bStaticVar);

    // Add a closure to the object
    void BindFunc(const SQChar *strName, SQFUNCTION pFunc, bool bStaticVar);
};

}; // namespace

#endif
