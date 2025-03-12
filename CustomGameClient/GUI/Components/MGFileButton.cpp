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
#include "MGFileButton.h"

extern CSoundData *_psdPress;

CMGFileButton::CMGFileButton(void) {
  mg_iState = FBS_NORMAL;
}

// refresh current text from description
void CMGFileButton::RefreshText(void) {
  CTString strText = mg_strDes;
  strText.OnlyFirstLine();
  SetText(strText);

  mg_strInfo = mg_strDes;
  mg_strInfo.RemovePrefix(GetText());
  mg_strInfo.DeleteChar(0);
}

void CMGFileButton::SaveDescription(void) {
  CTFileName fnFileNameDescription = mg_fnm.NoExt() + ".des";
  try {
    mg_strDes.Save_t(fnFileNameDescription);
  } catch (char *strError) {
    CPrintF("%s\n", strError);
  }
}

CMGFileButton *_pmgFileToSave = NULL;
void OnFileSaveOK(void) {
  if (_pmgFileToSave != NULL) {
    _pmgFileToSave->SaveYes();
  }
}

void CMGFileButton::DoSave(void) {
  if (FileExistsForWriting(mg_fnm)) {
    _pmgFileToSave = this;

    extern void OnFileSaveOK(void);
    CConfirmMenu::ChangeTo(LOCALIZE("OVERWRITE?"), &OnFileSaveOK, NULL, TRUE);

  } else {
    SaveYes();
  }
}

void CMGFileButton::SaveYes(void) {
  // [Cecil] Saving can only be done through the load/save menu
  ASSERT(GetParent() == &_pGUIM->gmLoadSaveMenu);
  ASSERT(_pGUIM->gmLoadSaveMenu.gm_bSave);

  // Call saving function
  BOOL bSucceeded = _pGUIM->gmLoadSaveMenu.gm_pAfterFileChosen(mg_fnm);

  // If saved
  if (bSucceeded) {
    // Save the description too
    SaveDescription();
  }
}

void CMGFileButton::DoLoad(void) {
#ifndef NDEBUG
  // [Cecil] Check for saving state if currently in load/save menu
  if (GetParent() == &_pGUIM->gmLoadSaveMenu) {
    ASSERT(!_pGUIM->gmLoadSaveMenu.gm_bSave);
  }
#endif

  // No file
  if (!FileExists(mg_fnm)) {
    // Do nothing
    return;
  }

  // [Cecil] Call loading function from the parent
  BOOL bSucceeded = ((CSelectListMenu *)GetParent())->gm_pAfterFileChosen(mg_fnm);

  // Should always succeed
  ASSERT(bSucceeded);
}

static CTString _strTmpDescription;
static CTString _strOrgDescription;

void CMGFileButton::StartEdit(void) {
  CMGEdit::OnActivate();
}

void CMGFileButton::OnActivate(void) {
  if (mg_fnm == "") {
    return;
  }

  PlayMenuSound(_psdPress);

  // if loading
  if (!_pGUIM->gmLoadSaveMenu.gm_bSave) {
    // load now
    DoLoad();
    // if saving
  } else {
    // switch to editing mode
    BOOL bWasEmpty = (GetText() == EMPTYSLOTSTRING);
    mg_strDes = _pGUIM->gmLoadSaveMenu.gm_strSaveDes;
    RefreshText();
    _strOrgDescription =
    _strTmpDescription = GetText();

    if (bWasEmpty) {
      _strOrgDescription = EMPTYSLOTSTRING;
    }

    mg_pstrToChange = &_strTmpDescription;
    StartEdit();
    mg_iState = FBS_SAVENAME;
  }
}

BOOL CMGFileButton::OnKeyDown(PressedMenuButton pmb) {
  if (mg_iState == FBS_NORMAL) {
    if (_pGUIM->gmLoadSaveMenu.gm_bSave || _pGUIM->gmLoadSaveMenu.gm_bManage) {
      if (pmb.iKey == VK_F2) {
        if (FileExistsForWriting(mg_fnm)) {
          // switch to renaming mode
          _strOrgDescription =
          _strTmpDescription = GetText();
          mg_pstrToChange = &_strTmpDescription;
          StartEdit();
          mg_iState = FBS_RENAME;
        }
        return TRUE;

      } else if (pmb.iKey == VK_DELETE) {
        if (FileExistsForWriting(mg_fnm)) {
          // delete the file, its description and thumbnail
          RemoveFile(mg_fnm);
          RemoveFile(mg_fnm.NoExt() + ".des");
          RemoveFile(mg_fnm.NoExt() + "Tbn.tex");
          // refresh menu
          _pGUIM->gmLoadSaveMenu.EndMenu();
          _pGUIM->gmLoadSaveMenu.StartMenu();
          OnSetFocus();
        }
        return TRUE;
      }
    }
    return CMenuGadget::OnKeyDown(pmb);
  } else {
    // go out of editing mode
    if (mg_bEditing) {
      if (pmb.Up() || pmb.Down()) {
        CMGEdit::OnKeyDown(PressedMenuButton(VK_ESCAPE, -1));
      }
    }
    return CMGEdit::OnKeyDown(pmb);
  }
}

void CMGFileButton::OnSetFocus(void) {
  mg_iState = FBS_NORMAL;

  if (_pGUIM->gmLoadSaveMenu.gm_bAllowThumbnails && mg_bEnabled) {
    SetThumbnail(mg_fnm);
  } else {
    ClearThumbnail();
  }

  pgmCurrentMenu->KillAllFocuses();
  CMGButton::OnSetFocus();
}

void CMGFileButton::OnKillFocus(void) {
  // go out of editing mode
  if (mg_bEditing) {
    OnKeyDown(PressedMenuButton(VK_ESCAPE, -1));
  }

  CMGEdit::OnKillFocus();
}

// override from edit gadget
void CMGFileButton::OnStringChanged(void) {
  // if saving
  if (mg_iState == FBS_SAVENAME) {
    // do the save
    mg_strDes = _strTmpDescription + "\n" + mg_strInfo;
    DoSave();
  // if renaming
  } else if (mg_iState == FBS_RENAME) {
    // do the rename
    mg_strDes = _strTmpDescription + "\n" + mg_strInfo;
    SaveDescription();
    // refresh menu
    _pGUIM->gmLoadSaveMenu.EndMenu();
    _pGUIM->gmLoadSaveMenu.StartMenu();
    OnSetFocus();
  }
}
void CMGFileButton::OnStringCanceled(void) {
  SetText(_strOrgDescription);
}

void CMGFileButton::Render(CDrawPort *pdp) {
  // render original gadget first
  CMGEdit::Render(pdp);

  // if currently selected
  if (mg_bFocused && mg_bEnabled) {
    // add info at the bottom if screen
    SetFontMedium(pdp, mg_fTextScale);

    PIXaabbox2D box = FloatBoxToPixBox(pdp, BoxSaveLoad(15.0));
    PIX pixI = box.Min()(1);
    PIX pixJ = box.Min()(2);

    COLOR col = _pGame->LCDGetColor(C_mlGREEN | 255, "file info");
    pdp->PutText(mg_strInfo, pixI, pixJ, col);
  }
}