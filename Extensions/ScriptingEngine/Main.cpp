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

static CTCriticalSection _csScripts;

// Permanent script VMs
static sq::VM *_pSignalVM = NULL;
static sq::VM *_pCommandVM = NULL;

CDynamicContainer<sq::VM> _cCustomScripts;

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
  CTSingleLock sl(&_csScripts, TRUE);

  if (_pCommandVM == NULL) return FALSE;
  return _pCommandVM->IsSuspended();
};

// Resume a previously suspended script execution
static CTString ShellResumeVM(void) {
  CTSingleLock sl(&_csScripts, TRUE);

  // No VM or it's not suspended
  if (_pCommandVM == NULL || !_pCommandVM->IsSuspended()) return "";

  _strLastCommandResult = "";
  bool bExecuted = _pCommandVM->Resume(&CommandReturnCallback);

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Execution error:\n%s\n", _pCommandVM->GetError());
    return "";
  }

  return _strLastCommandResult;
};

// Forcefully reset the global VM, in case it's stuck in a loop etc.
static void ShellResetVM(void) {
  CTSingleLock sl(&_csScripts, TRUE);

  if (_pCommandVM != NULL) {
    delete _pCommandVM;
    _pCommandVM = NULL;
  }
};

// Generic method for executing scripts
static BOOL ExecuteSquirrelScript(sq::VM *pVM, const CTString &strScript, BOOL bFile, const SQChar *strSourceName, sq::VM::FReturnValueCallback pCallback) {
  bool bExecuted;

  if (bFile) {
    pVM->SetName(strScript); // Use script file
    bExecuted = pVM->ExecuteFile(strScript, pCallback);
  } else {
    pVM->SetName(strSourceName); // Use source name
    bExecuted = pVM->ExecuteString(strScript, strSourceName, pCallback);
  }

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Execution error:\n%s\n", pVM->GetError());
    return FALSE;
  }

  return TRUE;
};

int SignalExecuteScript(void *pScript) {
  if (pScript == NULL) return FALSE;

  CTSingleLock sl(&_csScripts, TRUE);

  // Create a new VM
  SignalReset(NULL);
  _pSignalVM = new sq::VM(true);

  const CTString &strScript = *(const CTString *)pScript;
  return ExecuteSquirrelScript(_pSignalVM, strScript, FALSE, "ScriptingEngine::ExecuteScript", &SignalReturnCallback);
};

int SignalIsSuspended(void *) {
  CTSingleLock sl(&_csScripts, TRUE);

  if (_pSignalVM == NULL) return FALSE;
  return _pSignalVM->IsSuspended();
};

int SignalResume(void *) {
  CTSingleLock sl(&_csScripts, TRUE);

  // No VM or it's not suspended
  if (_pSignalVM == NULL || !_pSignalVM->IsSuspended()) return TRUE;

  bool bExecuted = _pSignalVM->Resume(&SignalReturnCallback);

  // Error during the execution
  if (!bExecuted) {
    CPrintF("^cff0000Execution error:\n%s\n", _pSignalVM->GetError());
    return FALSE;
  }

  return TRUE;
};

int SignalReset(void *) {
  CTSingleLock sl(&_csScripts, TRUE);

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

  CTSingleLock sl(&_csScripts, TRUE);

  // Create a new VM
  ShellResetVM();
  _pCommandVM = new sq::VM(true);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(_pCommandVM, strScript, FALSE, "scr_ExecuteString", &CommandReturnCallback);

  return _strLastCommandResult;
};

// Execute a script from the provided file and return the result
static CTString ShellExecuteFile(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  CTSingleLock sl(&_csScripts, TRUE);

  // Create a new VM
  ShellResetVM();
  _pCommandVM = new sq::VM(true);

  _strLastCommandResult = "";
  ExecuteSquirrelScript(_pCommandVM, strScript, TRUE, "scr_ExecuteFile", &CommandReturnCallback);

  return _strLastCommandResult;
};

