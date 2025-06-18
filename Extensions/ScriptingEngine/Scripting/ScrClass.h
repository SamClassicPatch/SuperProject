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

#ifndef CECIL_INCL_SQCLASS_H
#define CECIL_INCL_SQCLASS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ScrInstances.h"
#include "ScrTable.h"

namespace sq {

#define SQCLASS_FACTORY_TABLE "@@class_factory_table@@"
#define SQCLASS_SETTER_TABLE  "@@class_set_table@@"
#define SQCLASS_GETTER_TABLE  "@@class_get_table@@"

// Abstract base for native Squirrel class declarations
class AbstractClass : public TableBase {
  protected:
    const CTString m_strClassName; // Class name in Squirrel

    Table m_sqtSetters; // Table of setter methods per field
    Table m_sqtGetters; // Table of getter methods per field

    // Construct a new Squirrel class declaration and take ownership of it
    AbstractClass(HSQUIRRELVM vmSet, const SQChar *strName) : TableBase(vmSet), m_strClassName(strName)
    {
    };

    // Initialize a specific native class on creation
    void Init(SQFUNCTION pConstructorMethod, SQFUNCTION pSetMethod, SQFUNCTION pGetMethod);

    // Create new factory of a specific type
    virtual AbstractFactory *NewFactory(void) = 0;

    // Get type info for a factory
    virtual const std::type_info &GetFactoryType(void) = 0;

    // Get factory for the current class (create new one if not found)
    AbstractFactory *GetFactory(void);

  public:
    // Get class name
    inline const CTString &GetName(void) const {
      return m_strClassName;
    };

    // Set static value under some key
    template<class Type> inline
    void SetStaticValue(const SQChar *strName, const Type &val) {
      BindValue(strName, val, true);
    };

  // Metamethods
  protected:

    // Helper method for retrieving a variable name as a key and a setter/getter function as a value
    static bool GetFunctionForVariable(HSQUIRRELVM v, const SQChar **pstrVariable, SQUserPointer *ppFunc);

    static SQInteger ClassWeakRef(HSQUIRRELVM v) {
      // Push weak reference of the instance
      sq_weakref(v, -1);
      return 1;
    };

    static SQInteger ClassTypeOf(HSQUIRRELVM v) {
      // Duplicate pushed class name as a return value
      sq_push(v, -1);
      return 1;
    };
};

// Native Squirrel class of a specific type
template<class Type>
class Class : public AbstractClass {
  public:
    // Optional class functions
    typedef SQInteger (*FConstructor)(HSQUIRRELVM v, Type &val);
    typedef SQInteger (*FSetter)(HSQUIRRELVM v, Type &val, SQInteger iValueInStack);
    typedef SQInteger (*FGetter)(HSQUIRRELVM v, Type &val);
    typedef SQInteger (*FToString)(HSQUIRRELVM v, Type &val);

    // Instance factory for this class
    class Factory : public AbstractFactory {
      public:
        Factory(FConstructor pSetConstructor, Type *pSetValueReference) :
          AbstractFactory(pSetConstructor, pSetValueReference, typeid(Type))
        {
        };

        virtual InstanceAny *NewInstance(HSQUIRRELVM vmSet, SQInteger idx) {
          InstanceAny *pNew;

          // Create instance with a reference to the value
          if (m_pValReference != NULL) {
            pNew = new Instance<Type &>(vmSet, this, *(Type *)m_pValReference);

          // Create instance with its own value
          } else {
            pNew = new Instance<Type>(vmSet, this);
          }

          // Couldn't set it up
          if (!pNew->Setup(idx)) {
            delete pNew;
            return NULL;
          }

          return pNew;
        };
    };

  private:
    FConstructor m_pConstructor; // Optional constructor method
    Type *m_pValReference; // Original value for all class instances to reference

  private:
    // Cannot be reassigned
    Class(const Class &other);
    void operator=(const Class &other);

  public:
    // Construct a Squirrel class declaration that creates instances with own copies of values
    Class(HSQUIRRELVM vmSet, const SQChar *strName, FConstructor pSetConstructor) :
      AbstractClass(vmSet, strName), m_pConstructor(pSetConstructor), m_pValReference(NULL)
    {
      Init(&ClassConstructor, &ClassSet, &ClassGet);
    };

    // Construct a Squirrel class declaration that creates instances with a reference to some external value
    Class(HSQUIRRELVM vmSet, const SQChar *strName, FConstructor pSetConstructor, Type &valSetReference) :
      AbstractClass(vmSet, strName), m_pConstructor(pSetConstructor), m_pValReference(&valSetReference)
    {
      Init(&ClassConstructor, &ClassSet, &ClassGet);
    };

