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
#include "MServers.h"

// [Cecil] Classics patch
#include <CoreLib/Query/QueryManager.h>

CTString _strServerFilter[7];
CMGButton mgServerColumn[7];
CMGEdit mgServerFilter[7];

// [Cecil] Available master servers
static const char *_astrServerNames[] = {
  "333networks",
  "The Errorist Network",
  "GoneSpy",
  "42amsterdam",
};

static const char *_astrServerValues[] = {
  "333networks.com",
  "master.errorist.eu",
  "master.gonespy.com",
  "42amsterdam.net",
};

static const INDEX _ctMS = ARRAYCOUNT(_astrServerValues);

#define INVALID_MS_BUTTON LOCALIZE("Custom")

// [Cecil] Find index of the current master server
static INDEX GetCurrentMasterServer(void) {
  static CSymbolPtr pstrLegacyMS("ms_strLegacyMS");

  // Fail-safe
  if (pstrLegacyMS.Exists())
  {
    for (INDEX i = 0; i < _ctMS; i++) {
      if (_astrServerValues[i] == pstrLegacyMS.GetString()) return i;
    }
  }

  // Past the limit
  return -1;
};

// [Cecil] Select new master server
static void SwitchMasterServer(void) {
  static CSymbolPtr pstrLegacyMS("ms_strLegacyMS");
  CMGButton &mgServer = _pGUIM->gmServersMenu.gm_mgMasterServer;

  // Fail-safe
  if (!pstrLegacyMS.Exists()) {
    mgServer.SetText(INVALID_MS_BUTTON);
    return;
  }

  // Find index of the current master server
  INDEX i = GetCurrentMasterServer();

  // Get the next one and wrap it around (if current one is -1, starts from the beginning)
  i = (i + 1) % _ctMS;

  // Set button name and master server
  mgServer.SetText(_astrServerNames[i]);
  pstrLegacyMS.GetString() = _astrServerValues[i];

  // Call UpdateInternalGameSpyMS() to update the actual master server
  CShellSymbol *pss = pstrLegacyMS._pss;

  if (pss->ss_pPostFunc != NULL) {
    pss->ss_pPostFunc(pss);
  }
};

static void RefreshServerList(void) {
  // [Cecil] Own method
  IMasterServer::EnumSessions(_pGUIM->gmServersMenu.m_bInternet);
};

static void SortByColumn(int i) {
  CServersMenu &gmCurrent = _pGUIM->gmServersMenu;

  if (gmCurrent.gm_mgList.mg_iSort == i) {
    gmCurrent.gm_mgList.mg_bSortDown = !gmCurrent.gm_mgList.mg_bSortDown;
  } else {
    gmCurrent.gm_mgList.mg_bSortDown = FALSE;
  }

  gmCurrent.gm_mgList.mg_iSort = i;
};

static void SortByServer(void)  { SortByColumn(0); }
static void SortByMap(void)     { SortByColumn(1); }
static void SortByPing(void)    { SortByColumn(2); }
static void SortByPlayers(void) { SortByColumn(3); }
static void SortByGame(void)    { SortByColumn(4); }
static void SortByMod(void)     { SortByColumn(5); }
static void SortByVer(void)     { SortByColumn(6); }

