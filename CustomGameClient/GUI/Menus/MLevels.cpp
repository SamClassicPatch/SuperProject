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
#include "LevelInfo.h"
#include "MLevels.h"

// [Cecil] For menu reloading
extern INDEX sam_bShowAllLevels;

static CTString _strLastTitleFilter = "";

// [Cecil] Toggle level visibility and reload the menu
static void ChangeLevelVisibility(void) {
  // Toggle visibility
  sam_bShowAllLevels = !sam_bShowAllLevels;

  // Reload the menu
  CLevelsMenu &gmCurrent = _pGUIM->gmLevelsMenu;
  gmCurrent.EndMenu();
  gmCurrent.StartMenu();
};

// [Cecil] Toggle level format and reload the menu
static void ToggleMountedLevels(void) {
  // Choose level format to show
  switch (sam_iShowLevelFormat) {
    case -1:           sam_iShowLevelFormat = E_LF_CURRENT; break;
  #if CLASSIC_TSE_FUSION_MODE
    case E_LF_CURRENT: sam_iShowLevelFormat = E_LF_TFE; break;
    case E_LF_TFE:     sam_iShowLevelFormat = E_LF_SSR; break;
  #endif
    default:           sam_iShowLevelFormat = -1;
  }

  // Reload the menu
  CLevelsMenu &gmCurrent = _pGUIM->gmLevelsMenu;
  gmCurrent.EndMenu();
  gmCurrent.StartMenu();
};

void CLevelsMenu::Initialize_t(void) {
  gm_strName = "Levels";
  gm_pmgSelectedByDefault = &gm_mgManualLevel[0];

  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("CHOOSE LEVEL"));
  AddChild(&gm_mgTitle);

  for (INDEX iLabel = 0; iLabel < LEVELS_ON_SCREEN; iLabel++) {
    INDEX iPrev = (LEVELS_ON_SCREEN + iLabel - 1) % LEVELS_ON_SCREEN;
    INDEX iNext = (iLabel + 1) % LEVELS_ON_SCREEN;
    // initialize label gadgets
    gm_mgManualLevel[iLabel].mg_pmgUp = &gm_mgManualLevel[iPrev];
    gm_mgManualLevel[iLabel].mg_pmgDown = &gm_mgManualLevel[iNext];
    gm_mgManualLevel[iLabel].mg_boxOnScreen = BoxMediumRow(iLabel - 1.0f);
    gm_mgManualLevel[iLabel].mg_pActivatedFunction = NULL; // never called!
    AddChild(&gm_mgManualLevel[iLabel]);
  }

  gm_mgArrowUp.SetupForMenu(this, AD_UP, &gm_mgManualLevel[0]);
  gm_mgArrowUp.mg_pmgDown = &gm_mgVisibility;
  gm_mgArrowDn.SetupForMenu(this, AD_DOWN, &gm_mgManualLevel[LEVELS_ON_SCREEN - 1]);
  gm_mgArrowDn.mg_pmgUp = &gm_mgTitleFilter;

  // [Cecil] Scrollbar between the arrows
  gm_mgScrollbar.mg_pmgUp = &gm_mgArrowUp;
  gm_mgScrollbar.mg_pmgDown = &gm_mgArrowDn;
  AddChild(&gm_mgScrollbar);

  gm_ctListVisible = LEVELS_ON_SCREEN;
  gm_pmgArrowUp = &gm_mgArrowUp;
  gm_pmgArrowDn = &gm_mgArrowDn;
  gm_pmgListTop = &gm_mgManualLevel[0];
  gm_pmgListBottom = &gm_mgManualLevel[LEVELS_ON_SCREEN - 1];

  // [Cecil] Levels filters
  gm_mgFiltersLabel.SetText(TRANS("FILTERING"));
  gm_mgFiltersLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgFiltersLabel.mg_boxOnScreen = BoxLeftColumn(-1.0f);
  gm_mgFiltersLabel.mg_iCenterI = -1;
  gm_mgFiltersLabel.mg_bEnabled = FALSE;
  gm_mgFiltersLabel.mg_bLabel = TRUE;
  AddChild(&gm_mgFiltersLabel);

  gm_mgVisibility.mg_strTip = TRANS("toggle visibility of all world files");
  gm_mgVisibility.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgVisibility.mg_boxOnScreen = BoxLeftColumn(0.0f);
  gm_mgVisibility.mg_iCenterI = -1;

  gm_mgVisibility.mg_pmgRight = &gm_mgManualLevel[0];
  gm_mgVisibility.mg_pmgUp = &gm_mgArrowUp;
  gm_mgVisibility.mg_pmgDown = &gm_mgLevelFormat;
  gm_mgVisibility.mg_pActivatedFunction = &ChangeLevelVisibility;
  AddChild(&gm_mgVisibility);

  gm_mgLevelFormat.mg_strTip = TRANS("list levels only in a specific world format");
  gm_mgLevelFormat.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgLevelFormat.mg_boxOnScreen = BoxLeftColumn(1.0f);
  gm_mgLevelFormat.mg_iCenterI = -1;

  gm_mgLevelFormat.mg_pmgRight = &gm_mgManualLevel[0];
  gm_mgLevelFormat.mg_pmgUp = &gm_mgVisibility;
  gm_mgLevelFormat.mg_pmgDown = &gm_mgTitleFilter;
  gm_mgLevelFormat.mg_pActivatedFunction = &ToggleMountedLevels;
  AddChild(&gm_mgLevelFormat);

  gm_mgTitleFilter.mg_strTip = TRANS("display title filter");
  gm_mgTitleFilter.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgTitleFilter.mg_boxOnScreen = BoxLeftColumn(2.0f);
  gm_mgTitleFilter.mg_iCenterI = -1;

  gm_mgTitleFilter.mg_pmgRight = &gm_mgManualLevel[0];
  gm_mgTitleFilter.mg_pmgUp = &gm_mgLevelFormat;
  gm_mgTitleFilter.mg_pmgDown = &gm_mgArrowDn;
  gm_mgTitleFilter.mg_pstrToChange = &sam_strLevelTitleFilter;
  AddChild(&gm_mgTitleFilter);
}

