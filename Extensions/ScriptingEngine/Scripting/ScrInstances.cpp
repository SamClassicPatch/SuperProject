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

// Get instance of a some native class from the stack with an optional factory matching
InstanceAny *InstanceAny::Retrieve(HSQUIRRELVM v, SQInteger idx, const AbstractFactory *pOptionalFactory) {
  InstanceAny *pInstance = NULL;
  SQRESULT r = sq_getinstanceup(v, idx, (SQUserPointer *)&pInstance, (SQUserPointer)pOptionalFactory, SQTrue);

  // Not an instance or an instance of a non-native script class
  if (SQ_FAILED(r)) return NULL;

  // Incomplete script class derived from a native class
  if (pInstance == NULL) {
    sq_throwerror(v, "missing base.constructor() call in the Squirrel class that extends a native class");
    return NULL;
  }

  return pInstance;
};

// Get instance of a some native class from the stack by finding and matching the factory using its type
InstanceAny *InstanceAny::OfType(HSQUIRRELVM v, SQInteger idx, const SQChar *strFactoryType) {
  // Make sure that a factory for this type exists
  const AbstractFactory *pExistingFactory = AbstractFactory::Find(v, strFactoryType);

  if (pExistingFactory == NULL) {
    sq_throwerror(v, "no instance factory");
    return NULL;
  }

  // Retrieve an instance of a native class
  InstanceAny *pInstance = Retrieve(v, idx, pExistingFactory);
  if (pInstance == NULL) return NULL;

  // Get factory from the type tag of this instance's class
  AbstractFactory *pFoundFactory = NULL;
  sq_gettypetag(v, idx, (SQUserPointer *)&pFoundFactory);

  if (pFoundFactory == NULL) {
    // Otherwise try to find it among type tags of the entire class hierarchy
    SQInteger iTop = sq_gettop(v);
    sq_getclass(v, idx);

    while (pFoundFactory == NULL) {
      sq_getbase(v, -1);
      sq_gettypetag(v, -1, (SQUserPointer *)&pFoundFactory);
    }

    sq_settop(v, iTop);
  }

  // This instance does not belong to the specified factory
  if (pFoundFactory == NULL) {
    ASSERTALWAYS("Instance has no factory in any of its type tags");
    sq_throwerror(v, "instance has no factory in any of its type tags");
    return NULL;

  } else if (pExistingFactory != pFoundFactory) {
    ASSERTALWAYS("Instance does not belong to the specified factory");
    sq_throwerror(v, "instance does not belong to the specified factory");
    return NULL;
  }

  return pInstance;
};

// Find a factory by its type in the VM's registry
AbstractFactory *AbstractFactory::Find(HSQUIRRELVM v, const SQChar *strFactoryType) {
  // Try to retrieve factory table from the registry
  sq_pushregistrytable(v);
  sq_pushstring(v, SQCLASS_FACTORY_TABLE, -1);

  // Factory table not found
  if (SQ_FAILED(sq_rawget(v, -2))) {
    sq_poptop(v); // Registry table [-1]
    return NULL;
  }

  // Get factory from the table
  sq_pushstring(v, strFactoryType, -1);

  AbstractFactory *pData = NULL;

  if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
    pData = GetFromStack(v, -1);
    sq_poptop(v); // Factory [-1]
  }

  sq_pop(v, 2); // Factory table [-1], registry table [-2]
  return pData;
};

}; // namespace
