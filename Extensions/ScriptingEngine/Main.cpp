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

static int PATCH_CALLTYPE SignalExecuteScript(void *pScriptArg);
static int PATCH_CALLTYPE SignalIsSuspended(void *);
static int PATCH_CALLTYPE SignalResume(void *);
static int PATCH_CALLTYPE SignalReset(void *);

CLASSICSPATCH_EXTENSION_SIGNALS_BEGIN {
  { "ExecuteScript", &SignalExecuteScript }, // Arg ptr : CTString
  { "IsSuspended",   &SignalIsSuspended },
  { "Resume",        &SignalResume },
  { "Reset",         &SignalReset },
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// Permanent script VMs
static sq::VM *_pSignalVM = NULL;
static sq::VM *_pCommandVM = NULL;

static bool SignalReturnCallback(sq::VM &vm) {
  // Output return value
  CTString str;

  if (vm.GetString(-1, str)) {
    CPrintF("%s\n", str.str_String);
  }

  return true;
};

static CTString _strLastCommandResult;

static bool CommandReturnCallback(sq::VM &vm) {
  // Save return value
  CTString str = "";
  vm.GetString(-1, str);

  // Must be a non-null value
  _strLastCommandResult = (str != "null") ? str : "";
  return true;
};

// Check if the current script execution is suspended
static INDEX ShellIsSuspended(void) {
  if (_pCommandVM == NULL) return FALSE;
  return _pCommandVM->IsSuspended();
};

// Resume a previously suspended script execution
static void ShellResumeVM(void) {
  // No VM or it's not suspended
  if (_pCommandVM == NULL || !_pCommandVM->IsSuspended()) return;

  bool bExecuted = _pCommandVM->Execute(&CommandReturnCallback);

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", _pCommandVM->GetError());
  }
};

// Forcefully reset the global VM, in case it's stuck in a loop etc.
static void ShellResetVM(void) {
  if (_pCommandVM != NULL) {
    delete _pCommandVM;
    _pCommandVM = NULL;
  }
};

// Generic method for executing scripts
static BOOL ExecuteSquirrelScript(sq::VM *pVM, const CTString &strScript, BOOL bFile, const SQChar *strSourceName, sq::VM::FReturnValueCallback pCallback) {
  if (bFile) {
    pVM->CompileFromFile(strScript);
  } else {
    pVM->CompileFromString(strScript, strSourceName);
  }

  // Error during the compilation
  if (!pVM->CanBeExecuted()) {
    CPrintF("^cff0000Compilation error:\n%s", pVM->GetError());
    return FALSE;
  }

  bool bExecuted = pVM->Execute(pCallback);

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", pVM->GetError());
    return FALSE;
  }

  return TRUE;
};

int SignalExecuteScript(void *pScript) {
  if (pScript == NULL) return FALSE;

  // Create a new VM
  SignalReset(NULL);
  _pSignalVM = new sq::VM(0xFFFFFFFF);

  const CTString &strScript = *(const CTString *)pScript;
  return ExecuteSquirrelScript(_pSignalVM, strScript, FALSE, "ScriptingEngine::ExecuteScript", &SignalReturnCallback);
};

int SignalIsSuspended(void *) {
  if (_pSignalVM == NULL) return FALSE;
  return _pSignalVM->IsSuspended();
};

int SignalResume(void *) {
  // No VM or it's not suspended
  if (_pSignalVM == NULL || !_pSignalVM->IsSuspended()) return TRUE;

  bool bExecuted = _pSignalVM->Execute(&SignalReturnCallback);

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", _pSignalVM->GetError());
    return FALSE;
  }

  return TRUE;
};

int SignalReset(void *) {
  if (_pSignalVM != NULL) {
    delete _pSignalVM;
    _pSignalVM = NULL;
  }

  return TRUE;
};

// Execute a script from the provided string and return the result
static CTString ShellExecuteString(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  // Create a new VM
  ShellResetVM();
  _pCommandVM = new sq::VM(0xFFFFFFFF);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(_pCommandVM, strScript, FALSE, "scr_ExecuteString", &CommandReturnCallback);

  return _strLastCommandResult;
};

// Execute a script from the provided file and return the result
static CTString ShellExecuteFile(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  // Create a new VM
  ShellResetVM();
  _pCommandVM = new sq::VM(0xFFFFFFFF);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(_pCommandVM, strScript, TRUE, "scr_ExecuteFile", &CommandReturnCallback);

  return _strLastCommandResult;
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Custom symbols
  GetPluginAPI()->RegisterMethod(TRUE, "INDEX",    "scr_IsSuspended",   "void",     &ShellIsSuspended);
  GetPluginAPI()->RegisterMethod(TRUE, "void",     "scr_ResumeVM",      "void",     &ShellResumeVM);
  GetPluginAPI()->RegisterMethod(TRUE, "void",     "scr_ResetVM",       "void",     &ShellResetVM);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteString", "CTString", &ShellExecuteString);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteFile",   "CTString", &ShellExecuteFile);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
