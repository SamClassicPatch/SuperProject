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

#ifndef CECIL_INCL_SQSCRIPTVM_H
#define CECIL_INCL_SQSCRIPTVM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Core/Networking/NetworkFunctions.h>

#include "ScrConstants.h"
#include "ScrTable.h"

namespace sq {

class VM {
  public:
    // Callback for pushing necessary arguments for closures before executing them
    typedef SQRESULT (*FPushArguments)(VM &vm);

    // Callback for handling return values at the end of the execution
    // Should return true if the return value isn't needed anymore and can be popped from the stack
    typedef bool (*FReturnValueCallback)(VM &vm);

  private:
    HSQUIRRELVM m_vm; // Squirrel VM itself
    CTString m_strName; // Display name of the environment for identification purposes
    bool m_bDebug; // Outputs debug information in console

    CTString m_strErrors; // Error message buffer
    bool m_bRuntimeError; // Signifies that there has been at least one runtime error
    INDEX m_iScriptDepth; // Included scripts depth
    int m_ctExecutionArgs; // Amount of arguments passed into Execute() that need to be popped after finishing a suspended execution

    // Cache for certain functions that need to be executed after the VM finishes running
    CTString m_strStartDemoRec; // Demo file for starting the recording or empty string to ignore
    bool m_bStopDemoRec; // Whether to stop recording a demo

  public:
    VM(bool bRegisterEngineInterfaces);
    ~VM();

    // Set VM environment name
    __forceinline void SetName(const CTString &strName) { m_strName = strName; };

    // Get VM environment name
    __forceinline const CTString &GetName(void) const { return m_strName; };

    // Toggle debug output
    __forceinline void SetDebug(bool bState) { m_bDebug = bState; };

    // Retrieve handler to a raw Squirrel VM
    __forceinline HSQUIRRELVM GetVM(void) const { return m_vm; };

    // Cast to a raw Squirrel VM
    __forceinline operator HSQUIRRELVM() const { return m_vm; };

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
      sq_throwerror(m_vm, GetError()); // Pass the error into the VM
    };

    // Clear the VM error
    __forceinline void ClearError(void) {
      m_strErrors = "";
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
    // Temporary struct for printing out unreachable objects upon returning from a function
    struct UnreachablePrint {
      VM &vm;
      bool bDebug;

      UnreachablePrint(VM *pvmSet, bool bSetDebug) : vm(*pvmSet), bDebug(bSetDebug) {};
      ~UnreachablePrint();
    };

    friend UnreachablePrint;

    // Print debug information in console
    void DebugOut(const char *strFormat, ...);

    // Compile script from a source file within the game folder
    static bool SqCompileSource(HSQUIRRELVM v, const CTString &strSourceFile);

    // Compile script from a character buffer with a given function name
    static bool SqCompileBuffer(HSQUIRRELVM v, const CTString &strScript, const SQChar *strSourceName);

    __forceinline bool Compile_internal(const CTString &strSource, const char *strSourceName);
    __forceinline bool AfterExecution(bool bWasSuspended, FReturnValueCallback pReturnCallback);

  // Running
  public:

    // Compile script from a source file and push it as a closure on top of the stack
    // Returns false if the compilation fails (use GetError() for more info)
    bool CompileFromFile(const CTString &strSourceFile);

    // Compile script from a string and push it as a closure on top of the stack
    // Returns false if the compilation fails (use GetError() for more info)
    bool CompileFromString(const CTString &strScript, const SQChar *strSourceName);

    // Check whether a closure in the stack can be executed
    bool CanBeExecuted(SQInteger idx);

    // Execute a closure on top of the stack with optional arguments pushed by the specified callback
    // If the callback for pushing arguments is unspecified (NULL), it pushes the root table as the sole argument ('this') by default
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool Execute(FPushArguments pPushArgs, FReturnValueCallback pReturnCallback);

    // Resume a suspended execution
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool Resume(FReturnValueCallback pReturnCallback);

    // Execute a nested script from a file
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool ExecuteFile(const CTString &strSourceFile, FReturnValueCallback pReturnCallback);

    // Execute a nested script from a string
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool ExecuteString(const CTString &strScript, const SQChar *strSourceName, FReturnValueCallback pReturnCallback);

    // Convert any object in the stack into a string
    bool GetString(SQInteger idx, CTString &strValue);

    // Retrieve the last value from some array in the stack
    template<class Type>
    bool ArrayPopLast(SQInteger idx, Type *pVal) {
      // Remove the last array element and push it
      if (SQ_FAILED(sq_arraypop(m_vm, idx, SQTrue))) return false;

      // Retrieve value into the supplied pointer
      Value<Type> val;
      bool bResult = val.Get(m_vm, -1);

      if (bResult) *pVal = val.val;

      sq_poptop(m_vm); // Remove the element
      return bResult;
    };

    // Retrieve the first value from some array in the stack
    template<class Type>
    bool ArrayPopFirst(SQInteger idx, Type *pVal) {
      // Reverse the array to pop from the other side and then reverse it again after popping the value
      if (SQ_FAILED(sq_arrayreverse(m_vm, idx))) return false;
      bool bResult = ArrayPopLast(idx, pVal);
      if (SQ_FAILED(sq_arrayreverse(m_vm, idx))) return false;

      return bResult;
    };

    // Display current contents of the stack in console
    void PrintCurrentStack(bool bOnlyCount = false, const char *strLabel = "Current stack");

  // Caching
  public:

    // Clear cached variables to prevent them from executing
    void ClearCache(void);

    // Start recording a new demo after executing a script
    void StartDemoRec(const CTString &strDemoFile);

    // Stop recording a demo after executing a script
    void StopDemoRec(void);

  // Built-in interfaces
  private:

    void RegisterCore(void);
    void RegisterEntities(void);
    void RegisterFileSystem(void);
    void RegisterInput(void);
    void RegisterMath(void);
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

// Macro for marking specific Squirrel functions as restricted for clients that are currently playing on a server
// i.e. not a host, playing online and with any local players that can be controlled (instead of observing others)
#define SQ_RESTRICT(_SqVM) { \
  if (INetwork::IsPlayingOnRemoteServer()) return sq_throwerror((_SqVM), "the usage of this function is restricted for server players"); \
}

}; // namespace

#endif
