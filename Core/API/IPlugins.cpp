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

#include "Modules/PluginStock.h"
#include <Engine/GameShell.h>

// Stock of plugin modules
static CPluginStock *_pPluginStock = NULL;

// List loaded plugin modules
static void ListPlugins(void) {
  const INDEX ctPlugins = _pPluginStock->GetTotalCount();

  if (ctPlugins == 0) {
    CPutString(TRANS("No plugins have been loaded!\n"));
    return;
  }

  CPutString(TRANS("^cffffffLoaded plugins:\n"));
  
  for (INDEX iPlugin = 0; iPlugin < ctPlugins; iPlugin++) {
    CPluginModule *pPlugin = _pPluginStock->st_ctObjects.Pointer(iPlugin);

    // Light blue - ON; Red - OFF
    CPutString(pPlugin->IsInitialized() ? "^c00ffff" : "^cff0000");

    // Indent the index
    const INDEX ctIndentLog10 = (iPlugin == 0) ? 0 : log10((FLOAT)iPlugin);
    const INDEX ctIndent = ClampDn(2 - ctIndentLog10, (INDEX)0);

    CPrintF("%*s%d - %s\n", ctIndent, "", iPlugin, pPlugin->GetName().str_String);

    // Print metadata
    const PluginInfo_t &info = pPlugin->pm_info;

    PatchVerString_t strVersion;
    MakeVersionString(strVersion, info.m_ulVersion);

    CPrintF(TRANS("  Name: %s\n"), info.m_strName);
    CPrintF(TRANS("  Author: %s\n"), info.m_strAuthor);
    CPrintF(TRANS("  Version: %s\n"), strVersion);
    CPrintF(TRANS("  Description: %s\n"), info.m_strDescription);
    CPutString("---\n");
  }
};

// Enable specific plugin
static void EnablePlugin(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iPlugin = NEXT_ARG(INDEX);

  if (iPlugin < 0 || iPlugin >= _pPluginStock->GetTotalCount()) {
    CPutString(TRANS("Invalid plugin index!\n"));
    return;
  }

  CPluginModule *pPlugin = _pPluginStock->st_ctObjects.Pointer(iPlugin);
  const char *strPlugin = pPlugin->GetName().str_String;

  if (pPlugin->IsInitialized()) {
    CPrintF(TRANS("'%s' plugin is already enabled!\n"), strPlugin);
    return;
  }

  pPlugin->Initialize();

  if (pPlugin->IsInitialized()) {
    CPrintF(TRANS("Successfully enabled '%s' plugin!\n"), strPlugin);
  } else {
    CPrintF(TRANS("Cannot enable '%s' plugin!\n"), strPlugin);
  }
};

// Disable specific plugin
static void DisablePlugin(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iPlugin = NEXT_ARG(INDEX);

  if (iPlugin < 0 || iPlugin >= _pPluginStock->GetTotalCount()) {
    CPutString(TRANS("Invalid plugin index!\n"));
    return;
  }

  CPluginModule *pPlugin = _pPluginStock->st_ctObjects.Pointer(iPlugin);
  const char *strPlugin = pPlugin->GetName().str_String;

  if (!pPlugin->IsInitialized()) {
    CPrintF(TRANS("'%s' plugin is already disabled!\n"), strPlugin);
    return;
  }

  pPlugin->Deactivate();

  CPrintF(TRANS("Successfully disabled '%s' plugin!\n"), strPlugin);
};

// Get plugin index by its display name
static INDEX GetPluginIndex(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strName = *NEXT_ARG(CTString *);

  const INDEX ctPlugins = _pPluginStock->GetTotalCount();

  for (INDEX iPlugin = 0; iPlugin < ctPlugins; iPlugin++) {
    CPluginModule *pPlugin = _pPluginStock->st_ctObjects.Pointer(iPlugin);

    // Check plugin name
    const PluginInfo_t &info = pPlugin->pm_info;

    if (info.m_strName == strName) {
      return iPlugin;
    }
  }

  return -1;
};