void CLevelsMenu::FillListItems(void) {
  // disable all items first
  for (INDEX i = 0; i < LEVELS_ON_SCREEN; i++) {
    gm_mgManualLevel[i].mg_bEnabled = FALSE;
    gm_mgManualLevel[i].SetText(LOCALIZE("<empty>"));
    gm_mgManualLevel[i].mg_iInList = -2;
  }

  BOOL bHasFirst = FALSE;
  BOOL bHasLast = FALSE;
  INDEX ctLabels = _lhFilteredLevels.Count();
  INDEX iLabel = 0;
  FOREACHINLIST(CLevelInfo, li_lnNode, _lhFilteredLevels, itli) {
    CLevelInfo &li = *itli;
    INDEX iInMenu = iLabel - gm_iListOffset;
    if ((iLabel >= gm_iListOffset) && (iLabel < (gm_iListOffset + LEVELS_ON_SCREEN))) {
      bHasFirst |= (iLabel == 0);
      bHasLast |= (iLabel == ctLabels - 1);
      gm_mgManualLevel[iInMenu].SetText(li.li_strName);
      gm_mgManualLevel[iInMenu].mg_fnmLevel = li.li_fnLevel;
      gm_mgManualLevel[iInMenu].mg_strTip = "";
      gm_mgManualLevel[iInMenu].mg_bEnabled = TRUE;
      gm_mgManualLevel[iInMenu].mg_iInList = iLabel;

      // [Cecil] Set button tip for formats that differ from the current one
      #define SET_FORMAT_TIP(_Tip) \
        { if (li.li_eFormat != E_LF_CURRENT) gm_mgManualLevel[iInMenu].mg_strTip = TRANS(_Tip); }

      // [Cecil] Mark TFE levels
      if (li.li_eFormat == E_LF_TFE) {
        SET_FORMAT_TIP("This level is from Serious Sam: The First Encounter");

    #if SE1_GAME != SS_TFE
      // [Cecil] Mark SSR levels
      } else if (li.li_eFormat == E_LF_SSR) {
        SET_FORMAT_TIP("This level is from Serious Sam: Revolution");
    #endif

      // [Cecil] Levels from other games cannot be played
      } else if (li.li_eFormat != E_LF_CURRENT) {
        CTString strFormat = "1.50";

        if (li.li_eFormat == E_LF_SSR) {
          strFormat = "SSR";
        }

        CTString strTip;
        strTip.PrintF(TRANS("This is a level in the %s format, it cannot be played!"), strFormat);

        gm_mgManualLevel[iInMenu].mg_strTip = strTip;
        gm_mgManualLevel[iInMenu].mg_bEnabled = FALSE;
      }
    }

    iLabel++;
  }

  // enable/disable up/down arrows
  gm_mgArrowUp.UpdateArrow(!bHasFirst && ctLabels > 0);
  gm_mgArrowDn.UpdateArrow(!bHasLast  && ctLabels > 0);

  // [Cecil] Disable scrollbar if can't scroll in either direction
  gm_mgScrollbar.UpdateScrollbar(gm_mgArrowUp.mg_bEnabled || gm_mgArrowDn.mg_bEnabled);
}

