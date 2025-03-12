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
#include "MSelectPlayers.h"

#define ADD_GADGET(gd, box, up, dn, lf, rt, txt) \
  gd.mg_boxOnScreen = box; \
  gd.mg_pmgUp = up; \
  gd.mg_pmgDown = dn; \
  gd.mg_pmgLeft = lf; \
  gd.mg_pmgRight = rt; \
  gd.SetText(txt); \
  AddChild(&gd);

extern CTString astrNoYes[2];
extern CTString astrSplitScreenRadioTexts[4];

static INDEX FindUnusedPlayer(void) {
  INDEX *ai = GetGameAPI()->aiMenuLocalPlayers;
  INDEX iPlayer = 0;
  for (; iPlayer < GetGameAPI()->GetProfileCount(); iPlayer++) {
    BOOL bUsed = FALSE;
    for (INDEX iLocal = 0; iLocal < MAX_GAME_LOCAL_PLAYERS; iLocal++) {
      if (ai[iLocal] == iPlayer) {
        bUsed = TRUE;
        break;
      }
    }
    if (!bUsed) {
      return iPlayer;
    }
  }
  ASSERT(FALSE);
  return iPlayer;
};

static void SelectPlayersFillMenu(void) {
  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  INDEX *ai = GetGameAPI()->aiMenuLocalPlayers;

  gmCurrent.gm_mgPlayer0Change.mg_iLocalPlayer = 0;
  gmCurrent.gm_mgPlayer1Change.mg_iLocalPlayer = 1;
  gmCurrent.gm_mgPlayer2Change.mg_iLocalPlayer = 2;
  gmCurrent.gm_mgPlayer3Change.mg_iLocalPlayer = 3;

  // [Cecil] Determine configuration via flags
  BOOL bDedicated = (gmCurrent.gm_ulConfigFlags & PLCF_DEDICATED);
  BOOL bObserving = (gmCurrent.gm_ulConfigFlags & PLCF_OBSERVING);

  if (bDedicated && GetGameAPI()->GetMenuSplitCfg() == CGame::SSC_DEDICATED) {
    gmCurrent.gm_mgDedicated.mg_iSelected = 1;
  } else {
    gmCurrent.gm_mgDedicated.mg_iSelected = 0;
  }

  gmCurrent.gm_mgDedicated.ApplyCurrentSelection();

  if (bObserving && GetGameAPI()->GetMenuSplitCfg() == CGame::SSC_OBSERVER) {
    gmCurrent.gm_mgObserver.mg_iSelected = 1;
  } else {
    gmCurrent.gm_mgObserver.mg_iSelected = 0;
  }

  gmCurrent.gm_mgObserver.ApplyCurrentSelection();

  if (GetGameAPI()->GetMenuSplitCfg() >= CGame::SSC_PLAY1) {
    gmCurrent.gm_mgSplitScreenCfg.mg_iSelected = GetGameAPI()->GetMenuSplitCfg();
    gmCurrent.gm_mgSplitScreenCfg.ApplyCurrentSelection();
  }

  BOOL bHasPlayers = TRUE;

  if (bDedicated && gmCurrent.gm_mgDedicated.mg_iSelected) {
    bObserving = FALSE;
    bHasPlayers = FALSE;
  }

  if (bObserving && gmCurrent.gm_mgObserver.mg_iSelected) {
    bHasPlayers = FALSE;
  }

  CMenuGadget *apmg[9];
  memset(apmg, 0, sizeof(apmg));
  INDEX i = 0;

  // [Cecil] Hide password field by default
  gmCurrent.gm_mgPassword.Disappear();

  if (bDedicated) {
    gmCurrent.gm_mgDedicated.Appear();
    apmg[i++] = &gmCurrent.gm_mgDedicated;
  } else {
    gmCurrent.gm_mgDedicated.Disappear();

    // [Cecil] Replace dedicated switch with a password field
    if (gmCurrent.gm_ulConfigFlags & PLCF_PASSWORD) {
      gmCurrent.gm_mgPassword.Appear();
      apmg[i++] = &gmCurrent.gm_mgPassword;
    }
  }

  if (bObserving) {
    gmCurrent.gm_mgObserver.Appear();
    apmg[i++] = &gmCurrent.gm_mgObserver;
  } else {
    gmCurrent.gm_mgObserver.Disappear();
  }

  for (INDEX iLocal = 0; iLocal < MAX_GAME_LOCAL_PLAYERS; iLocal++) {
    if (ai[iLocal] < 0 || ai[iLocal] >= GetGameAPI()->GetProfileCount()) {
      ai[iLocal] = 0;
    }
    for (INDEX iCopy = 0; iCopy < iLocal; iCopy++) {
      if (ai[iCopy] == ai[iLocal]) {
        ai[iLocal] = FindUnusedPlayer();
      }
    }
  }

  gmCurrent.gm_mgPlayer0Change.Disappear();
  gmCurrent.gm_mgPlayer1Change.Disappear();
  gmCurrent.gm_mgPlayer2Change.Disappear();
  gmCurrent.gm_mgPlayer3Change.Disappear();

  if (bHasPlayers) {
    gmCurrent.gm_mgSplitScreenCfg.Appear();
    apmg[i++] = &gmCurrent.gm_mgSplitScreenCfg;
    gmCurrent.gm_mgPlayer0Change.Appear();
    apmg[i++] = &gmCurrent.gm_mgPlayer0Change;
    if (gmCurrent.gm_mgSplitScreenCfg.mg_iSelected >= 1) {
      gmCurrent.gm_mgPlayer1Change.Appear();
      apmg[i++] = &gmCurrent.gm_mgPlayer1Change;
    }
    if (gmCurrent.gm_mgSplitScreenCfg.mg_iSelected >= 2) {
      gmCurrent.gm_mgPlayer2Change.Appear();
      apmg[i++] = &gmCurrent.gm_mgPlayer2Change;
    }
    if (gmCurrent.gm_mgSplitScreenCfg.mg_iSelected >= 3) {
      gmCurrent.gm_mgPlayer3Change.Appear();
      apmg[i++] = &gmCurrent.gm_mgPlayer3Change;
    }
  } else {
    gmCurrent.gm_mgSplitScreenCfg.Disappear();
  }
  apmg[i++] = &gmCurrent.gm_mgStart;

  // relink
  for (INDEX img = 0; img < GetGameAPI()->GetProfileCount(); img++) {
    if (apmg[img] == NULL) {
      continue;
    }
    INDEX imgPred = (img + 8 - 1) % 8;
    for (; imgPred != img; imgPred = (imgPred + 8 - 1) % 8) {
      if (apmg[imgPred] != NULL) {
        break;
      }
    }
    INDEX imgSucc = (img + 1) % 8;
    for (; imgSucc != img; imgSucc = (imgSucc + 1) % 8) {
      if (apmg[imgSucc] != NULL) {
        break;
      }
    }
    apmg[img]->mg_pmgUp = apmg[imgPred];
    apmg[img]->mg_pmgDown = apmg[imgSucc];
  }

  gmCurrent.gm_mgPlayer0Change.SetPlayerText();
  gmCurrent.gm_mgPlayer1Change.SetPlayerText();
  gmCurrent.gm_mgPlayer2Change.SetPlayerText();
  gmCurrent.gm_mgPlayer3Change.SetPlayerText();

  if (bHasPlayers && gmCurrent.gm_mgSplitScreenCfg.mg_iSelected >= 1) {
    gmCurrent.gm_mgNotes.SetText(LOCALIZE("Make sure you set different controls for each player!"));
  } else {
    gmCurrent.gm_mgNotes.SetText("");
  }
};

