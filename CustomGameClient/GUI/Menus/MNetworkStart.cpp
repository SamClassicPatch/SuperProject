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
#include "MNetworkStart.h"

static void UpdateNetworkLevel(INDEX iDummy) {
  const INDEX iGameType = _pGUIM->gmNetworkStartMenu.gm_mgGameType.mg_iSelected;
  const ULONG ulFlags = GetGameAPI()->GetSpawnFlagsForGameTypeSS(iGameType);
  ValidateLevelForFlags(ulFlags);

  _pGUIM->gmNetworkStartMenu.gm_mgLevel.SetText(FindLevelByFileName(GetGameAPI()->GetCustomLevel()).li_strName);
};

static void StartSelectLevelFromNetwork(void) {
  const INDEX iGameType = _pGUIM->gmNetworkStartMenu.gm_mgGameType.mg_iSelected;
  const ULONG ulFlags = GetGameAPI()->GetSpawnFlagsForGameTypeSS(iGameType);

  // [Cecil] Select multiplayer levels
  extern void StartSelectLevel(ULONG ulFlags, void (*pAfterChosen)(void), CGameMenu *pgmParent);
  StartSelectLevel(ulFlags, &CNetworkStartMenu::ChangeTo, &_pGUIM->gmNetworkStartMenu);
};

static void StartGameOptionsFromNetwork(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\GameOptions.cfg");
  CVarMenu::ChangeTo(LOCALIZE("GAME OPTIONS"), fnmConfig);
};

// [Cecil] Open server settings from the patch
static void StartPatchServerOptionsFromNetwork(void) {
  static DECLARE_CTFILENAME(fnmConfig, "Scripts\\Menu\\ClassicsPatch\\51_ServerSettings.cfg");
  CVarMenu::ChangeTo(TRANS("SERVER OPTIONS"), fnmConfig);
};

void StartNetworkGame(void) {
  GetGameAPI()->SetStartSplitCfg(GetGameAPI()->GetMenuSplitCfg());

  // [Cecil] Set start players from menu players
  GetGameAPI()->SetStartProfilesFromMenuProfiles();

  CTFileName fnWorld = GetGameAPI()->GetCustomLevel();

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_SERVER);
  
  // [Cecil] Pass byte container
  CSesPropsContainer sp;
  _pGame->SetMultiPlayerSession((CSessionProperties &)sp);

  // [Cecil] Start game through the API
  if (GetGameAPI()->NewGame(GetGameAPI()->SessionName(), fnWorld, (CSessionProperties &)sp)) {
    StopMenus();
    _gmRunningGameMode = GM_NETWORK;

    // if starting a dedicated server
    if (GetGameAPI()->GetMenuSplitCfg() == CGame::SSC_DEDICATED) {
      // pull down the console
      extern INDEX sam_bToggleConsole;
      sam_bToggleConsole = TRUE;
    }
  } else {
    _gmRunningGameMode = GM_NONE;
  }
};

static void StartSelectPlayersMenuFromNetwork(void) {
  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  gmCurrent.gm_ulConfigFlags = PLCF_DEDICATED | PLCF_OBSERVING;
  gmCurrent.gm_mgStart.mg_pActivatedFunction = &StartNetworkGame;
  ChangeToMenu(&gmCurrent);
};

