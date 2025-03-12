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

#ifndef SE_INCL_GAME_MENU_LOADSAVE_H
#define SE_INCL_GAME_MENU_LOADSAVE_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "GameMenu.h"
#include "MSelectionList.h"

#include "GUI/Components/MGArrow.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGFileButton.h"
#include "GUI/Components/MGTitle.h"

enum ELSSortType {
  LSSORT_NONE,
  LSSORT_NAMEUP,
  LSSORT_NAMEDN,
  LSSORT_FILEUP,
  LSSORT_FILEDN,
};

class CLoadSaveMenu : public CSelectListMenu {
  public:
    // Settings adjusted before starting the menu
    CTFileName gm_fnmDirectory; // directory that should be read
    CTFileName gm_fnmBaseName;  // base file name for saving (numbers are auto-added)
    CTFileName gm_fnmExt;       // accepted file extension
    ULONG gm_ulListFlags;       // [Cecil] Flags to pass into ListGameFiles()
    BOOL gm_bSave;              // set when chosing file for saving
    BOOL gm_bManage;            // set if managing (rename/delet is enabled)
    CTString gm_strSaveDes;     // default description (if saving)

    INDEX gm_iSortType; // sort type

    // Internal properties
    INDEX gm_iLastFile; // Index of the last saved file in numbered format

    void Initialize_t(void);
    void FillListItems(void);

    // Called to get info of a file from directory, or to skip it
    BOOL ParseFile(const CTFileName &fnm, CTString &strName);

    // Create new buttons with file infos
    void CreateButtons(void);
};

#endif /* include-once check. */