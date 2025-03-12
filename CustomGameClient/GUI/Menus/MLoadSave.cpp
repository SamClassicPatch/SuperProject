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
#include "MLoadSave.h"

void CLoadSaveMenu::Initialize_t(void) {
  gm_strName = "LoadSave";
  gm_pmgSelectedByDefault = &gm_amgButton[0];

  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  gm_mgNotes.mg_boxOnScreen = BoxMediumRow(10.0);
  gm_mgNotes.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgNotes.mg_iCenterI = -1;
  gm_mgNotes.mg_bEnabled = FALSE;
  gm_mgNotes.mg_bLabel = TRUE;
  AddChild(&gm_mgNotes);

  for (INDEX iLabel = 0; iLabel < SELECTLIST_BUTTONS_CT; iLabel++) {
    INDEX iPrev = (SELECTLIST_BUTTONS_CT + iLabel - 1) % SELECTLIST_BUTTONS_CT;
    INDEX iNext = (iLabel + 1) % SELECTLIST_BUTTONS_CT;
    // initialize label gadgets
    gm_amgButton[iLabel].mg_pmgUp = &gm_amgButton[iPrev];
    gm_amgButton[iLabel].mg_pmgDown = &gm_amgButton[iNext];
    gm_amgButton[iLabel].mg_boxOnScreen = BoxSaveLoad(iLabel);
    gm_amgButton[iLabel].mg_pActivatedFunction = NULL; // never called!
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
}

// Create new buttons with file infos
void CLoadSaveMenu::CreateButtons(void) {
  // List the directory
  CFileList afnmDir;
  ListGameFiles(afnmDir, gm_fnmDirectory, "", gm_ulListFlags);
  gm_iLastFile = -1;

  // For each file in the directory
  for (INDEX i = 0; i < afnmDir.Count(); i++) {
    CTFileName fnm = afnmDir[i];

    // If it can be parsed
    CTString strName;

    if (ParseFile(fnm, strName))
    {
      // Create new info for that file
      CFileInfo *pfi = new CFileInfo;
      pfi->fi_fnFile = fnm;
      pfi->fi_strName = strName;

      // Add it to the list
      gm_lhFileInfos.AddTail(pfi->fi_lnNode);
    }
  }

  // Sort if needed
  switch (gm_iSortType)
  {
    case LSSORT_NONE: break;

    case LSSORT_NAMEUP:
      gm_lhFileInfos.Sort(qsort_CompareFileInfos_NameUp, offsetof(CFileInfo, fi_lnNode));
      break;

    case LSSORT_NAMEDN:
      gm_lhFileInfos.Sort(qsort_CompareFileInfos_NameDn, offsetof(CFileInfo, fi_lnNode));
      break;

    case LSSORT_FILEUP:
      gm_lhFileInfos.Sort(qsort_CompareFileInfos_FileUp, offsetof(CFileInfo, fi_lnNode));
      break;

    case LSSORT_FILEDN:
      gm_lhFileInfos.Sort(qsort_CompareFileInfos_FileDn, offsetof(CFileInfo, fi_lnNode));
      break;

    default: ASSERT(FALSE);
  }

  // If saving
  if (gm_bSave)
  {
    // Add one info as empty slot
    CFileInfo *pfi = new CFileInfo;

    CTString strNumber;
    strNumber.PrintF("%04d", gm_iLastFile + 1);

    pfi->fi_fnFile = gm_fnmDirectory + gm_fnmBaseName + strNumber + gm_fnmExt;
    pfi->fi_strName = EMPTYSLOTSTRING;

    // Add it to the beginning
    gm_lhFileInfos.AddHead(pfi->fi_lnNode);
  }

  CSelectListMenu::CreateButtons();
};

void CLoadSaveMenu::FillListItems(void) {
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

      if (gm_bSave) {
        if (!FileExistsForWriting(gm_amgButton[iInMenu].mg_fnm)) {
          gm_amgButton[iInMenu].mg_strTip = LOCALIZE("Enter - save in new slot");
        } else {
          gm_amgButton[iInMenu].mg_strTip = LOCALIZE("Enter - save here, F2 - rename, Del - delete");
        }

      } else if (gm_bManage) {
        gm_amgButton[iInMenu].mg_strTip = LOCALIZE("Enter - load this, F2 - rename, Del - delete");
      } else {
        gm_amgButton[iInMenu].mg_strTip = LOCALIZE("Enter - load this");
      }
    }

    iLabel++;
  }

  // Toggle up and down arrows
  gm_mgArrowUp.UpdateArrow(!bHasFirst && ctLabels > 0);
  gm_mgArrowDn.UpdateArrow(!bHasLast  && ctLabels > 0);

  // [Cecil] Disable scrollbar if can't scroll in either direction
  gm_mgScrollbar.UpdateScrollbar(gm_mgArrowUp.mg_bEnabled || gm_mgArrowDn.mg_bEnabled);
};

// Called to get info of a file from directory, or to skip it
BOOL CLoadSaveMenu::ParseFile(const CTFileName &fnm, CTString &strName) {
  if (fnm.FileExt() != gm_fnmExt) {
    return FALSE;
  }

  CTFileName fnSaveGameDescription = fnm.NoExt() + ".des";

  try {
    strName.Load_t(fnSaveGameDescription);

  } catch (char *strError) {
    (void)strError;
    strName = fnm.FileName();

    if (fnm.FileExt() == ".ctl") {
      INDEX iCtl = -1;
      strName.ScanF("Controls%d", &iCtl);

      if (iCtl >= 0 && iCtl < GetGameAPI()->GetProfileCount()) {
        strName.PrintF(LOCALIZE("From player: %s"), GetGameAPI()->GetPlayerCharacter(iCtl)->GetNameForPrinting());
      }
    }
  }

  INDEX iFile = -1;
  fnm.FileName().ScanF((gm_fnmBaseName + "%d").str_String, &iFile);

  gm_iLastFile = Max(gm_iLastFile, iFile);

  return TRUE;
};
