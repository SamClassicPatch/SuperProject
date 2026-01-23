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
#include "LevelInfo.h"
#include "MenuStuff.h"
#include "MSplitStart.h"

static void UpdateSplitLevel(INDEX iDummy) {
  CSplitStartMenu &gmCurrent = _pGUIM->gmSplitStartMenu;

  const INDEX iGameType = gmCurrent.gm_mgGameType.mg_iSelected;
  const ULONG ulFlags = GetGameAPI()->GetSpawnFlagsForGameTypeSS(iGameType);
  ValidateLevelForFlags(ulFlags);

  gmCurrent.gm_mgLevel.SetText(FindLevelByFileName(GetGameAPI()->GetCustomLevel()).li_strName);
};

static void StartSelectLevelFromSplit(void) {
  const INDEX iGameType = _pGUIM->gmSplitStartMenu.gm_mgGameType.mg_iSelected;
  const ULONG ulFlags = GetGameAPI()->GetSpawnFlagsForGameTypeSS(iGameType);

  // [Cecil] Select multiplayer levels
  extern void StartSelectLevel(ULONG ulFlags, void (*pAfterChosen)(void), CGameMenu *pgmParent);
  StartSelectLevel(ulFlags, &CSplitStartMenu::ChangeTo, &_pGUIM->gmSplitStartMenu);
};

static void StartGameOptionsFromSplitScreen(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\GameOptions.cfg");
  CVarMenu::ChangeTo(LOCALIZE("GAME OPTIONS"), fnmConfig);
};

// [Cecil] Open gameplay settings from the patch
static void StartPatchGameplayOptionsFromSplitScreen(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\ClassicsPatch\\52_GameplaySettings.cfg");
  CVarMenu::ChangeTo(TRANS("GAME CUSTOMIZATION"), fnmConfig);
};

static void StartSplitScreenGame(void) {
  GetGameAPI()->SetStartSplitCfg(GetGameAPI()->GetMenuSplitCfg());

  // [Cecil] Set start players from menu players
  GetGameAPI()->SetStartProfilesFromMenuProfiles();

  CTFileName fnWorld = GetGameAPI()->GetCustomLevel();

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_LOCAL);

  // [Cecil] Pass byte container
  CSesPropsContainer sp;
  _pGame->SetMultiPlayerSession((CSessionProperties &)sp);

  // [Cecil] Start game through the API
  if (GetGameAPI()->NewGame(fnWorld.FileName(), fnWorld, (CSessionProperties &)sp)) {
    StopMenus();
    _gmRunningGameMode = GM_SPLIT_SCREEN;

  } else {
    _gmRunningGameMode = GM_NONE;
  }
};

static void StartSelectPlayersMenuFromSplit(void) {
  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  gmCurrent.gm_ulConfigFlags = 0;
  gmCurrent.gm_mgStart.mg_pActivatedFunction = &StartSplitScreenGame;
  ChangeToMenu(&gmCurrent);
}