static void SelectPlayersApplyMenu(void) {
  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;

  // [Cecil] Determine configuration via flags
  const BOOL bDedicated = (gmCurrent.gm_ulConfigFlags & PLCF_DEDICATED);
  const BOOL bObserving = (gmCurrent.gm_ulConfigFlags & PLCF_OBSERVING);

  if (bDedicated && gmCurrent.gm_mgDedicated.mg_iSelected) {
    GetGameAPI()->SetMenuSplitCfg(CGame::SSC_DEDICATED);
    return;
  }

  if (bObserving && gmCurrent.gm_mgObserver.mg_iSelected) {
    GetGameAPI()->SetMenuSplitCfg(CGame::SSC_OBSERVER);
    return;
  }

  GetGameAPI()->SetMenuSplitCfg(gmCurrent.gm_mgSplitScreenCfg.mg_iSelected);
};

static void UpdateSelectPlayers(INDEX i) {
  SelectPlayersApplyMenu();
  SelectPlayersFillMenu();
};

void CSelectPlayersMenu::Initialize_t(void) {
  gm_strName = "SelectPlayers";
  gm_pmgSelectedByDefault = &gm_mgStart;
  gm_ulConfigFlags = 0;

  // intialize split screen menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("SELECT PLAYERS"));
  AddChild(&gm_mgTitle);

  TRIGGER_MG(gm_mgDedicated, 0, gm_mgStart, gm_mgObserver, LOCALIZE("Dedicated:"), astrNoYes);
  gm_mgDedicated.mg_strTip = LOCALIZE("select to start dedicated server");
  gm_mgDedicated.mg_pOnTriggerChange = &UpdateSelectPlayers;

  // [Cecil] Connection password field
  gm_mgPassword.SetText(cli_strConnectPassword);
  gm_mgPassword.SetName(TRANS("Password:"));
  gm_mgPassword.mg_ctMaxStringLen = 100;
  gm_mgPassword.mg_pstrToChange = &cli_strConnectPassword;
  gm_mgPassword.mg_bHiddenText = TRUE;
  gm_mgPassword.mg_boxOnScreen = BoxMediumRow(0);
  gm_mgPassword.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgPassword.mg_iCenterI = -1;
  gm_mgPassword.mg_pmgUp = &gm_mgStart;
  gm_mgPassword.mg_pmgDown = &gm_mgObserver;
  gm_mgPassword.mg_strTip = TRANS("enter the password for connecting, observing or VIP access");
  AddChild(&gm_mgPassword);

  TRIGGER_MG(gm_mgObserver, 1, gm_mgDedicated, gm_mgSplitScreenCfg, LOCALIZE("Observer:"), astrNoYes);
  gm_mgObserver.mg_strTip = LOCALIZE("select to join in for observing, not for playing");
  gm_mgObserver.mg_pOnTriggerChange = &UpdateSelectPlayers;

  // split screen config trigger
  TRIGGER_MG(gm_mgSplitScreenCfg, 2, gm_mgObserver, gm_mgPlayer0Change, LOCALIZE("Number of players:"), astrSplitScreenRadioTexts);
  gm_mgSplitScreenCfg.mg_strTip = LOCALIZE("choose more than one player to play in split screen");
  gm_mgSplitScreenCfg.mg_pOnTriggerChange = &UpdateSelectPlayers;

  gm_mgPlayer0Change.mg_iCenterI = -1;
  gm_mgPlayer1Change.mg_iCenterI = -1;
  gm_mgPlayer2Change.mg_iCenterI = -1;
  gm_mgPlayer3Change.mg_iCenterI = -1;
  gm_mgPlayer0Change.mg_boxOnScreen = BoxMediumMiddle(4);
  gm_mgPlayer1Change.mg_boxOnScreen = BoxMediumMiddle(5);
  gm_mgPlayer2Change.mg_boxOnScreen = BoxMediumMiddle(6);
  gm_mgPlayer3Change.mg_boxOnScreen = BoxMediumMiddle(7);
  gm_mgPlayer0Change.mg_strTip =
    gm_mgPlayer1Change.mg_strTip =
    gm_mgPlayer2Change.mg_strTip =
    gm_mgPlayer3Change.mg_strTip = LOCALIZE("select profile for this player");
  AddChild(&gm_mgPlayer0Change);
  AddChild(&gm_mgPlayer1Change);
  AddChild(&gm_mgPlayer2Change);
  AddChild(&gm_mgPlayer3Change);

  gm_mgNotes.mg_boxOnScreen = BoxMediumRow(9.0);
  gm_mgNotes.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgNotes.mg_iCenterI = -1;
  gm_mgNotes.mg_bEnabled = FALSE;
  gm_mgNotes.mg_bLabel = TRUE;
  AddChild(&gm_mgNotes);
  gm_mgNotes.SetText("");

  ADD_GADGET(gm_mgStart, BoxBigRow(7), &gm_mgSplitScreenCfg, &gm_mgPlayer0Change, NULL, NULL, LOCALIZE("START"));
  gm_mgStart.mg_bfsFontSize = BFS_LARGE;
  gm_mgStart.mg_iCenterI = 0;
}

void CSelectPlayersMenu::StartMenu(void) {
  CGameMenu::StartMenu();
  SelectPlayersFillMenu();
  SelectPlayersApplyMenu();
}

void CSelectPlayersMenu::EndMenu(void) {
  SelectPlayersApplyMenu();
  CGameMenu::EndMenu();
}