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

#include "MenuStuff.h"

// [Cecil] Removed "ETRS" tag to not let strings be picked up by the Depend utility
#define RADIOTRANS(str) (str)

extern CTString astrNoYes[] = {
  RADIOTRANS("No"),
  RADIOTRANS("Yes"),
};

extern CTString astrComputerInvoke[] = {
  RADIOTRANS("Use"),
  RADIOTRANS("Double-click use"),
};

extern CTString astrWeapon[] = {
  RADIOTRANS("Only if new"),
  RADIOTRANS("Never"),
  RADIOTRANS("Always"),
  RADIOTRANS("Only if stronger"),
};

extern CTString astrCrosshair[] = {
  "",
  "Textures\\Interface\\Crosshairs\\Crosshair1.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair2.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair3.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair4.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair5.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair6.tex",
  "Textures\\Interface\\Crosshairs\\Crosshair7.tex",
};

// [Cecil] Uninitialized dynamic array
extern CTString *astrMaxPlayersRadioTexts = NULL;

// here, we just reserve space for up to 16 different game types
// actual names are added later
extern CTString astrGameTypeRadioTexts[] = {
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
};

extern INDEX ctGameTypeRadioTexts = 1;

// [Cecil] 16 difficulties that are filled later
extern CTString astrDifficultyRadioTexts[] = {
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
};

extern CTString astrSplitScreenRadioTexts[] = {
  RADIOTRANS("1"),
  RADIOTRANS("2 - split screen"),
  RADIOTRANS("3 - split screen"),
  RADIOTRANS("4 - split screen"),
};

extern CTString astrDisplayPrefsRadioTexts[] = {
  RADIOTRANS("Speed"),
  RADIOTRANS("Normal"),
  RADIOTRANS("Quality"),
  RADIOTRANS("Extreme quality"), // [Cecil]
  RADIOTRANS("Custom"),
};

// initialize game type strings table
void InitGameTypes(void) {
  // for each mode
  for (ctGameTypeRadioTexts = 0; ctGameTypeRadioTexts < ARRAYCOUNT(astrGameTypeRadioTexts); ctGameTypeRadioTexts++) {
    // get the text
    CTString strMode = GetGameAPI()->GetGameTypeNameSS(ctGameTypeRadioTexts);
    // if no mode modes
    if (strMode == "") {
      // stop
      break;
    }
    // add that mode
    astrGameTypeRadioTexts[ctGameTypeRadioTexts] = strMode;
  }

  // [Cecil] No game types added
  if (ctGameTypeRadioTexts == 0) {
    astrGameTypeRadioTexts[0] = "<???>";
    ctGameTypeRadioTexts = 1;
  }

  // [Cecil] Add difficulty names
  const INDEX ctDiffs = ClampUp(ClassicsModData_DiffArrayLength(), (int)ARRAYCOUNT(astrDifficultyRadioTexts));
  INDEX ctAdded = 0;

  for (; ctAdded < ctDiffs; ctAdded++) {
    CTString strDiff = ClassicsModData_GetDiff(ctAdded)->m_strName;

    // No more difficulties
    if (strDiff == "") break;

    astrDifficultyRadioTexts[ctAdded] = strDiff;
  }

  // [Cecil] No difficulties added
  if (ctAdded == 0) {
    astrDifficultyRadioTexts[0] = "<???>";
  }
}

int qsort_CompareFileInfos_NameUp(const void *elem1, const void *elem2) {
  const CFileInfo &fi1 = **(CFileInfo **)elem1;
  const CFileInfo &fi2 = **(CFileInfo **)elem2;
  return strcmp(fi1.fi_strName, fi2.fi_strName);
}

int qsort_CompareFileInfos_NameDn(const void *elem1, const void *elem2) {
  const CFileInfo &fi1 = **(CFileInfo **)elem1;
  const CFileInfo &fi2 = **(CFileInfo **)elem2;
  return -strcmp(fi1.fi_strName, fi2.fi_strName);
}

int qsort_CompareFileInfos_FileUp(const void *elem1, const void *elem2) {
  const CFileInfo &fi1 = **(CFileInfo **)elem1;
  const CFileInfo &fi2 = **(CFileInfo **)elem2;
  return strcmp(fi1.fi_fnFile, fi2.fi_fnFile);
}

int qsort_CompareFileInfos_FileDn(const void *elem1, const void *elem2) {
  const CFileInfo &fi1 = **(CFileInfo **)elem1;
  const CFileInfo &fi2 = **(CFileInfo **)elem2;
  return -strcmp(fi1.fi_fnFile, fi2.fi_fnFile);
}

GfxAPIType NormalizeGfxAPI(INDEX i) {
  switch (i) {
    case GAT_OGL: return GAT_OGL;
#ifdef SE1_D3D
    case GAT_D3D: return GAT_D3D;
#endif // SE1_D3D
    default: ASSERT(FALSE); return GAT_OGL;
  }
}

DisplayDepth NormalizeDepth(INDEX i) {
  switch (i) {
    case DD_DEFAULT: return DD_DEFAULT;
    case DD_16BIT:   return DD_16BIT;
    case DD_32BIT:   return DD_32BIT;
    default: ASSERT(FALSE); return DD_DEFAULT;
  }
};

// controls that are currently customized
CTFileName _fnmControlsToCustomize = CTString("");

void ControlsMenuOn() {
  _pGame->SavePlayersAndControls();
  try {
    GetGameAPI()->GetControls()->Load_t(_fnmControlsToCustomize);
  } catch (char *strError) {
    WarningMessage(strError);
  }
}

void ControlsMenuOff() {
  CControls &ctrl = *GetGameAPI()->GetControls();

  try {
    if (ctrl.ctrl_lhButtonActions.Count() > 0) {
      ctrl.Save_t(_fnmControlsToCustomize);
    }
  } catch (char *strError) {
    FatalError(strError);
  }
  FORDELETELIST(CButtonAction, ba_lnNode, ctrl.ctrl_lhButtonActions, itAct) {
    delete &itAct.Current();
  }
  _pGame->LoadPlayersAndControls();
}