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
#include "MGTrigger.h"

static INDEX GetNewLoopValue(PressedMenuButton pmb, INDEX iCurrent, INDEX ctMembers)
{
  INDEX iPrev = (iCurrent + ctMembers - 1) % ctMembers;
  INDEX iNext = (iCurrent + 1) % ctMembers;

  // return and right arrow set new text
  if (pmb.Next()) {
    return iNext;

  // left arrow and backspace sets prev text
  } else if (pmb.Prev()) {
    return iPrev;
  }

  return iCurrent;
}

CMGTrigger::CMGTrigger(void) {
  mg_pPreTriggerChange = NULL;
  mg_pOnTriggerChange = NULL;
  mg_iCenterI = 0;
  mg_iCenterJ = -1; // [Cecil]
  mg_bVisual = FALSE;
  mg_pRenderCallback = NULL; // [Cecil]
}

void CMGTrigger::ApplyCurrentSelection(void) {
  mg_iSelected = Clamp(mg_iSelected, 0L, mg_ctTexts - 1L);
  SetText(mg_astrTexts[mg_iSelected]);
}

void CMGTrigger::OnSetNextInList(PressedMenuButton pmb) {
  if (mg_pPreTriggerChange != NULL) {
    mg_pPreTriggerChange(mg_iSelected);
  }

  mg_iSelected = GetNewLoopValue(pmb, mg_iSelected, mg_ctTexts);
  SetText(mg_astrTexts[mg_iSelected]);

  if (mg_pOnTriggerChange != NULL) {
    (*mg_pOnTriggerChange)(mg_iSelected);
  }
}

BOOL CMGTrigger::OnKeyDown(PressedMenuButton pmb) {
  if (pmb.Prev() || pmb.Next()) {
    // key is handled
    if (mg_bEnabled) OnSetNextInList(pmb);
    return TRUE;
  }

  // key is not handled
  return FALSE;
}

void CMGTrigger::Render(CDrawPort *pdp) {
  SetFontMedium(pdp, 1.0f);

  PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  PIX pixIL = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioL;
  PIX pixIR = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;

  COLOR col = GetCurrentColor();

  // [Cecil] Align text vertically
  const PIX pixTextHeight = pdp->dp_FontData->GetHeight() * pdp->dp_fTextScaling;
  PIX pixTextY;

  if (mg_iCenterJ == -1) {
    pixTextY = box.Min()(2);

  } else if (mg_iCenterJ == +1) {
    pixTextY = box.Max()(2) - pixTextHeight;

  } else {
    pixTextY = box.Center()(2) - pixTextHeight * 0.5f;
  }

  // [Cecil] Render label
  const BOOL bHasLabel = (GetName() != "");

  if (bHasLabel) {
    if (mg_iCenterI == -1) {
      pdp->PutText(GetName(), box.Min()(1), pixTextY, col);
    } else {
      pdp->PutTextR(GetName(), pixIL, pixTextY, col);
    }
  }

  // Render non-visual string values or "none" for empty visual values
  if (!mg_bVisual || GetText() == "") {
    CTString strValue = (mg_bVisual ? LOCALIZE("none") : GetText());

    // [Cecil] Render value with any centering if there's no label
    if (!bHasLabel) {
      if (mg_iCenterI == -1) {
        pdp->PutText(strValue, box.Min()(1), pixTextY, col);
      } else if (mg_iCenterI == +1) {
        pdp->PutTextR(strValue, box.Max()(1), pixTextY, col);
      } else {
        pdp->PutTextC(strValue, box.Center()(1), pixTextY, col);
      }

    } else if (mg_iCenterI == -1) {
      pdp->PutTextR(strValue, box.Max()(1), pixTextY, col);
    } else {
      pdp->PutText(strValue, pixIR, pixTextY, col);
    }

  // [Cecil] Use custom rendering callback
  } else if (mg_pRenderCallback != NULL) {
    mg_pRenderCallback(this, pdp);

  } else {
    CTextureObject to;

    try {
      to.SetData_t(GetText());
      CTextureData *ptd = (CTextureData *)to.GetData();
      PIX pixSize = box.Size()(2);
      PIX pixCX = box.Max()(1) - pixSize / 2;
      PIX pixCY = box.Center()(2);

      pdp->PutTexture(&to, PIXaabbox2D(
        PIX2D(pixCX - pixSize / 2, pixCY - pixSize / 2),
        PIX2D(pixCX - pixSize / 2 + pixSize, pixCY - pixSize / 2 + pixSize)), C_WHITE | 255);

    } catch (char *strError) {
      CPrintF("%s\n", strError);
    }

    to.SetData(NULL);
  }
}