void CSplitStartMenu::Initialize_t(void) {
  gm_strName = "SplitStart";
  gm_pmgSelectedByDefault = &gm_mgStart;

  // intialize split screen menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("START SPLIT SCREEN"));
  AddChild(&gm_mgTitle);

  // game type trigger
  TRIGGER_MG(gm_mgGameType, 0, gm_mgStart, gm_mgDifficulty, LOCALIZE("Game type:"), astrGameTypeRadioTexts);
  gm_mgGameType.mg_ctTexts = ctGameTypeRadioTexts;
  gm_mgGameType.mg_strTip = LOCALIZE("choose type of multiplayer game");
  gm_mgGameType.mg_pOnTriggerChange = &UpdateSplitLevel;

  // difficulty trigger
  TRIGGER_MG(gm_mgDifficulty, 1, gm_mgGameType, gm_mgLevel, LOCALIZE("Difficulty:"), astrDifficultyRadioTexts);
  gm_mgDifficulty.mg_strTip = LOCALIZE("choose difficulty level");

  // level name
  gm_mgLevel.SetText("");
  gm_mgLevel.SetName(LOCALIZE("Level:"));
  gm_mgLevel.mg_boxOnScreen = BoxMediumRow(2);
  gm_mgLevel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgLevel.mg_iCenterI = -1;
  gm_mgLevel.mg_pmgUp = &gm_mgDifficulty;
  gm_mgLevel.mg_pmgDown = &gm_mgGameOptions;
  gm_mgLevel.mg_strTip = LOCALIZE("choose the level to start");
  gm_mgLevel.mg_pActivatedFunction = &StartSelectLevelFromSplit;
  AddChild(&gm_mgLevel);

  // options button
  gm_mgGameOptions.SetText(LOCALIZE("Game options"));
  gm_mgGameOptions.mg_boxOnScreen = BoxMediumRow(3);
  gm_mgGameOptions.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgGameOptions.mg_iCenterI = 0;
  gm_mgGameOptions.mg_pmgUp = &gm_mgLevel;
  gm_mgGameOptions.mg_pmgDown = &gm_mgPatchOptions;
  gm_mgGameOptions.mg_strTip = LOCALIZE("adjust game rules");
  gm_mgGameOptions.mg_pActivatedFunction = &StartGameOptionsFromSplitScreen;
  AddChild(&gm_mgGameOptions);

  // [Cecil] Gameplay options from the patch
  gm_mgPatchOptions.SetText(TRANS("Customize game"));
  gm_mgPatchOptions.mg_boxOnScreen = BoxMediumRow(4);
  gm_mgPatchOptions.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgPatchOptions.mg_iCenterI = 0;
  gm_mgPatchOptions.mg_pmgUp = &gm_mgGameOptions;
  gm_mgPatchOptions.mg_pmgDown = &gm_mgStart;
  gm_mgPatchOptions.mg_strTip = TRANS("adjust gameplay settings from the classics patch");
  gm_mgPatchOptions.mg_pActivatedFunction = &StartPatchGameplayOptionsFromSplitScreen;
  AddChild(&gm_mgPatchOptions);

  // start button
  gm_mgStart.mg_bfsFontSize = BFS_LARGE;
  gm_mgStart.mg_boxOnScreen = BoxBigRow(4);
  gm_mgStart.mg_pmgUp = &gm_mgPatchOptions;
  gm_mgStart.mg_pmgDown = &gm_mgGameType;
  gm_mgStart.SetText(LOCALIZE("START"));
  AddChild(&gm_mgStart);
  gm_mgStart.mg_pActivatedFunction = &StartSelectPlayersMenuFromSplit;
}

void CSplitStartMenu::StartMenu(void) {
  // [Cecil] Count active difficulties
  extern INDEX CountActiveDifficulties(void);
  gm_mgDifficulty.mg_ctTexts = CountActiveDifficulties();

  gm_mgGameType.mg_iSelected = Clamp(_pShell->GetINDEX("gam_iStartMode"), 0L, ctGameTypeRadioTexts - 1L);
  gm_mgGameType.ApplyCurrentSelection();
  gm_mgDifficulty.mg_iSelected = ClassicsModData_FindDiffByLevel(_pShell->GetINDEX("gam_iStartDifficulty"));
  gm_mgDifficulty.ApplyCurrentSelection();

  // [Cecil] Don't allow less players than amount of local ones
  const INDEX ctMinPlayers = ClampDn(_pShell->GetINDEX("gam_ctMaxPlayers"), MAX_GAME_LOCAL_PLAYERS);
  _pShell->SetINDEX("gam_ctMaxPlayers", ctMinPlayers);

  UpdateSplitLevel(0);
  CGameMenu::StartMenu();
}

void CSplitStartMenu::EndMenu(void) {
  _pShell->SetINDEX("gam_iStartDifficulty", ClassicsModData_GetDiff(gm_mgDifficulty.mg_iSelected)->m_iLevel);
  _pShell->SetINDEX("gam_iStartMode", gm_mgGameType.mg_iSelected);

  CGameMenu::EndMenu();
}

// [Cecil] Change to the menu
void CSplitStartMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmSplitStartMenu);
};
