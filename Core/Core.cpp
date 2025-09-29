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

#include "Base/CoreTimerHandler.h"
#include "Networking/NetworkFunctions.h"

static CTimerValue _tvClassicsPatchInit = __int64(0); // When Classics Patch was initialized

static bool _bClassicsPatchRunning = false;
static bool _bClassicsPatchCustomMod = false;

static EClassicsPatchAppType _eAppType = k_EClassicsPatchAppType_Unknown; // Running application type
static EClassicsPatchSeason _eSpecialEvent = k_EClassicsPatchSeason_None; // Current seasonal event

__int64 ClassicsCore_GetInitTime(void) {
  return _tvClassicsPatchInit.tv_llValue;
};

EClassicsPatchAppType ClassicsCore_GetAppType(void) {
  return _eAppType;
};

EClassicsPatchSeason ClassicsCore_GetSeason(void) {
  return _eSpecialEvent;
};

bool ClassicsCore_IsCustomModActive(void) {
  return _bClassicsPatchCustomMod;
};

void ClassicsCore_SetCustomMod(bool bState)
{
  // Don't let the state be changed
  static BOOL bCustomModSet = FALSE;
  if (bCustomModSet) return;
  bCustomModSet = TRUE;

#if !_PATCHCONFIG_CUSTOM_MOD
  bState = false;
#endif

  _bClassicsPatchCustomMod = bState;
};

EVerifyAPIResult ClassicsPatchAPI_VerifyInternal(PatchVer_t ulInterfaceVersion, ClassicsPatchErrMsg *pOutErrMsg) {
  // Classics Patch isn't currently running
  if (!_bClassicsPatchRunning) {
    strcpy(*pOutErrMsg, "Classics Patch hasn't been initialized yet");
    return k_EVerifyAPIResult_NotRunning;

  // Desired interface version is too new
  } else if (ulInterfaceVersion > CLASSICSPATCH_INTERFACE_VERSION) {
    strcpy(*pOutErrMsg, "Classics Patch API version is too old");
    return k_EVerifyAPIResult_VersionMismatch;
  }

  return k_EVerifyAPIResult_OK;
};

bool ClassicsPatchAPI_IsRunning(void) {
  return _bClassicsPatchRunning;
};

// Specify vanilla Bin directory as an extra DLL directory
static void SetVanillaBinDirectory(void) {
  // No need if the Bin folder is the same
  if (IDir::AppBin() == "Bin\\") return;

  // Load DLL directory method
  HINSTANCE hKernel = GetModuleHandleA("Kernel32.dll");

  // This should never happen, but still
  if (hKernel == NULL) return;

  typedef BOOL (*CSetDirFunc)(LPCSTR);
  CSetDirFunc pSetDirFunc = (CSetDirFunc)GetProcAddress(hKernel, "SetDllDirectoryA");

  // Set extra DLL directory
  if (pSetDirFunc != NULL) {
    pSetDirFunc((IDir::AppPath() + "Bin\\").str_String);
  }
};

void ClassicsPatch_Setup(EClassicsPatchAppType eApplicationType) {
  // Set application type
  _eAppType = eApplicationType;

  // Specify extra DLL directory
  SetVanillaBinDirectory();

  // Load configuration properties
  IConfig::global.Load();

  // Enable debug output for patcher actions
  bool bDebugPatcher = IConfig::global[k_EConfigProps_DebugPatcher].IsTrue();
  FuncPatch_SetDebug(bDebugPatcher);
};

// Physical interfaces
static CGameAPI *_pGameAPI = NULL;
static CPluginAPI *_pPluginAPI = NULL;
static CSteamAPI *_pSteamAPI = NULL;

CGameAPI   *GetGameAPI(void)   { return _pGameAPI; };
CPluginAPI *GetPluginAPI(void) { return _pPluginAPI; };
CSteamAPI  *GetSteamAPI(void)  { return _pSteamAPI; };

IClassicsGame    *ClassicsGame(void)    { return _pGameAPI; };
IClassicsPlugins *ClassicsPlugins(void) { return _pPluginAPI; };

// Non-physical interfaces
namespace IInitAPI {
  extern void Chat(void);
  extern void Config(void);
  extern void Core(void);
  extern void Hooks(void);
  extern void Patches(void);
};