// Constructor
CPluginAPI::CPluginAPI()
{
  // Create stock of plugin modules
  _pPluginStock = new CPluginStock;

  // Commands for manually toggling plugins
  _pShell->DeclareSymbol("user void ListPlugins(void);",    &ListPlugins);
  _pShell->DeclareSymbol("user void EnablePlugin(INDEX);",  &EnablePlugin);
  _pShell->DeclareSymbol("user void DisablePlugin(INDEX);", &DisablePlugin);
  _pShell->DeclareSymbol("user INDEX GetPluginIndex(CTString);", &GetPluginIndex);
};

// Destructor
CPluginAPI::~CPluginAPI()
{
  delete _pPluginStock;
  _pPluginStock = NULL;

  ASSERT(GetExtensionCount() == 0);
};

void CPluginAPI::LoadGameLib(const char *strSettingsFile) {
  // Already loaded
  if (_pGame != NULL) return;

  try {
    // Obtain Game plugin
    CPluginModule *pGameLib = LoadGamePlugin();

    // Create Game class
    CGame *(*pGameCreateFunc)(void) = NULL;
    pGameLib->GetSymbol_t(&pGameCreateFunc, "GAME_Create");

    _pGame = pGameCreateFunc();

  } catch (char *strError) {
    FatalError("Cannot load Game library:\n%s", strError);
  }

  CTString strSettings = strSettingsFile;

  // Initialize Game, if needed
  if (strSettings != "") {
    // Hook default fields
    GetGameAPI()->HookFields();

    // Can be used to hook new fields
    _pGame->Initialize(strSettings);

    // Set mod URL to the latest patch release
    if (_strModURL == "" || _strModURL.FindSubstr("croteam.com") != -1) {
      _strModURL = CLASSICSPATCH_URL_LATESTRELEASE;
    }

  #if _PATCHCONFIG_NEW_QUERY
    // Update internal master server
    extern void UpdateInternalGameSpyMS(INDEX);
    UpdateInternalGameSpyMS(0);
  #endif
  }
};

void *CPluginAPI::LoadGameGuiLib(const char *strSettingsFile) {
  static GameGUI_interface *pGameGUI = NULL;

  // Already loaded
  if (pGameGUI != NULL) return pGameGUI;

  try {
    // Obtain GameGUI plugin
    CPluginModule *pGameLib = LoadGameGuiPlugin();

    // Create GameGUI class
    GameGUI_interface *(*pGameGuiCreateFunc)(void) = NULL;
    pGameLib->GetSymbol_t(&pGameGuiCreateFunc, "GAMEGUI_Create");

    pGameGUI = pGameGuiCreateFunc();

  } catch (char *strError) {
    FatalError("Cannot load GameGUI library:\n%s", strError);
  }

  CTString strSettings = strSettingsFile;
  pGameGUI->Initialize(strSettings);

  return (void *)pGameGUI;
};

CPluginModule *CPluginAPI::LoadGamePlugin(void) {
  // Obtain Game library
  CPluginModule *pLib = GetPluginAPI()->LoadPlugin_t(IDir::FullLibPath("Game" + _strModExt));
  CPrintF(TRANS("Loading Game library '%s'...\n"), pLib->GetName());

  // Set metadata for vanilla library
  PluginInfo_t *pInfo = &pLib->pm_info;

  if (pInfo->m_ulVersion == 0) {
    pInfo->SetMetadata(MakeVersion(1, 0, _SE_BUILD_MINOR),
      "Croteam", "Game library", "Main component that provides game logic.");
  }

  return pLib;
};

CPluginModule *CPluginAPI::LoadGameGuiPlugin(void) {
  // Obtain Game library
  CPluginModule *pLib = GetPluginAPI()->LoadPlugin_t(IDir::FullLibPath("GameGUI" + _strModExt));
  CPrintF(TRANS("Loading Game GUI library '%s'...\n"), pLib->GetName());

  // Set metadata for vanilla library
  PluginInfo_t *pInfo = &pLib->pm_info;

  if (pInfo->m_ulVersion == 0) {
    pInfo->SetMetadata(MakeVersion(1, 0, _SE_BUILD_MINOR),
      "Croteam", "Game GUI library", "Serious Editor component that provides custom game interfaces.");
  }

  return pLib;
};

