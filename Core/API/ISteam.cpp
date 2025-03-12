/* Copyright (c) 2023-2025 Dreamy Cecil
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

#if _PATCHCONFIG_STEAM_API

// [Cecil] NOTE: Delay loaded, meaning that the library is actually hooked upon calling any function for the first time
#pragma comment(lib, "../Extras/Steamworks/redistributable_bin/steam_api.lib")

#endif // _PATCHCONFIG_STEAM_API

static INDEX steam_bAllowJoin = TRUE; // Allow friends to join the same game
static INDEX steam_bUseWebBrowser = TRUE; // Allow using web browser in Steam overlay
static INDEX steam_bHookScreenshots = TRUE; // Allow making screenshots from within the game

// Display debug information about interactions with Steam
// 0 - Disabled
// 1 - Only callbacks/single actions
// 2 - All interactions in real time
static INDEX steam_iDebugOutput = 0;

// Debug output macros
#define STEAM_DEBUG1 if (steam_iDebugOutput >= 1) CPrintF
#define STEAM_DEBUG2 if (steam_iDebugOutput >= 2) CPrintF

// Drawport for making Steam screenshots from within the game
static CDrawPort *_pdpScreenshot = NULL;

// Constructor
CSteamAPI::CSteamAPI() {
  Reset();

  // Commands for dynamically toggling Steam features
  _pShell->DeclareSymbol("persistent user INDEX steam_bAllowJoin;",       &steam_bAllowJoin);
  _pShell->DeclareSymbol("persistent user INDEX steam_bUseWebBrowser;",   &steam_bUseWebBrowser);
  _pShell->DeclareSymbol("persistent user INDEX steam_bHookScreenshots;", &steam_bHookScreenshots);

  _pShell->DeclareSymbol("persistent user INDEX steam_iDebugOutput;", &steam_iDebugOutput);
};

// Reset the API
void CSteamAPI::Reset(void) {
  bInitialized = FALSE;
  eApiState = k_ESteamAPIInitResult_NoSteamClient;

  bSteamOverlay = FALSE;
  strJoinCommandMidGame = "";

  iiScreenshot.Clear();
  bScreenshotRequested = FALSE;
};

// Initialize Steam API
void CSteamAPI::Init(void) {
#if _PATCHCONFIG_STEAM_API

  // Skip for dedicated servers
  if (ClassicsCore_IsServerApp()) {
    if (!IConfig::global[k_EConfigProps_SteamForServers]) return;

  // Skip for tools
  } else if (ClassicsCore_IsEditorApp() || ClassicsCore_IsModelerApp()) {
    if (!IConfig::global[k_EConfigProps_SteamForTools]) return;

  // Skip for unknown non-game applications
  } else if (!ClassicsCore_IsGameApp()) {
    return;
  }

  // Steam disabled or already initialized
  if (!IConfig::global[k_EConfigProps_SteamEnable] || bInitialized) return;
  bInitialized = TRUE;

  // Check if the module is even available
  HINSTANCE hApiLib = LoadLibraryA("steam_api.dll");

  if (hApiLib == NULL) {
    CPrintF("Failed to load 'steam_api.dll': %s\n", GetWindowsError(GetLastError()));
    return;
  }

  CPutString("Successfully loaded 'steam_api.dll'!\n");

  // Create file with Steam application ID
  static BOOL bAppIdFile = TRUE;

  if (bAppIdFile) {
    bAppIdFile = FALSE;

    FILE *file = fopen("steam_appid.txt", "w");

    if (file != NULL) {
      #define SAM_APP_ID CHOOSE_FOR_GAME("41050", "41060", "41060")
      fprintf(file, SAM_APP_ID);
      fclose(file);

      CPrintF(TRANS("Successfully created 'steam_appid.txt' with %s app ID!\n"), SAM_APP_ID);

    } else {
      CPrintF(TRANS("Cannot create 'steam_appid.txt': %s\n"), GetWindowsError(GetLastError()));
    }
  }

  // Try to initialize Steam API
  CPutString("Initializing Steam API... ");

  SteamErrMsg strError;
  eApiState = SteamAPI_InitEx(&strError);

  if (eApiState != k_ESteamAPIInitResult_OK) {
    CPrintF("Failed:\n  %s\n", strError);
    return;
  }

  CPutString("OK!\n");

  // Register callbacks
  cbOnGameOverlayActivated.Register(this, &CSteamAPI::OnGameOverlayActivated);
  cbOnGameJoinRequested.Register(this, &CSteamAPI::OnGameJoinRequested);
  cbOnScreenshotRequested.Register(this, &CSteamAPI::OnScreenshotRequested);
  cbOnScreenshotReady.Register(this, &CSteamAPI::OnScreenshotReady);

#else
  CPutString("Steam API is disabled in this build!\n");
#endif // _PATCHCONFIG_STEAM_API
};

// Shutdown Steam API
void CSteamAPI::End(void) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;

  // Unregister callbacks
  cbOnGameOverlayActivated.Unregister();
  cbOnGameJoinRequested.Unregister();
  cbOnScreenshotRequested.Unregister();
  cbOnScreenshotReady.Unregister();

  // Shut down Steam API
  CPutString("Shutting down Steam API... ");
  SteamAPI_Shutdown();
  CPutString("OK!\n");

#endif // _PATCHCONFIG_STEAM_API

  Reset();
};

// Check if Steam has been initialized and can be used
BOOL CSteamAPI::IsUsable(void) {
#if _PATCHCONFIG_STEAM_API
  // Enabled and initialized correctly
  return (IConfig::global[k_EConfigProps_SteamEnable] && bInitialized && eApiState == k_ESteamAPIInitResult_OK);

#else
  return FALSE;
#endif
};

// Interact with Steam once in a while
void CSteamAPI::Update(void) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;

  // Update every 1/20 of a second
  static CTimerValue _tvLastCheck(-1.0f);
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  if ((tvNow - _tvLastCheck).GetSeconds() < 0.05f) return;
  _tvLastCheck = tvNow;

  STEAM_DEBUG2("----- CSteamAPI::Update() - %fs -----\n", tvNow.GetSeconds());

  // Update Steam callbacks
  GetSteamAPI()->UpdateCallbacks();

  if (ClassicsCore_IsGameApp()) {
    // While connected to a server
    if (!_pNetwork->IsServer() && GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn()) {
      // Set address of the joined server
      CSymbolPtr piNetPort("net_iPort");
      CTString strAddress(0, "%s:%d", GetGameAPI()->JoinAddress(), piNetPort.GetIndex());
      GetSteamAPI()->SetJoinAddress(strAddress);

    } else {
      // Reset join address
      GetSteamAPI()->SetJoinAddress("");
    }
  }

  // If requested a custom screenshot and it's present
  if (bScreenshotRequested && iiScreenshot.ii_Picture != NULL) {
    // Write it and then reset requested screenshot
    WriteScreenshot(iiScreenshot);

    iiScreenshot.Clear();
    bScreenshotRequested = FALSE;
  }

#endif // _PATCHCONFIG_STEAM_API
};

// Set server address to be used in "Join game" option
void CSteamAPI::SetJoinAddress(const CTString &strAddress) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;
  STEAM_DEBUG2("CSteamAPI::SetJoinAddress(\"%s\") - ", strAddress);

  // Reset if needed
  if (!steam_bAllowJoin || strAddress == "") {
    SteamFriends()->SetRichPresence("connect", NULL);
    STEAM_DEBUG2("reset\n");
    return;
  }

  CTString strArgs(0, "+connect %s", strAddress);

  // Add mod argument
  if (_fnmMod != "") {
    // Get mod directory name by removing last slash and "Mods\\"
    CTString strCurrentMod = _fnmMod;
    strCurrentMod.str_String[strCurrentMod.Length() - 1] = '\0';
    strCurrentMod.RemovePrefix("Mods\\");

    strArgs += " +game " + strCurrentMod;
  }

  SteamFriends()->SetRichPresence("connect", strArgs);
  STEAM_DEBUG2("set to '%s'\n", strArgs);

#endif // _PATCHCONFIG_STEAM_API
};

// Activate Steam Overlay web browser directly to the specified URL
BOOL CSteamAPI::OpenWebPage(const char *strURL) {
  if (!steam_bUseWebBrowser || !IsUsable()) return FALSE;

#if _PATCHCONFIG_STEAM_API
  SteamFriends()->ActivateGameOverlayToWebPage(strURL);
  STEAM_DEBUG1("CSteamAPI::OpenWebPage(\"%s\") - opened webpage\n", strURL);
#endif

  return TRUE;
};

// Set drawport that will be used for making Steam screenshots from within the game
// This makes Steam send screenshot requests instead of capturing the entire game window automatically
void CSteamAPI::SetScreenshotHook(CDrawPort *pdpScreenshotSurface) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;
  _pdpScreenshot = pdpScreenshotSurface;

  // Update the hook state only if it doesn't match
  bool bHookScreenshots = (steam_bHookScreenshots && _pdpScreenshot != NULL);

  if (SteamScreenshots()->IsScreenshotsHooked() != bHookScreenshots) {
    SteamScreenshots()->HookScreenshots(bHookScreenshots);

    STEAM_DEBUG1("CSteamAPI::SetScreenshotHook(%s) - hook %s\n",
      (_pdpScreenshot != NULL ? "<valid drawport>" : "<no drawport>"),
      (bHookScreenshots ? "enabled" : "disabled"));
  }

#endif // _PATCHCONFIG_STEAM_API
};

// Make a custom Steam screenshot by manually writing a bitmap from image info
void CSteamAPI::WriteScreenshot(CImageInfo &ii) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;
  STEAM_DEBUG1("CSteamAPI::WriteScreenshot() - ");

  if (ii.ii_BitsPerPixel != 24) {
    STEAM_DEBUG1("ERROR: Screenshot is not in 24-bit format\n");
    return;
  }

  const PIX pixW = ii.ii_Width;
  const PIX pixH = ii.ii_Height;

  if (pixW <= 0 || pixH <= 0) {
    STEAM_DEBUG1("ERROR: Invalid screenshot dimensions\n");
    return;
  }

  const ULONG ulSize = pixW * pixH * (ii.ii_BitsPerPixel / 8);
  ScreenshotHandle hShot = SteamScreenshots()->WriteScreenshot(ii.ii_Picture, ulSize, pixW, pixH);

  // Set world display name as the location
  if (GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn()) {
    const CTString strLocation = IWorld::GetWorld()->GetName().Undecorated();

    if (strLocation != "") {
      // Convert name string in system locale (ANSI code page) to Unicode
      wchar_t wstrUnicode[k_cubUFSTagValueMax] = { 0 };
      MultiByteToWideChar(CP_ACP, 0, strLocation.str_String, -1, wstrUnicode, k_cubUFSTagValueMax);

      // Convert Unicode string to UTF-8
      char strUTF8[k_cubUFSTagValueMax] = { 0 };
      WideCharToMultiByte(CP_UTF8, 0, wstrUnicode, -1, strUTF8, k_cubUFSTagValueMax, NULL, NULL);

      SteamScreenshots()->SetLocation(hShot, strUTF8);
    }
  }

  STEAM_DEBUG1("OK: Wrote screenshot\n");

#endif // _PATCHCONFIG_STEAM_API
};

// Force Steam to take a screenshot (equivalent to manually pressing a screenshot button, e.g. F12)
void CSteamAPI::TriggerScreenshot(void) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return;
  STEAM_DEBUG1("CSteamAPI::TriggerScreenshot()\n");

  SteamScreenshots()->TriggerScreenshot();

#endif
};

// Display text input for game controllers in big picture mode that doesn't obstruct a text field of some size
BOOL CSteamAPI::ShowFloatingGamepadTextInput(PIX pixFieldX, PIX pixFieldY, PIX pixFieldW, PIX pixFieldH) {
#if _PATCHCONFIG_STEAM_API

  if (!IsUsable()) return FALSE;
  STEAM_DEBUG1("CSteamAPI::ShowFloatingGamepadTextInput(%d, %d, %d, %d) - ", pixFieldX, pixFieldY, pixFieldW, pixFieldH);

  bool bShown = SteamUtils()->ShowFloatingGamepadTextInput(
    k_EFloatingGamepadTextInputModeModeSingleLine, pixFieldX, pixFieldY, pixFieldW, pixFieldH);

  STEAM_DEBUG1(bShown ? "shown\n" : "not shown\n");
  return bShown;

#else
  return FALSE;
#endif
};

#if _PATCHCONFIG_STEAM_API

// Update Steam callbacks (should be called each frame/timer tick)
void CSteamAPI::UpdateCallbacks(void) {
  if (!IsUsable()) return;

  STEAM_DEBUG2("CSteamAPI::UpdateCallbacks() - running callbacks\n");
  SteamAPI_RunCallbacks();
};

void CSteamAPI::OnGameOverlayActivated(GameOverlayActivated_t *pCallback) {
  bSteamOverlay = pCallback->m_bActive;
  STEAM_DEBUG1("CSteamAPI::OnGameOverlayActivated() - %s\n", bSteamOverlay ? "activated" : "deactivated");
};

void CSteamAPI::OnGameJoinRequested(GameRichPresenceJoinRequested_t *pCallback) {
  strJoinCommandMidGame = pCallback->m_rgchConnect;
  STEAM_DEBUG1("CSteamAPI::OnGameJoinRequested() - received '%s'\n", strJoinCommandMidGame);
};

void CSteamAPI::OnScreenshotRequested(ScreenshotRequested_t *pCallback) {
  STEAM_DEBUG1("CSteamAPI::OnScreenshotRequested() - ");

  // Trust observer camera to take the actual screenshot
  if (GetGameAPI()->GetCamera().IsActive()) {
    // [Cecil] FIXME: If Steam screenshot button differs from the observer camera (either one is rebound)
    // and Steam's button is used, no screenshot will be made here because observer camera is never called
    bScreenshotRequested = TRUE;
    STEAM_DEBUG1("OK: Requested a screenshot from observer camera\n");
    return;
  }

  if (_pdpScreenshot == NULL) {
    STEAM_DEBUG1("ERROR: No drawport to make a screenshot from\n");
    return;
  }

  // Take a new screenshot
  _pdpScreenshot->GrabScreen(iiScreenshot, 0);
  bScreenshotRequested = TRUE;

  STEAM_DEBUG1("OK: Took a new screenshot\n");
};

void CSteamAPI::OnScreenshotReady(ScreenshotReady_t *pCallback) {
  const char *strHandle = (pCallback->m_hLocal != INVALID_SCREENSHOT_HANDLE ? "<valid handle>" : "<no handle>");
  const char *strResult = "ERROR: Unknown result";

  switch (pCallback->m_eResult) {
    case k_EResultOK:        strResult = "OK: Successfully added screenshot to the library"; break;
    case k_EResultFail:      strResult = "ERROR: Couldn't load or parse the screenshot"; break;
    case k_EResultIOFailure: strResult = "ERROR: Couldn't save the screenshot to the disk"; break;
  }

  STEAM_DEBUG1("CSteamAPI::OnScreenshotReady(%s) - %s\n", strHandle, strResult);
};

#endif // _PATCHCONFIG_STEAM_API