void CNetworkStartMenu::Initialize_t(void) {
  gm_strName = "NetworkStart";
  gm_pmgSelectedByDefault = &gm_mgStart;

  // title
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("START SERVER"));
  AddChild(&gm_mgTitle);

  // session name edit box
  gm_mgSessionName.SetText(GetGameAPI()->SessionName());
  gm_mgSessionName.SetName(LOCALIZE("Session name:"));
  gm_mgSessionName.mg_ctMaxStringLen = 25;
  gm_mgSessionName.mg_pstrToChange = &GetGameAPI()->SessionName();
  gm_mgSessionName.mg_boxOnScreen = BoxMediumRow(1);
  gm_mgSessionName.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgSessionName.mg_iCenterI = -1;
  gm_mgSessionName.mg_pmgUp = &gm_mgStart;
  gm_mgSessionName.mg_pmgDown = &gm_mgGameType;
  gm_mgSessionName.mg_strTip = LOCALIZE("name the session to start");
  AddChild(&gm_mgSessionName);

  // game type trigger
  TRIGGER_MG(gm_mgGameType, 2, gm_mgSessionName, gm_mgDifficulty, LOCALIZE("Game type:"), astrGameTypeRadioTexts);
  gm_mgGameType.mg_ctTexts = ctGameTypeRadioTexts;
  gm_mgGameType.mg_strTip = LOCALIZE("choose type of multiplayer game");
  gm_mgGameType.mg_pOnTriggerChange = &UpdateNetworkLevel;

  // difficulty trigger
  TRIGGER_MG(gm_mgDifficulty, 3, gm_mgGameType, gm_mgLevel, LOCALIZE("Difficulty:"), astrDifficultyRadioTexts);
  gm_mgDifficulty.mg_strTip = LOCALIZE("choose difficulty level");

  // level name
  gm_mgLevel.SetText("");
  gm_mgLevel.SetName(LOCALIZE("Level:"));
  gm_mgLevel.mg_boxOnScreen = BoxMediumRow(4);
  gm_mgLevel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgLevel.mg_iCenterI = -1;
  gm_mgLevel.mg_pmgUp = &gm_mgDifficulty;
  gm_mgLevel.mg_pmgDown = &gm_mgMaxPlayers;
  gm_mgLevel.mg_strTip = LOCALIZE("choose the level to start");
  gm_mgLevel.mg_pActivatedFunction = &StartSelectLevelFromNetwork;
  AddChild(&gm_mgLevel);

  // [Cecil] Create entries for each max players configuration (other than 1)
  const INDEX ctMaxPlayersEntries = ICore::MAX_GAME_PLAYERS - 1;

  if (astrMaxPlayersRadioTexts == NULL) {
    astrMaxPlayersRadioTexts = new CTString[ctMaxPlayersEntries];

    for (INDEX i = 0; i < ctMaxPlayersEntries; i++) {
      astrMaxPlayersRadioTexts[i].PrintF("%d", i + 2);
    }
  }

  // max players trigger
  TRIGGER_MG(gm_mgMaxPlayers, 5, gm_mgLevel, gm_mgWaitAllPlayers, LOCALIZE("Max players:"), astrMaxPlayersRadioTexts);
  gm_mgMaxPlayers.mg_ctTexts = ctMaxPlayersEntries; // [Cecil] Update amount of entries
  gm_mgMaxPlayers.mg_strTip = LOCALIZE("choose maximum allowed number of players");

  // wait all players trigger
  TRIGGER_MG(gm_mgWaitAllPlayers, 6, gm_mgMaxPlayers, gm_mgVisible, LOCALIZE("Wait for all players:"), astrNoYes);
  gm_mgWaitAllPlayers.mg_strTip = LOCALIZE("if on, game won't start until all players have joined");

  // server visible trigger
  TRIGGER_MG(gm_mgVisible, 7, gm_mgMaxPlayers, gm_mgGameOptions, LOCALIZE("Server visible:"), astrNoYes);
  gm_mgVisible.mg_strTip = LOCALIZE("invisible servers are not listed, cleints have to join manually");

  // options button
  gm_mgGameOptions.SetText(LOCALIZE("Game options"));
  gm_mgGameOptions.mg_boxOnScreen = BoxMediumRow(8);
  gm_mgGameOptions.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgGameOptions.mg_iCenterI = 0;
  gm_mgGameOptions.mg_pmgUp = &gm_mgVisible;
  gm_mgGameOptions.mg_pmgDown = &gm_mgPatchOptions;
  gm_mgGameOptions.mg_strTip = LOCALIZE("adjust game rules");
  gm_mgGameOptions.mg_pActivatedFunction = &StartGameOptionsFromNetwork;
  AddChild(&gm_mgGameOptions);

  // [Cecil] Server options from the patch
  gm_mgPatchOptions.SetText(TRANS("Server options"));
  gm_mgPatchOptions.mg_boxOnScreen = BoxMediumRow(9);
  gm_mgPatchOptions.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgPatchOptions.mg_iCenterI = 0;
  gm_mgPatchOptions.mg_pmgUp = &gm_mgGameOptions;
  gm_mgPatchOptions.mg_pmgDown = &gm_mgStart;
  gm_mgPatchOptions.mg_strTip = TRANS("adjust server settings from the classics patch");
  gm_mgPatchOptions.mg_pActivatedFunction = &StartPatchServerOptionsFromNetwork;
  AddChild(&gm_mgPatchOptions);

  // start button
  gm_mgStart.mg_bfsFontSize = BFS_LARGE;
  gm_mgStart.mg_boxOnScreen = BoxBigRow(7);
  gm_mgStart.mg_pmgUp = &gm_mgPatchOptions;
  gm_mgStart.mg_pmgDown = &gm_mgSessionName;
  gm_mgStart.SetText(LOCALIZE("START"));
  AddChild(&gm_mgStart);
  gm_mgStart.mg_pActivatedFunction = &StartSelectPlayersMenuFromNetwork;
}

