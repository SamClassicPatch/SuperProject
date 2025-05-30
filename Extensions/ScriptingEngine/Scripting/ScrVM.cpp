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
static bool SqCompileSource(HSQUIRRELVM v, const CTString &strSourceFile) {
  // Open the source file using an engine stream
  CTFileStream strm;

  try {
    strm.Open_t(strSourceFile);

  } catch (char *strError) {
    sq_throwerror(v, strError);
    return false;
  }

  // Compile the script by reading characters from the stream
  SQRESULT r = sq_compile(v, SqLexerFeed, &strm, strSourceFile.str_String, SQTrue);
  strm.Close();

  return SQ_SUCCEEDED(r);
};

// Compile script from a character buffer with a given function name
static bool SqCompileBuffer(HSQUIRRELVM v, const CTString &strScript, const char *strSourceName) {
  SQRESULT r = sq_compilebuffer(v, strScript, strScript.Length(), strSourceName, SQTrue);
  return SQ_SUCCEEDED(r);
};

namespace sq {

// Get a script VM class from a Squirrel VM
__forceinline VM &GetVM(HSQUIRRELVM v) {
  return *(VM *)sq_getsharedforeignptr(v);
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
  GetVM(v).PushError(strPrint);

  va_end(arg);
};

// Compiler error output
static void HandlerCompilerError(HSQUIRRELVM v,
  const SQChar *strError, const SQChar *strSource, SQInteger iLn, SQInteger iCh)
{
  // Clear the last error
  GetVM(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    pCallback(v, _SC("%s (ln %d, ch %d) error: %s\n"), strSource, iLn, iCh, strError);
  }
};

// Runtime error output
static SQInteger HandlerRuntimeError(HSQUIRRELVM v) {
  // Clear the last error
  GetVM(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    const SQChar *strError = NULL;

    if (sq_gettop(v) >= 1) {
      if (SQ_SUCCEEDED(sq_getstring(v, 2, &strError))) {
        pCallback(v, _SC("AN ERROR HAS OCCURRED [%s]\n"), strError);
      } else {
        pCallback(v, _SC("AN ERROR HAS OCCURRED [unknown]\n"));
      }

      sqstd_printcallstack(v);
    }
  }

  return SQ_OK;
};

VM::VM(ULONG ulInitFlags) : m_bDebug(false), m_bCompiled(false), m_iStackTop(0), m_bReturnValue(false),
  m_pReturnValueCallback(NULL)
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

bool VM::Compile_internal(const CTString &strSource, const char *strSourceName, bool bReturnValue) {
  // If the VM wasn't suspended last time
  if (!IsSuspended()) {
    m_iStackTop = sq_gettop(m_vm);

    // Compile new script (pushes the entire script as a closure on top of the stack)
    if (strSourceName != NULL) {
      m_bCompiled = SqCompileBuffer(m_vm, strSource, strSourceName);
    } else {
      m_bCompiled = SqCompileSource(m_vm, strSource);
    }

    m_bReturnValue = bReturnValue;
    PrintCurrentStack(false, "After compile"); // Print the stack
  }

  return m_bCompiled;
};

// Compile script from a source file before executing it
bool VM::CompileFromFile(const CTString &strSourceFile, bool bReturnValue) {
  return Compile_internal(strSourceFile, NULL, bReturnValue);
};

// Compile script from a string before executing it
bool VM::CompileFromString(const CTString &strScript, const char *strSourceName, bool bReturnValue) {
  if (strSourceName == NULL) {
    ASSERTALWAYS("Compiling Squirrel script from a string but the source name is NULL");
    strSourceName = "";
  }

  return Compile_internal(strScript, strSourceName, bReturnValue);
};

// Temporary struct for printing out unreachable objects upon returning from a function
struct UnreachablePrint {
  VM &vm;
  bool bDebug;

  UnreachablePrint(VM *pVM, bool bSetDebug) : vm(*pVM), bDebug(bSetDebug) {};

  ~UnreachablePrint() {
    if (!bDebug) return;

    // Check amount of unreachable objects
    HSQUIRRELVM sqvm = vm.GetVM();
    SQInteger ctRefs = -1;

    // Push array of unreachable objects or null if there are none
    sq_resurrectunreachable(sqvm);

    // If it pushed an array
    if (sq_gettype(sqvm, -1) != OT_NULL) {
      // Get closure
      sq_pushstring(sqvm, "len", -1);
      sq_get(sqvm, -2);

      // Push array as the argument and call it
      sq_push(sqvm, -2);
      sq_call(sqvm, 1, SQTrue, SQTrue);
      sq_getinteger(sqvm, -1, &ctRefs);

      // Pop return value and closure
      sq_pop(sqvm, 2);
    }

    sq_poptop(sqvm); // Pop array or null
    CPrintF("[SQ] " DEBUGOUT_INFO("sq_resurrectunreachable()") " = %d (stack: %d)\n", ctRefs, sq_gettop(sqvm));
  };
};

// Execute a compiled script or resume a suspended execution
bool VM::Execute(void) {
  UnreachablePrint unreachable(this, m_bDebug);

  // Compilation failed (reuses previous error)
  if (!m_bCompiled) return false;

  // If the VM is still suspended
  if (IsSuspended()) {
    // Resume it
    SQRESULT r = sq_wakeupvm(m_vm, SQTrue, (m_bReturnValue ? SQTrue : SQFalse), SQTrue, SQFalse);
    if (SQ_FAILED(r)) return false;

  // Run a compiled script
  } else {
    // Push root table as 'this' for the script
    sq_pushroottable(m_vm);

    // Call the script closure without removing it from the stack
    // Push the return value on top (or 'null' if doesn't return anything)
    SQRESULT r = sq_call(m_vm, 1, (m_bReturnValue ? SQTrue : SQFalse), SQTrue);
    if (SQ_FAILED(r)) return false;
  }

  PrintCurrentStack(false, "After call"); // Print the stack

  // Quit if the VM is suspended
  if (IsSuspended()) {
    DebugOut(DEBUGOUT_INFO("VM is suspended") " (stack init: %d)", m_iStackTop);
    return true;
  }

  // Execute the callback for the return value
  if (m_bReturnValue) {
    if (m_bDebug) {
      CTString strReturn;

      if (GetString(-1, strReturn)) {
        CPrintF("[SQ] " DEBUGOUT_TYPE("Return value") " = '%s'\n", strReturn.str_String);
      } else {
        CPrintF("[SQ] Cannot retrieve the return value\n");
      }
    }

    if (m_pReturnValueCallback != NULL) {
      m_pReturnValueCallback(*this);
    }
  }

  // Restore the stack top by popping values in this order:
  // 1. Return value (if it's been pushed)
  // 2. Initial root table after resuming a suspended script at least once
  // 3. Original script closure after the initial compilation
  sq_settop(m_vm, m_iStackTop);
  DebugOut(DEBUGOUT_INFO("VM has finished running") " (stack init/end: %d/%d)", m_iStackTop, sq_gettop(m_vm));

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
    CPrintF(" %d\n", ct);
    return;
  }

  if (ct <= 0) {
    CPutString(" Nothing\n");
    return;
  }

  CPutString("\n");

  for (INDEX i = ct; i > 0; i--) {
    SQObjectType eType = sq_gettype(m_vm, i);
    CTString strObj = "ERROR - COULD NOT GET STRING";

    if (GetString(i, strObj)) {
      strObj = "'" + strObj + "'";
    }

    CPrintF("[SQ] [%d] " DEBUGOUT_TYPE("%s") " = %s\n", i, sq_gettypename(eType), strObj);
  }
};

}; // namespace
