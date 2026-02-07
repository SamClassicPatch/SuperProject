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
#include "MConfirm.h"

// [Cecil] Reset popup box
static void ResetPopup(CConfirmMenu *pgm, FLOAT fHeight) {
  pgm->gm_fPopupSize = fHeight;
  pgm->gm_mgConfirmLabel.mg_boxOnScreen = BoxPopupLabel(fHeight);
};

static void ConfirmYes(void) {
  CConfirmMenu &gmCurrent = _pGUIM->gmConfirmMenu;

  if (gmCurrent._pConfirmedYes != NULL) {
    gmCurrent._pConfirmedYes();
  }

  MenuGoToParent();
};

static void ConfirmNo(void) {
  CConfirmMenu &gmCurrent = _pGUIM->gmConfirmMenu;

  if (gmCurrent._pConfirmedNo != NULL) {
    gmCurrent._pConfirmedNo();
  }

  MenuGoToParent();
};

void CConfirmMenu::Initialize_t(void) {
  gm_strName = "Confirm";
  gm_pmgSelectedByDefault = &gm_mgConfirmYes;

  const FLOAT fHeight = 0.2f;

  AddChild(&gm_mgConfirmLabel);
  gm_mgConfirmLabel.mg_iCenterI = 0;
  gm_mgConfirmLabel.mg_bfsFontSize = BFS_LARGE;

  AddChild(&gm_mgConfirmYes);
  gm_mgConfirmYes.mg_boxOnScreen = BoxPopupYesLarge(fHeight);
  gm_mgConfirmYes.mg_pActivatedFunction = &ConfirmYes;
  gm_mgConfirmYes.mg_pmgLeft = gm_mgConfirmYes.mg_pmgRight = &gm_mgConfirmNo;
  gm_mgConfirmYes.mg_iCenterI = 1;
  gm_mgConfirmYes.mg_bfsFontSize = BFS_LARGE;

  AddChild(&gm_mgConfirmNo);
  gm_mgConfirmNo.mg_boxOnScreen = BoxPopupNoLarge(fHeight);
  gm_mgConfirmNo.mg_pActivatedFunction = &ConfirmNo;
  gm_mgConfirmNo.mg_pmgLeft = gm_mgConfirmNo.mg_pmgRight = &gm_mgConfirmYes;
  gm_mgConfirmNo.mg_iCenterI = -1;
  gm_mgConfirmNo.mg_bfsFontSize = BFS_LARGE;

  _pConfirmedYes = NULL;
  _pConfirmedNo = NULL;

  ResetPopup(this, fHeight);
  SetText("");
}

void CConfirmMenu::BeLarge(FLOAT fHeight) {
  ResetPopup(this, fHeight);

  gm_mgConfirmLabel.mg_bfsFontSize = BFS_LARGE;
  gm_mgConfirmYes.mg_bfsFontSize = BFS_LARGE;
  gm_mgConfirmNo.mg_bfsFontSize = BFS_LARGE;

  gm_mgConfirmLabel.mg_iCenterI = 0;
  gm_mgConfirmYes.mg_boxOnScreen = BoxPopupYesLarge(fHeight);
  gm_mgConfirmNo.mg_boxOnScreen = BoxPopupNoLarge(fHeight);

  // [Cecil] Top right and top left
  gm_mgConfirmYes.mg_iCenterI = +1;
  gm_mgConfirmYes.mg_iCenterJ = -1;
  gm_mgConfirmNo.mg_iCenterI = -1;
  gm_mgConfirmNo.mg_iCenterJ = -1;
}

void CConfirmMenu::BeSmall(FLOAT fHeight) {
  ResetPopup(this, fHeight);

  gm_mgConfirmLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgConfirmYes.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgConfirmNo.mg_bfsFontSize = BFS_MEDIUM;

  gm_mgConfirmLabel.mg_iCenterI = -1;
  gm_mgConfirmYes.mg_boxOnScreen = BoxPopupYesSmall(fHeight);
  gm_mgConfirmNo.mg_boxOnScreen = BoxPopupNoSmall(fHeight);

  // [Cecil] Center buttons
  gm_mgConfirmYes.mg_iCenterI = 0;
  gm_mgConfirmYes.mg_iCenterJ = 0;
  gm_mgConfirmNo.mg_iCenterI = 0;
  gm_mgConfirmNo.mg_iCenterJ = 0;
}

// [Cecil] Set label and button text
void CConfirmMenu::SetText(const CTString &strLabel, const CTString &strYes, const CTString &strNo) {
  gm_strLabel = strLabel;
  gm_mgConfirmLabel.SetText("");
  gm_mgConfirmYes.SetText(strYes == "" ? LOCALIZE("YES") : strYes);
  gm_mgConfirmNo.SetText(strNo == "" ? LOCALIZE("NO") : strNo);
};

// return TRUE if handled
BOOL CConfirmMenu::OnKeyDown(PressedMenuButton pmb) {
  if (pmb.Back(TRUE) && gm_mgConfirmNo.mg_pActivatedFunction != NULL) {
    gm_mgConfirmNo.OnActivate();
    return TRUE;
  }
  return CGameMenu::OnKeyDown(pmb);
}

// [Cecil] Extra processing
void CConfirmMenu::PreRender(CDrawPort *pdp) {
  // Don't bother formatting any text if there's nothing to format or it's already set
  if (gm_strLabel == "" || gm_mgConfirmLabel.GetText() != "") return;

  // Format the label string and set it to the label gadget
  const PIXaabbox2D boxArea = FloatBoxToPixBox(pdp, gm_mgConfirmLabel.mg_boxOnScreen);
  const PIX pixMaxWidth = boxArea.Size()(1);
  gm_mgConfirmLabel.SetText(IData::FormatStringForWidth(pdp, pixMaxWidth, gm_strLabel));
};

// [Cecil] Change to the menu
void CConfirmMenu::ChangeTo(const CTString &strLabel, CConfirmFunc pFuncYes, CConfirmFunc pFuncNo,
  BOOL bBigLabel, const CTString &strYes, const CTString &strNo, FLOAT fHeight)
{
  CConfirmMenu &gm = _pGUIM->gmConfirmMenu;

  gm._pConfirmedYes = pFuncYes;
  gm._pConfirmedNo = pFuncNo;
  gm.SetText(strLabel, strYes, strNo);

  if (bBigLabel) {
    gm.BeLarge(fHeight);
  } else {
    gm.BeSmall(fHeight);
  }

  ChangeToMenu(&gm);
};