// [Cecil] Count active difficulties for selection lists
INDEX CountActiveDifficulties(void) {
  const INDEX ct = ClampUp(ClassicsModData_DiffArrayLength(), (int)ARRAYCOUNT(astrDifficultyRadioTexts));
  INDEX i = 0;

  for (; i < ct; i++) {
    // No more active difficulties
    if (!ClassicsModData_IsDiffActive(i)) break;
  }

  // At least one
  return ClampDn(i, (INDEX)1);
};

void CNetworkStartMenu::StartMenu(void) {
  // [Cecil] Count active difficulties
  gm_mgDifficulty.mg_ctTexts = CountActiveDifficulties();

  gm_mgGameType.mg_iSelected = Clamp(_pShell->GetINDEX("gam_iStartMode"), 0L, ctGameTypeRadioTexts - 1L);
  gm_mgGameType.ApplyCurrentSelection();
  gm_mgDifficulty.mg_iSelected = ClassicsModData_FindDiffByLevel(_pShell->GetINDEX("gam_iStartDifficulty"));
  gm_mgDifficulty.ApplyCurrentSelection();

  _pShell->SetINDEX("gam_iStartMode", GetGameAPI()->GetGameMode(1)); // [Cecil] API

  INDEX ctMaxPlayers = _pShell->GetINDEX("gam_ctMaxPlayers");

  // [Cecil] 16 -> ICore::MAX_GAME_PLAYERS
  if (ctMaxPlayers < 2 || ctMaxPlayers > ICore::MAX_GAME_PLAYERS) {
    ctMaxPlayers = 2;
    _pShell->SetINDEX("gam_ctMaxPlayers", ctMaxPlayers);
  }

  gm_mgMaxPlayers.mg_iSelected = ctMaxPlayers - 2;
  gm_mgMaxPlayers.ApplyCurrentSelection();

  gm_mgWaitAllPlayers.mg_iSelected = Clamp(_pShell->GetINDEX("gam_bWaitAllPlayers"), 0L, 1L);
  gm_mgWaitAllPlayers.ApplyCurrentSelection();

  gm_mgVisible.mg_iSelected = _pShell->GetINDEX("ser_bEnumeration");
  gm_mgVisible.ApplyCurrentSelection();

  UpdateNetworkLevel(0);

  CGameMenu::StartMenu();
}

void CNetworkStartMenu::EndMenu(void) {
  _pShell->SetINDEX("gam_iStartDifficulty", ClassicsModData_GetDiff(gm_mgDifficulty.mg_iSelected)->m_iLevel);
  _pShell->SetINDEX("gam_iStartMode", gm_mgGameType.mg_iSelected);
  _pShell->SetINDEX("gam_bWaitAllPlayers", gm_mgWaitAllPlayers.mg_iSelected);
  _pShell->SetINDEX("gam_ctMaxPlayers", gm_mgMaxPlayers.mg_iSelected + 2);
  _pShell->SetINDEX("ser_bEnumeration", gm_mgVisible.mg_iSelected);

  CGameMenu::EndMenu();
}

// [Cecil] Change to the menu
void CNetworkStartMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmNetworkStartMenu);
};
