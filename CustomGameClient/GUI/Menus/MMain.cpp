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
#include "MMain.h"

extern CTFileName _fnmModToLoad;
extern CTFileName _fnmModSelected;

// [Cecil] Open extras menu
static void StartExtrasMenu(void) {
  ChangeToMenu(&_pGUIM->gmExtras);
};

static void ModLoadYes(void) {
  _fnmModToLoad = _fnmModSelected;
};

static BOOL LSLoadMod(const CTFileName &fnm) {
  _fnmModSelected = fnm;
  CConfirmMenu::ChangeTo(LOCALIZE("LOAD THIS MOD?"), &ModLoadYes, NULL, TRUE);
  return TRUE;
};

void StartModsLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("CHOOSE MOD"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_NAMEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString("Mods\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".des");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadMod;

  // [Cecil] Set current menu as the parent menu
  ChangeToMenu(&gmCurrent);
};

// [Cecil] Update extras button
void CMainMenu::UpdateExtras(void) {
  // Extras button
  if (sam_bExtrasMenu) {
    gm_mgExtras.SetText(TRANS("EXTRAS"));
    gm_mgExtras.mg_strTip = TRANS("modifications, credits and more");
    gm_mgExtras.mg_pActivatedFunction = &StartExtrasMenu;

  // Vanilla mods button
  } else {
    gm_mgExtras.SetText(LOCALIZE("MODS"));
    gm_mgExtras.mg_strTip = LOCALIZE("run one of installed game modifications");
    gm_mgExtras.mg_pActivatedFunction = &StartModsLoadMenu;
  }
};

static CTFileName _fnDemoToPlay;

void StartDemoPlay(void) {
  GetGameAPI()->SetStartSplitCfg(CGame::SSC_OBSERVER);
  // play the demo
  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_LOCAL);
  if (_pGame->StartDemoPlay(_fnDemoToPlay)) {
    // exit menu and pull up the console
    StopMenus();
    if (GetGameAPI()->GetConState() != CS_OFF) {
      GetGameAPI()->SetConState(CS_TURNINGOFF);
    }
    _gmRunningGameMode = GM_DEMO;
  } else {
    _gmRunningGameMode = GM_NONE;
  }
};

BOOL LSLoadDemo(const CTFileName &fnm) {
  // call local players menu
  _fnDemoToPlay = fnm;
  StartDemoPlay();
  return TRUE;
};

