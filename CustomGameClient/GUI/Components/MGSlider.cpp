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
#include "MGSlider.h"

extern PIX _pixCursorPosI;
extern PIX _pixCursorPosJ;

CMGSlider::CMGSlider() {
  mg_iMinPos = 0;
  mg_iMaxPos = 16;
  mg_iCurPos = 8;
  mg_pOnSliderChange = NULL;
  mg_fFactor = 1.0f;
}

void CMGSlider::ApplyCurrentPosition(void) {
  mg_iCurPos = Clamp(mg_iCurPos, mg_iMinPos, mg_iMaxPos);
  FLOAT fStretch = FLOAT(mg_iCurPos) / (mg_iMaxPos - mg_iMinPos);
  mg_fFactor = fStretch;

  if (mg_pOnSliderChange != NULL) {
    mg_pOnSliderChange(mg_iCurPos);
  }
}

void CMGSlider::ApplyGivenPosition(INDEX iMin, INDEX iMax, INDEX iCur) {
  mg_iMinPos = iMin;
  mg_iMaxPos = iMax;
  mg_iCurPos = iCur;
  ApplyCurrentPosition();
}

// [Cecil] Global method for getting a slider box
PIXaabbox2D GetHorSliderBox(CDrawPort *pdp, FLOATaabbox2D boxOnScreen, BOOL bHasLabel) {
  PIXaabbox2D box = FloatBoxToPixBox(pdp, boxOnScreen);
  PIX pixJ = box.Min()(2);
  PIX pixJSize = box.Size()(2) * 0.95f;

  PIX pixIR, pixISizeR;

  // [Cecil] Use the entire box if there's no text
  if (!bHasLabel) {
    pixIR = box.Min()(1);
    pixISizeR = box.Size()(1) - 1;

  } else {
    pixIR = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;
    pixISizeR = box.Size()(1) * _fGadgetSideRatioL;
  }

  return PIXaabbox2D(PIX2D(pixIR + 1, pixJ), PIX2D(pixIR + pixISizeR - 2, pixJ + pixJSize - 2));
};

// [Cecil] Separate method
BOOL CMGSlider::OnLMB(void) {
  // get position of slider box on screen
  PIXaabbox2D boxSlider = GetHorSliderBox(_pdpMenu, mg_boxOnScreen, GetText() != "");

  // if mouse is within
  if (boxSlider >= PIX2D(_pixCursorPosI, _pixCursorPosJ)) {
    // set new position exactly where mouse pointer is
    FLOAT fRatio = FLOAT(_pixCursorPosI - boxSlider.Min()(1)) / boxSlider.Size()(1);
    fRatio = (fRatio - 0.01f) / (0.99f - 0.01f);
    fRatio = Clamp(fRatio, 0.0f, 1.0f);
    mg_iCurPos = fRatio * (mg_iMaxPos - mg_iMinPos) + mg_iMinPos;
    ApplyCurrentPosition();
    return TRUE;
  }

  return FALSE;
};

BOOL CMGSlider::OnKeyDown(PressedMenuButton pmb) {
  // [Cecil] Increase/decrease the value
  const INDEX iPower = pmb.ChangeValue();

  if (iPower != 0) {
    mg_iCurPos = Clamp(mg_iCurPos + iPower, mg_iMinPos, mg_iMaxPos);
    ApplyCurrentPosition();
    return TRUE;
  }

  // if lmb pressed
  if (pmb.iKey == VK_LBUTTON && OnLMB()) {
    return TRUE;
  }

  return CMenuGadget::OnKeyDown(pmb);
}

// [Cecil] Adjust the slider by holding a button
BOOL CMGSlider::OnMouseHeld(PressedMenuButton pmb)
{
  // Only when it's the last pressed slider
  if (_pmgLastPressedGadget == this && pmb.iKey == VK_LBUTTON && OnLMB()) {
    return TRUE;
  }

  return CMenuGadget::OnMouseHeld(pmb);
};

void CMGSlider::Render(CDrawPort *pdp) {
  SetFontMedium(pdp, mg_fTextScale);

  // get geometry
  COLOR col = GetCurrentColor();

  // [Cecil] If there's any text
  const BOOL bHasLabel = (GetText() != "");

  if (bHasLabel) {
    // Print text on the left side
    PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
    PIX pixTextX = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioL;
    PIX pixTextY = box.Min()(2);

    pdp->PutTextR(GetText(), pixTextX, pixTextY, col);
  }

  // [Cecil] Use pre-calculated slider box for rendering
  PIXaabbox2D boxSlider = GetHorSliderBox(pdp, mg_boxOnScreen, bHasLabel);
  const PIX pixX = boxSlider.Min()(1) + 1;
  const PIX pixY = boxSlider.Min()(2) + 1;
  const PIX2D vSize = boxSlider.Size();

  // draw box around slider
  _pGame->LCDDrawBox(0, -1, boxSlider, _pGame->LCDGetColor(C_GREEN | 255, "slider box"));

  // draw filled part of slider
  pdp->Fill(pixX, pixY, (vSize(1) - 2) * mg_fFactor, (vSize(2) - 2), col);

  // [Cecil] Align text vertically
  const PIX pixTextHeight = pdp->dp_FontData->GetHeight() * pdp->dp_fTextScaling;
  PIX pixTextY;

  if (mg_iCenterJ == -1) {
    pixTextY = boxSlider.Min()(2);

  } else if (mg_iCenterJ == +1) {
    pixTextY = boxSlider.Max()(2) - pixTextHeight;

  } else {
    pixTextY = boxSlider.Center()(2) - pixTextHeight * 0.5f;
  }

  // print percentage text
  CTString strPercentage;
  strPercentage.PrintF("%d%%", (int)floor(mg_fFactor * 100 + 0.5f));
  pdp->PutTextC(strPercentage, pixX + vSize(1) / 2, pixTextY + 1, col);
}