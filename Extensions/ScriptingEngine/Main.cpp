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

// Link Squirrel
#pragma comment(lib, "squirrel.lib")
#pragma comment(lib, "sqstdlib.lib")

// Define extension
CLASSICSPATCH_DEFINE_EXTENSION("PATCH_EXT_scripting", k_EPluginFlagEngine, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Scripting Engine",
  "Engine for executing scripts written in the Squirrel scripting language.");

static int PATCH_CALLTYPE ExecuteScript(void *pScriptArg);

CLASSICSPATCH_EXTENSION_SIGNALS_BEGIN {
  { "ExecuteScript", &ExecuteScript }, // Arg ptr : CTString
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// Permanent script VM
static sq::VM *_pVM = NULL;

// Check if the current script execution is suspended
static INDEX IsSuspended(void) {
  if (_pVM == NULL) return FALSE;
  return _pVM->IsSuspended();
};

// Resume a previously suspended script execution
static void ResumeVM(void) {
  // No VM or it's not suspended
  if (_pVM == NULL || !_pVM->IsSuspended()) return;

  bool bExecuted = _pVM->Execute();

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", _pVM->GetError());
  }
};

// Forcefully reset the global VM, in case it's stuck in a loop etc.
static void ResetVM(void) {
  delete _pVM;
  _pVM = NULL;
};

// Generic method for executing scripts
static BOOL ExecuteSquirrelScript(const CTString &strScript, BOOL bFile, sq::VM::FReturnValueCallback pCallback) {
  // Create a new VM
  if (_pVM == NULL) _pVM = new sq::VM(0xFFFFFFFF);

  _pVM->m_pReturnValueCallback = pCallback;

  const bool bExpectReturnValue = true; // Switch for testing
  bool bCompiled;

  if (bFile) {
    bCompiled = _pVM->CompileFromFile(strScript, bExpectReturnValue);
  } else {
    bCompiled = _pVM->CompileFromString(strScript, "Shell", bExpectReturnValue);
  }

  // Error during the compilation
  if (!bCompiled) {
    CPrintF("^cff0000Compilation error:\n%s", _pVM->GetError());
    return FALSE;
  }

  bool bExecuted = _pVM->Execute();

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", _pVM->GetError());
    return FALSE;
  }

  return TRUE;
};

static void SignalReturnCallback(sq::VM &vm) {
  // Output return value
  CTString str;

  if (vm.GetString(-1, str)) {
    CPrintF("%s\n", str.str_String);
  }
};

int ExecuteScript(void *pScript) {
  if (pScript == NULL) return FALSE;

  const CTString &strScript = *(const CTString *)pScript;
  return ExecuteSquirrelScript(strScript, FALSE, &SignalReturnCallback);
};

static CTString _strLastCommandResult;

static void CommandReturnCallback(sq::VM &vm) {
  // Save return value
  CTString str = "";
  vm.GetString(-1, str);

  // Must be a non-null value
  _strLastCommandResult = (str != "null") ? str : "";
};

// Execute a script from the provided string and return the result
static CTString ExecuteString(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(strScript, FALSE, &CommandReturnCallback);

  return _strLastCommandResult;
};

// Execute a script from the provided file and return the result
static CTString ExecuteFile(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(strScript, TRUE, &CommandReturnCallback);

  return _strLastCommandResult;
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Custom symbols
  GetPluginAPI()->RegisterMethod(TRUE, "INDEX",    "scr_IsSuspended",   "void",     &IsSuspended);
  GetPluginAPI()->RegisterMethod(TRUE, "void",     "scr_ResumeVM",      "void",     &ResumeVM);
  GetPluginAPI()->RegisterMethod(TRUE, "void",     "scr_ResetVM",       "void",     &ResetVM);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteString", "CTString", &ExecuteString);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteFile",   "CTString", &ExecuteFile);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
