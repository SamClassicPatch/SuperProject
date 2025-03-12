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
#include "MNetworkJoin.h"

static void StartSelectServerLAN(void) {
  CServersMenu &gmCurrent = _pGUIM->gmServersMenu;

  gmCurrent.m_bInternet = FALSE;
  ChangeToMenu(&gmCurrent);
};

static void StartSelectServerNET(void) {
  CServersMenu &gmCurrent = _pGUIM->gmServersMenu;

  gmCurrent.m_bInternet = TRUE;
  ChangeToMenu(&gmCurrent);
};

void CNetworkJoinMenu::Initialize_t(void) {
  gm_strName = "NetworkJoin";
  gm_pmgSelectedByDefault = &gm_mgLAN;

  // title
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("JOIN GAME"));
  AddChild(&gm_mgTitle);

  gm_mgLAN.mg_bfsFontSize = BFS_LARGE;
  gm_mgLAN.mg_boxOnScreen = BoxBigRow(1.0f);
  gm_mgLAN.mg_pmgUp = &gm_mgOpen;
  gm_mgLAN.mg_pmgDown = &gm_mgNET;
  gm_mgLAN.SetText(LOCALIZE("SEARCH LAN"));
  gm_mgLAN.mg_strTip = LOCALIZE("search local network for servers");
  AddChild(&gm_mgLAN);
  gm_mgLAN.mg_pActivatedFunction = &StartSelectServerLAN;

  gm_mgNET.mg_bfsFontSize = BFS_LARGE;
  gm_mgNET.mg_boxOnScreen = BoxBigRow(2.0f);
  gm_mgNET.mg_pmgUp = &gm_mgLAN;
  gm_mgNET.mg_pmgDown = &gm_mgOpen;
  gm_mgNET.SetText(LOCALIZE("SEARCH INTERNET"));
  gm_mgNET.mg_strTip = LOCALIZE("search internet for servers");
  AddChild(&gm_mgNET);
  gm_mgNET.mg_pActivatedFunction = &StartSelectServerNET;

  gm_mgOpen.mg_bfsFontSize = BFS_LARGE;
  gm_mgOpen.mg_boxOnScreen = BoxBigRow(3.0f);
  gm_mgOpen.mg_pmgUp = &gm_mgNET;
  gm_mgOpen.mg_pmgDown = &gm_mgLAN;
  gm_mgOpen.SetText(LOCALIZE("SPECIFY SERVER"));
  gm_mgOpen.mg_strTip = LOCALIZE("type in server address to connect to");
  AddChild(&gm_mgOpen);
  gm_mgOpen.mg_pActivatedFunction = &CNetworkOpenMenu::ChangeTo;
}

// [Cecil] Change to the menu
void CNetworkJoinMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmNetworkJoinMenu);
};
