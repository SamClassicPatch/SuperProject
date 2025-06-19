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

// Squirrel standard library
#include <Extras/squirrel3/include/sqstdblob.h>
#include <Extras/squirrel3/include/sqstdsystem.h>
#include <Extras/squirrel3/include/sqstdio.h>
#include <Extras/squirrel3/include/sqstdmath.h>
#include <Extras/squirrel3/include/sqstdstring.h>
#include <Extras/squirrel3/include/sqstdaux.h>

// Debug output color tags
#define DEBUGOUT_R "^C"
#define DEBUGOUT_INFO(_String) "^cffdf00" _String DEBUGOUT_R
#define DEBUGOUT_TYPE(_String) "^cbf9f00" _String DEBUGOUT_R

namespace sq {

// Helper function for reading one script character at a time for sq_compile()
static SQInteger SqLexerFeed(SQUserPointer pData)
{
  CTFileStream &strm = *(CTFileStream *)pData;

  // No more data to read
  if (strm.AtEOF()) return 0;

  // Read one more character from the stream
  // 'UBYTE' instead of 'char' to preserve the character from some ANSI code page
  UBYTE ch;
  strm >> ch;

  return ch;
};

// Compile script from a source file within the game folder
static void SqCompileSource(HSQUIRRELVM v, const CTString &strSourceFile) {
  // Open the source file using an engine stream
  CTFileStream strm;

  try {
    strm.Open_t(strSourceFile);

  } catch (char *strError) {
    sq_throwerror(v, strError);

    VM &vm = GetVMClass(v);
    vm.ClearError();
    vm.PushError(strError);
    vm.PushError("\n");

    // Push a null instead of a closure on error
    sq_pushnull(v);
    return;
  }

  // Compile the script by reading characters from the stream
  SQRESULT r = sq_compile(v, SqLexerFeed, &strm, strSourceFile.str_String, SQTrue);
  strm.Close();

  // Push a null instead of a closure on error
  if (SQ_FAILED(r)) sq_pushnull(v);
};

// Compile script from a character buffer with a given function name
static void SqCompileBuffer(HSQUIRRELVM v, const CTString &strScript, const SQChar *strSourceName) {
  SQRESULT r = sq_compilebuffer(v, strScript, strScript.Length(), strSourceName, SQTrue);

  // Push a null instead of a closure on error
  if (SQ_FAILED(r)) sq_pushnull(v);
};

// Message output
static void HandlerPrintF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  CPutString(strPrint);

  va_end(arg);
};

// Error output
static void HandlerErrorF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  GetVMClass(v).PushError(strPrint);

  va_end(arg);
};

// Compiler error output
static void HandlerCompilerError(HSQUIRRELVM v,
  const SQChar *strError, const SQChar *strSource, SQInteger iLn, SQInteger iCh)
{
  // Clear the last error
  GetVMClass(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    pCallback(v, "%s (ln %d, ch %d) error: %s\n", strSource, (int)iLn, (int)iCh, strError);
  }
};

// Runtime error output
static SQInteger HandlerRuntimeError(HSQUIRRELVM v) {
  // Clear the last error
  VM &vm = GetVMClass(v);
  vm.ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    const SQChar *strError = NULL;
    bool bErrorMessage = (sq_gettop(v) >= 2 && SQ_SUCCEEDED(sq_getstring(v, 2, &strError)));

    // Print the last error as a simple string to avoid cascading output
    if (vm.RuntimeErrorOccurred()) {
      if (bErrorMessage) {
        pCallback(v, "%s", strError);
      } else {
        pCallback(v, "unknown\n");
      }

    // Print detailed infomation about the error
    } else {
      if (bErrorMessage) {
        pCallback(v, "AN ERROR HAS OCCURRED [%s]\n", strError);
      } else {
        pCallback(v, "AN ERROR HAS OCCURRED [unknown]\n");
      }

      sqstd_printcallstack(v);
    }
  }

  // Runtime error has occurred
  vm.MarkRuntimeError();
  return SQ_OK;
};

