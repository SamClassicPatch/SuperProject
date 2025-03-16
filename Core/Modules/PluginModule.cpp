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

// Separate references to extensions
static CDynamicContainer<CPluginModule> _cExtensions;

int ClassicsExtensions_GetExtensionCount(void) {
  return (int)_cExtensions.Count();
};

HPatchPlugin ClassicsExtensions_GetExtensionByIndex(int iExtension) {
  if (iExtension < 0 || iExtension >= _cExtensions.Count()) return NULL;

  return _cExtensions.Pointer(iExtension);
};

HPatchPlugin ClassicsExtensions_GetExtensionByName(const char *strExtension) {
  FOREACHINDYNAMICCONTAINER(_cExtensions, CPluginModule, itPlugin) {
    CPluginModule *pPlugin = itPlugin;
    const PluginInfo_t &info = pPlugin->pm_info;

    ASSERT(info.m_strExtensionIdentifier != NULL);

    if (strcmp(info.m_strExtensionIdentifier, strExtension) == 0) {
      return pPlugin;
    }
  }

  return NULL;
};

ExtensionProp_t *ClassicsExtensions_FindProperty(HPatchPlugin hPlugin, const char *strProperty) {
  if (hPlugin == NULL) return NULL;
  return hPlugin->FindProperty(strProperty);
};

ExtensionProp_t *ClassicsExtensions_FindPropertyOfType(HPatchPlugin hPlugin, const char *strProperty, ExtensionProp_t::EType eType) {
  if (hPlugin == NULL) return NULL;
  return hPlugin->FindPropertyOfType(strProperty, eType);
};

bool ClassicsExtensions_GetBool(HPatchPlugin hPlugin, const char *strProperty, bool *pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Bool);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._bool;
  return true;
};

bool ClassicsExtensions_GetInt(HPatchPlugin hPlugin, const char *strProperty, int *pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Int);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._int;
  return true;
};

bool ClassicsExtensions_GetFloat(HPatchPlugin hPlugin, const char *strProperty, float *pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Float);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._float;
  return true;
};

bool ClassicsExtensions_GetDouble(HPatchPlugin hPlugin, const char *strProperty, double *pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Double);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._double;
  return true;
};

bool ClassicsExtensions_GetString(HPatchPlugin hPlugin, const char *strProperty, const char **pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_String);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._string;
  return true;
};

bool ClassicsExtensions_GetData(HPatchPlugin hPlugin, const char *strProperty, void **pValue) {
  if (hPlugin == NULL) return false;

  const ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Data);
  if (pProp == NULL) return false;

  *pValue = pProp->m_value._data;
  return true;
};

bool ClassicsExtensions_SetBool(HPatchPlugin hPlugin, const char *strProperty, bool bValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Bool);
  if (pProp == NULL) return false;

  pProp->m_value._bool = bValue;
  return true;
};

bool ClassicsExtensions_SetInt(HPatchPlugin hPlugin, const char *strProperty, int iValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Int);
  if (pProp == NULL) return false;

  pProp->m_value._int = iValue;
  return true;
};

bool ClassicsExtensions_SetFloat(HPatchPlugin hPlugin, const char *strProperty, float fValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Float);
  if (pProp == NULL) return false;

  pProp->m_value._float = fValue;
  return true;
};

bool ClassicsExtensions_SetDouble(HPatchPlugin hPlugin, const char *strProperty, double fValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Double);
  if (pProp == NULL) return false;

  pProp->m_value._double = fValue;
  return true;
};

bool ClassicsExtensions_SetString(HPatchPlugin hPlugin, const char *strProperty, const char *strValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_String);
  if (pProp == NULL) return false;

  pProp->m_value._string = strValue;
  return true;
};

bool ClassicsExtensions_SetData(HPatchPlugin hPlugin, const char *strProperty, void *pValue) {
  if (hPlugin == NULL) return false;

  ExtensionProp_t *pProp = hPlugin->FindPropertyOfType(strProperty, ExtensionProp_t::k_EType_Data);
  if (pProp == NULL) return false;

  pProp->m_value._data = pValue;
  return true;
};

FExtensionSignal ClassicsExtensions_FindSignal(HPatchPlugin hPlugin, const char *strSignal) {
  if (hPlugin == NULL) return false;
  return hPlugin->FindSignal(strSignal);
};

