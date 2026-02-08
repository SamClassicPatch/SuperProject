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

#ifndef CECIL_INCL_SQINSTANCES_H
#define CECIL_INCL_SQINSTANCES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <typeinfo>

namespace sq {

class AbstractFactory;

// Abstract instance data holder
class InstanceAny {
  private:
    HSQUIRRELVM m_vm; // Squirrel VM this instance belongs to
    HSQOBJECT m_obj;  // Reference to the Squirrel instance

    AbstractFactory *m_pFactory; // Factory that created this instance for reference

  public:
    CListNode m_lnOfClass; // Node in a list of instances of the respective factory

  protected:
    // Constructor for inherited instances of a specific type
    InstanceAny(HSQUIRRELVM vmSet, AbstractFactory *pFactory) : m_vm(vmSet) {
      sq_resetobject(&m_obj);
      m_pFactory = pFactory;
    };

    // Release hook function for Squirrel user data
    static SQInteger CleanupHook(SQUserPointer pUserData, SQInteger iSize) {
      (void)iSize;

      InstanceAny *pInstance = (InstanceAny *)pUserData;
      delete pInstance; // Virtual destructor should handle everything

      return SQ_OK;
    };

  public:
    // Destructor
    virtual ~InstanceAny() {};

    // Get instance's VM
    __forceinline HSQUIRRELVM GetVM(void) const { return m_vm; };

    // Get instance handler
    __forceinline HSQOBJECT GetObj(void) const { return m_obj; };

    // Get instance factory
    __forceinline AbstractFactory *GetFactory(void) const { return m_pFactory; };

    // Setup this class from an instance in the stack
    bool Setup(SQInteger idx);

  // Public interface
  public:

    // Get instance of a some native class from the stack with an optional factory matching
    static InstanceAny *Retrieve(HSQUIRRELVM v, SQInteger idx, const AbstractFactory *pFactory = NULL);

    // Get instance of a some native class from the stack by finding and matching the factory using its type
    // This is a helper method strictly for metamethods of a specific class
    static InstanceAny *OfType(HSQUIRRELVM v, SQInteger idx, const SQChar *strFactoryType);

    // Shortcut for retrieving an instance of a specific class type
    #define InstanceOfType(v, idx, Type) \
      ((Instance< Type > *)InstanceAny::OfType(v, idx, typeid(Type).raw_name()))
};

// Class instance that holds a value of a specific type
// The value may be a pointer/reference for holding references to existing values instead
template<class Type>
class Instance : public InstanceAny {
  public:
    Type val; // Instantiated value

    // Constructor for instances with a default value
    Instance(HSQUIRRELVM vmSet, AbstractFactory *pFactory) : InstanceAny(vmSet, pFactory), val()
    {
    };

    // Constructor for instances with an existing value
    Instance(HSQUIRRELVM vmSet, AbstractFactory *pFactory, Type valSet) : InstanceAny(vmSet, pFactory), val(valSet)
    {
    };
};

// Abstract instance factory that keeps track of instances created by a specific class
// Always stored in the registry table of a Squirrel VM
class AbstractFactory {
  public:
    void *m_pConstructorFunc; // Pointer to optional constructor function
    void *m_pValReference; // Pointer to optional data for referencing existing values

    const std::type_info &m_tiType; // Which class type this factory refers to
    CListHead m_lhInstances; // List of dynamically created class instances

  protected:
    // Constructor for inherited classes of specific types
    AbstractFactory(void *pSetConstructor, void *pSetValueReference, const std::type_info &tiSetType) :
      m_pConstructorFunc(pSetConstructor), m_pValReference(pSetValueReference), m_tiType(tiSetType)
    {
    };

  public:
    // Destructor
    virtual ~AbstractFactory() {
      // Destroy created instances
      FORDELETELIST(InstanceAny, m_lnOfClass, m_lhInstances, itinst) {
        delete &itinst.Current();
      }
    };

    // Release hook function for Squirrel user data
    static SQInteger CleanupHook(SQUserPointer pUserData, SQInteger iSize) {
      (void)iSize;

      AbstractFactory *pFactory = *(AbstractFactory **)pUserData;
      delete pFactory; // Virtual destructor should handle everything

      return SQ_OK;
    };

  // Public interface
  public:

    // Create a class instance for holding specific data from an instance in the stack
    // Throws Squirrel error and returns NULL on error
    virtual InstanceAny *NewInstance(HSQUIRRELVM vmSet, SQInteger idx) = 0;

    // Find a factory by its type in the VM's registry
    // This is a helper method strictly for metamethods of a specific class
    static AbstractFactory *Find(HSQUIRRELVM v, const SQChar *strFactoryType);

    // Retrieve pointer to a factory from the stack
    static inline AbstractFactory *GetFromStack(HSQUIRRELVM v, SQInteger idx) {
      SQUserPointer pUserData = NULL;
      sq_getuserdata(v, idx, &pUserData, NULL);
      return *((AbstractFactory **)pUserData);
    };
};

// Setup this class from an instance in the stack
inline bool InstanceAny::Setup(SQInteger idx) {
  // Set user pointer to this physical instance
  if (SQ_FAILED(sq_setinstanceup(m_vm, idx, this))) return false;

  // Set cleanup hook for deleting dynamically allocated instances
  sq_setreleasehook(m_vm, idx, &CleanupHook);
  sq_getstackobj(m_vm, idx, &m_obj);

  // Add to the list of instances of the factory
  m_pFactory->m_lhInstances.AddTail(m_lnOfClass);
  return true;
};

}; // namespace

#endif
