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
#include "MNetworkOpen.h"

extern CTFileName _fnmModToLoad;
extern CTString _strModServerJoin;
extern CTString _strURLToVisit;

CTFileName _fnmModSelected;
static CTString _strModURLSelected;
static CTString _strModServerSelected;

static void ExitAndSpawnExplorer(void) {
  _bRunning = FALSE;
  _bQuitScreen = FALSE;
  _strURLToVisit = _strModURLSelected;
};

static void ModNotInstalled(void) {
  CTString strNoMod;
  strNoMod.PrintF(LOCALIZE("You don't have MOD '%s' installed.\nDo you want to visit its web site?"), _fnmModSelected.str_String);

  // [Cecil] Append the link that will be opened
  strNoMod += "\n\n";
  strNoMod += TRANS("The following link will open in your web browser:\n");
  strNoMod += _strModURLSelected;

  CConfirmMenu::ChangeTo(strNoMod, &ExitAndSpawnExplorer, NULL, FALSE, "", "", 0.3f);
};

static void ModConnect(void) {
  _fnmModToLoad = _fnmModSelected;
  _strModServerJoin = _strModServerSelected;
};

static void ModConnectConfirm(void) {
  if (_fnmModSelected == " ") {
    _fnmModSelected = CTString("SeriousSam");
  }

  CTFileName fnmModPath = "Mods\\" + _fnmModSelected + "\\";
  if (!FileExists(fnmModPath + "BaseWriteInclude.lst")
   && !FileExists(fnmModPath + "BaseWriteExclude.lst")
   && !FileExists(fnmModPath + "BaseBrowseInclude.lst")
   && !FileExists(fnmModPath + "BaseBrowseExclude.lst")) {
    ModNotInstalled();
    return;
  }

  CPrintF(LOCALIZE("Server is running a different MOD (%s).\nYou need to reload to connect.\n"), _fnmModSelected);

  CConfirmMenu::ChangeTo(LOCALIZE("CHANGE THE MOD?"), &ModConnect, NULL, TRUE);
};

void JoinNetworkGame(void) {
  GetGameAPI()->SetStartSplitCfg(GetGameAPI()->GetMenuSplitCfg());

  // [Cecil] Set start players from menu players
  GetGameAPI()->SetStartProfilesFromMenuProfiles();

  GetGameAPI()->SetNetworkProvider(CGameAPI::NP_CLIENT);
  if (_pGame->JoinGame(CNetworkSession(GetGameAPI()->JoinAddress()))) {
    StopMenus();
    _gmRunningGameMode = GM_NETWORK;
  } else {
    if (_pNetwork->ga_strRequiredMod != "") {
      extern CTString _strModServerJoin;
      char strModName[256] = {0};
      char strModURL[256] = {0};
      _pNetwork->ga_strRequiredMod.ScanF("%250[^\\]\\%s", &strModName, &strModURL);
      _fnmModSelected = CTString(strModName);
      _strModURLSelected = strModURL;
      if (_strModURLSelected == "") {
        _strModURLSelected = "http://www.croteam.com/mods/Old";
      }
      _strModServerSelected.PrintF("%s:%s", GetGameAPI()->JoinAddress(), _pShell->GetValue("net_iPort"));
      ModConnectConfirm();
    }
    _gmRunningGameMode = GM_NONE;
  }
};

// [Cecil] Open menu for joining a remote server
void StartJoinServerMenu(void) {
  // Setup player selection menu in advance (but don't change to it!)
  CSelectPlayersMenu &gmCurrent = _pGUIM->gmSelectPlayersMenu;
  gmCurrent.gm_ulConfigFlags = PLCF_OBSERVING | PLCF_PASSWORD;
  gmCurrent.gm_mgStart.mg_pActivatedFunction = &JoinNetworkGame;

  // Open network settings list
  extern void StartNetworkSettingsMenu(void);
  StartNetworkSettingsMenu();

  // Go to the player selection menu afterwards
  extern CGameMenu *_pgmAfterNetSetting;
  _pgmAfterNetSetting = &gmCurrent;
};

void CNetworkOpenMenu::Initialize_t(void) {
  gm_strName = "NetworkOpen";
  gm_pmgSelectedByDefault = &gm_mgJoin;

  // intialize network join menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("JOIN"));
  AddChild(&gm_mgTitle);

  gm_mgAddressLabel.SetText(LOCALIZE("Address:"));
  gm_mgAddressLabel.mg_boxOnScreen = BoxMediumLeft(1);
  gm_mgAddressLabel.mg_iCenterI = -1;
  AddChild(&gm_mgAddressLabel);

  gm_mgAddress.SetText(GetGameAPI()->JoinAddress());
  gm_mgAddress.mg_ctMaxStringLen = 20;
  gm_mgAddress.mg_pstrToChange = &GetGameAPI()->JoinAddress();
  gm_mgAddress.mg_boxOnScreen = BoxMediumMiddle(1);
  gm_mgAddress.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgAddress.mg_iCenterI = -1;
  gm_mgAddress.mg_pmgUp = &gm_mgJoin;
  gm_mgAddress.mg_pmgDown = &gm_mgPort;
  gm_mgAddress.mg_strTip = LOCALIZE("specify server address");
  AddChild(&gm_mgAddress);

  gm_mgPortLabel.SetText(LOCALIZE("Port:"));
  gm_mgPortLabel.mg_boxOnScreen = BoxMediumLeft(2);
  gm_mgPortLabel.mg_iCenterI = -1;
  AddChild(&gm_mgPortLabel);

  gm_mgPort.SetText("");
  gm_mgPort.mg_ctMaxStringLen = 10;
  gm_mgPort.mg_pstrToChange = &gm_strPort;
  gm_mgPort.mg_boxOnScreen = BoxMediumMiddle(2);
  gm_mgPort.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgPort.mg_iCenterI = -1;
  gm_mgPort.mg_pmgUp = &gm_mgAddress;
  gm_mgPort.mg_pmgDown = &gm_mgJoin;
  gm_mgPort.mg_strTip = LOCALIZE("specify server address");
  AddChild(&gm_mgPort);

  gm_mgJoin.mg_boxOnScreen = BoxMediumMiddle(3);
  gm_mgJoin.mg_pmgUp = &gm_mgPort;
  gm_mgJoin.mg_pmgDown = &gm_mgAddress;
  gm_mgJoin.SetText(LOCALIZE("Join"));
  AddChild(&gm_mgJoin);
  gm_mgJoin.mg_pActivatedFunction = &StartJoinServerMenu;
}

void CNetworkOpenMenu::StartMenu(void) {
  gm_strPort = _pShell->GetValue("net_iPort");
  gm_mgPort.SetText(gm_strPort);
}

void CNetworkOpenMenu::EndMenu(void) {
  _pShell->SetValue("net_iPort", gm_strPort);
}

// [Cecil] Change to the menu
void CNetworkOpenMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmNetworkOpenMenu);
};
