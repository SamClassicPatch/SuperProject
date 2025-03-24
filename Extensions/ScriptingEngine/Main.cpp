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

int ExecuteScript(void *pScript) {
  if (pScript == NULL) return FALSE;

  const CTString &strScript = *(const CTString *)pScript;

  // Create a permanent VM
  static sq::VM vm;
  HSQUIRRELVM sqvm = vm.GetVM();

  SQRESULT r = SQ_OK;
  bool bWasSuspended = false;

  // If the VM wasn't suspended last time
  if (vm.GetState() != SQ_VMSTATE_SUSPENDED) {
    // Compile new script (pushes the entire script as a closure on top of the stack)
    r = vm.CompileScript(strScript, "Shell");

  // Otherwise it was
  } else {
    bWasSuspended = true;
  }

  // If no compilation error occurred
  if (SQ_SUCCEEDED(r)) {
    // And the VM is still suspended
    if (vm.GetState() == SQ_VMSTATE_SUSPENDED) {
      // Resume it
      r = sq_wakeupvm(sqvm, FALSE, FALSE, TRUE, FALSE);

    // Otherwise run it anew
    } else {
      // Push root table as 'this' for the script
      sq_pushroottable(sqvm);

      // Call the script closure without removing it from the stack
      r = sq_call(sqvm, 1, FALSE, TRUE);
    }

    // If the VM executed everything correctly
    if (SQ_SUCCEEDED(r)) {
      // And it wasn't suspended as a result
      if (vm.GetState() != SQ_VMSTATE_SUSPENDED) {
        // Pop the script closure
        sq_poptop(sqvm);
      }

    // Error during the execution
    } else {
      CPrintF("Runtime error:\n%s", vm.GetError());
      return FALSE;
    }

  // Error during the compilation
  } else {
    CPrintF("Compilation error:\n%s", vm.GetError());
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
