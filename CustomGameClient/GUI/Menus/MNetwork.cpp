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
#include "MNetwork.h"

static CTFileName _fnGameToLoad;

static void StartNetworkLoadGame(void) {
  GetGameAPI()->SetStartSplitCfg(GetGameAPI()->GetMenuSplitCfg());

  // [Cecil] Set start players from menu players
  GetGameAPI()->SetStartProfilesFromMenuProfiles();

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_SERVER);

  if (_pGame->LoadGame(_fnGameToLoad)) {
    StopMenus();
    _gmRunningGameMode = GM_NETWORK;

  } else {
    _gmRunningGameMode = GM_NONE;
  }
};

static BOOL LSLoadNetwork(const CTFileName &fnm) {
  // call local players menu
  _fnGameToLoad = fnm;

  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  gmCurrent.gm_ulConfigFlags = PLCF_OBSERVING;
  gmCurrent.gm_mgStart.mg_pActivatedFunction = &StartNetworkLoadGame;
  ChangeToMenu(&gmCurrent);
  return TRUE;
};

void StartNetworkQuickLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_NETWORK;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("QUICK LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\Network\\Quick\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadNetwork;

  extern void SetQuickLoadNotes(void);
  SetQuickLoadNotes();

  ChangeToMenu(&gmCurrent);
};

void StartNetworkLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_NETWORK;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\Network\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadNetwork;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

void CNetworkMenu::Initialize_t(void) {
  gm_strName = "Network";
  gm_pmgSelectedByDefault = &gm_mgJoin;

  // intialize network menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("NETWORK"));
  AddChild(&gm_mgTitle);

  gm_mgJoin.mg_bfsFontSize = BFS_LARGE;
  gm_mgJoin.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgJoin.mg_pmgUp = &gm_mgLoad;
  gm_mgJoin.mg_pmgDown = &gm_mgStart;
  gm_mgJoin.SetText(LOCALIZE("JOIN GAME"));
  gm_mgJoin.mg_strTip = LOCALIZE("join a network game");
  AddChild(&gm_mgJoin);
  gm_mgJoin.mg_pActivatedFunction = &CNetworkJoinMenu::ChangeTo;

  gm_mgStart.mg_bfsFontSize = BFS_LARGE;
  gm_mgStart.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgStart.mg_pmgUp = &gm_mgJoin;
  gm_mgStart.mg_pmgDown = &gm_mgQuickLoad;
  gm_mgStart.SetText(LOCALIZE("START SERVER"));
  gm_mgStart.mg_strTip = LOCALIZE("start a network game server");
  AddChild(&gm_mgStart);
  gm_mgStart.mg_pActivatedFunction = &CNetworkStartMenu::ChangeTo;

  gm_mgQuickLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuickLoad.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgQuickLoad.mg_pmgUp = &gm_mgStart;
  gm_mgQuickLoad.mg_pmgDown = &gm_mgLoad;
  gm_mgQuickLoad.SetText(LOCALIZE("QUICK LOAD"));
  gm_mgQuickLoad.mg_strTip = LOCALIZE("load a quick-saved game (F9)");
  AddChild(&gm_mgQuickLoad);
  gm_mgQuickLoad.mg_pActivatedFunction = &StartNetworkQuickLoadMenu;

  gm_mgLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgLoad.mg_boxOnScreen = BoxBigRow(4.0f);
  gm_mgLoad.mg_pmgUp = &gm_mgQuickLoad;
  gm_mgLoad.mg_pmgDown = &gm_mgJoin;
  gm_mgLoad.SetText(LOCALIZE("LOAD"));
  gm_mgLoad.mg_strTip = LOCALIZE("start server and load a network game (server only)");
  AddChild(&gm_mgLoad);
  gm_mgLoad.mg_pActivatedFunction = &StartNetworkLoadMenu;
}

void CNetworkMenu::StartMenu(void) {
  CGameMenu::StartMenu();
}

// [Cecil] Change to the menu
void CNetworkMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmNetworkMenu);
};