// Create a new VM for custom scripts
inline sq::VM *NewScriptVM(const CTString &strFile) {
  sq::VM *pVM = new sq::VM(true);
  pVM->SetName(strFile);

  // Error during the initial execution
  if (!pVM->ExecuteFile(strFile, NULL)) {
    CPrintF("^cff0000Could not load custom script (%s):\n%s\n", strFile.str_String, pVM->GetError());
    delete pVM;
    return NULL;
  }

  return pVM;
};

// Prepare Squirrel VMs for every custom script in some directory
static void LoadCustomScripts(void) {
  CTSingleLock sl(&_csScripts, TRUE);

  CFileList aScripts;
  ListGameFiles(aScripts, "Scripts\\Squirrel\\Custom\\", "*.nut", 0);

  const INDEX ct = aScripts.Count();

  for (INDEX i = 0; i < ct; i++) {
    const CTFileName &fnm = aScripts[i];

    sq::VM *pVM = NewScriptVM(fnm);
    if (pVM != NULL) _cCustomScripts.Add(pVM);
  }
};

// Free all loaded custom scripts
static void ClearCustomScripts(void) {
  CTSingleLock sl(&_csScripts, TRUE);

  FOREACHINDYNAMICCONTAINER(_cCustomScripts, sq::VM, itvm) {
    delete &itvm.Current();
  }

  _cCustomScripts.Clear();
};

// Reload all custom scripts
static void ReloadCustomScripts(void) {
  ClearCustomScripts();
  LoadCustomScripts();
};

// Load an additional custom script or reload an existing one under the same file
static void LoadCustomScript(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  CTSingleLock sl(&_csScripts, TRUE);

  // Load a new script first (in case it errors out and the old one needs to be preserved)
  sq::VM *pVM = NewScriptVM(strScript);
  if (pVM == NULL) return;

  // Remove an existing script under the same file
  FOREACHINDYNAMICCONTAINER(_cCustomScripts, sq::VM, itvm) {
    sq::VM *pOldVM = itvm;

    if (pOldVM->GetName() == strScript) {
      _cCustomScripts.Remove(pOldVM);
      delete pOldVM; // Delete old script
      break;
    }
  }

  // Add a new script
  _cCustomScripts.Add(pVM);
};

