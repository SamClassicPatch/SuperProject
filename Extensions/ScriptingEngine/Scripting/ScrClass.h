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

#define SQCLASS_PTRNAME(_ClassName) (CTString(0, "@@%s_ptr_type@@", _ClassName).str_String)

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
    static bool GetFunctionForVariable(HSQUIRRELVM v, const SQChar **pstrVariable, bool &bIndex, SQUserPointer *ppFunc);

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

// Individual native Squirrel class of a specific type
template<class Type>
class InternalClass : public AbstractClass {
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

  private:
    FConstructor m_pConstructor; // Optional constructor method
    Type *m_pValReference; // Original value for all class instances to reference

  private:
    // Cannot be reassigned
    InternalClass(const InternalClass &other);
    void operator=(const InternalClass &other);

  public:
    // Construct a Squirrel class declaration that creates instances with own copies of values
    InternalClass(HSQUIRRELVM vmSet, const SQChar *strName, bool bPtrType, FConstructor pSetConstructor) :
      AbstractClass(vmSet, bPtrType ? SQCLASS_PTRNAME(strName) : strName), m_pConstructor(pSetConstructor), m_pValReference(NULL)
    {
      Init(bPtrType ? &ClassPtrConstructor : &ClassConstructor, &ClassSet, &ClassGet);
    };

    // Construct a Squirrel class declaration that creates instances with a reference to some external value
    InternalClass(HSQUIRRELVM vmSet, const SQChar *strName, bool bPtrType, FConstructor pSetConstructor, Type &valSetReference) :
      AbstractClass(vmSet, bPtrType ? SQCLASS_PTRNAME(strName) : strName), m_pConstructor(pSetConstructor), m_pValReference(&valSetReference)
    {
      Init(bPtrType ? &ClassPtrConstructor : &ClassConstructor, &ClassSet, &ClassGet);
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
    inline void RegisterMetamethod(EMetamethod eType, FNextIndex pFunction) {
      if (eType != E_MM_NEXTI) {
        ASSERTALWAYS("Unknown metamethod type for FNextIndex function!");
        return;
      }

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
      return new Class<Type>::Factory(m_pConstructor, m_pValReference);
    };

    // Get type info for a factory
    virtual const std::type_info &GetFactoryType(void) {
      return typeid(Type);
    };

  // Metamethods
  protected:

    static SQInteger ClassConstructor(HSQUIRRELVM v);
    static SQInteger ClassPtrConstructor(HSQUIRRELVM v);
    static SQInteger ClassSet(HSQUIRRELVM v);
    static SQInteger ClassGet(HSQUIRRELVM v);

    static SQInteger ClassMetaOther(HSQUIRRELVM v); // FOther
    static SQInteger ClassMetaSelf(HSQUIRRELVM v); // FSelf
    static SQInteger ClassMetaNextIndex(HSQUIRRELVM v); // FNextIndex
};

class AbstractClassRegistrar {
  public:
    virtual const AbstractClass &GetCopyClass(void) const = 0;
    virtual const AbstractClass &GetPointerClass(void) const = 0;
};

// Registrar for a native Squirrel class of a specific type with an additional pointer type
template<class Type>
class Class : public AbstractClassRegistrar {
  public:
    typedef typename InternalClass<Type>::FConstructor FConstructor;
    typedef typename InternalClass<Type>::FSetter      FSetter;
    typedef typename InternalClass<Type>::FGetter      FGetter;

    typedef typename InternalClass<Type>::FOther     FOther;
    typedef typename InternalClass<Type>::FSelf      FSelf;
    typedef typename InternalClass<Type>::FNextIndex FNextIndex;

    // Instance factory for this class
    class Factory : public AbstractFactory {
      public:
        Factory(FConstructor pSetConstructor, Type *pSetValueReference) :
          AbstractFactory(pSetConstructor, pSetValueReference, typeid(Type))
        {
        };