    // Add getter and optional setter methods for working with some variable
    inline void RegisterVar(const SQChar *strVariable, FGetter pGetter, FSetter pSetter) {
      if (pGetter != NULL) m_sqtGetters.SetValue(strVariable, (void *)pGetter);
      if (pSetter != NULL) m_sqtSetters.SetValue(strVariable, (void *)pSetter);
    };

    // Add metamethod for converting instances into strings
    inline void RegisterToString(FToString pToString) {
      sq_pushobject(m_vm, m_obj); // Push class

      sq_pushstring(m_vm, "_tostring", -1);
      sq_pushstring(m_vm, GetFactoryType().raw_name(), -1); // Free var
      sq_pushuserpointer(m_vm, pToString); // Free var
      sq_newclosure(m_vm, &ClassToString, 2);
      sq_setnativeclosurename(m_vm, -1, "_tostring");
      sq_newslot(m_vm, -3, SQFalse);

      sq_poptop(m_vm); // Pop class
    };

  protected:

    // Create new factory of a specific type
    virtual AbstractFactory *NewFactory(void) {
      return new Factory(m_pConstructor, m_pValReference);
    };

    // Get type info for a factory
    virtual const std::type_info &GetFactoryType(void) {
      return typeid(Type);
    };

  // Metamethods
  protected:

    static SQInteger ClassConstructor(HSQUIRRELVM v);
    static SQInteger ClassSet(HSQUIRRELVM v);
    static SQInteger ClassGet(HSQUIRRELVM v);
    static SQInteger ClassToString(HSQUIRRELVM v);
};

template<class Type> inline
SQInteger Class<Type>::ClassConstructor(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  // Create a new instance using a factory
  AbstractFactory *pFactory = AbstractFactory::Find(v, strFactoryType);

  if (pFactory == NULL) {
    sq_throwerror(v, "no instance factory");
    return SQ_ERROR;
  }

  // Couldn't create a new instance
  InstanceAny *pInstance = pFactory->NewInstance(v, 1);
  if (pInstance == NULL) return SQ_ERROR;

  // No custom constructor to call
  if (pFactory->m_pConstructorFunc == NULL) return 0;

  // Call custom constructor
  FConstructor pFunc = (FConstructor)pFactory->m_pConstructorFunc;
  Type *pRef = (Type *)pFactory->m_pValReference;

  if (pRef != NULL) {
    return pFunc(v, *pRef);
  }

  return pFunc(v, ((Instance<Type> *)pInstance)->val);
};

template<class Type> inline
SQInteger Class<Type>::ClassSet(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -2, &strFactoryType);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Retrieve setter method for the variable
  const SQChar *strVariable;
  SQUserPointer pPtrToFunc;

  if (!GetFunctionForVariable(v, &strVariable, &pPtrToFunc)) {
    SQChar strError[256];
    scsprintf(strError, 256, "variable '%s' is read-only or does not exist", strVariable);

    return sq_throwerror(v, strError);
  }

  // Call setter method for the data
  FSetter pFunc = (FSetter)pPtrToFunc;
  Type *pRef = (Type *)pInstance->GetFactory()->m_pValReference;

  if (pRef != NULL) {
    return pFunc(v, *pRef, 3);
  }

  return pFunc(v, ((Instance<Type> *)pInstance)->val, 3);
};

template<class Type> inline
SQInteger Class<Type>::ClassGet(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -2, &strFactoryType);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Retrieve getter method for the variable
  const SQChar *strVariable;
  SQUserPointer pPtrToFunc;

  if (!GetFunctionForVariable(v, &strVariable, &pPtrToFunc)) {
    SQChar strError[256];
    scsprintf(strError, 256, "variable '%s' cannot be read from", strVariable);

    return sq_throwerror(v, strError);
  }

  // Call getter method for the data
  FGetter pFunc = (FGetter)pPtrToFunc;
  Type *pRef = (Type *)pInstance->GetFactory()->m_pValReference;

  if (pRef != NULL) {
    return pFunc(v, *pRef);
  }

  return pFunc(v, ((Instance<Type> *)pInstance)->val);
};

template<class Type> inline
SQInteger Class<Type>::ClassToString(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Call conversion data for the data
  FToString pFunc = (FToString)pToStringFunc;
  Type *pRef = (Type *)pInstance->GetFactory()->m_pValReference;

  if (pRef != NULL) {
    return pFunc(v, *pRef);
  }

  return pFunc(v, ((Instance<Type> *)pInstance)->val);
};

}; // namespace

#endif