VM::VM(ULONG ulInitFlags) : m_bDebug(false), m_bRuntimeError(false)
{
  // Create a new VM and bind this wrapper class to it
  m_vm = sq_open(1024);
  sq_setsharedforeignptr(m_vm, this);

  // Register things in the root table
  sq_pushroottable(m_vm);
  {
    // Register standard libraries
    if (ulInitFlags & VMLIB_STDBLOB)   sqstd_register_bloblib(m_vm);
    if (ulInitFlags & VMLIB_STDIO)     sqstd_register_iolib(m_vm);
    if (ulInitFlags & VMLIB_STDSYSTEM) sqstd_register_systemlib(m_vm);
    if (ulInitFlags & VMLIB_STDMATH)   sqstd_register_mathlib(m_vm);
    if (ulInitFlags & VMLIB_STDSTRING) sqstd_register_stringlib(m_vm);

    // Register engine libraries
    if (ulInitFlags & VMLIB_ENTITIES)   RegisterEntities();
    if (ulInitFlags & VMLIB_FILESYSTEM) RegisterFileSystem();
    if (ulInitFlags & VMLIB_INPUT)      RegisterInput();
    if (ulInitFlags & VMLIB_MESSAGE)    RegisterMessage();
    if (ulInitFlags & VMLIB_NETWORK)    RegisterNetwork();
    if (ulInitFlags & VMLIB_SHELL)      RegisterShell();
    if (ulInitFlags & VMLIB_TIMER)      RegisterTimer();
    if (ulInitFlags & VMLIB_WORLD)      RegisterWorld();

    // Set handler functions
    sq_setprintfunc(m_vm, &HandlerPrintF, &HandlerErrorF);
    sq_setcompilererrorhandler(m_vm, &HandlerCompilerError);

    sq_newclosure(m_vm, &HandlerRuntimeError, 0);
    sq_seterrorhandler(m_vm);
  }
  sq_poptop(m_vm);
};

VM::~VM() {
  sq_close(m_vm);
};

// Print debug information in console
void VM::DebugOut(const char *strFormat, ...) {
  if (!m_bDebug) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString strPrint;
  strPrint.VPrintF(strFormat, arg);
  CPutString("[SQ] " + strPrint + "\n");

  va_end(arg);
};

void VM::Compile_internal(const CTString &strSource, const SQChar *strSourceName) {
  // Compile new script (pushes the entire script as a closure on top of the stack)
  if (strSourceName != NULL) {
    SqCompileBuffer(m_vm, strSource, strSourceName);
  } else {
    SqCompileSource(m_vm, strSource);
  }

  PrintCurrentStack(false, "After compile"); // Print the stack
};

// Compile script from a source file and push it as a function on top of the stack
void VM::CompileFromFile(const CTString &strSourceFile) {
  Compile_internal(strSourceFile, NULL);
};

// Compile script from a string and push it as a function on top of the stack
void VM::CompileFromString(const CTString &strScript, const SQChar *strSourceName) {
  if (strSourceName == NULL) {
    ASSERTALWAYS("Compiling Squirrel script from a string but the source name is NULL");
    strSourceName = "";
  }

  Compile_internal(strScript, strSourceName);
};

// Temporary struct for printing out unreachable objects upon returning from a function
struct UnreachablePrint {
  HSQUIRRELVM vm;
  bool bDebug;

  UnreachablePrint(HSQUIRRELVM vmSet, bool bSetDebug) : vm(vmSet), bDebug(bSetDebug) {};

  ~UnreachablePrint() {
    if (!bDebug) return;

    // Check amount of unreachable objects
    SQInteger ctRefs = -1;

    // Push array of unreachable objects or null if there are none
    if (SQ_FAILED(sq_resurrectunreachable(vm))) {
      CPrintF("[SQ] " DEBUGOUT_INFO("sq_resurrectunreachable()") " error\n");
      return;
    }

    // If it pushed an array
    if (sq_gettype(vm, -1) == OT_ARRAY) {
      // Get closure
      sq_pushstring(vm, "len", -1);
      sq_get(vm, -2);

      // Push array as the argument and call it
      sq_push(vm, -2);
      sq_call(vm, 1, SQTrue, SQTrue);
      sq_getinteger(vm, -1, &ctRefs);

      // Pop return value and closure
      sq_pop(vm, 2);
    }

    sq_poptop(vm); // Pop array or null
    CPrintF("[SQ] " DEBUGOUT_INFO("sq_resurrectunreachable()") " = %d (stack: %d)\n", (int)ctRefs, (int)sq_gettop(vm));
  };
};

