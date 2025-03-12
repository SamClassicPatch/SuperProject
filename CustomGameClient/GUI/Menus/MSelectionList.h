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

#ifndef SE_INCL_GAME_MENU_SELECTIONLIST_H
#define SE_INCL_GAME_MENU_SELECTIONLIST_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "GameMenu.h"
#include "GUI/Components/MGArrow.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGFileButton.h"
#include "GUI/Components/MGTitle.h"
#include "Cecil/MGScrollbar.h"

#define SELECTLIST_BUTTONS_CT 14

class CSelectListMenu : public CGameMenu {
  public:
    // Settings adjusted before starting the menu
    CTString gm_strSelected;   // Button that's selected initially
    BOOL gm_bAllowThumbnails;  // Sets thumbnail for a currently selected button

    // Function to activate when button is pressed
    // Returns TRUE if loading or saving has succeeded - description is saved automatically
    BOOL (*gm_pAfterFileChosen)(const CTFileName &fnm);

    // Internal properties
    CListHead gm_lhFileInfos; // All file infos to list

    CMGTitle gm_mgTitle;
    CMGButton gm_mgNotes;
    CMGFileButton gm_amgButton[SELECTLIST_BUTTONS_CT];
    CMGArrow gm_mgArrowUp;
    CMGArrow gm_mgArrowDn;
    CMGScrollbar gm_mgScrollbar; // [Cecil]

    void Initialize_t(void);
    void StartMenu(void);
    void EndMenu(void);
    void FillListItems(void);

    // Create new buttons with file infos
    virtual void CreateButtons(void) {};
};

#endif /* include-once check. */