// Run a specific function for all loaded custom scripts
void RunCustomScripts(const SQChar *strFunc, sq::VM::FPushArguments pPushArgs, sq::VM::FReturnValueCallback pReturnCallback) {
  // Sync all threads that might be running the scripts from the same VMs
  CTSingleLock sl(&_csScripts, TRUE);

  CDynamicContainer<sq::VM> cToRemove;

  FOREACHINDYNAMICCONTAINER(_cCustomScripts, sq::VM, itvm) {
    sq::VM &vm = *itvm;

    // Skip if no such function in the script
    if (!vm.Root().PushObject(strFunc)) continue;

    if (!vm.Execute(pPushArgs, pReturnCallback)) {
      // Pass execution error
      sq_throwerror(vm, vm.GetError());
      CPrintF("^cff0000%s (%s):\n%s\n", strFunc, vm.GetName().str_String, vm.GetError());
      continue;
    }

    // Not suspended - proceed with business as usual
    if (!vm.IsSuspended()) continue;

    // Warn about suspension not being supported
    CPrintF("^cffff00%s (%s):\n%s\n", strFunc, vm.GetName().str_String,
      "suspend() function is not supported in custom scripts! Attempting to resume...");

    // Try to resume execution for a few times until it works
    INDEX ctResumeAttempts = 1000;

    while (--ctResumeAttempts >= 0 && vm.IsSuspended()) {
      if (!vm.Resume(pReturnCallback)) {
        // Pass execution error
        sq_throwerror(vm, vm.GetError());
        CPrintF("^cff0000%s (%s):\n%s\n", strFunc, vm.GetName().str_String, vm.GetError());
        break;
      }
    }

    // Remove the script altogether if it couldn't get resumed after all of the attempts
    if (vm.IsSuspended()) {
      CPrintF("^cff0000%s (%s):\n%s\n", strFunc, vm.GetName().str_String,
        "Could not resume custom script execution after many attempts, unloading the script...");
      cToRemove.Add(&vm);
    }
  }

  // Remove errored scripts
  FOREACHINDYNAMICCONTAINER(cToRemove, sq::VM, itvmRemove) {
    _cCustomScripts.Remove(itvmRemove);
    delete &itvmRemove.Current(); // Delete old script
  }
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  _csScripts.cs_iIndex = -1;

  // Register plugin events
  events.m_processing->OnStep  = &IProcessingEvents_OnStep;
  events.m_processing->OnFrame = &IProcessingEvents_OnFrame;

  events.m_rendering->OnPreDraw    = &IRenderingEvents_OnPreDraw;
  events.m_rendering->OnPostDraw   = &IRenderingEvents_OnPostDraw;
  events.m_rendering->OnRenderView = &IRenderingEvents_OnRenderView;

  //events.m_network->OnServerPacket = &INetworkEvents_OnServerPacket;
  //events.m_network->OnClientPacket = &INetworkEvents_OnClientPacket;
  events.m_network->OnAddPlayer    = &INetworkEvents_OnAddPlayer;
  events.m_network->OnRemovePlayer = &INetworkEvents_OnRemovePlayer;

  events.m_packet->OnCharacterConnect = &IPacketEvents_OnCharacterConnect;
  events.m_packet->OnCharacterChange  = &IPacketEvents_OnCharacterChange;
  events.m_packet->OnPlayerAction     = &IPacketEvents_OnPlayerAction;
  events.m_packet->OnChatMessage      = &IPacketEvents_OnChatMessage;

  events.m_game->OnGameStart   = &IGameEvents_OnGameStart;
  events.m_game->OnChangeLevel = &IGameEvents_OnChangeLevel;
  events.m_game->OnGameStop    = &IGameEvents_OnGameStop;
  events.m_game->OnGameSave    = &IGameEvents_OnGameSave;
  events.m_game->OnGameLoad    = &IGameEvents_OnGameLoad;

  events.m_demo->OnDemoPlay  = &IDemoEvents_OnDemoPlay;
  events.m_demo->OnDemoStart = &IDemoEvents_OnDemoStart;
  events.m_demo->OnDemoStop  = &IDemoEvents_OnDemoStop;

  //events.m_world->OnWorldLoad = &IWorldEvents_OnWorldLoad;

  events.m_listener->OnSendEvent     = &IListenerEvents_OnSendEvent;
  events.m_listener->OnReceiveItem   = &IListenerEvents_OnReceiveItem;
  events.m_listener->OnCallProcedure = &IListenerEvents_OnCallProcedure;

  events.m_timer->OnTick   = &ITimerEvents_OnTick;
  events.m_timer->OnSecond = &ITimerEvents_OnSecond;

  // Custom symbols
  GetPluginAPI()->RegisterMethod(TRUE, "INDEX",    "scr_IsSuspended",   "void",     &ShellIsSuspended);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ResumeVM",      "void",     &ShellResumeVM);
  GetPluginAPI()->RegisterMethod(TRUE, "void",     "scr_ResetVM",       "void",     &ShellResetVM);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteString", "CTString", &ShellExecuteString);
  GetPluginAPI()->RegisterMethod(TRUE, "CTString", "scr_ExecuteFile",   "CTString", &ShellExecuteFile);

  // Load custom scripts
  GetPluginAPI()->RegisterMethod(TRUE, "void", "scr_ReloadCustomScripts", "void", &ReloadCustomScripts);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "scr_LoadCustomScript", "CTString", &LoadCustomScript);
  LoadCustomScripts();
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
  // Free custom scripts
  ClearCustomScripts();
};