bool ClassicsExtensions_CallSignal(HPatchPlugin hPlugin, const char *strSignal, int *piResult, void *pData) {
  if (hPlugin == NULL) return false;
  return hPlugin->CallSignal(strSignal, piResult, pData);
};

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

  // Prepare the extension
  if (pm_info.m_strExtensionIdentifier != NULL) {
    // Set handle to the extension
    HPatchPlugin *phExtension = (HPatchPlugin *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(EXTENSIONMODULE_LOCALHANDLE));

    if (phExtension != NULL) {
      *phExtension = this;
    }

    // Retrieve array of extension properties
    ExtensionProp_t *aPropsArray = (ExtensionProp_t *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(EXTENSIONMODULE_PROPSARRAY));
    size_t *pctPropsCount = (size_t *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(EXTENSIONMODULE_PROPSCOUNT));

    if (aPropsArray != NULL && pctPropsCount != NULL) {
      pm_aExtensionProps = aPropsArray;
      pm_ctExtensionProps = *pctPropsCount;
    }

    // Retrieve array of extension signals
    ExtensionSignal_t *aSignalsArray = (ExtensionSignal_t *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(EXTENSIONMODULE_SIGNALARRAY));
    size_t *pctSignalsCount = (size_t *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(EXTENSIONMODULE_SIGNALCOUNT));

    if (aSignalsArray != NULL && pctSignalsCount != NULL) {
      pm_aExtensionSignals = aSignalsArray;
      pm_ctExtensionSignals = *pctSignalsCount;
    }
  }

  // Start the plugin
  if (pm_pStartupFunc != NULL) {
    pm_pStartupFunc(&pm_props, pm_events);
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
    pm_pShutdownFunc(&pm_props);
  }

  // Unregister plugin events
  ResetPluginEvents(&pm_events);

  // Reset extension properties and signals
  pm_aExtensionProps = NULL;
  pm_ctExtensionProps = 0;

  pm_aExtensionSignals = NULL;
  pm_ctExtensionSignals = 0;

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

  pm_aExtensionProps = NULL;
  pm_ctExtensionProps = 0;

  pm_aExtensionSignals = NULL;
  pm_ctExtensionSignals = 0;
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

// Find extension property in the plugin
ExtensionProp_t *CPluginModule::FindProperty(const char *strProperty) {
  const size_t ct = pm_ctExtensionProps;
  if (ct == 0 || pm_aExtensionProps == NULL) return false;

  for (size_t i = 0; i < ct; i++) {
    ExtensionProp_t &prop = pm_aExtensionProps[i];

    if (strcmp(prop.m_strProperty, strProperty) == 0) return &prop;
  }

  return NULL;
};

// Find extension property of a specific type in the plugin
ExtensionProp_t *CPluginModule::FindPropertyOfType(const char *strProperty, ExtensionProp_t::EType eType) {
  const size_t ct = pm_ctExtensionProps;
  if (ct == 0 || pm_aExtensionProps == NULL) return false;

  for (size_t i = 0; i < ct; i++) {
    ExtensionProp_t &prop = pm_aExtensionProps[i];

    if (prop.m_eType != eType) continue;
    if (strcmp(prop.m_strProperty, strProperty) == 0) return &prop;
  }

  return NULL;
};

// Find function of some extension signal in the plugin
FExtensionSignal CPluginModule::FindSignal(const char *strSignal) {
  const size_t ct = pm_ctExtensionSignals;
  if (ct == 0 || pm_aExtensionSignals == NULL) return false;

  for (size_t i = 0; i < ct; i++) {
    ExtensionSignal_t &signal = pm_aExtensionSignals[i];

    if (strcmp(signal.m_strSignal, strSignal) == 0) return signal.m_pHandler;
  }

  return NULL;
};

// Call function of some extension signal in the plugin
bool CPluginModule::CallSignal(const char *strSignal, int *piResult, void *pData) {
  FExtensionSignal pSignal = FindSignal(strSignal);
  if (pSignal == NULL) return false;

  // Optional result
  if (piResult != NULL) {
    *piResult = pSignal(pData);
  } else {
    pSignal(pData);
  }

  return true;
};

// Module cleanup
void CPluginModule::Clear(void) {
  // Remove from the list of extensions
  if (_cExtensions.IsMember(this)) {
    _cExtensions.Remove(this);
  }

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
  FInfoFunc           *ppGetInfoFunc  = (FInfoFunc *)          GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_GETINFO));
  FModuleStartupFunc  *ppStartupFunc  = (FModuleStartupFunc *) GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_STARTUP));
  FModuleShutdownFunc *ppShutdownFunc = (FModuleShutdownFunc *)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEPOINTER_SHUTDOWN));

  if (ppGetInfoFunc != NULL) {
    pm_pGetInfoFunc = *ppGetInfoFunc;
  } else {
    // Try looking up the function itself
    pm_pGetInfoFunc = (FInfoFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_GETINFO));
  }

  if (ppStartupFunc != NULL) {
    pm_pStartupFunc = *ppStartupFunc;
  } else {
    // Try looking up the function itself
    pm_pStartupFunc = (FModuleStartupFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_STARTUP));
  }

  if (ppShutdownFunc != NULL) {
    pm_pShutdownFunc = *ppShutdownFunc;
  } else {
    // Try looking up the function itself
    pm_pShutdownFunc = (FModuleShutdownFunc)GetProcAddress(GetHandle(), CLASSICSPATCH_STRINGIFY(PLUGINMODULEMETHOD_SHUTDOWN));
  }

  // [Cecil] NOTE: Don't throw anything here, otherwise non-plugin libraries cannot be loaded (such as vanilla Game & GameGUI)
  if (pm_pGetInfoFunc == NULL) return;

  // Try to get information about the plugin immediately
  pm_pGetInfoFunc(&pm_info);

  // Add to the list of extensions
  if (pm_info.m_strExtensionIdentifier != NULL) {
    ASSERTMSG(pm_info.m_strExtensionIdentifier[0] != '\0', "Extension identifier is an empty string!");

    ASSERT(!_cExtensions.IsMember(this));
    _cExtensions.Add(this);
  }
};