void CPluginAPI::LoadPlugins(ULONG ulUtilityFlags) {
  // List all library files
  CFileList afnmGameDir;

  #define LIST_PLUGINS_FLAGS (FLF_RECURSIVE | FLF_IGNORELISTS | FLF_IGNOREGRO)
  #define LIST_PLUGINS_FLAGS_MOD (LIST_PLUGINS_FLAGS | FLF_ONLYMOD | FLF_REUSELIST)

  ListGameFiles(afnmGameDir, IDir::AppBin()    + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS);
  ListGameFiles(afnmGameDir, IDir::AppModBin() + "Plugins\\", "*.dll", LIST_PLUGINS_FLAGS_MOD);

  CPrintF("--- Loading user plugins (flags: 0x%X) ---\n", ulUtilityFlags);

  // Load every plugin
  for (INDEX i = 0; i < afnmGameDir.Count(); i++)
  {
    CPrintF("  %d - %s\n", i + 1, afnmGameDir[i].str_String);

    try {
      // Try to load the plugin
      GetPluginAPI()->ObtainPlugin_t(afnmGameDir[i], ulUtilityFlags);

    } catch (char *strError) {
      // Plugin initialization failed
      CPrintF("^cffff00%s\n", strError);
    }
  }

  CPrintF("--- Done! ---\n");
};

void CPluginAPI::ReleasePlugins(ULONG ulUtilityFlags) {
  CPrintF("--- Releasing user plugins (flags: 0x%X) ---\n", ulUtilityFlags);

  // Gather plugins that need to be released
  CDynamicContainer<CPluginModule> cToRelease;

  FOREACHPLUGIN(itPlugin) {
    const PluginInfo_t &info = itPlugin->pm_info;

    // Matching utility flags
    if (info.m_ulFlags & ulUtilityFlags) {
      cToRelease.Add(itPlugin);
    }
  }

  // Release plugin modules one by one
  FOREACHINDYNAMICCONTAINER(cToRelease, CPluginModule, itRelease) {
    _pPluginStock->Release(itRelease);
  }

  CPrintF("--- Done! ---\n");
};

// Obtain pointer to a plugin module of specific utility types
CPluginModule *CPluginAPI::ObtainPlugin_t(const CTFileName &fnmModule, ULONG ulUtilityFlags)
{
  // Obtain info from the plugin
  CPluginModule *pPlugin = _pPluginStock->Obtain_t(fnmModule);
  const PluginInfo_t &info = pPlugin->pm_info;

  const CTString strFileName = fnmModule.FileName() + fnmModule.FileExt();

  // Check used API version and utility types
  BOOL bVersion = (info.m_ulAPI <= CLASSICSPATCH_INTERFACE_VERSION);
  BOOL bUtility = (info.m_ulFlags & ulUtilityFlags) != 0;

  // Warn about loading cancelling
  if (!bVersion) {
    CPrintF(TRANS("'%s' loading cancelled: API version is too new (%u)\n"), strFileName, info.m_ulAPI);
  } else if (!bUtility) {
    CPrintF(TRANS("'%s' loading cancelled: Mismatching utility flags (0x%X)\n"), strFileName, info.m_ulFlags);
  }

  // If cannot load some plugin
  if (!bVersion || !bUtility) {
    // Release it
    _pPluginStock->Release(pPlugin);

    return NULL;
  }

  // Initialize the plugin and return it
  pPlugin->Initialize();

  return pPlugin;
};

// Load plugin module without safety checks
CPluginModule *CPluginAPI::LoadPlugin_t(const CTFileName &fnmModule)
{
  CPluginModule *pPlugin = _pPluginStock->Obtain_t(fnmModule);
  pPlugin->Initialize();

  return pPlugin;
};

// Get loaded plugins
CDynamicContainer<CPluginModule> &CPluginAPI::GetPlugins(void) {
  return _pPluginStock->st_ctObjects;
};

