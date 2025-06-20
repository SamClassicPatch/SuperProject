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

#ifndef CECIL_INCL_SQSCRIPTVM_H
#define CECIL_INCL_SQSCRIPTVM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ScrConstants.h"
#include "ScrTable.h"

namespace sq {

// Flag for initializing specific APIs for a VM
enum InitFlagsVM {
  VMLIB_NONE      = 0,

  // Squirrel STL
  VMLIB_STDBLOB   = (1 << 0), // Blob library
  VMLIB_STDIO     = (1 << 1), // IO library
  VMLIB_STDSYSTEM = (1 << 2), // System library
  VMLIB_STDMATH   = (1 << 3), // Math library
  VMLIB_STDSTRING = (1 << 4), // String library

  // Serious Engine API
  VMLIB_ENTITIES   = (1 << 10),
  VMLIB_FILESYSTEM = (1 << 11),
  VMLIB_INPUT      = (1 << 12),
  VMLIB_MESSAGE    = (1 << 13),
  VMLIB_NETWORK    = (1 << 14),
  VMLIB_SHELL      = (1 << 15),
  VMLIB_TIMER      = (1 << 16),
  VMLIB_WORLD      = (1 << 17),
};

class VM {
  public:
    // Callback for handling return values at the end of the execution
    // Should return true if the return value isn't needed anymore and can be popped from the stack
    typedef bool (*FReturnValueCallback)(VM &vm);

  private:
    HSQUIRRELVM m_vm; // Squirrel VM itself
    bool m_bDebug; // Outputs debug information in console

    CTString m_strErrors; // Error message buffer
    bool m_bRuntimeError; // Signifies that there has been at least one runtime error

  public:
    VM(ULONG ulInitFlags = VMLIB_NONE);
    ~VM();

    // Toggle debug output
    __forceinline void SetDebug(bool bState) {
      m_bDebug = bState;
    };

    // Retrieve handler to a raw Squirrel VM
    __forceinline HSQUIRRELVM GetVM(void) const {
      return m_vm;
    };

    // Cast to a raw Squirrel VM
    __forceinline operator HSQUIRRELVM() const {
      return m_vm;
    };

    // Get last VM error
    __forceinline const char *GetError(void) const {
      // No error
      if (m_strErrors == "") return "no error";

      return m_strErrors.str_String;
    };

  // Squirrel handlers
  private:

    // Push one VM error
    __forceinline void PushError(const CTString &strError) {
      m_strErrors += strError;
    };

    // Set the VM error
    __forceinline void SetError(const CTString &strError) {
      m_strErrors = strError;
    };

    // Message output
    static void HandlerPrintF(HSQUIRRELVM v, const char *str, ...);

    // Error output
    static void HandlerErrorF(HSQUIRRELVM v, const char *str, ...);

    // Compiler error output
    static void HandlerCompilerError(HSQUIRRELVM v,
      const SQChar *strError, const SQChar *strSource, SQInteger iLn, SQInteger iCh);

    // Runtime error output
    static SQInteger HandlerRuntimeError(HSQUIRRELVM v);

  // Squirrel wrappers
  public:

    // Retrieve current VM state
    __forceinline SQInteger GetState(void) const {
      return sq_getvmstate(m_vm);
    };

    // Check if the VM is currently idle
    __forceinline bool IsIdle(void) const {
      return GetState() == SQ_VMSTATE_IDLE;
    };

    // Check if the VM is in the middle of execution
    __forceinline bool IsRunning(void) const {
      return GetState() == SQ_VMSTATE_RUNNING;
    };

    // Check if the VM is currently suspended
    __forceinline bool IsSuspended(void) const {
      return GetState() == SQ_VMSTATE_SUSPENDED;
    };

  private:
    // Print debug information in console
    void DebugOut(const char *strFormat, ...);

    // Compile script from a source file within the game folder
    static void SqCompileSource(HSQUIRRELVM v, const CTString &strSourceFile);

    // Compile script from a character buffer with a given function name
    static void SqCompileBuffer(HSQUIRRELVM v, const CTString &strScript, const SQChar *strSourceName);

    __forceinline void Compile_internal(const CTString &strSource, const char *strSourceName);

  // Running
  public:

    // Compile script from a source file and push it as a closure on top of the stack
    // Pushes null instead if the compilation fails (use GetError() for more info)
    void CompileFromFile(const CTString &strSourceFile);

    // Compile script from a string and push it as a closure on top of the stack
    // Pushes null instead the compilation fails (use GetError() for more info)
    void CompileFromString(const CTString &strScript, const SQChar *strSourceName);

    // Check whether a closure on top of the stack can be executed
    bool CanBeExecuted(void);

    // Execute a closure on top of the stack or resume a suspended execution
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool Execute(FReturnValueCallback pReturnCallback);

    // Convert any object in the stack into a string
    bool GetString(SQInteger idx, CTString &strValue);

    // Display current contents of the stack in console
    void PrintCurrentStack(bool bOnlyCount = false, const char *strLabel = "Current stack");

  // Serious Engine libraries
  private:

    void RegisterEntities(void);
    void RegisterFileSystem(void);
    void RegisterInput(void);
    void RegisterMessage(void);
    void RegisterNetwork(void);
    void RegisterShell(void);
    void RegisterTimer(void);
    void RegisterWorld(void);

  // VM interactions
  public:

    // Access const table
    __forceinline ConstTable Const(void) { return ConstTable(GetVM()); };

    // Access root table
    __forceinline RootTable Root(void) { return RootTable(GetVM()); };

    // Access registry table
    __forceinline RegistryTable Registry(void) { return RegistryTable(GetVM()); };
};

// Get a script VM class from a Squirrel VM
__forceinline VM &GetVMClass(HSQUIRRELVM v) {
  SQUserPointer pVM = sq_getsharedforeignptr(v);
  ASSERT(pVM != NULL);
  return *(VM *)pVM;
};

}; // namespace

#endif
