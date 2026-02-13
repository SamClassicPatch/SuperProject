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

#include "StdH.h"

namespace sq {

void AbstractClass::Init(SQFUNCTION pConstructorMethod, SQFUNCTION pSetMethod, SQFUNCTION pGetMethod) {
  AbstractFactory *pFactory = GetFactory();
  const char *strFactoryType = GetFactoryType().raw_name();

  // Create a new class and store a reference to it
  sq_newclass(m_vm, SQFalse);
  sq_getstackobj(m_vm, -1, &m_obj);
  sq_addref(m_vm, &m_obj);

  // Set typetag from the pointer to the instance factory
  sq_settypetag(m_vm, -1, (SQUserPointer)pFactory);

  // Create static tables of setter & getter methods
  m_sqtSetters = AddTable(SQCLASS_SETTER_TABLE, true);
  m_sqtGetters = AddTable(SQCLASS_GETTER_TABLE, true);

  // Define metamethods
  sq_pushstring(m_vm, "constructor", -1);
  sq_pushstring(m_vm, strFactoryType, -1); // Free var
  sq_newclosure(m_vm, pConstructorMethod, 1);
  sq_setnativeclosurename(m_vm, -1, "constructor");
  sq_newslot(m_vm, -3, SQFalse);

  sq_pushstring(m_vm, "_set", -1);
  sq_pushobject(m_vm, m_sqtSetters.GetObj()); // Free var
  sq_pushstring(m_vm, strFactoryType, -1); // Free var
  sq_newclosure(m_vm, pSetMethod, 2);
  sq_setnativeclosurename(m_vm, -1, "_set");
  sq_newslot(m_vm, -3, SQFalse);

  sq_pushstring(m_vm, "_get", -1);
  sq_pushobject(m_vm, m_sqtGetters.GetObj()); // Free var
  sq_pushstring(m_vm, strFactoryType, -1); // Free var
  sq_newclosure(m_vm, pGetMethod, 2);
  sq_setnativeclosurename(m_vm, -1, "_get");
  sq_newslot(m_vm, -3, SQFalse);

  sq_pushstring(m_vm, "weakref", -1);
  sq_newclosure(m_vm, &ClassWeakRef, 0);
  sq_setnativeclosurename(m_vm, -1, "weakref");
  sq_newslot(m_vm, -3, SQFalse);

  sq_pushstring(m_vm, "_typeof", -1);
  sq_pushstring(m_vm, m_strClassName.str_String, -1); // Free var
  sq_newclosure(m_vm, &ClassTypeOf, 1);
  sq_setnativeclosurename(m_vm, -1, "_typeof");
  sq_newslot(m_vm, -3, SQFalse);

  sq_poptop(m_vm); // Pop class
};

AbstractFactory *AbstractClass::GetFactory(void) {
  // Try to retrieve factory table from the registry
  sq_pushregistrytable(m_vm);
  sq_pushstring(m_vm, SQCLASS_FACTORY_TABLE, -1);

  // If it doesn't exist yet
  if (SQ_FAILED(sq_rawget(m_vm, -2))) {
    sq_newtable(m_vm); // Push new table that will be used afterwards

    // Push name as a key and the table again as a value
    sq_pushstring(m_vm, SQCLASS_FACTORY_TABLE, -1);
    sq_push(m_vm, -2);

    // Add this table to the registry and leave the initially pushed table on the top
    sq_rawset(m_vm, -4);
  }

  // Try to retrieve existing factory from the table
  const char *strFactoryType = GetFactoryType().raw_name();
  sq_pushstring(m_vm, strFactoryType, -1);

  AbstractFactory *pFactory;

  // Retrieve it from user data
  if (SQ_SUCCEEDED(sq_rawget(m_vm, -2))) {
    pFactory = AbstractFactory::GetFromStack(m_vm, -1);

  // Otherwise create a new factory in the table
  } else {
    // Push new factory
    AbstractFactory **pUserData = (AbstractFactory **)sq_newuserdata(m_vm, sizeof(AbstractFactory *));
    *pUserData = NewFactory();
    sq_setreleasehook(m_vm, -1, &AbstractFactory::CleanupHook);

    pFactory = *pUserData;

    // Push type as a key and the factory again as a value
    sq_pushstring(m_vm, strFactoryType, -1);
    sq_push(m_vm, -2);

    // Add this factory to the table and leave the initially pushed factory on the top
    sq_rawset(m_vm, -4);
  }

  sq_pop(m_vm, 3); // Factory [-1], factory table [-2], registry table [-3]
  return pFactory;
};

// Helper method for retrieving a variable name as a key and a setter/getter function as a value
bool AbstractClass::GetFunctionForVariable(HSQUIRRELVM v, const SQChar **pstrVariable, bool &bIndex, SQUserPointer *ppFunc) {
  sq_push(v, 2); // Push variable name argument on top

  *pstrVariable = NULL;
  bIndex = (sq_gettype(v, -1) != OT_STRING);

  // Remember it as an actual string by converting it, if needed (pushes a new string)
  if (SQ_SUCCEEDED(sq_tostring(v, -1))) {
    sq_getstring(v, -1, pstrVariable);
    sq_poptop(v);
  }

  // Try to find a method for this variable in the table
  if (SQ_FAILED(sq_rawget(v, -2))) return false;

  // Retrieve a pointer to the method and pop it
  SQRESULT r = sq_getuserpointer(v, -1, ppFunc);
  sq_poptop(v);

  return SQ_SUCCEEDED(r);
};

}; // namespace
