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

#ifndef CECIL_INCL_SQOBJECT_H
#define CECIL_INCL_SQOBJECT_H

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
    bool Contains(const SQChar *strKey) const;

    // Get value of a slot under some key from the object, if it exists
    Object GetValue(const SQChar *strKey) const;

    // Get size of the Squirrel object
    SQInteger GetSize(void) const;

    // Set value of a slot under some key from the object
    // Returns false if the value could not be set (e.g. wrong type or key doesn't exist)
    template<class Type> inline
    bool SetValue(const SQChar *strKey, const Type &val) const {
      sq_pushobject(m_vm, m_obj); // Push object

      // Push key and value to set
      sq_pushstring(m_vm, strKey, -1);
      Value<Type>(val).Push(m_vm);

      if (SQ_FAILED(sq_set(m_vm, -2))) {
        sq_pop(m_vm, 3); // Value [-1], key [-2], object [-3]
        return false;
      }

      sq_poptop(m_vm); // Pop object
      return true;
    };

    // Get value of a slot under some key from the object
    // Returns false if the value could not be retrieved (e.g. wrong type or key doesn't exist)
    template<class Type> inline
    bool GetValue(const SQChar *strKey, Type *pVal) const {
      sq_pushobject(m_vm, m_obj); // Push object

      // Push value from some slot by its key
      sq_pushstring(m_vm, strKey, -1);

      if (SQ_FAILED(sq_get(m_vm, -2))) {
        sq_poptop(m_vm); // Pop object
        return false;
      }

      // Retrieve value into the supplied pointer
      Value<Type> val;
      bool bResult = val.Get(m_vm);

      if (bResult) *pVal = val.val;

      sq_pop(m_vm, 2); // Value [-1], object [-2]
      return bResult;
    };

  // Bindings for various values (public interface)
  public:

    // Add a closure to the object
    void RegisterFunc(const SQChar *strKey, SQFUNCTION pFunc, bool bStatic);

    // Add a closure to the object using Squirrel declaration
    void RegisterFunc(const SQRegFunction &regfunc);

  // Bindings for various values (to be used selectively for more complex structure implementations)
  protected:

    // Add a value to the object
    inline void BindAnyValue(const AbstractValue &valKey, const AbstractValue &valValue, bool bStatic) {
      sq_pushobject(m_vm, m_obj); // Push object

      // Create a new slot with the value under a key
      valKey.Push(m_vm);
      valValue.Push(m_vm);
      sq_newslot(m_vm, -3, bStatic);

      sq_poptop(m_vm); // Pop object
    };

    // Add a value to the object under a variable name
    template<class Type> __forceinline
    void BindValue(const SQChar *valKey, const Type &valValue, bool bStatic) {
      BindAnyValue(Value<CTString>(valKey), Value<Type>(valValue), bStatic);
    };

    // Add a value to the object under an index
    template<class Type> __forceinline
    void BindValue(SQInteger valKey, const Type &valValue, bool bStatic) {
      BindAnyValue(Value<SQInteger>(valKey), Value<Type>(valValue), bStatic);
    };

    // Special cases for the compiler to catch string literals of any length

    // Add a string value to the object under a variable name
    __forceinline void BindValue(const SQChar *valKey, const SQChar *valValue, bool bStatic) {
      BindAnyValue(Value<CTString>(valKey), Value<CTString>(valValue), bStatic);
    };

    // Add a string value to the object under an index
    __forceinline void BindValue(SQInteger valKey, const SQChar *valValue, bool bStatic) {
      BindAnyValue(Value<SQInteger>(valKey), Value<CTString>(valValue), bStatic);
    };
};

}; // namespace

#endif
