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
#include "MenuStuff.h"
#include "MSinglePlayer.h"
#include "LevelInfo.h"

extern CTString sam_strTrainingLevel;
extern CTString sam_strTechTestLevel;
extern BOOL _bPlayerMenuFromSinglePlayer;

extern void (*_pAfterLevelChosen)(void);

static void StartSinglePlayerNewMenu(void) {
  GetGameAPI()->SetCustomLevel(sam_strFirstLevel);
  ChangeToMenu(&_pGUIM->gmSinglePlayerNewMenu);
};

// [Cecil] Open level or category selection screen
void StartSelectLevel(ULONG ulFlags, void (*pAfterChosen)(void), CGameMenu *pgmParent) {
  // Set levels with appropriate gamemode flags
  _pGUIM->gmLevelsMenu.gm_ulSpawnFlags = ulFlags;

  // Go to level categories, if there are any
  CGameMenu *pgmCurrent = &_pGUIM->gmLevelsMenu;

  if (sam_bLevelCategories && _aLevelCategories.Count() != 0) {
    pgmCurrent = &_pGUIM->gmLevelCategories;
  }

  _pAfterLevelChosen = pAfterChosen;

  // [Cecil] Rewind visited menus to the parent
  extern CGameMenu *_pgmRewindToAfterLevelChosen;
  _pgmRewindToAfterLevelChosen = pgmParent;

  ChangeToMenu(pgmCurrent);
};

static void StartSelectLevelFromSingle(void) {
  // [Cecil] Select singleplayer levels
  StartSelectLevel(SPF_SINGLEPLAYER, &CSinglePlayerNewMenu::ChangeTo, NULL);
};

static BOOL LSLoadSinglePlayer(const CTFileName &fnm) {
  GetGameAPI()->SetStartSplitCfg(CGame::SSC_PLAY1);

  // [Cecil] Reset start player indices
  GetGameAPI()->ResetStartProfiles();
  GetGameAPI()->SetProfileForStart(0, GetGameAPI()->GetProfileForSP());

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_LOCAL);

  if (_pGame->LoadGame(fnm)) {
    StopMenus();
    _gmRunningGameMode = GM_SINGLE_PLAYER;

  } else {
    _gmRunningGameMode = GM_NONE;
  }
  return TRUE;
};

void SetQuickLoadNotes(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  if (_pShell->GetINDEX("gam_iQuickSaveSlots") <= 8) {
    gmCurrent.gm_mgNotes.SetText(LOCALIZE(
      "In-game QuickSave shortcuts:\n"
      "F6 - save a new QuickSave\n"
      "F9 - load the last QuickSave\n"));
  } else {
    gmCurrent.gm_mgNotes.SetText("");
  }
};

void StartSinglePlayerQuickLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_SINGLE_PLAYER;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("QUICK LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory.PrintF("SaveGame\\Player%d\\Quick\\", GetGameAPI()->GetProfileForSP());
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadSinglePlayer;

  SetQuickLoadNotes();

  ChangeToMenu(&gmCurrent);
};

void StartSinglePlayerLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_SINGLE_PLAYER;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("LOAD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory.PrintF("SaveGame\\Player%d\\", GetGameAPI()->GetProfileForSP());
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadSinglePlayer;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

// same function for saving in singleplay, network and splitscreen
BOOL LSSaveAnyGame(const CTFileName &fnm) {
  if (_pGame->SaveGame(fnm)) {
    StopMenus();
    return TRUE;
  }

  return FALSE;
};

void StartSinglePlayerSaveMenu(void) {
  if (_gmRunningGameMode != GM_SINGLE_PLAYER) {
    return;
  }

  // if no live players
  if (_pGame->GetPlayersCount() > 0 && _pGame->GetLivePlayersCount() <= 0) {
    // do nothing
    return;
  }

  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_SINGLE_PLAYER;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("SAVE"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = TRUE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory.PrintF("SaveGame\\Player%d\\", GetGameAPI()->GetProfileForSP());
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmBaseName = CTString("SaveGame");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSSaveAnyGame;
  gmCurrent.gm_mgNotes.SetText("");
  gmCurrent.gm_strSaveDes = _pGame->GetDefaultGameDescription(TRUE);

  ChangeToMenu(&gmCurrent);
};

static void StartTraining(void) {
  GetGameAPI()->SetCustomLevel(sam_strTrainingLevel);
  ChangeToMenu(&_pGUIM->gmSinglePlayerNewMenu);
};

void StartSinglePlayerGame(void) {
  GetGameAPI()->SetStartSplitCfg(CGame::SSC_PLAY1);

  // [Cecil] Reset start player indices
  GetGameAPI()->ResetStartProfiles();
  GetGameAPI()->SetProfileForStart(0, GetGameAPI()->GetProfileForSP());

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_LOCAL);

  // [Cecil] Pass byte container
  CSesPropsContainer sp;
  _pGame->SetSinglePlayerSession((CSessionProperties &)sp);

  // [Cecil] Start game through the API
  if (GetGameAPI()->NewGame(GetGameAPI()->GetCustomLevel(), GetGameAPI()->GetCustomLevel(), (CSessionProperties &)sp)) {
    StopMenus();
    _gmRunningGameMode = GM_SINGLE_PLAYER;

  } else {
    _gmRunningGameMode = GM_NONE;
  }
};

static void StartTechTest(void) {
  GetGameAPI()->SetCustomLevel(sam_strTechTestLevel);

  // [Cecil] Use difficulties and game modes from the API
  _pShell->SetINDEX("gam_iStartDifficulty", ClassicsModData_GetDiff(2)->m_iLevel);
  _pShell->SetINDEX("gam_iStartMode", GetGameAPI()->GetGameMode(1));
  StartSinglePlayerGame();
};

static void StartChangePlayerMenuFromSinglePlayer(void) {
  _iLocalPlayer = -1;
  _bPlayerMenuFromSinglePlayer = TRUE;
  _pGUIM->gmPlayerProfile.gm_piCurrentPlayer = GetGameAPI()->piSinglePlayer;
  ChangeToMenu(&_pGUIM->gmPlayerProfile);
};

static void StartSinglePlayerGameOptions(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\SPOptions.cfg");
  CVarMenu::ChangeTo(LOCALIZE("GAME OPTIONS"), fnmConfig);
};

void CSinglePlayerMenu::Initialize_t(void) {
  gm_strName = "SinglePlayer";
  gm_pmgSelectedByDefault = &gm_mgNewGame;

  // intialize single player menu
  gm_mgTitle.SetName(LOCALIZE("SINGLE PLAYER"));
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  gm_mgPlayerLabel.mg_boxOnScreen = BoxBigRow(-1.0f);
  gm_mgPlayerLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgPlayerLabel.mg_iCenterI = -1;
  gm_mgPlayerLabel.mg_bEnabled = FALSE;
  gm_mgPlayerLabel.mg_bLabel = TRUE;
  AddChild(&gm_mgPlayerLabel);

  gm_mgNewGame.SetText(LOCALIZE("NEW GAME"));
  gm_mgNewGame.mg_bfsFontSize = BFS_LARGE;
  gm_mgNewGame.mg_boxOnScreen = BoxBigRow(0.0f);
  gm_mgNewGame.mg_strTip = LOCALIZE("start new game with current player");
  AddChild(&gm_mgNewGame);
  gm_mgNewGame.mg_pmgUp = &gm_mgOptions;
  gm_mgNewGame.mg_pmgDown = &gm_mgCustom;
  gm_mgNewGame.mg_pActivatedFunction = &StartSinglePlayerNewMenu;

  gm_mgCustom.SetText(LOCALIZE("CUSTOM LEVEL"));
  gm_mgCustom.mg_bfsFontSize = BFS_LARGE;
  gm_mgCustom.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgCustom.mg_strTip = LOCALIZE("start new game on a custom level");
  AddChild(&gm_mgCustom);
  gm_mgCustom.mg_pmgUp = &gm_mgNewGame;
  gm_mgCustom.mg_pmgDown = &gm_mgQuickLoad;
  gm_mgCustom.mg_pActivatedFunction = &StartSelectLevelFromSingle;

  gm_mgQuickLoad.SetText(LOCALIZE("QUICK LOAD"));
  gm_mgQuickLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuickLoad.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgQuickLoad.mg_strTip = LOCALIZE("load a quick-saved game (F9)");
  AddChild(&gm_mgQuickLoad);
  gm_mgQuickLoad.mg_pmgUp = &gm_mgCustom;
  gm_mgQuickLoad.mg_pmgDown = &gm_mgLoad;
  gm_mgQuickLoad.mg_pActivatedFunction = &StartSinglePlayerQuickLoadMenu;

  gm_mgLoad.SetText(LOCALIZE("LOAD"));
  gm_mgLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgLoad.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgLoad.mg_strTip = LOCALIZE("load a saved game of current player");
  AddChild(&gm_mgLoad);
  gm_mgLoad.mg_pmgUp = &gm_mgQuickLoad;
  gm_mgLoad.mg_pmgDown = &gm_mgTraining;
  gm_mgLoad.mg_pActivatedFunction = &StartSinglePlayerLoadMenu;

  gm_mgTraining.SetText(LOCALIZE("TRAINING"));
  gm_mgTraining.mg_bfsFontSize = BFS_LARGE;
  gm_mgTraining.mg_boxOnScreen = BoxBigRow(4.0f);
  gm_mgTraining.mg_strTip = LOCALIZE("start training level - KarnakDemo");
  AddChild(&gm_mgTraining);
  gm_mgTraining.mg_pmgUp = &gm_mgLoad;
  gm_mgTraining.mg_pmgDown = &gm_mgTechTest;
  gm_mgTraining.mg_pActivatedFunction = &StartTraining;

  gm_mgTechTest.SetText(LOCALIZE("TECHNOLOGY TEST"));
  gm_mgTechTest.mg_bfsFontSize = BFS_LARGE;
  gm_mgTechTest.mg_boxOnScreen = BoxBigRow(5.0f);
  gm_mgTechTest.mg_strTip = LOCALIZE("start technology testing level");
  AddChild(&gm_mgTechTest);
  gm_mgTechTest.mg_pmgUp = &gm_mgTraining;
  gm_mgTechTest.mg_pmgDown = &gm_mgPlayersAndControls;
  gm_mgTechTest.mg_pActivatedFunction = &StartTechTest;

  gm_mgPlayersAndControls.mg_bfsFontSize = BFS_LARGE;
  gm_mgPlayersAndControls.mg_boxOnScreen = BoxBigRow(6.0f);
  gm_mgPlayersAndControls.mg_pmgUp = &gm_mgTechTest;
  gm_mgPlayersAndControls.mg_pmgDown = &gm_mgOptions;
  gm_mgPlayersAndControls.SetText(LOCALIZE("PLAYERS AND CONTROLS"));
  gm_mgPlayersAndControls.mg_strTip = LOCALIZE("change currently active player or adjust controls");
  AddChild(&gm_mgPlayersAndControls);
  gm_mgPlayersAndControls.mg_pActivatedFunction = &StartChangePlayerMenuFromSinglePlayer;

  gm_mgOptions.SetText(LOCALIZE("GAME OPTIONS"));
  gm_mgOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgOptions.mg_boxOnScreen = BoxBigRow(7.0f);
  gm_mgOptions.mg_strTip = LOCALIZE("adjust miscellaneous game options");
  AddChild(&gm_mgOptions);
  gm_mgOptions.mg_pmgUp = &gm_mgPlayersAndControls;
  gm_mgOptions.mg_pmgDown = &gm_mgNewGame;
  gm_mgOptions.mg_pActivatedFunction = &StartSinglePlayerGameOptions;
}

void CSinglePlayerMenu::StartMenu(void) {
  gm_mgTraining.mg_bEnabled = GetGameAPI()->IsMenuEnabledSS("Training");
  gm_mgTechTest.mg_bEnabled = GetGameAPI()->IsMenuEnabledSS("Technology Test");

  if (gm_mgTraining.mg_bEnabled) {
    AddChild(&gm_mgTraining);

    gm_mgLoad.mg_boxOnScreen = BoxBigRow(3.0f);
    gm_mgLoad.mg_pmgUp = &gm_mgQuickLoad;
    gm_mgLoad.mg_pmgDown = &gm_mgTraining;

    gm_mgTraining.mg_boxOnScreen = BoxBigRow(4.0f);
    gm_mgTraining.mg_pmgUp = &gm_mgLoad;
    gm_mgTraining.mg_pmgDown = &gm_mgTechTest;

    gm_mgTechTest.mg_boxOnScreen = BoxBigRow(5.0f);
    gm_mgTechTest.mg_pmgUp = &gm_mgTraining;
    gm_mgTechTest.mg_pmgDown = &gm_mgPlayersAndControls;

    gm_mgPlayersAndControls.mg_boxOnScreen = BoxBigRow(6.0f);
    gm_mgOptions.mg_boxOnScreen = BoxBigRow(7.0f);

  } else {
    // [Cecil] Unparent training button
    gm_mgTraining.SetParent(NULL);

    gm_mgLoad.mg_boxOnScreen = BoxBigRow(3.0f);
    gm_mgLoad.mg_pmgUp = &gm_mgQuickLoad;
    gm_mgLoad.mg_pmgDown = &gm_mgTechTest;

    gm_mgTechTest.mg_boxOnScreen = BoxBigRow(4.0f);
    gm_mgTechTest.mg_pmgUp = &gm_mgLoad;
    gm_mgTechTest.mg_pmgDown = &gm_mgPlayersAndControls;

    gm_mgPlayersAndControls.mg_boxOnScreen = BoxBigRow(5.0f);
    gm_mgOptions.mg_boxOnScreen = BoxBigRow(6.0f);
  }

  CGameMenu::StartMenu();

  CPlayerCharacter &pc = *GetGameAPI()->GetPlayerCharacter(GetGameAPI()->GetProfileForSP());

  CTString strPlayer;
  strPlayer.PrintF(LOCALIZE("Player: %s\n"), pc.GetNameForPrinting());

  gm_mgPlayerLabel.SetText(strPlayer);
}

// [Cecil] Change to the menu
void CSinglePlayerMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmSinglePlayerMenu);
};
