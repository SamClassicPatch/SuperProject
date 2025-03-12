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

#ifndef SE_INCL_GAME_MENU_VAR_H
#define SE_INCL_GAME_MENU_VAR_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "GameMenu.h"
#include "GUI/Components/MGArrow.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGTitle.h"
#include "GUI/Components/MGVarButton.h"
#include "Cecil/MGScrollbar.h"

class CVarMenu : public CGameMenu {
  public:
    CTFileName gm_fnmMenuCFG;

    CMGTitle gm_mgTitle;
    CMGVarButton gm_mgVar[VARS_ON_SCREEN];
    CMGButton gm_mgApply;
    CMGArrow gm_mgArrowUp;
    CMGArrow gm_mgArrowDn;
    CMGScrollbar gm_mgScrollbar; // [Cecil]

    // [Cecil] Set when applying option changes
    BOOL gm_bApplying;

    // [Cecil] Option tabs
    CStaticStackArray<CMGButton> gm_agmTabs;
    INDEX gm_iTab; // Currently selected tab

    // [Cecil] Current position before applying changes
    INDEX gm_iLastListWantedItem;
    INDEX gm_iLastTab;

    void Initialize_t(void);
    void FillListItems(void);
    void StartMenu(void);
    void EndMenu(void);
    void Think(void);

    // [Cecil] Change to the menu
    static void ChangeTo(const CTString &strTitle, const CTFileName &fnmConfig, const CTString &strMenu = "Var");
};

#endif /* include-once check. */