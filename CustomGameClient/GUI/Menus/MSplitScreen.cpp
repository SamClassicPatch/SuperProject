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
#include "MSplitScreen.h"

static CTFileName _fnGameToLoad;

static void StartSplitScreenGameLoad(void) {
  GetGameAPI()->SetStartSplitCfg(GetGameAPI()->GetMenuSplitCfg());

  // [Cecil] Set start players from menu players
  GetGameAPI()->SetStartProfilesFromMenuProfiles();

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_LOCAL);

  if (_pGame->LoadGame(_fnGameToLoad)) {
    StopMenus();
    _gmRunningGameMode = GM_SPLIT_SCREEN;

  } else {
    _gmRunningGameMode = GM_NONE;
  }
}

static BOOL LSLoadSplitScreen(const CTFileName &fnm) {
  // call local players menu
  _fnGameToLoad = fnm;

  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  gmCurrent.gm_ulConfigFlags = 0;
  gmCurrent.gm_mgStart.mg_pActivatedFunction = &StartSplitScreenGameLoad;
  ChangeToMenu(&gmCurrent);
  return TRUE;
};

void StartSplitScreenQuickLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_SPLIT_SCREEN;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("QUICK LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\SplitScreen\\Quick\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadSplitScreen;

  extern void SetQuickLoadNotes(void);
  SetQuickLoadNotes();

  ChangeToMenu(&gmCurrent);
};

void StartSplitScreenLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_SPLIT_SCREEN;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\SplitScreen\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadSplitScreen;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

void CSplitScreenMenu::Initialize_t(void) {
  gm_strName = "SplitScreen";
  gm_pmgSelectedByDefault = &gm_mgStart;

  // intialize split screen menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("SPLIT SCREEN"));
  AddChild(&gm_mgTitle);

  gm_mgStart.mg_bfsFontSize = BFS_LARGE;
  gm_mgStart.mg_boxOnScreen = BoxBigRow(0);
  gm_mgStart.mg_pmgUp = &gm_mgLoad;
  gm_mgStart.mg_pmgDown = &gm_mgQuickLoad;
  gm_mgStart.SetText(LOCALIZE("NEW GAME"));
  gm_mgStart.mg_strTip = LOCALIZE("start new split-screen game");
  AddChild(&gm_mgStart);
  gm_mgStart.mg_pActivatedFunction = &CSplitStartMenu::ChangeTo;

  gm_mgQuickLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuickLoad.mg_boxOnScreen = BoxBigRow(1);
  gm_mgQuickLoad.mg_pmgUp = &gm_mgStart;
  gm_mgQuickLoad.mg_pmgDown = &gm_mgLoad;
  gm_mgQuickLoad.SetText(LOCALIZE("QUICK LOAD"));
  gm_mgQuickLoad.mg_strTip = LOCALIZE("load a quick-saved game (F9)");
  AddChild(&gm_mgQuickLoad);
  gm_mgQuickLoad.mg_pActivatedFunction = &StartSplitScreenQuickLoadMenu;

  gm_mgLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgLoad.mg_boxOnScreen = BoxBigRow(2);
  gm_mgLoad.mg_pmgUp = &gm_mgQuickLoad;
  gm_mgLoad.mg_pmgDown = &gm_mgStart;
  gm_mgLoad.SetText(LOCALIZE("LOAD"));
  gm_mgLoad.mg_strTip = LOCALIZE("load a saved split-screen game");
  AddChild(&gm_mgLoad);
  gm_mgLoad.mg_pActivatedFunction = &StartSplitScreenLoadMenu;
}

void CSplitScreenMenu::StartMenu(void) {
  CGameMenu::StartMenu();
}

// [Cecil] Change to the menu
void CSplitScreenMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmSplitScreenMenu);
};
