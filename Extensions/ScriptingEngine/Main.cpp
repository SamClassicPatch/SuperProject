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

// Generic method for executing scripts
static BOOL ExecuteSquirrelScript(const CTString &strScript, BOOL bFile, sq::VM::FReturnValueCallback pCallback) {
  // Create a permanent VM
  static sq::VM vm(0xFFFFFFFF);
  vm.m_pReturnValueCallback = pCallback;

  const bool bExpectReturnValue = true; // Switch for testing
  bool bCompiled;

  if (bFile) {
    bCompiled = vm.CompileFromFile(strScript, bExpectReturnValue);
  } else {
    bCompiled = vm.CompileFromString(strScript, "Shell", bExpectReturnValue);
  }

  // Error during the compilation
  if (!bCompiled) {
    CPrintF("^cff0000Compilation error:\n%s", vm.GetError());
    return FALSE;
  }

  bool bExecuted = vm.Execute();

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Runtime error:\n%s", vm.GetError());
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
  //return ExecuteSquirrelScript(strScript, FALSE);
  return ExecuteSquirrelScript("SeriousSam_ClassicsPatch\\_Script.nut", TRUE, &SignalReturnCallback);
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
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteString", "CTString", &ExecuteString);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteFile",   "CTString", &ExecuteFile);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
