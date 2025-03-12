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

#ifndef SE_INCL_GAME_MENU_LEVELS_H
#define SE_INCL_GAME_MENU_LEVELS_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "GameMenu.h"
#include "GUI/Components/MGArrow.h"
#include "GUI/Components/MGLevelButton.h"
#include "GUI/Components/MGTitle.h"
#include "Cecil/MGScrollbar.h"

class CLevelsMenu : public CGameMenu {
  public:
    CMGTitle gm_mgTitle;
    CMGLevelButton gm_mgManualLevel[LEVELS_ON_SCREEN];
    CMGArrow gm_mgArrowUp;
    CMGArrow gm_mgArrowDn;
    CMGScrollbar gm_mgScrollbar; // [Cecil]

    CMGButton gm_mgFiltersLabel; // [Cecil] Filters label
    CMGButton gm_mgVisibility;   // [Cecil] Level visibility switch
    CMGButton gm_mgLevelFormat;  // [Cecil] Level format switch
    CMGEdit gm_mgTitleFilter;    // [Cecil] Level title filter

    ULONG gm_ulSpawnFlags; // [Cecil] Level visibility flags
    INDEX gm_iCategory; // [Cecil] Level category

    void Initialize_t(void);
    void FillListItems(void);
    void StartMenu(void);
    void Think(void); // [Cecil]

    // [Cecil] Default constructor
    CLevelsMenu() : CGameMenu(), gm_ulSpawnFlags(0), gm_iCategory(-1)
    {
    };
};

#endif /* include-once check. */