void ClassicsPatch_Init(void)
{
  // Already running
  ASSERT(!_bClassicsPatchRunning);
  if (_bClassicsPatchRunning) return;
  _bClassicsPatchRunning = true;

  // Report patch version
  CPrintF("Initializing Serious Sam Classics Patch v%s for %s v%s...\n",
    ClassicsCore_GetVersionName(), CHOOSE_FOR_GAME("TFE", "TSE", "TSE"), _SE_VER_STRING);

  // Remember initialization time
  _tvClassicsPatchInit = _pTimer->GetHighPrecisionTimer();

  // Initial preparation
  {
    // Allow more characters in console by default
    ICore::ReinitConsole(160, 512);

    // Load custom include/exclude lists for mods
    if (_fnmMod != "") {
      IFiles::LoadStringList(_aBaseWriteInc, CTString("BaseWriteInclude.lst"));
      IFiles::LoadStringList(_aBaseWriteExc, CTString("BaseWriteExclude.lst"));
      IFiles::LoadStringList(_aBaseBrowseInc, CTString("BaseBrowseInclude.lst"));
      IFiles::LoadStringList(_aBaseBrowseExc, CTString("BaseBrowseExclude.lst"));
    }

    // Disable custom mod if it was never set
    ClassicsCore_SetCustomMod(false);

    // Force a specific seasonal event
    const INDEX iForceEvent = IConfig::global[k_EConfigProps_ForceSeasonalEvent].GetIndex();

    if (iForceEvent >= 0 && iForceEvent < k_EClassicsPatchSeason_Max) {
      _eSpecialEvent = (EClassicsPatchSeason)iForceEvent;

    // Determine current seasonal event from local time
    } else {
      time_t iTime;
      time(&iTime);
      tm *tmLocal = localtime(&iTime);

      const int iDay = tmLocal->tm_mday;

      switch (tmLocal->tm_mon) {
        case  0: if (              iDay <= 15) _eSpecialEvent = k_EClassicsPatchSeason_Christmas;   break; // January
        case  1: if (iDay >= 10 && iDay <= 18) _eSpecialEvent = k_EClassicsPatchSeason_Valentine;   break; // February
        case  2: if (iDay >= 19 && iDay <= 23) _eSpecialEvent = k_EClassicsPatchSeason_Birthday;    break; // March
        case  5: if (iDay >= 20 && iDay <= 24) _eSpecialEvent = k_EClassicsPatchSeason_Anniversary; break; // June
        case  9: /** Everyday is Halloween **/ _eSpecialEvent = k_EClassicsPatchSeason_Halloween;   break; // October
        case 11: if (iDay >= 15              ) _eSpecialEvent = k_EClassicsPatchSeason_Christmas;   break; // December
      }
    }
  }

  // Initialize interfaces
  IInitAPI::Chat();
  IInitAPI::Config();
  IInitAPI::Core();
  IInitAPI::Hooks();
  IInitAPI::Patches();
  _pGameAPI = new CGameAPI;
  _pPluginAPI = new CPluginAPI;
  _pSteamAPI = new CSteamAPI;

  // Various initializations
  {
    // Initialize networking
    INetwork::Initialize();

    // Initialize Steam API
    GetSteamAPI()->Init();

    // Load core plugins
    GetPluginAPI()->LoadPlugins(k_EPluginFlagEngine);
  }

  // Create timer handler for constant functionatily
  _pTimerHandler = new CCoreTimerHandler;
  _pTimer->AddHandler(_pTimerHandler);
};

void ClassicsPatch_Shutdown(void)
{
  // Not running yet
  ASSERT(_bClassicsPatchRunning);
  if (!_bClassicsPatchRunning) return;
  _bClassicsPatchRunning = false;

  // Clean up the patches
  _EnginePatches.Cleanup();

  // Destroy timer handler
  _pTimer->RemHandler(_pTimerHandler);
  delete _pTimerHandler;

  // Various cleanups
  {
    // Save configuration properties
    IConfig::global.Save();

    // [Cecil] NOTE: Forcefully releasing all potentially used plugins because it's already painful enough to load them with
    // the appropriate flags from wherever I need, so I'm not even gonna bother freeing them with the same flags manually.
    // At this point nothing should be using the patch (or the plugins) anyway since it's being shut down.

    // Release all loaded plugins
    GetPluginAPI()->ReleasePlugins(k_EPluginFlagAll, TRUE);

    // Shutdown Steam API
    GetSteamAPI()->End();
  }

  // Destroy interfaces
  delete _pGameAPI;   _pGameAPI   = NULL;
  delete _pPluginAPI; _pPluginAPI = NULL;
  delete _pSteamAPI;  _pSteamAPI  = NULL;
};

// Internal API with all the interfaces
class CInternalClassicsPatchAPI : public IClassicsPatchAPI {
  public:
    IClassicsChat m_apiChat;
    IClassicsConfig m_apiConfig;
    IClassicsGameplayExt m_apiGameplayExt;
    IClassicsCore m_apiCore;
    IClassicsModData m_apiModData;
    IClassicsFuncPatches m_apiFuncPatches;
    IClassicsPackets m_apiPackets;
    IClassicsExtensions m_apiExtensions;
    IClassicsINI m_apiINI;

  public:
    virtual IClassicsChat *Chat(void) { return &m_apiChat; };
    virtual IClassicsConfig *Config(void) { return &m_apiConfig; };
    virtual IClassicsGameplayExt *GameplayExt(void) { return &m_apiGameplayExt; };
    virtual IClassicsCore *Core(void) { return &m_apiCore; };
    virtual IClassicsGame *Game(void) { return _pGameAPI; };
    virtual IClassicsModData *ModData(void) { return &m_apiModData; };
    virtual IClassicsPlugins *Plugins(void) { return _pPluginAPI; };
    virtual IClassicsFuncPatches *FuncPatches(void) { return &m_apiFuncPatches; };
    virtual IClassicsPackets *Packets(void) { return &m_apiPackets; };
    virtual IClassicsExtensions *Extensions(void) { return &m_apiExtensions; };
    virtual IClassicsINI *INI(void) { return &m_apiINI; };
};

static CInternalClassicsPatchAPI _ClassicsPatchAPI;

IClassicsPatchAPI *ClassicsPatchAPI(void) {
  return &_ClassicsPatchAPI;
};