void CPluginAPI::RegisterSymbol(PluginSymbol_t &ps, const char *strSymbolName, const char *strDefaultValue)
{
  // Get symbol if it already exists
  CShellSymbol *pss = _pShell->GetSymbol(strSymbolName, TRUE);

  // Set existing symbol
  if (pss != NULL) {
    ps.m_pShellSymbol = pss;
    return;
  }

  // External declaration
  CTString strDeclaration;
  CTString strDeclFlags = "";

  // Symbol flags
  if (ps.m_ulFlags & SSF_CONSTANT)   strDeclFlags += "const ";
  if (ps.m_ulFlags & SSF_PERSISTENT) strDeclFlags += "persistent ";
  if (ps.m_ulFlags & SSF_USER)       strDeclFlags += "user ";

  // Symbol type
  static const char *astrTypes[3] = { "INDEX", "FLOAT", "CTString" };
  const char *strType = astrTypes[ps.m_eSymbolType];

  // E.g. "extern persistent user INDEX iSymbol = (INDEX)0;"
  strDeclaration.PrintF("extern %s%s %s = (%s)%s;", strDeclFlags, strType, strSymbolName, strType, strDefaultValue);
  _pShell->Execute(strDeclaration);

  // Assign newly declared symbol
  ps.m_pShellSymbol = _pShell->GetSymbol(strSymbolName, TRUE);
};

void CPluginAPI::RegisterMethod(bool bUser, const char *strReturnType, const char *strFunctionName, const char *strArgumentTypes, void *pFunction)
{
  // Get symbol if it already exists
  CShellSymbol *pss = _pShell->GetSymbol(strFunctionName, TRUE);

  // Replace method of the existing symbol
  if (pss != NULL) {
    pss->ss_pvValue = pFunction;
    return;
  }

  // Declaration
  CTString strDeclaration;
  const char *strDeclFlags = (bUser ? "user " : "");

  // E.g. "user INDEX GetValue(void);"
  strDeclaration.PrintF("%s%s %s(%s);", strDeclFlags, strReturnType, strFunctionName, strArgumentTypes);
  _pShell->DeclareSymbol(strDeclaration, pFunction);
};

void CPluginSymbol::Register(const char *strSymbolName, const char *strPreFunc, const char *strPostFunc)
{
  CTString strValue;

  switch (m_eSymbolType) {
    case k_ESymbolIndex: strValue.PrintF("%d", m_iDefault); break;
    case k_ESymbolFloat: strValue.PrintF("%f", m_fDefault); break;

    case k_ESymbolString: {
      strValue = m_strDefault;
      const INDEX iLast = strValue.Length() - 1;

      // Go from the end
      for (INDEX i = iLast; i >= 0; i--) {
        // Double all backslashes and add them to quotes
        if (m_strDefault[i] == '\\'
         || m_strDefault[i] == '"') {
          strValue.InsertChar(i, '\\');
        }
      }

      // Surround with quotes
      strValue = "\"" + strValue + "\"";
    } break;
  }

  GetPluginAPI()->RegisterSymbol(*this, strSymbolName, strValue);
  ASSERT(m_pShellSymbol != NULL);

  // Assign pre-function
  if (strcmp(strPreFunc, "") != 0) {
    CShellSymbol *pssPreFunc = _pShell->GetSymbol(strPreFunc, TRUE);
    ASSERT(pssPreFunc != NULL);

    if (pssPreFunc != NULL) {
      m_pShellSymbol->ss_pPreFunc = (BOOL (*)(void *))pssPreFunc->ss_pvValue;
    }
  }

  // Assign post-function
  if (strcmp(strPostFunc, "") != 0) {
    CShellSymbol *pssPostFunc = _pShell->GetSymbol(strPostFunc, TRUE);
    ASSERT(pssPostFunc != NULL);

    if (pssPostFunc != NULL) {
      m_pShellSymbol->ss_pPostFunc = (void (*)(void *))pssPostFunc->ss_pvValue;
    }
  }
};
