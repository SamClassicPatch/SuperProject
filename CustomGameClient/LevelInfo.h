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

extern CListHead _lhAutoDemos;
extern CListHead _lhAllLevels;
extern CListHead _lhFilteredLevels;

// [Cecil] Categories with level lists
extern CStaticArray<CFileList> _aLevelCategories;

class CLevelInfo {
  public:
    CListNode li_lnNode;
    CTFileName li_fnLevel;
    CTString li_strName;
    ULONG li_ulSpawnFlags;
    ELevelFormat li_eFormat; // [Cecil] Level format type

    CLevelInfo(void);
    CLevelInfo(const CLevelInfo &li);
    void operator=(const CLevelInfo &li);
};

// find all levels that match given flags
void FilterLevels(ULONG ulSpawnFlags, INDEX iCategory); // [Cecil] Level category

// init level-info subsystem
void LoadLevelsList(void);
// cleanup level-info subsystem
void ClearLevelsList(void);
// get level info for its filename
CLevelInfo FindLevelByFileName(const CTFileName &fnm);
// if level doesn't support given flags, find one that does
void ValidateLevelForFlags(ULONG ulSpawnFlags);

// [Cecil] Display list of available levels
void ListLevels(SHELL_FUNC_ARGS);

// init list of autoplay demos
void LoadDemosList(void);
// clear list of autoplay demos
void ClearDemosList(void);
