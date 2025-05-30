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

#include "StdH.h"

namespace sq {

// Assignment operator
Object &Object::operator=(const Object &other) {
  Release();

  m_vm = other.m_vm;
  m_obj = other.m_obj;
  m_bOwner = other.m_bOwner;

  // This class references the object now
  sq_addref(m_vm, &m_obj);
  return *this;
};

// Clear the class from the object
void Object::Release(void) {
  // Stop referencing the last object
  sq_release(m_vm, &m_obj);

  if (m_bOwner) sq_resetobject(&m_obj);
  m_bOwner = false;
};

// Check if a slot exists under some key
bool Object::Contains(const SQChar *strName) const {
  sq_pushobject(m_vm, m_obj); // Push object
  sq_pushstring(m_vm, strName, -1);

  if (SQ_FAILED(sq_get(m_vm, -2))) {
    sq_poptop(m_vm); // Pop object
    return false;
  }

  sq_pop(m_vm, 2); // Pop value and object
  return true;
};

// Get value of a slot under some key from the object, if it exists
Object Object::GetValue(const SQChar *strKey) const {
  sq_pushobject(m_vm, m_obj); // Push object
  sq_pushstring(m_vm, strKey, -1);

  if (SQ_FAILED(sq_get(m_vm, -2))) {
    sq_poptop(m_vm); // Pop object
    return Object(); // Return a null object
  }

  HSQOBJECT objSlotValue;
  sq_getstackobj(m_vm, -1, &objSlotValue);
  Object ret(m_vm, objSlotValue);

  sq_pop(m_vm, 2); // Pop value and object
  return ret;
};

// Get size of the Squirrel object
SQInteger Object::GetSize(void) const {
  sq_pushobject(m_vm, m_obj);
  SQInteger iSize = sq_getsize(m_vm, -1);
  sq_poptop(m_vm);

  return iSize;
};

// Register function using Squirrel declaration
void Object::RegisterFunc(const SQRegFunction &regfunc) {
  sq_pushobject(m_vm, m_obj); // Push object
  sq_pushstring(m_vm, regfunc.name, -1);

  // Create new closure and set it up
  sq_newclosure(m_vm, regfunc.f, 0);

  sq_setparamscheck(m_vm, regfunc.nparamscheck, regfunc.typemask);
  sq_setnativeclosurename(m_vm, -1, regfunc.name);

  // Create a new slot with the closure under a name
  sq_newslot(m_vm, -3, SQFalse);

  sq_poptop(m_vm); // Pop object
};

// Add a value to the object
void Object::BindValue(const SQChar *strName, const Value &val, bool bStaticVar) {
  sq_pushobject(m_vm, m_obj); // Push object

  // Create a new slot with the value under a name
  sq_pushstring(m_vm, strName, -1);
  val.Push(m_vm);
  sq_newslot(m_vm, -3, bStaticVar);

  sq_poptop(m_vm); // Pop object
};

// Add a closure to the object
void Object::BindFunc(const SQChar *strName, SQFUNCTION pFunc, bool bStaticVar) {
  sq_pushobject(m_vm, m_obj); // Push object

  // Create a new slot with the closure under a name
  sq_pushstring(m_vm, strName, -1);
  sq_newclosure(m_vm, pFunc, 0);
  sq_newslot(m_vm, -3, bStaticVar);

  sq_poptop(m_vm); // Pop object
};

}; // namespace
