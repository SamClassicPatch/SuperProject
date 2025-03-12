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

#ifndef SE_INCL_MENUMANAGER_H
#define SE_INCL_MENUMANAGER_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "MConfirm.h"
#include "MControls.h"
#include "MCustomizeAxis.h"
#include "MCustomizeKeyboard.h"
#include "MCredits.h"
#include "MHighScore.h"
#include "MInGame.h"
#include "MLevels.h"
#include "MMain.h"
#include "MNetwork.h"
#include "MNetworkJoin.h"
#include "MNetworkOpen.h"
#include "MNetworkStart.h"
#include "MOptions.h"
#include "MRenderingOptions.h"
#include "MServers.h"
#include "MSinglePlayer.h"
#include "MSinglePlayerNew.h"
#include "MSplitScreen.h"
#include "MSplitStart.h"
#include "MVar.h"
#include "Cecil/MLevelCategories.h" // [Cecil]
#include "Cecil/MExtras.h" // [Cecil]
#include "Cecil/MPatchCredits.h" // [Cecil]

class CMenuManager {
  public:
    // [Cecil] List of previously visited menus
    // Each time the "Back" button is pressed, it pops the last menu and switches
    // to it, otherwise returns to the game (if it's active) or to the main menu
    CStaticStackArray<CGameMenu *> aVisitedMenus;

    CConfirmMenu gmConfirmMenu;
    CMainMenu gmMainMenu;
    CExtrasMenu gmExtras; // [Cecil]
    CPatchCreditsMenu gmPatchCredits; // [Cecil]
    CInGameMenu gmInGameMenu;
    CSinglePlayerMenu gmSinglePlayerMenu;
    CLevelCategoriesMenu gmLevelCategories; // [Cecil]
    CSinglePlayerNewMenu gmSinglePlayerNewMenu;
    CLevelsMenu gmLevelsMenu;
    CVarMenu gmVarMenu;
    CPlayerProfileMenu gmPlayerProfile;
    CControlsMenu gmControls;
    CLoadSaveMenu gmLoadSaveMenu;
    CHighScoreMenu gmHighScoreMenu;
    CCustomizeKeyboardMenu gmCustomizeKeyboardMenu;
    CServersMenu gmServersMenu;
    CCustomizeAxisMenu gmCustomizeAxisMenu;
    COptionsMenu gmOptionsMenu;
    CNetworkMenu gmNetworkMenu;
    CNetworkJoinMenu gmNetworkJoinMenu;
    CNetworkStartMenu gmNetworkStartMenu;
    CNetworkOpenMenu gmNetworkOpenMenu;
    CSplitScreenMenu gmSplitScreenMenu;
    CSplitStartMenu gmSplitStartMenu;
    CSelectPlayersMenu gmSelectPlayersMenu;
};

extern CMenuManager *_pGUIM; // TODO: Make singleton!

#endif /* include-once check. */