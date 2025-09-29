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
#include "LevelInfo.h"
#include "VarList.h"
#include "MenuGadget.h"

// [Cecil] Side size ratio
FLOAT _fGadgetSideRatioL = 0.47f;
FLOAT _fGadgetSideRatioR = 0.53f;

extern CSoundData *_psdSelect;

BOOL _bDefiningKey = FALSE;
EValueEdit _eEditingValue = VED_NONE;
CMenuGadget *_pmgLastActivatedGadget = NULL;

// [Cecil] Last pressed gadget
CMenuGadget *_pmgLastPressedGadget = NULL;
PressedMenuButton _pmbLastPressedButton(-1, -1);

CMenuGadget::CMenuGadget(void) {
  mg_pmgLeft = NULL;
  mg_pmgRight = NULL;
  mg_pmgUp = NULL;
  mg_pmgDown = NULL;

  mg_bVisible = TRUE;
  mg_bEnabled = TRUE;
  mg_bLabel = FALSE;
  mg_bFocused = FALSE;
  mg_iInList = -1; // not in list
}

void CMenuGadget::OnActivate(void) {
  NOTHING;
}

// return TRUE if handled
BOOL CMenuGadget::OnKeyDown(PressedMenuButton pmb) {
  // if return pressed
  if (pmb.Apply(TRUE)) {
    // activate
    OnActivate();
    // key is handled
    return TRUE;
  }
  // key is not handled
  return FALSE;
}

// [Cecil] Releasing some button
BOOL CMenuGadget::OnKeyUp(PressedMenuButton pmb) {
  return FALSE;
};

// [Cecil] Process held mouse buttons
BOOL CMenuGadget::OnMouseHeld(PressedMenuButton pmb) {
  return FALSE;
};

BOOL CMenuGadget::OnChar(MSG msg) {
  // key is not handled
  return FALSE;
}

void CMenuGadget::OnSetFocus(void) {
  mg_bFocused = TRUE;
  if (!IsSeparator()) {
    PlayMenuSound(_psdSelect, FALSE); // [Cecil] Don't play over other sounds
  }
}

void CMenuGadget::OnKillFocus(void) {
  mg_bFocused = FALSE;
}

void CMenuGadget::Appear(void) {
  mg_bVisible = TRUE;
}

void CMenuGadget::Disappear(void) {
  mg_bVisible = FALSE;
  mg_bFocused = FALSE;
}

void CMenuGadget::Think(void) {}

void CMenuGadget::OnMouseOver(PIX pixI, PIX pixJ) {}

// get current color for the gadget
COLOR CMenuGadget::GetCurrentColor(void) {
  // use normal colors
  COLOR colUnselected = _pGame->LCDGetColor(C_GREEN, "unselected");
  COLOR colSelected = _pGame->LCDGetColor(C_WHITE, "selected");
  // if disabled
  if (!mg_bEnabled) {
    // use a bit darker colors
    colUnselected = _pGame->LCDGetColor(C_dGREEN, "disabled unselected");
    colSelected = _pGame->LCDGetColor(C_GRAY, "disabled selected");
    // if label
    if (mg_bLabel) {
      // use white
      colUnselected = colSelected = _pGame->LCDGetColor(C_WHITE, "label");
    }
  }
  // use unselected color
  COLOR colRet = colUnselected;
  // if selected
  if (mg_bFocused) {
    // oscilate towards selected color
    CTimerValue tvPatch(ClassicsCore_GetInitTime()); // [Cecil]
    DOUBLE tmNow = (_pTimer->GetHighPrecisionTimer() - tvPatch).GetSeconds();
    colRet = LerpColor((colUnselected >> 1) & 0x7F7F7F7F, colSelected, (FLOAT)sin(tmNow * 10.0) * 0.5f + 0.5f);
  }

  return colRet | CT_OPAQUE;
}

void CMenuGadget::Render(CDrawPort *pdp) {}