        virtual InstanceAny *NewInstance(HSQUIRRELVM vmSet, SQInteger idx, SQUserPointer pData = NULL) {
          InstanceAny *pNew;

          // Create a pointer instance
          if (pData != NULL) {
            if (m_pValReference != NULL) {
              pNew = new InstancePtr<Type>(vmSet, this, (Type *)m_pValReference);
            } else {
              pNew = new InstancePtr<Type>(vmSet, this, (Type *)pData);
            }

          // Create a copy instance
          } else {
            if (m_pValReference != NULL) {
              pNew = new InstanceCopy<Type &>(vmSet, this, *(Type *)m_pValReference);
            } else {
              pNew = new InstanceCopy<Type>(vmSet, this);
            }
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
    InternalClass<Type> m_sqcCopy; // Class as is that holds a unique copy of the value
    InternalClass<Type> m_sqcPtr; // Pointer class that holds a reference to some value

  private:
    // Cannot be reassigned
    Class(const Class &other);
    void operator=(const Class &other);

  public:
    // Construct a Squirrel class declaration that creates instances with own copies of values
    Class(HSQUIRRELVM vmSet, const SQChar *strName, FConstructor pSetConstructor) :
      m_sqcCopy(vmSet, strName, false, pSetConstructor),
      m_sqcPtr(vmSet, strName, true, pSetConstructor)
    {
    };

    // Construct a Squirrel class declaration that creates instances with a reference to some external value
    Class(HSQUIRRELVM vmSet, const SQChar *strName, FConstructor pSetConstructor, Type &valSetReference) :
      m_sqcCopy(vmSet, strName, false, pSetConstructor, valSetReference),
      m_sqcPtr(vmSet, strName, true, pSetConstructor, valSetReference)
    {
    };

    // Add getter and optional setter methods for working with some variable
    inline void RegisterVar(const SQChar *strVariable, FGetter pGetter, FSetter pSetter) {
      m_sqcCopy.RegisterVar(strVariable, pGetter, pSetter);
      m_sqcPtr.RegisterVar(strVariable, pGetter, pSetter);
    };

    // Add getter and optional setter methods for working with some index
    inline void RegisterIndex(SQInteger iIndex, FGetter pGetter, FSetter pSetter) {
      m_sqcCopy.RegisterIndex(iIndex, pGetter, pSetter);
      m_sqcPtr.RegisterIndex(iIndex, pGetter, pSetter);
    };

    // Add a custom metamethod
    inline void RegisterMetamethod(EMetamethod eType, FOther pFunction) {
      m_sqcCopy.RegisterMetamethod(eType, pFunction);
      m_sqcPtr.RegisterMetamethod(eType, pFunction);
    };

    // Add a custom metamethod
    inline void RegisterMetamethod(EMetamethod eType, FSelf pFunction) {
      m_sqcCopy.RegisterMetamethod(eType, pFunction);
      m_sqcPtr.RegisterMetamethod(eType, pFunction);
    };

    // Add a custom metamethod for iteration
    inline void RegisterMetamethod(EMetamethod eType, FNextIndex pFunction) {
      m_sqcCopy.RegisterMetamethod(eType, pFunction);
      m_sqcPtr.RegisterMetamethod(eType, pFunction);
    };

    // Register function using Squirrel declaration
    inline void RegisterFunc(const SQRegFunction &regfunc) {
      m_sqcCopy.RegisterFunc(regfunc);
      m_sqcPtr.RegisterFunc(regfunc);
    };

  public:
    virtual const AbstractClass &GetCopyClass(void) const {
      return m_sqcCopy;
    };

    virtual const AbstractClass &GetPointerClass(void) const {
      return m_sqcPtr;
    };
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassConstructor(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  // Create a new instance using a factory
  AbstractFactory *pFactory = AbstractFactory::Find(v, strFactoryType);
  if (pFactory == NULL) return sq_throwerror(v, "no instance factory");

  // Create a new copy instance
  InstanceAny *pInstance = pFactory->NewInstance(v, 1);
  if (pInstance == NULL) return SQ_ERROR;

  // No custom constructor to call
  if (pFactory->m_pConstructorFunc == NULL) return 0;

  // Call custom constructor
  FConstructor pFunc = (FConstructor)pFactory->m_pConstructorFunc;
  Type *pSelf = (Type *)pFactory->m_pValReference;

  if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;

  return pFunc(v, *pSelf);
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassPtrConstructor(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  // Create a new instance using a factory
  AbstractFactory *pFactory = AbstractFactory::Find(v, strFactoryType);
  if (pFactory == NULL) return sq_throwerror(v, "no instance factory");

  // Retrieve passed data
  SQUserPointer pData;

  if (SQ_FAILED(sq_getuserpointer(v, 2, &pData)) || pData == NULL) {
    return sq_throwerror(v, "couldn't get passed data for a pointer instance");
  }

  // Create a new pointer instance
  InstanceAny *pInstance = pFactory->NewInstance(v, 1, pData);
  if (pInstance == NULL) return SQ_ERROR;

  // No need to call a constructor on a pointer instance, otherwise the value will be reset for no reason
  return 0;
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassSet(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -2, &strFactoryType);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Retrieve setter method for the variable
  const SQChar *strVariable;
  bool bIndex;
  SQUserPointer pPtrToFunc;

  if (!GetFunctionForVariable(v, &strVariable, bIndex, &pPtrToFunc)) {
    SQChar strError[256];

    if (bIndex) {
      scsprintf(strError, 256, "index '%s' is read-only or does not exist", strVariable);
    } else {
      scsprintf(strError, 256, "variable '%s' is read-only or does not exist", strVariable);
    }

    return sq_throwerror(v, strError);
  }

  // Call setter method for the data
  FSetter pFunc = (FSetter)pPtrToFunc;
  Type *pSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  if (pInstance->IsPointer()) {
    if (pSelf == NULL) pSelf = ((InstancePtr<Type> *)pInstance)->pval;
  } else {
    if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;
  }

  return pFunc(v, *pSelf, 3);
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassGet(HSQUIRRELVM v) {
  // Retrieve passed factory type
  const SQChar *strFactoryType;
  sq_getstring(v, -2, &strFactoryType);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Retrieve getter method for the variable
  const SQChar *strVariable;
  bool bIndex;
  SQUserPointer pPtrToFunc;

  if (!GetFunctionForVariable(v, &strVariable, bIndex, &pPtrToFunc)) {
    SQChar strError[256];

    if (bIndex) {
      scsprintf(strError, 256, "index '%s' cannot be read from", strVariable);
    } else {
      scsprintf(strError, 256, "variable '%s' cannot be read from", strVariable);
    }

    return sq_throwerror(v, strError);
  }

  // Call getter method for the data
  FGetter pFunc = (FGetter)pPtrToFunc;
  Type *pSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  if (pInstance->IsPointer()) {
    if (pSelf == NULL) pSelf = ((InstancePtr<Type> *)pInstance)->pval;
  } else {
    if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;
  }

  return pFunc(v, *pSelf);
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassMetaOther(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Call conversion data for the data
  FOther pFunc = (FOther)pToStringFunc;
  Type *pSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pInstance->IsPointer()) {
    if (pSelf == NULL) pSelf = ((InstancePtr<Type> *)pInstance)->pval;
  } else {
    if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;
  }

  // Pass other value as an index in the stack
  return pFunc(v, *pSelf, 2);
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassMetaSelf(HSQUIRRELVM v) {
  const SQChar *strFactoryType;
  sq_getstring(v, -1, &strFactoryType);

  SQUserPointer pToStringFunc;
  sq_getuserpointer(v, -2, &pToStringFunc);

  // Get current instance
  InstanceAny *pInstance = InstanceAny::OfType(v, 1, strFactoryType);
  if (pInstance == NULL) return SQ_ERROR;

  // Call conversion data for the data
  FSelf pFunc = (FSelf)pToStringFunc;
  Type *pSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pInstance->IsPointer()) {
    if (pSelf == NULL) pSelf = ((InstancePtr<Type> *)pInstance)->pval;
  } else {
    if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;
  }

  return pFunc(v, *pSelf);
};

template<class Type> inline
SQInteger InternalClass<Type>::ClassMetaNextIndex(HSQUIRRELVM v) {
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
  Type *pSelf = (Type *)pInstance->GetFactory()->m_pValReference;

  // Reference the instance data if there's no external reference
  if (pInstance->IsPointer()) {
    if (pSelf == NULL) pSelf = ((InstancePtr<Type> *)pInstance)->pval;
  } else {
    if (pSelf == NULL) pSelf = &((InstanceCopy<Type> *)pInstance)->val;
  }

  // Pass previous index only if it has been retrieved
  return pFunc(v, *pSelf, bPrevIndex ? &iPrev : NULL);
};

}; // namespace

#endif
