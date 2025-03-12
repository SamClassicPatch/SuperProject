/* Copyright (c) 2002-2012 Croteam Ltd.
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
#include "MenuPrinting.h"
#include "MOptions.h"

extern BOOL _bPlayerMenuFromSinglePlayer;
extern CTString sam_strNetworkSettings;

static void StartVideoOptionsMenu(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\VideoOptions_ClassicsPatch.cfg");
  CVarMenu::ChangeTo(LOCALIZE("VIDEO"), fnmConfig, "VideoOptions");
};

static void StartAudioOptionsMenu(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\AudioOptions_ClassicsPatch.cfg");
  CVarMenu::ChangeTo(LOCALIZE("AUDIO"), fnmConfig, "AudioOptions");
};

static void StartChangePlayerMenuFromOptions(void) {
  _bPlayerMenuFromSinglePlayer = FALSE;
  _pGUIM->gmPlayerProfile.gm_piCurrentPlayer = GetGameAPI()->piSinglePlayer;
  ChangeToMenu(&_pGUIM->gmPlayerProfile);
};

// [Cecil] Menu to switch to after selecting a network setting
CGameMenu *_pgmAfterNetSetting = NULL;

static BOOL LSLoadNetSettings(const CTFileName &fnm) {
  sam_strNetworkSettings = fnm;
  CTString strCmd;
  strCmd.PrintF("include \"%s\"", (const char *)sam_strNetworkSettings);
  _pShell->Execute(strCmd);

  // Exit network settings menu
  MenuGoToParent();

  // [Cecil] And then switch to the next menu
  if (_pgmAfterNetSetting != NULL) {
    ChangeToMenu(_pgmAfterNetSetting);
    _pgmAfterNetSetting = NULL;
  }

  return TRUE;
};

void StartNetworkSettingsMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("CONNECTION SETTINGS"));
  gmCurrent.gm_bAllowThumbnails = FALSE;
  gmCurrent.gm_iSortType = LSSORT_FILEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString("Scripts\\NetSettings\\");
  gmCurrent.gm_strSelected = sam_strNetworkSettings;
  gmCurrent.gm_fnmExt = CTString(".ini");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadNetSettings;

  if (sam_strNetworkSettings == "") {
    gmCurrent.gm_mgNotes.SetText(LOCALIZE(
      "Before joining a network game,\n"
      "you have to adjust your connection parameters.\n"
      "Choose one option from the list.\n"
      "If you have problems with connection, you can adjust\n"
      "these parameters again from the Options menu.\n"));
  } else {
    gmCurrent.gm_mgNotes.SetText("");
  }

  ChangeToMenu(&gmCurrent);

  // [Cecil] Reset next menu (can be configured after calling this whole function)
  _pgmAfterNetSetting = NULL;
};

// [Cecil] Load config settings from the patch
static BOOL LSLoadPatchConfig(const CTFileName &fnm) {
  CVarMenu::ChangeTo(TRANS("CLASSICS PATCH"), fnm);
  return TRUE;
};

// [Cecil] Open list of option configs from the patch
static void StartPatchOptionsMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(TRANS("CLASSICS PATCH"));
  gmCurrent.gm_bAllowThumbnails = FALSE;
  gmCurrent.gm_iSortType = LSSORT_FILEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString("Scripts\\ClassicsPatch\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".cfg");
  // Ignore lists to prevents mods from hiding patch options when excluding the entire "Scripts" directory
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD | FLF_IGNORELISTS;
  gmCurrent.gm_pAfterFileChosen = &LSLoadPatchConfig;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

static BOOL LSLoadCustom(const CTFileName &fnm) {
  CVarMenu::ChangeTo(LOCALIZE("ADVANCED OPTIONS"), fnm);
  return TRUE;
};

static void StartCustomLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("ADVANCED OPTIONS"));
  gmCurrent.gm_bAllowThumbnails = FALSE;
  gmCurrent.gm_iSortType = LSSORT_NAMEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString("Scripts\\CustomOptions\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".cfg");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadCustom;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

static BOOL LSLoadAddon(const CTFileName &fnm) {
  extern INDEX _iAddonExecState;
  extern CTFileName _fnmAddonToExec;
  _iAddonExecState = 1;
  _fnmAddonToExec = fnm;
  return TRUE;
};

static void StartAddonsLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("EXECUTE ADDON"));
  gmCurrent.gm_bAllowThumbnails = FALSE;
  gmCurrent.gm_iSortType = LSSORT_NAMEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString(SCRIPTS_ADDONS_DIR);
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".ini");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadAddon;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

void COptionsMenu::Initialize_t(void) {
  gm_strName = "Options";
  gm_pmgSelectedByDefault = &gm_mgVideoOptions;

  // intialize options menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("OPTIONS"));
  AddChild(&gm_mgTitle);

  gm_mgVideoOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgVideoOptions.mg_boxOnScreen = BoxBigRow(0.0f);
  gm_mgVideoOptions.mg_pmgUp = &gm_mgAddonOptions;
  gm_mgVideoOptions.mg_pmgDown = &gm_mgAudioOptions;
  gm_mgVideoOptions.SetText(LOCALIZE("VIDEO OPTIONS"));
  gm_mgVideoOptions.mg_strTip = LOCALIZE("set video mode and driver");
  AddChild(&gm_mgVideoOptions);
  gm_mgVideoOptions.mg_pActivatedFunction = &StartVideoOptionsMenu;

  gm_mgAudioOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgAudioOptions.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgAudioOptions.mg_pmgUp = &gm_mgVideoOptions;
  gm_mgAudioOptions.mg_pmgDown = &gm_mgPlayerProfileOptions;
  gm_mgAudioOptions.SetText(LOCALIZE("AUDIO OPTIONS"));
  gm_mgAudioOptions.mg_strTip = LOCALIZE("set audio quality and volume");
  AddChild(&gm_mgAudioOptions);
  gm_mgAudioOptions.mg_pActivatedFunction = &StartAudioOptionsMenu;

  gm_mgPlayerProfileOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgPlayerProfileOptions.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgPlayerProfileOptions.mg_pmgUp = &gm_mgAudioOptions;
  gm_mgPlayerProfileOptions.mg_pmgDown = &gm_mgNetworkOptions;
  gm_mgPlayerProfileOptions.SetText(LOCALIZE("PLAYERS AND CONTROLS"));
  gm_mgPlayerProfileOptions.mg_strTip = LOCALIZE("change currently active player or adjust controls");
  AddChild(&gm_mgPlayerProfileOptions);
  gm_mgPlayerProfileOptions.mg_pActivatedFunction = &StartChangePlayerMenuFromOptions;

  gm_mgNetworkOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgNetworkOptions.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgNetworkOptions.mg_pmgUp = &gm_mgPlayerProfileOptions;
  gm_mgNetworkOptions.mg_pmgDown = &gm_mgPatchOptions;
  gm_mgNetworkOptions.SetText(LOCALIZE("NETWORK CONNECTION"));
  gm_mgNetworkOptions.mg_strTip = LOCALIZE("choose your connection parameters");
  AddChild(&gm_mgNetworkOptions);
  gm_mgNetworkOptions.mg_pActivatedFunction = &StartNetworkSettingsMenu;

  // [Cecil] Patch settings
  gm_mgPatchOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgPatchOptions.mg_boxOnScreen = BoxBigRow(4.0f);
  gm_mgPatchOptions.mg_pmgUp = &gm_mgNetworkOptions;
  gm_mgPatchOptions.mg_pmgDown = &gm_mgCustomOptions;
  gm_mgPatchOptions.SetText(TRANS("CLASSICS PATCH"));
  gm_mgPatchOptions.mg_strTip = TRANS("settings for the classics patch");
  AddChild(&gm_mgPatchOptions);
  gm_mgPatchOptions.mg_pActivatedFunction = &StartPatchOptionsMenu;

  gm_mgCustomOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgCustomOptions.mg_boxOnScreen = BoxBigRow(5.0f);
  gm_mgCustomOptions.mg_pmgUp = &gm_mgPatchOptions;
  gm_mgCustomOptions.mg_pmgDown = &gm_mgAddonOptions;
  gm_mgCustomOptions.SetText(LOCALIZE("ADVANCED OPTIONS"));
  gm_mgCustomOptions.mg_strTip = LOCALIZE("for advanced users only");
  AddChild(&gm_mgCustomOptions);
  gm_mgCustomOptions.mg_pActivatedFunction = &StartCustomLoadMenu;

  gm_mgAddonOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgAddonOptions.mg_boxOnScreen = BoxBigRow(6.0f);
  gm_mgAddonOptions.mg_pmgUp = &gm_mgCustomOptions;
  gm_mgAddonOptions.mg_pmgDown = &gm_mgVideoOptions;
  gm_mgAddonOptions.SetText(LOCALIZE("EXECUTE ADDON"));
  gm_mgAddonOptions.mg_strTip = LOCALIZE("choose from list of addons to execute");
  AddChild(&gm_mgAddonOptions);
  gm_mgAddonOptions.mg_pActivatedFunction = &StartAddonsLoadMenu;
}

// [Cecil] Change to the menu
void COptionsMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmOptionsMenu);
};
