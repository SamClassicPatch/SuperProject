/* Copyright (c) 2022-2025 Dreamy Cecil
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

#include "PluginModule.h"

// Current plugin in the process of initialization
CPluginModule *_pInitializingPlugin = NULL;

// Clean up event interfaces
inline void ResetPluginEvents(PluginEvents_t *pEvents) {
  #define CLEAR_EVENT_FIELDS(_Interface) \
    memset(_Interface, NULL, sizeof(*_Interface))

  CLEAR_EVENT_FIELDS(pEvents->m_processing);
  CLEAR_EVENT_FIELDS(pEvents->m_rendering);
  CLEAR_EVENT_FIELDS(pEvents->m_network);
  CLEAR_EVENT_FIELDS(pEvents->m_packet);
  CLEAR_EVENT_FIELDS(pEvents->m_game);
  CLEAR_EVENT_FIELDS(pEvents->m_demo);
  CLEAR_EVENT_FIELDS(pEvents->m_world);
  CLEAR_EVENT_FIELDS(pEvents->m_listener);
  CLEAR_EVENT_FIELDS(pEvents->m_timer);

  #undef CLEAR_EVENT_FIELDS
};

// Create event interfaces
PluginEvents_t::PluginEvents_t() :
  m_processing(new IProcessingEvents),
  m_rendering (new IRenderingEvents),
  m_network   (new INetworkEvents),
  m_packet    (new IPacketEvents),
  m_game      (new IGameEvents),
  m_demo      (new IDemoEvents),
  m_world     (new IWorldEvents),
  m_listener  (new IListenerEvents),
  m_timer     (new ITimerEvents)
{
  ResetPluginEvents(this);
};

// Destroy event interfaces
PluginEvents_t::~PluginEvents_t() {
  delete m_processing;
  delete m_rendering;
  delete m_network;
  delete m_packet;
  delete m_game;
  delete m_demo;
  delete m_world;
  delete m_listener;
  delete m_timer;

  memset(this, NULL, sizeof(PluginEvents_t));
};

// Constructor
CPluginModule::CPluginModule() {
  ResetFields();
};

// Destructor
CPluginModule::~CPluginModule() {
  Clear();
};

// Module initialization
void CPluginModule::Initialize(void) {
  if (IsInitialized()) return;

  // Set to the current plugin
  CPluginModule *pLastPlugin = _pInitializingPlugin;
  _pInitializingPlugin = this;

  // Start the plugin
  if (pm_pStartupFunc != NULL) {
    pm_pStartupFunc(pm_props, pm_events);
  }

  // Restore last plugin
  _pInitializingPlugin = pLastPlugin;

  pm_bInitialized = TRUE;
};

// Module deactivation
void CPluginModule::Deactivate(void) {
  if (!IsInitialized()) return;

  // Destroy all patches
  for (INDEX i = 0; i < pm_aPatches.Count(); i++) {
    DestroyPatch(pm_aPatches[i]);
  }

  pm_aPatches.PopAll();

  // Stop the plugin
  if (pm_pShutdownFunc != NULL) {
    pm_pShutdownFunc(pm_props);
  }

  // Unregister plugin events
  ResetPluginEvents(&pm_events);

  pm_bInitialized = FALSE;
};

// Reset class fields
void CPluginModule::ResetFields(void) {
  pm_hLibrary = NULL;
  pm_bInitialized = FALSE;

  pm_pGetInfoFunc = NULL;
  pm_pStartupFunc = NULL;
  pm_pShutdownFunc = NULL;

  pm_info.m_ulFlags = k_EPluginFlagManual;
  pm_info.SetMetadata(0, "Unknown", "No name", "None");
  pm_info.m_strExtensionIdentifier = NULL;

  pm_props.Clear();
  ResetPluginEvents(&pm_events);
};

// Add new function patch on startup
void CPluginModule::AddPatch(HFuncPatch hPatch)
{
  // Check if it already exists
  for (INDEX i = 0; i < pm_aPatches.Count(); i++) {
    if (pm_aPatches[i] == hPatch) return;
  }

  pm_aPatches.Add(hPatch);
};

// Get specific symbol from the module
void *CPluginModule::GetSymbol_t(const char *strSymbolName) {
  // No module
  if (GetHandle() == NULL) {
    ThrowF_t(TRANS("Plugin module has not been loaded yet!"));
  }

  void *pSymbol = GetProcAddress(GetHandle(), strSymbolName);

  // No symbol
  if (pSymbol == NULL) {
    ThrowF_t(TRANS("Cannot find '%s' symbol in '%s'!"), strSymbolName, GetName());
  }

  return pSymbol;
};

// Module cleanup
void CPluginModule::Clear(void) {
  // Release DLL
  if (pm_hLibrary != NULL) {
    Deactivate();
    FreeLibrary(pm_hLibrary);
  }

  // Reset class
  ResetFields();
  CSerial::Clear();
};

// Load plugin module (override non-virtual CSerial::Load_t)
void CPluginModule::Load_t(const CTFileName &fnmDLL)
{
  ASSERT(!IsUsed());

  // Mark that just changed
  MarkChanged();

  // Remember filename
  ser_FileName = fnmDLL;

  // Load plugin's configuration file
  const CTString strConfig = IDir::AppModBin() + "Plugins\\" + fnmDLL.FileName() + ".ini";

  try {
    pm_props.Load_t(strConfig, TRUE);

  } catch (char *strError) {
    (void)strError;
  }

  // Load library from file
  CTFileName fnmExpanded;
  ExpandFilePath(EFP_READ | EFP_NOZIPS, fnmDLL, fnmExpanded);

  // Load dll
  pm_hLibrary = ILib::LoadLib(fnmExpanded);

  // Pointers to the main plugin methods
  CInfoFunc           *ppGetInfoFunc  = (CInfoFunc *)          GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_GETINFO));
  CModuleStartupFunc  *ppStartupFunc  = (CModuleStartupFunc *) GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_STARTUP));
  CModuleShutdownFunc *ppShutdownFunc = (CModuleShutdownFunc *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_SHUTDOWN));

  if (ppGetInfoFunc != NULL) {
    pm_pGetInfoFunc = *ppGetInfoFunc;
  } else {
    // Try looking up the function itself
    pm_pGetInfoFunc = (CInfoFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_GETINFO));
  }

  if (ppStartupFunc != NULL) {
    pm_pStartupFunc = *ppStartupFunc;
  } else {
    // Try looking up the function itself
    pm_pStartupFunc = (CModuleStartupFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_STARTUP));
  }

  if (ppShutdownFunc != NULL) {
    pm_pShutdownFunc = *ppShutdownFunc;
  } else {
    // Try looking up the function itself
    pm_pShutdownFunc = (CModuleShutdownFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_SHUTDOWN));
  }

  // Try to get information about the plugin immediately
  if (pm_pGetInfoFunc != NULL) {
    pm_pGetInfoFunc(&pm_info);

  } else {
    ThrowF_t(TRANS("Cannot retrieve any info about the plugin!"));
  }
};
