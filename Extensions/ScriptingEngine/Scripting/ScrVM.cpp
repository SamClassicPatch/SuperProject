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

// Squirrel standard library
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
bool VM::SqCompileSource(HSQUIRRELVM v, const CTString &strSourceFile) {
  VM &vm = GetVMClass(v);

  // Open the source file using an engine stream
  CTFileStream strm;

  try {
    // Compile the script by reading characters from the stream
    strm.Open_t(strSourceFile);
    SQRESULT r = sq_compile(v, SqLexerFeed, &strm, strSourceFile.str_String, SQTrue);
    strm.Close();

    // Pass compilation error
    if (SQ_FAILED(r)) {
      sq_throwerror(v, vm.GetError());
      return false;
    }

  } catch (char *strError) {
    vm.SetError(strError);
    return false;
  }

  return true;
};

// Compile script from a character buffer with a given function name
bool VM::SqCompileBuffer(HSQUIRRELVM v, const CTString &strScript, const SQChar *strSourceName) {
  VM &vm = GetVMClass(v);

  SQRESULT r = sq_compilebuffer(v, strScript, strScript.Length(), strSourceName, SQTrue);

  // Pass compilation error
  if (SQ_FAILED(r)) {
    sq_throwerror(v, vm.GetError());
    return false;
  }

  return true;
};

// Message output
void VM::HandlerPrintF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  CPutString(strPrint);

  va_end(arg);
};

// Error output
void VM::HandlerErrorF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  GetVMClass(v).PushError(strPrint);

  va_end(arg);
};

// Compiler error output
void VM::HandlerCompilerError(HSQUIRRELVM v,
  const SQChar *strError, const SQChar *strSource, SQInteger iLn, SQInteger iCh)
{
  // Clear the last error to allow new ones to be pushed
  GetVMClass(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    pCallback(v, "%s (ln %d, ch %d) error: %s", strSource, (int)iLn, (int)iCh, strError);
  }
};