void CLevelsMenu::StartMenu(void) {
  // [Cecil] Hide filtering options
  if (!sam_bLevelFiltering) {
    gm_mgFiltersLabel.mg_iInList = -2;
    gm_mgVisibility.mg_iInList = -2;
    gm_mgLevelFormat.mg_iInList = -2;
    gm_mgTitleFilter.mg_iInList = -2;

    sam_iShowLevelFormat = -1;
    sam_strLevelTitleFilter = "";

    _strLastTitleFilter = "";
    gm_mgTitleFilter.SetText("");

  // [Cecil] Show filtering options
  } else {
    gm_mgFiltersLabel.mg_iInList = -1;
    gm_mgVisibility.mg_iInList = -1;
    gm_mgLevelFormat.mg_iInList = -1;
    gm_mgTitleFilter.mg_iInList = -1;

    // Set level visibility switch text
    gm_mgVisibility.SetText(sam_bShowAllLevels ? TRANS("Show visible") : TRANS("Show all"));

    // Set format switch text
    CTString strFormat = "???";

    switch (sam_iShowLevelFormat) {
      case -1: strFormat = TRANS("All"); break;
      case E_LF_TFE: strFormat = "TFE"; break;
      case E_LF_TSE: strFormat = "TSE"; break;
      case E_LF_SSR: strFormat = "SSR"; break;
      case E_LF_150: strFormat = "1.50"; break;
    }

    strFormat = TRANS("Format: ") + strFormat;
    gm_mgLevelFormat.SetText(strFormat);

    // Set title filter
    _strLastTitleFilter = sam_strLevelTitleFilter;
    gm_mgTitleFilter.SetText(sam_strLevelTitleFilter);
  }

  // [Cecil] Filter levels using local spawn flags
  FilterLevels(gm_ulSpawnFlags, gm_iCategory);

  // set default parameters for the list
  gm_iListOffset = 0;
  gm_ctListTotal = _lhFilteredLevels.Count();
  gm_iListWantedItem = 0;

  // for each level
  INDEX i = 0;
  FOREACHINLIST(CLevelInfo, li_lnNode, _lhFilteredLevels, itlid) {
    CLevelInfo &lid = *itlid;

    // if it is the chosen one
    if (lid.li_fnLevel == GetGameAPI()->GetCustomLevel()) {
      // demand focus on it
      gm_iListWantedItem = i;
      break;
    }

    i++;
  }

  CGameMenu::StartMenu();
}

// [Cecil] Update level list
void CLevelsMenu::Think(void) {
  // Reload menu upon changing the title filter
  if (_strLastTitleFilter != sam_strLevelTitleFilter) {
    _strLastTitleFilter = sam_strLevelTitleFilter;

    EndMenu();
    StartMenu();
  }
};
