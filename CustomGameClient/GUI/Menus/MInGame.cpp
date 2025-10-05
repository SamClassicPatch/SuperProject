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
#include "MInGame.h"

extern BOOL LSSaveAnyGame(const CTFileName &fnm);

void StartCurrentLoadMenu(void) {
  if (_gmRunningGameMode == GM_NETWORK) {
    void StartNetworkLoadMenu(void);
    StartNetworkLoadMenu();
  } else if (_gmRunningGameMode == GM_SPLIT_SCREEN) {
    void StartSplitScreenLoadMenu(void);
    StartSplitScreenLoadMenu();
  } else {
    void StartSinglePlayerLoadMenu(void);
    StartSinglePlayerLoadMenu();
  }
};

static void StartNetworkSaveMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  if (_gmRunningGameMode != GM_NETWORK) {
    return;
  }

  _gmMenuGameMode = GM_NETWORK;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("SAVE"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = TRUE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\Network\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmBaseName = CTString("SaveGame");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSSaveAnyGame;
  gmCurrent.gm_mgNotes.SetText("");
  gmCurrent.gm_strSaveDes = _pGame->GetDefaultGameDescription(TRUE);

  ChangeToMenu(&gmCurrent);
}

static void StartSplitScreenSaveMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  if (_gmRunningGameMode != GM_SPLIT_SCREEN) {
    return;
  }

  _gmMenuGameMode = GM_SPLIT_SCREEN;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("SAVE"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = TRUE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("SaveGame\\SplitScreen\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmBaseName = CTString("SaveGame");
  gmCurrent.gm_fnmExt = CTString(".sav");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSSaveAnyGame;
  gmCurrent.gm_mgNotes.SetText("");
  gmCurrent.gm_strSaveDes = _pGame->GetDefaultGameDescription(TRUE);

  ChangeToMenu(&gmCurrent);
};

void StartCurrentSaveMenu(void) {
  if (_gmRunningGameMode == GM_NETWORK) {
    StartNetworkSaveMenu();
  } else if (_gmRunningGameMode == GM_SPLIT_SCREEN) {
    StartSplitScreenSaveMenu();
  } else {
    extern void StartSinglePlayerSaveMenu(void);
    StartSinglePlayerSaveMenu();
  }
};

static void StartCurrentQuickLoadMenu(void) {
  if (_gmRunningGameMode == GM_NETWORK) {
    extern void StartNetworkQuickLoadMenu(void);
    StartNetworkQuickLoadMenu();
  } else if (_gmRunningGameMode == GM_SPLIT_SCREEN) {
    extern void StartSplitScreenQuickLoadMenu(void);
    StartSplitScreenQuickLoadMenu();
  } else {
    extern void StartSinglePlayerQuickLoadMenu(void);
    StartSinglePlayerQuickLoadMenu();
  }
};

static void QuickSaveFromMenu(void) {
  _pShell->SetINDEX("gam_bQuickSave", 2); // force save with reporting
  StopMenus();
};

static void StopCurrentGame(void) {
  _pGame->StopGame();
  _gmRunningGameMode = GM_NONE;

  StopMenus();
  StartMenus();
};

static void StopConfirm(void) {
  CConfirmMenu::ChangeTo(LOCALIZE("ARE YOU SERIOUS?"), &StopCurrentGame, NULL, TRUE);
};

extern void ExitConfirm(void);

// [Cecil] Toggle the observer camera and return to the game
static void ToggleOCAM(void) {
  BOOL &bToggle = GetGameAPI()->GetCamera().GetState();
  bToggle = !bToggle;

  StopMenus();
};

void CInGameMenu::Initialize_t(void) {
  gm_strName = "InGame";
  gm_pmgSelectedByDefault = &gm_mgQuickLoad;

  // intialize main menu
  gm_mgTitle.SetName(LOCALIZE("GAME"));
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  gm_mgLabel1.SetText("");
  gm_mgLabel1.mg_boxOnScreen = BoxMediumRow(-2.0);
  gm_mgLabel1.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgLabel1.mg_iCenterI = -1;
  gm_mgLabel1.mg_bEnabled = FALSE;
  gm_mgLabel1.mg_bLabel = TRUE;
  AddChild(&gm_mgLabel1);

  gm_mgLabel2.SetText("");
  gm_mgLabel2.mg_boxOnScreen = BoxMediumRow(-1.0);
  gm_mgLabel2.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgLabel2.mg_iCenterI = -1;
  gm_mgLabel2.mg_bEnabled = FALSE;
  gm_mgLabel2.mg_bLabel = TRUE;
  AddChild(&gm_mgLabel2);

  gm_mgQuickLoad.SetText(LOCALIZE("QUICK LOAD"));
  gm_mgQuickLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuickLoad.mg_boxOnScreen = BoxBigRow(0.0f);
  gm_mgQuickLoad.mg_strTip = LOCALIZE("load a quick-saved game (F9)");
  AddChild(&gm_mgQuickLoad);
  gm_mgQuickLoad.mg_pmgUp = &gm_mgQuit;
  gm_mgQuickLoad.mg_pmgDown = &gm_mgQuickSave;
  gm_mgQuickLoad.mg_pmgLeft = &gm_mgOCAM;
  gm_mgQuickLoad.mg_pActivatedFunction = &StartCurrentQuickLoadMenu;

  gm_mgQuickSave.SetText(LOCALIZE("QUICK SAVE"));
  gm_mgQuickSave.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuickSave.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgQuickSave.mg_strTip = LOCALIZE("quick-save current game (F6)");
  AddChild(&gm_mgQuickSave);
  gm_mgQuickSave.mg_pmgUp = &gm_mgQuickLoad;
  gm_mgQuickSave.mg_pmgDown = &gm_mgLoad;
  gm_mgQuickSave.mg_pmgLeft = &gm_mgOCAM;
  gm_mgQuickSave.mg_pActivatedFunction = &QuickSaveFromMenu;

  gm_mgLoad.SetText(LOCALIZE("LOAD"));
  gm_mgLoad.mg_bfsFontSize = BFS_LARGE;
  gm_mgLoad.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgLoad.mg_strTip = LOCALIZE("load a saved game");
  AddChild(&gm_mgLoad);
  gm_mgLoad.mg_pmgUp = &gm_mgQuickSave;
  gm_mgLoad.mg_pmgDown = &gm_mgSave;
  gm_mgLoad.mg_pmgLeft = &gm_mgOCAM;
  gm_mgLoad.mg_pActivatedFunction = &StartCurrentLoadMenu;

  gm_mgSave.SetText(LOCALIZE("SAVE"));
  gm_mgSave.mg_bfsFontSize = BFS_LARGE;
  gm_mgSave.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgSave.mg_strTip = LOCALIZE("save current game (each player has own slots!)");
  AddChild(&gm_mgSave);
  gm_mgSave.mg_pmgUp = &gm_mgLoad;
  gm_mgSave.mg_pmgDown = &gm_mgDemoRec;
  gm_mgSave.mg_pmgLeft = &gm_mgOCAM;
  gm_mgSave.mg_pActivatedFunction = &StartCurrentSaveMenu;

  gm_mgDemoRec.mg_boxOnScreen = BoxBigRow(4.0f);
  gm_mgDemoRec.mg_bfsFontSize = BFS_LARGE;
  gm_mgDemoRec.mg_pmgUp = &gm_mgSave;
  gm_mgDemoRec.mg_pmgDown = &gm_mgHighScore;
  gm_mgDemoRec.mg_pmgLeft = &gm_mgOCAM;
  gm_mgDemoRec.SetText("Text not set");
  AddChild(&gm_mgDemoRec);
  gm_mgDemoRec.mg_pActivatedFunction = NULL;

  gm_mgHighScore.SetText(LOCALIZE("HIGH SCORES"));
  gm_mgHighScore.mg_bfsFontSize = BFS_LARGE;
  gm_mgHighScore.mg_boxOnScreen = BoxBigRow(5.0f);
  gm_mgHighScore.mg_strTip = LOCALIZE("view list of top ten best scores");
  AddChild(&gm_mgHighScore);
  gm_mgHighScore.mg_pmgUp = &gm_mgDemoRec;
  gm_mgHighScore.mg_pmgDown = &gm_mgOptions;
  gm_mgHighScore.mg_pmgLeft = &gm_mgOCAM;
  gm_mgHighScore.mg_pActivatedFunction = &CHighScoreMenu::ChangeTo;

  gm_mgOptions.SetText(LOCALIZE("OPTIONS"));
  gm_mgOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgOptions.mg_boxOnScreen = BoxBigRow(6.0f);
  gm_mgOptions.mg_strTip = LOCALIZE("adjust video, audio and input options");
  AddChild(&gm_mgOptions);
  gm_mgOptions.mg_pmgUp = &gm_mgHighScore;
  gm_mgOptions.mg_pmgDown = &gm_mgStop;
  gm_mgOptions.mg_pmgLeft = &gm_mgOCAM;
  gm_mgOptions.mg_pActivatedFunction = &COptionsMenu::ChangeTo;

  gm_mgStop.SetText(LOCALIZE("STOP GAME"));
  gm_mgStop.mg_bfsFontSize = BFS_LARGE;
  gm_mgStop.mg_boxOnScreen = BoxBigRow(7.0f);
  gm_mgStop.mg_strTip = LOCALIZE("stop currently running game");
  AddChild(&gm_mgStop);
  gm_mgStop.mg_pmgUp = &gm_mgOptions;
  gm_mgStop.mg_pmgDown = &gm_mgQuit;
  gm_mgStop.mg_pmgLeft = &gm_mgOCAM;
  gm_mgStop.mg_pActivatedFunction = &StopConfirm;

  gm_mgQuit.SetText(LOCALIZE("QUIT"));
  gm_mgQuit.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuit.mg_boxOnScreen = BoxBigRow(8.0f);
  gm_mgQuit.mg_strTip = LOCALIZE("exit game immediately");
  AddChild(&gm_mgQuit);
  gm_mgQuit.mg_pmgUp = &gm_mgStop;
  gm_mgQuit.mg_pmgDown = &gm_mgQuickLoad;
  gm_mgQuit.mg_pmgLeft = &gm_mgOCAM;
  gm_mgQuit.mg_pActivatedFunction = &ExitConfirm;

  // [Cecil] Observer camera
  gm_mgOCAM.SetText(TRANS("Photo mode"));
  gm_mgOCAM.mg_strTip = TRANS("toggle photo mode via the observer camera");
  gm_mgOCAM.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgOCAM.mg_boxOnScreen = BoxLeftColumn(15.0f);
  gm_mgOCAM.mg_iCenterI = -1;

  gm_mgOCAM.mg_pmgRight = &gm_mgQuit;
  gm_mgOCAM.mg_pActivatedFunction = &ToggleOCAM;

  AddChild(&gm_mgOCAM);
}

static void SetDemoStartStopRecText(void);

static void StopRecordingDemo(void) {
  _pNetwork->StopDemoRec();
  SetDemoStartStopRecText();
};

static BOOL LSSaveDemo(const CTFileName &fnm) {
  // save the demo
  if (_pGame->StartDemoRec(fnm)) {
    StopMenus();

    return TRUE;
  }

  return FALSE;
};

void StartDemoSaveMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  if (_gmRunningGameMode == GM_NONE) {
    return;
  }

  _gmMenuGameMode = GM_DEMO;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("RECORD DEMO"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEUP;
  gmCurrent.gm_bSave = TRUE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("Demos\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmBaseName = CTString("Demo");
  gmCurrent.gm_fnmExt = CTString(".dem");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSSaveDemo;
  gmCurrent.gm_mgNotes.SetText("");
  gmCurrent.gm_strSaveDes = _pGame->GetDefaultGameDescription(FALSE);

  ChangeToMenu(&gmCurrent);
};

void SetDemoStartStopRecText(void) {
  CInGameMenu &gmCurrent = _pGUIM->gmInGameMenu;

  if (_pNetwork->IsRecordingDemo()) {
    gmCurrent.gm_mgDemoRec.SetText(LOCALIZE("STOP RECORDING"));
    gmCurrent.gm_mgDemoRec.mg_strTip = LOCALIZE("stop current recording");
    gmCurrent.gm_mgDemoRec.mg_pActivatedFunction = &StopRecordingDemo;
  } else {
    gmCurrent.gm_mgDemoRec.SetText(LOCALIZE("RECORD DEMO"));
    gmCurrent.gm_mgDemoRec.mg_strTip = LOCALIZE("start recording current game");
    gmCurrent.gm_mgDemoRec.mg_pActivatedFunction = &StartDemoSaveMenu;
  }
};

void CInGameMenu::StartMenu(void) {
  gm_mgQuickLoad.mg_bEnabled = _pNetwork->IsServer();
  gm_mgQuickSave.mg_bEnabled = _pNetwork->IsServer();
  gm_mgLoad.mg_bEnabled = _pNetwork->IsServer();
  gm_mgSave.mg_bEnabled = _pNetwork->IsServer();
  gm_mgDemoRec.mg_bEnabled = TRUE; //_pNetwork->IsServer();
  SetDemoStartStopRecText();

  // [Cecil] Check if player name should be displayed on the pause menu
  BOOL bPlayerName = (_gmRunningGameMode == GM_SINGLE_PLAYER);
#if SE1_GAME == SS_REV
  bPlayerName |= _pNetwork->IsServer();
#endif

  if (bPlayerName) {
    CPlayerCharacter &pc = *GetGameAPI()->GetPlayerCharacter(GetGameAPI()->GetProfileForSP());

    CTString strPlayer;
    strPlayer.PrintF(LOCALIZE("Player: %s"), pc.GetNameForPrinting());

    gm_mgLabel1.SetText(strPlayer);
    gm_mgLabel2.SetText("");

#if SE1_GAME != SS_REV
  } else if (_pNetwork->IsServer()) {
    CTString strHost, strAddress;
    CTString strHostName;
    _pNetwork->GetHostName(strHost, strAddress);
    if (strHost == "") {
      strHostName = LOCALIZE("<not started yet>");
    } else {
      strHostName = strHost + " (" + strAddress + ")";
    }

    gm_mgLabel1.SetText(LOCALIZE("Address: ") + strHostName);
    gm_mgLabel2.SetText("");
#endif

  } else {
    CTString strConfig;
    strConfig = LOCALIZE("<not adjusted>");
    extern CTString sam_strNetworkSettings;
    if (sam_strNetworkSettings != "") {
      LoadStringVar(CTFileName(sam_strNetworkSettings).NoExt() + ".des", strConfig);
      strConfig.OnlyFirstLine();
    }

    gm_mgLabel1.SetText(LOCALIZE("Connected to: ") + GetGameAPI()->JoinAddress());
    gm_mgLabel2.SetText(LOCALIZE("Connection: ") + strConfig);
  }

  CGameMenu::StartMenu();
}

// [Cecil] Change to the menu
void CInGameMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmInGameMenu);
};
