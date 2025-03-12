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
#include "MSelectionList.h"

void CSelectListMenu::Initialize_t(void) {
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  gm_mgNotes.mg_boxOnScreen = BoxMediumRow(10.0);
  gm_mgNotes.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgNotes.mg_iCenterI = -1;
  gm_mgNotes.mg_bEnabled = FALSE;
  gm_mgNotes.mg_bLabel = TRUE;
  AddChild(&gm_mgNotes);

  for (INDEX iLabel = 0; iLabel < SELECTLIST_BUTTONS_CT; iLabel++)
  {
    INDEX iPrev = (SELECTLIST_BUTTONS_CT + iLabel - 1) % SELECTLIST_BUTTONS_CT;
    INDEX iNext = (iLabel + 1) % SELECTLIST_BUTTONS_CT;

    // Initialize label gadgets
    gm_amgButton[iLabel].mg_pmgUp = &gm_amgButton[iPrev];
    gm_amgButton[iLabel].mg_pmgDown = &gm_amgButton[iNext];
    gm_amgButton[iLabel].mg_boxOnScreen = BoxSaveLoad(iLabel);
    gm_amgButton[iLabel].mg_pActivatedFunction = NULL; // Never called
    gm_amgButton[iLabel].mg_iCenterI = -1;

    AddChild(&gm_amgButton[iLabel]);
  }

  gm_mgArrowUp.SetupForMenu(this, AD_UP, &gm_amgButton[0]);
  gm_mgArrowDn.SetupForMenu(this, AD_DOWN, &gm_amgButton[SELECTLIST_BUTTONS_CT - 1]);

  // [Cecil] Scrollbar between the arrows
  gm_mgScrollbar.mg_pmgUp = &gm_mgArrowUp;
  gm_mgScrollbar.mg_pmgDown = &gm_mgArrowDn;
  AddChild(&gm_mgScrollbar);

  gm_ctListVisible = SELECTLIST_BUTTONS_CT;
  gm_pmgArrowUp = &gm_mgArrowUp;
  gm_pmgArrowDn = &gm_mgArrowDn;
  gm_pmgListTop = &gm_amgButton[0];
  gm_pmgListBottom = &gm_amgButton[SELECTLIST_BUTTONS_CT - 1];
};

void CSelectListMenu::StartMenu(void) {
  // Delete all file infos
  FORDELETELIST(CFileInfo, fi_lnNode, gm_lhFileInfos, itfi) {
    delete &itfi.Current();
  }

  // Create new buttons with file infos
  CreateButtons();

  // Set default parameters for the list
  gm_iListOffset = 0;
  gm_ctListTotal = gm_lhFileInfos.Count();

  // Find which one should be selected
  gm_iListWantedItem = 0;

  if (gm_strSelected != "") {
    INDEX i = 0;

    FOREACHINLIST(CFileInfo, fi_lnNode, gm_lhFileInfos, itfi) {
      CFileInfo &fi = *itfi;

      if (fi.fi_fnFile == gm_strSelected) {
        gm_iListWantedItem = i;
        break;
      }

      i++;
    }
  }

  CGameMenu::StartMenu();
}

void CSelectListMenu::EndMenu(void) {
  // Delete all file infos
  FORDELETELIST(CFileInfo, fi_lnNode, gm_lhFileInfos, itfi) {
    delete &itfi.Current();
  }

  CGameMenu::EndMenu();
};

void CSelectListMenu::FillListItems(void) {
  // Disable all items first
  for (INDEX i = 0; i < SELECTLIST_BUTTONS_CT; i++) {
    gm_amgButton[i].mg_bEnabled = FALSE;
    gm_amgButton[i].SetText(LOCALIZE("<empty>"));
    gm_amgButton[i].mg_strTip = "";
    gm_amgButton[i].mg_iInList = -2;
  }

  BOOL bHasFirst = FALSE;
  BOOL bHasLast = FALSE;
  INDEX ctLabels = gm_lhFileInfos.Count();
  INDEX iLabel = 0;

  FOREACHINLIST(CFileInfo, fi_lnNode, gm_lhFileInfos, itfi) {
    CFileInfo &fi = *itfi;
    INDEX iInMenu = iLabel - gm_iListOffset;

    if (iLabel >= gm_iListOffset && iLabel < gm_iListOffset + SELECTLIST_BUTTONS_CT) {
      bHasFirst |= (iLabel == 0);
      bHasLast |= (iLabel == ctLabels - 1);
      
      gm_amgButton[iInMenu].mg_iInList = iLabel;
      gm_amgButton[iInMenu].mg_strDes = fi.fi_strName;
      gm_amgButton[iInMenu].mg_fnm = fi.fi_fnFile;
      gm_amgButton[iInMenu].mg_bEnabled = TRUE;
      gm_amgButton[iInMenu].RefreshText();

      gm_amgButton[iInMenu].mg_strTip = LOCALIZE("Enter - load this");
    }

    iLabel++;
  }

  // Toggle up and down arrows
  gm_mgArrowUp.UpdateArrow(!bHasFirst && ctLabels > 0);
  gm_mgArrowDn.UpdateArrow(!bHasLast  && ctLabels > 0);

  // [Cecil] Disable scrollbar if can't scroll in either direction
  gm_mgScrollbar.UpdateScrollbar(gm_mgArrowUp.mg_bEnabled || gm_mgArrowDn.mg_bEnabled);
};