// Check whether a closure on top of the stack can be executed
bool VM::CanBeExecuted(void) {
  // Nothing in the stack
  if (sq_gettop(m_vm) <= 0) return false;

  // Needs a closure on top
  SQObjectType eType = sq_gettype(m_vm, -1);
  return (eType == OT_CLOSURE || eType == OT_NATIVECLOSURE);
};

// Execute a function on top of the stack or resume a suspended execution
bool VM::Execute(FReturnValueCallback pReturnCallback) {
  UnreachablePrint unreachable(m_vm, m_bDebug);

  // Clear runtime error
  m_bRuntimeError = false;

  bool bWasSuspended = IsSuspended();
  bool bError = true; // Not executed by default

  // Resume a suspended VM
  if (bWasSuspended) {
    // Resume it
    SQRESULT r = sq_wakeupvm(m_vm, SQTrue, SQTrue, SQTrue, SQFalse);
    bError = SQ_FAILED(r);

  // Execute a compiled closure
  } else if (CanBeExecuted()) {
    // Push root table as 'this' for the script
    sq_pushroottable(m_vm);

    // Call the script closure without removing it from the stack
    // Push the return value on top (or 'null' if doesn't return anything)
    SQRESULT r = sq_call(m_vm, 1, SQTrue, SQTrue);
    bError = SQ_FAILED(r);
  }

  PrintCurrentStack(false, "After call"); // Print the stack

  // Pop executed script closure and leave on error
  if (bError) {
    sq_poptop(m_vm);
    return false;
  }

  // Quit if the VM hasn't finished its execution
  if (IsSuspended()) {
    DebugOut(DEBUGOUT_INFO("VM is suspended"));
    return true;
  }

  // The initial table must be popped here because sq_call() doesn't pop its arguments after being suspended
  if (bWasSuspended) sq_remove(m_vm, -2); // Pop initial root table argument
  sq_remove(m_vm, -2); // Pop executed script closure

  // Handle the return value
  SQInteger iStack = sq_gettop(m_vm);
  ASSERT(iStack > 0);

  if (m_bDebug) {
    CTString strReturn;

    if (GetString(-1, strReturn)) {
      CPrintF("[SQ] " DEBUGOUT_TYPE("Return value") " = '%s'\n", strReturn.str_String);
    } else {
      CPrintF("[SQ] Cannot retrieve the return value\n");
    }
  }

  // Execute callback with the return value on top of the stack
  bool bPopReturn = true;

  if (pReturnCallback != NULL) {
    bPopReturn = pReturnCallback(*this);
  }

  // Restore stack top minus the return value
  if (bPopReturn) sq_settop(m_vm, iStack - 1);

  DebugOut(DEBUGOUT_INFO("VM has finished running") " (stack: %d)", (int)sq_gettop(m_vm));
  return true;
};

// Convert any object in the stack into a string
bool VM::GetString(SQInteger idx, CTString &strValue) {
  SQRESULT r;
  const SQChar *str;

  // Cast into a string and retrieve it
  r = sq_tostring(m_vm, idx);
  if (SQ_FAILED(r)) return false;

  r = sq_getstring(m_vm, -1, &str);
  if (SQ_FAILED(r)) return false;

  strValue = str;

  // Pop the string
  sq_poptop(m_vm);
  return true;
};

// Display current contents of the stack in console
void VM::PrintCurrentStack(bool bOnlyCount, const char *strLabel) {
  if (!m_bDebug) return;

  CPrintF("[SQ] " DEBUGOUT_INFO("[%s]:"), strLabel);
  SQInteger ct = sq_gettop(m_vm);

  if (bOnlyCount) {
    CPrintF(" %d\n", (int)ct);
    return;
  }

  if (ct <= 0) {
    CPutString(" Nothing\n");
    return;
  }

  CPutString("\n");

  for (SQInteger i = ct; i > 0; i--) {
    SQObjectType eType = sq_gettype(m_vm, i);
    CTString strObj = "ERROR - COULD NOT GET STRING";

    if (GetString(i, strObj)) {
      strObj = "'" + strObj + "'";
    }

    CPrintF("[SQ] [%d] " DEBUGOUT_TYPE("%s") " = %s\n", (int)i, sq_gettypename(eType), strObj);
  }
};

}; // namespace
