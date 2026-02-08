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

// Supported metamethod types for classes
enum EMetamethod {
  E_MM_ADD,
  E_MM_SUB,
  E_MM_MUL,
  E_MM_DIV,
  E_MM_UNM,
  E_MM_MODULO,
  E_MM_NEXTI,
  E_MM_CMP,
  E_MM_CLONED,
  E_MM_TOSTRING,
};

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
    typedef SQInteger (*FSetter)(HSQUIRRELVM v, Type &val, SQInteger idxValue);
    typedef SQInteger (*FGetter)(HSQUIRRELVM v, Type &val);

    // Metamethod for performing an operation between itself and another value
    typedef SQInteger (*FOther)(HSQUIRRELVM v, Type &val, SQInteger idxOther);

    // Metamethod for performing an operation on itself
    typedef SQInteger (*FSelf)(HSQUIRRELVM v, Type &val);

    // Metamethod for iterating through the class
    typedef SQInteger (*FNextIndex)(HSQUIRRELVM v, Type &val, const SQInteger *piPrev);

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

    // Add getter and optional setter methods for working with some index
    inline void RegisterIndex(SQInteger iIndex, FGetter pGetter, FSetter pSetter) {
      if (pGetter != NULL) m_sqtGetters.SetIndexedValue(iIndex, (void *)pGetter);
      if (pSetter != NULL) m_sqtSetters.SetIndexedValue(iIndex, (void *)pSetter);
    };

    // Add a custom metamethod
    inline void RegisterMetamethod(EMetamethod eType, FOther pFunction) {
      const SQChar *strName = NULL;

      switch (eType) {
        case E_MM_ADD:    strName = "_add"; break;
        case E_MM_SUB:    strName = "_sub"; break;
        case E_MM_MUL:    strName = "_mul"; break;
        case E_MM_DIV:    strName = "_div"; break;
        case E_MM_MODULO: strName = "_modulo"; break;
        case E_MM_CMP:    strName = "_cmp"; break;
        case E_MM_CLONED: strName = "_cloned"; break;

        default: {
          ASSERTALWAYS("Unknown metamethod type for FOther function!");
          return;
        }
      }

      ASSERT(strName != NULL);
      sq_pushobject(m_vm, m_obj); // Push class

      sq_pushstring(m_vm, strName, -1);
      sq_pushstring(m_vm, GetFactoryType().raw_name(), -1); // Free var
      sq_pushuserpointer(m_vm, pFunction); // Free var
      sq_newclosure(m_vm, &ClassMetaOther, 2);
      sq_setnativeclosurename(m_vm, -1, strName);
      sq_newslot(m_vm, -3, SQFalse);

      sq_poptop(m_vm); // Pop class
    };

    // Add a custom metamethod
    inline void RegisterMetamethod(EMetamethod eType, FSelf pFunction) {
      const SQChar *strName = NULL;

      switch (eType) {
        case E_MM_UNM:      strName = "_unm"; break;
        case E_MM_TOSTRING: strName = "_tostring"; break;

        default: {
          ASSERTALWAYS("Unknown metamethod type for FSelf function!");
          return;
        }
      }

      ASSERT(strName != NULL);
      sq_pushobject(m_vm, m_obj); // Push class

      sq_pushstring(m_vm, strName, -1);
      sq_pushstring(m_vm, GetFactoryType().raw_name(), -1); // Free var
      sq_pushuserpointer(m_vm, pFunction); // Free var
      sq_newclosure(m_vm, &ClassMetaSelf, 2);
      sq_setnativeclosurename(m_vm, -1, strName);
      sq_newslot(m_vm, -3, SQFalse);

      sq_poptop(m_vm); // Pop class
    };

    // Add a custom metamethod for iteration
    inline void RegisterMetamethod(FNextIndex pFunction) {
      sq_pushobject(m_vm, m_obj); // Push class

      sq_pushstring(m_vm, "_nexti", -1);
      sq_pushstring(m_vm, GetFactoryType().raw_name(), -1); // Free var
      sq_pushuserpointer(m_vm, pFunction); // Free var
      sq_newclosure(m_vm, &ClassMetaNextIndex, 2);
      sq_setnativeclosurename(m_vm, -1, "_nexti");
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

    static SQInteger ClassMetaOther(HSQUIRRELVM v); // FOther
    static SQInteger ClassMetaSelf(HSQUIRRELVM v); // FSelf
    static SQInteger ClassMetaNextIndex(HSQUIRRELVM v); // FNextIndex
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
SQInteger Class<Type>::ClassMetaOther(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Call conversion data for the data
  FOther pFunc = (FOther)pToStringFunc;
  Type *pRefSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pRefSelf == NULL) pRefSelf = &((Instance<Type> *)pInstance)->val;

  // Pass other value as an index in the stack
  return pFunc(v, *pRefSelf, 2);
};

template<class Type> inline
SQInteger Class<Type>::ClassMetaSelf(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Call conversion data for the data
  FSelf pFunc = (FSelf)pToStringFunc;
  Type *pRefSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pRefSelf == NULL) pRefSelf = &((Instance<Type> *)pInstance)->val;

  return pFunc(v, *pRefSelf);
};

template<class Type> inline
SQInteger Class<Type>::ClassMetaNextIndex(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Get previous index
  SQInteger iPrev;
  bool bPrevIndex = false;

  if (sq_gettype(v, 2) != OT_NULL) {
    bPrevIndex = SQ_SUCCEEDED(sq_getinteger(v, 2, &iPrev));
  }

  // Call conversion data for the data
  FNextIndex pFunc = (FNextIndex)pToStringFunc;
  Type *pRefSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pRefSelf == NULL) pRefSelf = &((Instance<Type> *)pInstance)->val;

  // Pass previous index only if it has been retrieved
  return pFunc(v, *pRefSelf, bPrevIndex ? &iPrev : NULL);
};

}; // namespace

#endif