void CServersMenu::Initialize_t(void) {
  gm_strName = "Servers";
  gm_pmgSelectedByDefault = &gm_mgList;

  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("CHOOSE SERVER"));
  AddChild(&gm_mgTitle);

  gm_mgList.mg_boxOnScreen = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(1, 1));
  gm_mgList.mg_pmgLeft = &gm_mgList; // make sure it can get focus
  gm_mgList.mg_bEnabled = TRUE;
  AddChild(&gm_mgList);

  ASSERT(ARRAYCOUNT(mgServerColumn) == ARRAYCOUNT(mgServerFilter));
  for (INDEX i = 0; i < ARRAYCOUNT(mgServerFilter); i++) {
    mgServerColumn[i].SetText("");
    mgServerColumn[i].mg_boxOnScreen = BoxPlayerEdit(5.0);
    mgServerColumn[i].mg_bfsFontSize = BFS_SMALL;
    mgServerColumn[i].mg_iCenterI = -1;
    mgServerColumn[i].mg_pmgUp = &gm_mgList;
    mgServerColumn[i].mg_pmgDown = &mgServerFilter[i];
    AddChild(&mgServerColumn[i]);

    mgServerFilter[i].mg_ctMaxStringLen = 25;
    mgServerFilter[i].mg_boxOnScreen = BoxPlayerEdit(5.0);
    mgServerFilter[i].mg_bfsFontSize = BFS_SMALL;
    mgServerFilter[i].mg_iCenterI = -1;
    mgServerFilter[i].mg_pmgUp = &mgServerColumn[i];
    mgServerFilter[i].mg_pmgDown = &gm_mgList;
    AddChild(&mgServerFilter[i]);
    mgServerFilter[i].mg_pstrToChange = &_strServerFilter[i];
    mgServerFilter[i].SetText(*mgServerFilter[i].mg_pstrToChange);
  }

  gm_mgRefresh.SetText(LOCALIZE("REFRESH"));
  gm_mgRefresh.mg_boxOnScreen = BoxLeftColumn(15.0);
  gm_mgRefresh.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgRefresh.mg_iCenterI = -1;
  gm_mgRefresh.mg_pmgUp = &gm_mgList;
  gm_mgRefresh.mg_pmgDown = &gm_mgList;
  gm_mgRefresh.mg_pActivatedFunction = &RefreshServerList;
  AddChild(&gm_mgRefresh);

  // [Cecil] Master server switch
  gm_mgMasterServer.mg_strTip = TRANS("switch master server for the Legacy protocol");
  gm_mgMasterServer.mg_boxOnScreen = BoxRightColumn(15.0);
  gm_mgMasterServer.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgMasterServer.mg_iCenterI = +1;
  gm_mgMasterServer.mg_pmgUp = &gm_mgList;
  gm_mgMasterServer.mg_pmgDown = &gm_mgList;
  gm_mgMasterServer.mg_pActivatedFunction = &SwitchMasterServer;
  AddChild(&gm_mgMasterServer);

  CTString astrColumns[7];
  mgServerColumn[0].SetText(LOCALIZE("Server"));
  mgServerColumn[1].SetText(LOCALIZE("Map"));
  mgServerColumn[2].SetText(LOCALIZE("Ping"));
  mgServerColumn[3].SetText(LOCALIZE("Players"));
  mgServerColumn[4].SetText(LOCALIZE("Game"));
  mgServerColumn[5].SetText(LOCALIZE("Mod"));
  mgServerColumn[6].SetText(LOCALIZE("Ver"));

  mgServerColumn[0].mg_pActivatedFunction = &SortByServer;
  mgServerColumn[1].mg_pActivatedFunction = &SortByMap;
  mgServerColumn[2].mg_pActivatedFunction = &SortByPing;
  mgServerColumn[3].mg_pActivatedFunction = &SortByPlayers;
  mgServerColumn[4].mg_pActivatedFunction = &SortByGame;
  mgServerColumn[5].mg_pActivatedFunction = &SortByMod;
  mgServerColumn[6].mg_pActivatedFunction = &SortByVer;

  mgServerColumn[0].mg_strTip = LOCALIZE("sort by server");
  mgServerColumn[1].mg_strTip = LOCALIZE("sort by map");
  mgServerColumn[2].mg_strTip = LOCALIZE("sort by ping");
  mgServerColumn[3].mg_strTip = LOCALIZE("sort by players");
  mgServerColumn[4].mg_strTip = LOCALIZE("sort by game");
  mgServerColumn[5].mg_strTip = LOCALIZE("sort by mod");
  mgServerColumn[6].mg_strTip = LOCALIZE("sort by version");

  mgServerFilter[0].mg_strTip = LOCALIZE("filter by server");
  mgServerFilter[1].mg_strTip = LOCALIZE("filter by map");
  mgServerFilter[2].mg_strTip = LOCALIZE("filter by ping (ie. <200)");
  mgServerFilter[3].mg_strTip = LOCALIZE("filter by players (ie. >=2)");
  mgServerFilter[4].mg_strTip = LOCALIZE("filter by game (ie. coop)");
  mgServerFilter[5].mg_strTip = LOCALIZE("filter by mod");
  mgServerFilter[6].mg_strTip = LOCALIZE("filter by version");
}

void CServersMenu::StartMenu(void) {
  RefreshServerList();

  // [Cecil] Select current master server name
  INDEX i = GetCurrentMasterServer();

  if (i == -1) {
    gm_mgMasterServer.SetText(INVALID_MS_BUTTON);
  } else {
    gm_mgMasterServer.SetText(_astrServerNames[i]);
  }

  CGameMenu::StartMenu();
}

void CServersMenu::Think(void) {
  if (!_pNetwork->ga_bEnumerationChange) {
    return;
  }
  _pNetwork->ga_bEnumerationChange = FALSE;
}

// [Cecil] Change to the menu
void CServersMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmServersMenu);
};