// Runtime error output
SQInteger VM::HandlerRuntimeError(HSQUIRRELVM v) {
  // Clear the last error to allow new ones to be pushed
  VM &vm = GetVMClass(v);
  vm.ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    const SQChar *strError = NULL;
    bool bErrorMessage = (sq_gettop(v) >= 2 && SQ_SUCCEEDED(sq_getstring(v, 2, &strError)));

    // Print the last error as a simple string to avoid cascading output
    if (vm.m_bRuntimeError) {
      if (bErrorMessage) {
        pCallback(v, "%s", strError);
      } else {
        pCallback(v, "unknown");
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
  vm.m_bRuntimeError = true;
  return SQ_OK;
};

VM::VM(bool bRegisterEngineInterfaces) : m_bDebug(false), m_bRuntimeError(false), m_iScriptDepth(0), m_ctExecutionArgs(0)
{
  // Create a new VM and bind this wrapper class to it
  m_vm = sq_open(1024);
  sq_setsharedforeignptr(m_vm, this);

  // Register standard libraries in the root table and set handler functions
  sq_pushroottable(m_vm);

  sqstd_register_mathlib(m_vm);
  sqstd_register_stringlib(m_vm);

  sq_setprintfunc(m_vm, &HandlerPrintF, &HandlerErrorF);
  sq_setcompilererrorhandler(m_vm, &HandlerCompilerError);
  sq_newclosure(m_vm, &HandlerRuntimeError, 0);
  sq_seterrorhandler(m_vm);

  sq_poptop(m_vm);

  // Register built-in libraries
  if (bRegisterEngineInterfaces) {
    RegisterUtils(); // Goes first for all the base classes
    RegisterEntities();
    RegisterFileSystem();
    RegisterInput();
    RegisterMath();
    RegisterMessage();
    RegisterNetwork();
    RegisterShell();
    RegisterTimer();
    RegisterWorld();
  }
};

VM::~VM() {
  sq_close(m_vm);
};

VM::UnreachablePrint::~UnreachablePrint() {
  if (!bDebug) return;

  // Check amount of unreachable objects
  SQInteger ctRefs = -1;

  // Push array of unreachable objects or null if there are none
  if (SQ_FAILED(sq_resurrectunreachable(vm))) {
    vm.DebugOut(DEBUGOUT_INFO("sq_resurrectunreachable()") " error");
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
  vm.DebugOut(DEBUGOUT_INFO("sq_resurrectunreachable()") " = %d (stack: %d)", (int)ctRefs, (int)sq_gettop(vm));
};

// Print debug information in console
void VM::DebugOut(const char *strFormat, ...) {
  if (!m_bDebug) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString strPrint;
  strPrint.VPrintF(strFormat, arg);

  if (GetName() != "") {
    CPutString("[" + GetName() + "] " + strPrint + "\n");
  } else {
    CPutString("[SQ] " + strPrint + "\n");
  }

  va_end(arg);
};

bool VM::Compile_internal(const CTString &strSource, const SQChar *strSourceName) {
  // Compile new script (pushes the entire script as a closure on top of the stack)
  if (strSourceName != NULL) {
    if (!SqCompileBuffer(m_vm, strSource, strSourceName)) return false;
  } else {
    if (!SqCompileSource(m_vm, strSource)) return false;
  }

  PrintCurrentStack(false, "After compile"); // Print the stack
  return true;
};

// Compile script from a source file and push it as a closure on top of the stack
bool VM::CompileFromFile(const CTString &strSourceFile) {
  return Compile_internal(strSourceFile, NULL);
};

// Compile script from a string and push it as a closure on top of the stack
bool VM::CompileFromString(const CTString &strScript, const SQChar *strSourceName) {
  if (strSourceName == NULL) {
    ASSERTALWAYS("Compiling Squirrel script from a string but the source name is NULL");
    strSourceName = "";
  }

  return Compile_internal(strScript, strSourceName);
};

// Check whether a closure on top of the stack can be executed
bool VM::CanBeExecuted(SQInteger idx) {
  // Nothing in the stack
  if (sq_gettop(m_vm) <= 0) return false;

  // Needs a closure on top
  SQObjectType eType = sq_gettype(m_vm, idx);
  return (eType == OT_CLOSURE || eType == OT_NATIVECLOSURE);
};

bool VM::AfterExecution(bool bWasSuspended, FReturnValueCallback pReturnCallback) {
  // Apply cached functions after any VM execution
  if (m_bStopDemoRec) {
    _pNetwork->StopDemoRec();
  }

  if (m_strStartDemoRec != "") {
    // Stop last recording before a new one, if it's still going
    _pNetwork->StopDemoRec();

    try {
      _pNetwork->StartDemoRec_t(m_strStartDemoRec);
    } catch (char *strError) {
      // Don't throw script error
      CPrintF(LOCALIZE("Cannot start recording: %s\n"), strError);
    }
  }

  ClearCache();

  // Quit if the VM hasn't finished its execution
  if (IsSuspended()) {
    DebugOut(DEBUGOUT_INFO("VM is suspended"));
    return true;
  }

  // The initial arguments must be popped here because sq_call() doesn't pop its arguments after being suspended
  if (bWasSuspended) {
    for (int iPop = 0; iPop < m_ctExecutionArgs; iPop++) {
      sq_remove(m_vm, -2);
    }
  }

  m_ctExecutionArgs = 0;
  sq_remove(m_vm, -2); // Pop executed script closure

  // Handle the return value
  SQInteger iStack = sq_gettop(m_vm);
  ASSERT(iStack > 0);

  if (m_bDebug) {
    CTString strReturn;

    if (GetString(-1, strReturn)) {
      DebugOut(DEBUGOUT_TYPE("Return value") " = '%s'", strReturn.str_String);
    } else {
      DebugOut("Cannot retrieve the return value");
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

// Execute a closure on top of the stack with optional arguments pushed by the specified callback
bool VM::Execute(FPushArguments pPushArgs, FReturnValueCallback pReturnCallback) {
  UnreachablePrint unreachable(this, m_bDebug);
  ClearCache();

  // Clear runtime error
  m_bRuntimeError = false;

  if (IsSuspended()) {
    SetError("trying to call a new closure during suspended execution");
    sq_poptop(m_vm); // Pop whatever was supposed to be executed
    return false;

  } else if (!CanBeExecuted(-1)) {
    SetError("no closure in the stack that can be called");
    sq_poptop(m_vm); // Pop whatever was supposed to be executed
    return false;
  }

  // Execute a compiled closure
  m_ctExecutionArgs = 0;
  const SQInteger iTop = sq_gettop(m_vm);

  // Push arguments for the closure
  if (pPushArgs != NULL) {
    // If couldn't push some arguments
    if (SQ_FAILED(pPushArgs(*this))) {
      // Pop whatever arguments have been pushed
      const SQInteger ctPushed = sq_gettop(m_vm) - iTop;
      sq_pop(m_vm, ctPushed + 1); // +1 for the script closure
      return false;
    }

  } else {
    // Push root table as 'this' for the script
    sq_pushroottable(m_vm);
  }

  // Get amount of pushed arguments
  const SQInteger ctArgs = sq_gettop(m_vm) - iTop;

  // Call the script closure without removing it from the stack
  // Push the return value on top (or 'null' if doesn't return anything)
  SQRESULT r = sq_call(m_vm, ctArgs, SQTrue, SQTrue);
  bool bError = SQ_FAILED(r);

  PrintCurrentStack(false, "After execute"); // Print the stack

  if (bError) {
    sq_poptop(m_vm); // Pop executed script closure
    return false;
  }

  // Remember amount of extra arguments after suspension
  if (IsSuspended()) m_ctExecutionArgs = ctArgs;

  return AfterExecution(false, pReturnCallback);
};

// Resume a suspended execution
bool VM::Resume(FReturnValueCallback pReturnCallback) {
  UnreachablePrint unreachable(this, m_bDebug);
  ClearCache();

  // Not executed by default
  bool bError = true;

  if (!IsSuspended()) {
    SetError("trying to resume execution that isn't suspended");

  // Resume a suspended VM
  } else {
    // Resume it
    SQRESULT r = sq_wakeupvm(m_vm, SQTrue, SQTrue, SQTrue, SQFalse);
    bError = SQ_FAILED(r);
  }

  PrintCurrentStack(false, "After resume"); // Print the stack

  // Pop executed script closure and leave on error
  if (bError) {
    sq_poptop(m_vm);
    return false;
  }

  return AfterExecution(true, pReturnCallback);
};

// Execute a nested script from a file
bool VM::ExecuteFile(const CTString &strSourceFile, FReturnValueCallback pReturnCallback) {
  // Nested too deep
  if (m_iScriptDepth > 16) {
    SetError("too many nested scripts");
    return false;
  }

  // Compile a script from a file and execute it in place
  if (!CompileFromFile(strSourceFile)) return false;

  bool bExecuted = true;
  m_iScriptDepth++;

  if (!Execute(NULL, pReturnCallback)) {
    // Pass the error from the included script
    sq_throwerror(m_vm, GetError());
    bExecuted = false;
  }

  m_iScriptDepth--;
  return bExecuted;
};

// Execute a nested script from a string
bool VM::ExecuteString(const CTString &strScript, const SQChar *strSourceName, FReturnValueCallback pReturnCallback) {
  // Nested too deep
  if (m_iScriptDepth > 16) {
    SetError("too many nested scripts");
    return false;
  }

  // Compile a script from a string and execute it in place
  if (!CompileFromString(strScript, strSourceName)) return false;

  bool bExecuted = true;
  m_iScriptDepth++;

  if (!Execute(NULL, pReturnCallback)) {
    // Pass the error from the included script
    sq_throwerror(m_vm, GetError());
    bExecuted = false;
  }

  m_iScriptDepth--;
  return bExecuted;
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

  SQInteger ct = sq_gettop(m_vm);

  if (bOnlyCount) {
    DebugOut(DEBUGOUT_INFO("[%s]:") " %d", strLabel, (int)ct);
    return;
  }

  if (ct <= 0) {
    DebugOut(DEBUGOUT_INFO("[%s]:") " Nothing", strLabel);
    return;
  }

  DebugOut(DEBUGOUT_INFO("[%s]:"), strLabel);

  for (SQInteger i = ct; i > 0; i--) {
    SQObjectType eType = sq_gettype(m_vm, i);
    CTString strObj = "ERROR - COULD NOT GET STRING";

    if (GetString(i, strObj)) {
      strObj = "'" + strObj + "'";
    }

    int iFromTop = i - ct - 1;
    int iFromBtm = i;
    DebugOut("[%d : %d] " DEBUGOUT_TYPE("%s") " = %s", iFromTop, iFromBtm, sq_gettypename(eType), strObj);
  }
};

// Clear cached variables to prevent them from executing
void VM::ClearCache(void) {
  m_strStartDemoRec = "";
  m_bStopDemoRec = false;
};

// Start recording a new demo after executing a script
void VM::StartDemoRec(const CTString &strDemoFile) {
  m_strStartDemoRec = strDemoFile;
  m_bStopDemoRec = false;
};

// Stop recording a demo after executing a script
void VM::StopDemoRec(void) {
  m_strStartDemoRec = "";
  m_bStopDemoRec = true;
};

}; // namespace
