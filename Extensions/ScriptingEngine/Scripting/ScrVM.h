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
};

class VM {
  private:
    HSQUIRRELVM m_vm; // Squirrel VM itself
    bool m_bDebug; // Outputs debug information in console
    CTString m_strErrors; // Error message buffer

    // Compilation stage
    bool m_bCompiled; // Whether the VM has anything to execute
    INDEX m_iStackTop; // Amount of elements in the VM's stack before another compilation
    bool m_bReturnValue; // Whether the compiled script should push a return value after execution

  public:
    typedef void (*FReturnValueCallback)(VM &vm);
    FReturnValueCallback m_pReturnValueCallback;

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

    // Get last VM error
    __forceinline const char *GetError(void) const {
      // No error
      if (m_strErrors == "") return "no error\n";

      return m_strErrors.str_String;
    };

    // Push one VM error
    __forceinline void PushError(const CTString &strError) {
      m_strErrors += strError;
    };

    // Clear the VM error
    __forceinline void ClearError(void) {
      m_strErrors = "";
    };

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

    __forceinline bool Compile_internal(const CTString &strSource, const char *strSourceName, bool bReturnValue);

  // Running
  public:

    // Compile script from a source file before executing it
    // Returns false if the compilation fails (use GetError() for more info)
    bool CompileFromFile(const CTString &strSourceFile, bool bReturnValue);

    // Compile script from a string before executing it
    // Returns false if the compilation fails (use GetError() for more info)
    bool CompileFromString(const CTString &strScript, const char *strSourceName, bool bReturnValue);

    // Execute a compiled script or resume a suspended execution
    // Returns false if a runtime error occurs (use GetError() for more info)
    bool Execute(void);

    // Convert any object in the stack into a string
    bool GetString(SQInteger idx, CTString &strValue);

    // Display current contents of the stack in console
    void PrintCurrentStack(bool bOnlyCount = false, const char *strLabel = "Current stack");
};

}; // namespace

#endif