static void StartDemoLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  _gmMenuGameMode = GM_DEMO;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("PLAY DEMO"));
  gmCurrent.gm_bAllowThumbnails = TRUE;
  gmCurrent.gm_iSortType = LSSORT_FILEDN;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = TRUE;
  gmCurrent.gm_fnmDirectory = CTString("Demos\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".dem");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadDemo;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

static void ExitGame(void) {
  _bRunning = FALSE;
  _bQuitScreen = TRUE;
};

void ExitConfirm(void) {
  CConfirmMenu::ChangeTo(LOCALIZE("ARE YOU SERIOUS?"), &ExitGame, NULL, TRUE);
};

static CStringStack _astrTOTD;
static BOOL _bForceTOTD = FALSE;

static void DisplayTOTD(void);

// [Cecil] Shuffle between tips of the day
static void NextTOTD(void) {
  // No tips
  if (_astrTOTD.Count() == 0) return;

  // Get next tip
  sam_iCurrentTOTD = (sam_iCurrentTOTD + 1) % _astrTOTD.Count();

  // Display a new tip again when returning to the main menu
  _bForceTOTD = TRUE;
};

// [Cecil] Display the tip of the day on screen
void DisplayTOTD(void) {
  static const CTString strNoTip = "No tip of the day available :(";
  const CTString *pstrTip = &strNoTip;

  if (_astrTOTD.Count() != 0) {
    // Select a new random tip based on the current day
    if (sam_iCurrentTOTD == -1) {
      time_t tmNow = time(NULL);
      sam_iCurrentTOTD = (tmNow / 86400) % _astrTOTD.Count();
    }

    INDEX iTip = Clamp(sam_iCurrentTOTD, (INDEX)0, INDEX(_astrTOTD.Count() - 1));
    pstrTip = &_astrTOTD[iTip];
  }

  CConfirmMenu::ChangeTo(TRANS("Did you know?") + CTString("\n\n") + *pstrTip, &NextTOTD, NULL, FALSE, TRANS("NEXT TIP"), TRANS("SERIOUSLY AWESOME!"), 0.4f);
};

void CMainMenu::Initialize_t(void) {
  gm_strName = "Main";
  gm_pmgSelectedByDefault = &gm_mgSingle;

  // [Cecil] Load the list of tips of the day and pick one at random based on the current day
  CTString strTip;
  char strLine[1024];
  _astrTOTD.PopAll();

  try {
    CTFileStream strm;
    strm.Open_t(CTString("Data\\ClassicsPatch\\TOTD.txt"));

    while (!strm.AtEOF()) {
      IData::GetLineFromStream_t(strm, strLine, 1024);

      strTip = strLine;
      strTip.TrimSpacesLeft();
      strTip.TrimSpacesRight();

      // Skip empty lines
      if (strTip == "") continue;

      // Simple substitution for inline line breaks
      while (strTip.ReplaceSubstr("\\n", "\n"));

      _astrTOTD.Add(strTip);
    }

  } catch (char *strError) {
    (void)strError;
  }

  // intialize main menu
  gm_mgVersionLabel.SetText(sam_strVersion);
  gm_mgVersionLabel.mg_boxOnScreen = BoxVersion();
  gm_mgVersionLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgVersionLabel.mg_iCenterI = +1;
  gm_mgVersionLabel.mg_bEnabled = FALSE;
  gm_mgVersionLabel.mg_bLabel = TRUE;
  AddChild(&gm_mgVersionLabel);

  extern CTString sam_strModName;
  gm_mgModLabel.SetText(sam_strModName);
  gm_mgModLabel.mg_boxOnScreen = BoxMediumRow(-2.0f);
  gm_mgModLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgModLabel.mg_iCenterI = 0;
  gm_mgModLabel.mg_bEnabled = FALSE;
  gm_mgModLabel.mg_bLabel = TRUE;
  AddChild(&gm_mgModLabel);

  gm_mgSingle.SetText(LOCALIZE("SINGLE PLAYER"));
  gm_mgSingle.mg_bfsFontSize = BFS_LARGE;
  gm_mgSingle.mg_boxOnScreen = BoxBigRow(0.0f);
  gm_mgSingle.mg_strTip = LOCALIZE("single player game menus");
  AddChild(&gm_mgSingle);
  gm_mgSingle.mg_pmgUp = &gm_mgQuit;
  gm_mgSingle.mg_pmgDown = &gm_mgNetwork;
  gm_mgSingle.mg_pActivatedFunction = &CSinglePlayerMenu::ChangeTo;

  gm_mgNetwork.SetText(LOCALIZE("NETWORK"));
  gm_mgNetwork.mg_bfsFontSize = BFS_LARGE;
  gm_mgNetwork.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgNetwork.mg_strTip = LOCALIZE("LAN/iNet multiplayer menus");
  AddChild(&gm_mgNetwork);
  gm_mgNetwork.mg_pmgUp = &gm_mgSingle;
  gm_mgNetwork.mg_pmgDown = &gm_mgSplitScreen;
  gm_mgNetwork.mg_pActivatedFunction = &CNetworkMenu::ChangeTo;

  gm_mgSplitScreen.SetText(LOCALIZE("SPLIT SCREEN"));
  gm_mgSplitScreen.mg_bfsFontSize = BFS_LARGE;
  gm_mgSplitScreen.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgSplitScreen.mg_strTip = LOCALIZE("play with multiple players on one computer");
  AddChild(&gm_mgSplitScreen);
  gm_mgSplitScreen.mg_pmgUp = &gm_mgNetwork;
  gm_mgSplitScreen.mg_pmgDown = &gm_mgDemo;
  gm_mgSplitScreen.mg_pActivatedFunction = &CSplitScreenMenu::ChangeTo;

  gm_mgDemo.SetText(LOCALIZE("DEMO"));
  gm_mgDemo.mg_bfsFontSize = BFS_LARGE;
  gm_mgDemo.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgDemo.mg_strTip = LOCALIZE("play a game demo");
  AddChild(&gm_mgDemo);
  gm_mgDemo.mg_pmgUp = &gm_mgSplitScreen;
  gm_mgDemo.mg_pmgDown = &gm_mgExtras;
  gm_mgDemo.mg_pActivatedFunction = &StartDemoLoadMenu;

  // [Cecil] Setup extras button
  UpdateExtras();
  gm_mgExtras.mg_bfsFontSize = BFS_LARGE;
  gm_mgExtras.mg_boxOnScreen = BoxBigRow(4.0f);
  gm_mgExtras.mg_pmgUp = &gm_mgDemo;
  gm_mgExtras.mg_pmgDown = &gm_mgHighScore;
  AddChild(&gm_mgExtras);

  gm_mgHighScore.SetText(LOCALIZE("HIGH SCORES"));
  gm_mgHighScore.mg_bfsFontSize = BFS_LARGE;
  gm_mgHighScore.mg_boxOnScreen = BoxBigRow(5.0f);
  gm_mgHighScore.mg_strTip = LOCALIZE("view list of top ten best scores");
  AddChild(&gm_mgHighScore);
  gm_mgHighScore.mg_pmgUp = &gm_mgExtras;
  gm_mgHighScore.mg_pmgDown = &gm_mgOptions;
  gm_mgHighScore.mg_pActivatedFunction = &CHighScoreMenu::ChangeTo;

  gm_mgOptions.SetText(LOCALIZE("OPTIONS"));
  gm_mgOptions.mg_bfsFontSize = BFS_LARGE;
  gm_mgOptions.mg_boxOnScreen = BoxBigRow(6.0f);
  gm_mgOptions.mg_strTip = LOCALIZE("adjust video, audio and input options");
  AddChild(&gm_mgOptions);
  gm_mgOptions.mg_pmgUp = &gm_mgHighScore;
  gm_mgOptions.mg_pmgDown = &gm_mgQuit;
  gm_mgOptions.mg_pActivatedFunction = &COptionsMenu::ChangeTo;

  gm_mgQuit.SetText(LOCALIZE("QUIT"));
  gm_mgQuit.mg_bfsFontSize = BFS_LARGE;
  gm_mgQuit.mg_boxOnScreen = BoxBigRow(7.0f);
  gm_mgQuit.mg_strTip = LOCALIZE("exit game immediately");
  AddChild(&gm_mgQuit);
  gm_mgQuit.mg_pmgUp = &gm_mgOptions;
  gm_mgQuit.mg_pmgDown = &gm_mgSingle;
  gm_mgQuit.mg_pActivatedFunction = &ExitConfirm;

  // [Cecil] Tip of the day
  gm_mgTOTD.SetText(TRANS("Tip of the day!"));
  gm_mgTOTD.mg_strTip = TRANS("display a randomized tip or a fun fact of today about classics patch");
  gm_mgTOTD.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgTOTD.mg_boxOnScreen = BoxLeftColumn(14.0f);
  gm_mgTOTD.mg_iCenterI = -1;

  gm_mgTOTD.mg_pmgRight = &gm_mgQuit;
  gm_mgTOTD.mg_pActivatedFunction = &DisplayTOTD;

  AddChild(&gm_mgTOTD);
}

void CMainMenu::StartMenu(void) {
  gm_mgSingle.mg_bEnabled      = GetGameAPI()->IsMenuEnabledSS("Single Player");
  gm_mgNetwork.mg_bEnabled     = GetGameAPI()->IsMenuEnabledSS("Network");
  gm_mgSplitScreen.mg_bEnabled = GetGameAPI()->IsMenuEnabledSS("Split Screen");
  gm_mgHighScore.mg_bEnabled   = GetGameAPI()->IsMenuEnabledSS("High Score");

  // [Cecil] Update extras button
  UpdateExtras();

  // [Cecil] Update tip of the day button
  CMenuGadget *pmgTOTD = NULL;

  if (sam_bTipOfTheDay) {
    pmgTOTD = &gm_mgTOTD;
    AddChild(&gm_mgTOTD);

  } else {
    gm_mgTOTD.Expunge();
  }

  gm_mgSingle.mg_pmgLeft =
  gm_mgNetwork.mg_pmgLeft =
  gm_mgSplitScreen.mg_pmgLeft =
  gm_mgDemo.mg_pmgLeft =
  gm_mgExtras.mg_pmgLeft =
  gm_mgHighScore.mg_pmgLeft =
  gm_mgOptions.mg_pmgLeft =
  gm_mgQuit.mg_pmgLeft = pmgTOTD;

  CGameMenu::StartMenu();
}

// [Cecil] Menu update
void CMainMenu::Think(void) {
  // Show another tip, if needed
  if (_bForceTOTD) {
    _bForceTOTD = FALSE;
    DisplayTOTD();
  }
};

// [Cecil] Change to the menu
void CMainMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmMainMenu);
};
