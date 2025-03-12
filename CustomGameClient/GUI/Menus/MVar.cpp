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
#include "VarList.h"
#include "MVar.h"

// [Cecil] For tab switching
extern CMenuGadget *_pmgUnderCursor;

extern BOOL _bVarChanged;

// [Cecil] Switch to a specific options tab
static void SwitchOptionsTab(INDEX iNewTab) {
  // Tabs are disabled
  if (!sam_bOptionTabs) return;

  CVarMenu &gm = _pGUIM->gmVarMenu;

  // Same tab
  if (gm.gm_iTab == iNewTab) {
    return;
  }

  _pmgUnderCursor = NULL;

  // Select a new tab
  gm.gm_iTab = iNewTab;

  // Disable current tab button
  for (INDEX iTab = 0; iTab < gm.gm_agmTabs.Count(); iTab++) {
    CMGButton &mgTab = gm.gm_agmTabs[iTab];
    mgTab.mg_bEnabled = (iTab != gm.gm_iTab);
  }

  // Reload menu
  gm.gm_iListOffset = 0;
  gm.gm_ctListTotal = _aTabs[gm.gm_iTab].lhVars.Count();
  gm.gm_iListWantedItem = gm.gm_iLastListWantedItem;
  gm.CGameMenu::StartMenu();
};

// [Cecil] Switch to another options tab
static void SelectOptionsTab(void) {
  CMGButton &mgTab = (CMGButton &)*_pmgLastActivatedGadget;
  SwitchOptionsTab(mgTab.mg_iIndex);
};

static void VarApply(void) {
  CVarMenu &gmCurrent = _pGUIM->gmVarMenu;
  gmCurrent.gm_bApplying = TRUE; // [Cecil] Applying changes

  FlushVarSettings(TRUE);
  gmCurrent.EndMenu();
  gmCurrent.StartMenu();

  // [Cecil] Select the top option by default
  gmCurrent.gm_pmgSelectedByDefault->OnKillFocus();
  gmCurrent.gm_pmgSelectedByDefault = &gmCurrent.gm_mgVar[0];
  gmCurrent.gm_pmgSelectedByDefault->OnSetFocus();
};

void CVarMenu::Initialize_t(void) {
  gm_strName = "Var";
  gm_pmgSelectedByDefault = &gm_mgVar[0];
  gm_bApplying = FALSE; // [Cecil] Reset

  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName("");
  AddChild(&gm_mgTitle);

  for (INDEX iLabel = 0; iLabel < VARS_ON_SCREEN; iLabel++) {
    INDEX iPrev = (VARS_ON_SCREEN + iLabel - 1) % VARS_ON_SCREEN;
    INDEX iNext = (iLabel + 1) % VARS_ON_SCREEN;

    // initialize label gadgets
    CMGVarButton &mgVar = gm_mgVar[iLabel];
    mgVar.mg_pmgUp = &gm_mgVar[iPrev];
    mgVar.mg_pmgDown = &gm_mgVar[iNext];
    mgVar.mg_pmgLeft = &gm_mgApply;

    mgVar.mg_bfsFontSize = BFS_MEDIUM; // [Cecil] Set to medium size
    mgVar.mg_boxOnScreen = BoxMediumRow(iLabel - 1.0f);
    mgVar.mg_pActivatedFunction = NULL; // never called!

    AddChild(&gm_mgVar[iLabel]);
  }

  gm_mgApply.mg_boxOnScreen = BoxMediumRow(16.5f);
  gm_mgApply.mg_bfsFontSize = BFS_LARGE;
  gm_mgApply.mg_iCenterI = 1;
  gm_mgApply.mg_pmgLeft =
    gm_mgApply.mg_pmgRight =
    gm_mgApply.mg_pmgUp =
    gm_mgApply.mg_pmgDown = &gm_mgVar[0];
  gm_mgApply.SetText(LOCALIZE("APPLY"));
  gm_mgApply.mg_strTip = LOCALIZE("apply changes");
  AddChild(&gm_mgApply);
  gm_mgApply.mg_pActivatedFunction = &VarApply;

  gm_mgArrowUp.SetupForMenu(this, AD_UP, &gm_mgVar[0]);
  gm_mgArrowDn.SetupForMenu(this, AD_DOWN, &gm_mgVar[VARS_ON_SCREEN - 1]);

  // [Cecil] Scrollbar between the arrows
  gm_mgScrollbar.mg_pmgUp = &gm_mgArrowUp;
  gm_mgScrollbar.mg_pmgDown = &gm_mgArrowDn;
  AddChild(&gm_mgScrollbar);

  gm_ctListVisible = VARS_ON_SCREEN;
  gm_pmgArrowUp = &gm_mgArrowUp;
  gm_pmgArrowDn = &gm_mgArrowDn;
  gm_pmgListTop = &gm_mgVar[0];
  gm_pmgListBottom = &gm_mgVar[VARS_ON_SCREEN - 1];

  // [Cecil] First tab
  gm_iTab = 0;
  gm_iLastListWantedItem = 0;
  gm_iLastTab = 0;
}

