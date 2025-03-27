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

static void ReturnCallback(sq::VM &vm) {
  // Output return value
  CTString str;

  if (vm.GetString(-1, str)) {
    CPrintF("%s\n", str.str_String);
  }
};

int ExecuteScript(void *pScript) {
  if (pScript == NULL) return FALSE;

  const CTString &strScript = *(const CTString *)pScript;

  // Create a permanent VM
  static sq::VM vm;
  vm.m_pReturnValueCallback = &ReturnCallback;

  const bool bExpectReturnValue = true; // Switch for testing

  bool bCompiled = vm.CompileFromString(strScript, "Shell", bExpectReturnValue);

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

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