void CVarMenu::FillListItems(void) {
  // disable all items first
  for (INDEX i = 0; i < VARS_ON_SCREEN; i++) {
    gm_mgVar[i].mg_bEnabled = FALSE;
    gm_mgVar[i].mg_pvsVar = NULL;
    gm_mgVar[i].mg_iInList = -2;
  }

  // [Cecil] Current tab
  CListHead &lhTab = _aTabs[gm_iTab].lhVars;

  BOOL bHasFirst = FALSE;
  BOOL bHasLast = FALSE;
  INDEX ctLabels = lhTab.Count();
  INDEX iLabel = 0;

  FOREACHINLIST(CVarSetting, vs_lnNode, lhTab, itvs) {
    CVarSetting &vs = *itvs;
    INDEX iInMenu = iLabel - gm_iListOffset;

    if (iLabel >= gm_iListOffset && iLabel < gm_iListOffset + VARS_ON_SCREEN) {
      bHasFirst |= (iLabel == 0);
      bHasLast |= (iLabel == ctLabels - 1);

      CMGVarButton &mgVar = gm_mgVar[iInMenu];
      mgVar.mg_pvsVar = &vs;
      mgVar.mg_strTip = vs.vs_strTip;
      mgVar.mg_bEnabled = gm_mgVar[iInMenu].IsEnabled();
      mgVar.mg_iInList = iLabel;

      // [Cecil] Set variable settings
      mgVar.SetName(vs.vs_strName);
      mgVar.SetText(vs.vs_strValue);
      mgVar.mg_pstrToChange = &vs.vs_strValue;
      mgVar.mg_bHiddenText = vs.vs_bHidden;
    }
    iLabel++;
  }
  // enable/disable up/down arrows
  gm_mgArrowUp.UpdateArrow(!bHasFirst && ctLabels > 0);
  gm_mgArrowDn.UpdateArrow(!bHasLast  && ctLabels > 0);

  // [Cecil] Disable scrollbar if can't scroll in either direction
  gm_mgScrollbar.UpdateScrollbar(gm_mgArrowUp.mg_bEnabled || gm_mgArrowDn.mg_bEnabled);
}

void CVarMenu::StartMenu(void) {
  LoadVarSettings(gm_fnmMenuCFG);

  // [Cecil] Add tab buttons (if more than just "All options")
  const INDEX ctTabs = _aTabs.Count();

  if (sam_bOptionTabs && ctTabs > 1) {
    for (INDEX iTab = 0; iTab < ctTabs; iTab++) {
      const CVarTab &tab = _aTabs[iTab];

      CMGButton &mgTab = gm_agmTabs.Push();
      mgTab.mg_iIndex = iTab;
      mgTab.mg_bfsFontSize = BFS_MEDIUM;
      mgTab.mg_iCenterI = -1;

      mgTab.mg_bEnabled = (iTab != 0);
      mgTab.mg_boxOnScreen = BoxLeftColumn(iTab - 1.0f);
      mgTab.mg_pActivatedFunction = &SelectOptionsTab;

      // Connect previous button to the current one
      if (iTab > 0) {
        mgTab.mg_pmgUp = &gm_agmTabs[iTab - 1];
        gm_agmTabs[iTab - 1].mg_pmgDown = &mgTab;
      }

      mgTab.SetText(tab.strName);

      AddChild(&mgTab);
    }
  }

  // [Cecil] Reset current tab
  gm_iTab = 0;

  // [Cecil] Select last tab
  if (gm_iLastTab != 0) {
    SwitchOptionsTab(gm_iLastTab);

  } else {
    // set default parameters for the list
    gm_iListOffset = 0;
    gm_ctListTotal = _aTabs[gm_iTab].lhVars.Count();
    gm_iListWantedItem = gm_iLastListWantedItem;

    CGameMenu::StartMenu();
  }

  // [Cecil] Reset state before applying
  gm_bApplying = FALSE;
  gm_iLastListWantedItem = 0;
  gm_iLastTab = 0;
}

void CVarMenu::EndMenu(void) {
  // [Cecil] Remember state before applying
  if (gm_bApplying) {
    // Recalculate gm_iListWantedItem for scrolling back after applying
    gm_iLastListWantedItem = gm_iListOffset + gm_ctListVisible / 2;
    gm_iLastTab = gm_iTab;
  }

  // disable all items first
  for (INDEX i = 0; i < VARS_ON_SCREEN; i++) {
    gm_mgVar[i].mg_bEnabled = FALSE;
    gm_mgVar[i].mg_pvsVar = NULL;
    gm_mgVar[i].mg_iInList = -2;
  }

  // [Cecil] Remove all tabs
  gm_agmTabs.Clear();

  FlushVarSettings(FALSE);
  _bVarChanged = FALSE;

  // [Cecil] Make menu elements disappear
  CGameMenu::EndMenu();
}

void CVarMenu::Think(void) {
  gm_mgApply.mg_bEnabled = _bVarChanged;
}

// [Cecil] Change to the menu
void CVarMenu::ChangeTo(const CTString &strTitle, const CTFileName &fnmConfig, const CTString &strMenu) {
  _pGUIM->gmVarMenu.gm_mgTitle.SetName(strTitle);
  _pGUIM->gmVarMenu.gm_fnmMenuCFG = fnmConfig;
  _pGUIM->gmVarMenu.gm_strName = strMenu;
  ChangeToMenu(&_pGUIM->gmVarMenu);
};
