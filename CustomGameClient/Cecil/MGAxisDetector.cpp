/* Copyright (c) 2023-2025 Dreamy Cecil
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

#include "MGAxisDetector.h"
#include "GUI/Menus/MCustomizeAxis.h"

CMGAxisDetector::CMGAxisDetector() : CMGButton() {
  mg_bDetecting = FALSE;
};

// Start axis detection
void CMGAxisDetector::OnActivate(void) {
  extern CSoundData *_psdPress;
  PlayMenuSound(_psdPress);
  mg_bDetecting = TRUE;
};

// Update axis detection
void CMGAxisDetector::Think(void) {
  if (!mg_bDetecting) return;
  _bDefiningKey = TRUE;

  _pInput->SetJoyPolling(TRUE);
  _pInput->GetInput(FALSE);

  BOOL bFinishDetecting = FALSE;

  // Cancel detection on Escape or Right Click
  if (_pInput->GetButtonState(KID_ESCAPE)
   || _pInput->GetButtonState(KID_MOUSE2)) {
    FinishDetection(-1);
    return;
  }

  // Cancel detection on controller buttons
  for (INDEX iCtrl = 0; iCtrl < MAX_JOYSTICKS; iCtrl++) {
    const INDEX iFirstButton = FIRST_JOYBUTTON + iCtrl * SDL_CONTROLLER_BUTTON_MAX;

    if (_pInput->GetButtonState(iFirstButton + SDL_CONTROLLER_BUTTON_B)
     || _pInput->GetButtonState(iFirstButton + SDL_CONTROLLER_BUTTON_BACK)) {
      FinishDetection(-1);
      return;
    }
  }

  // Compatibility
  #if !_PATCHCONFIG_EXTEND_INPUT
    static const FLOAT inp_fAxisPressThreshold = 0.2f;
  #endif

  // Check every axis
  for (INDEX iAxis = 1; iAxis < MAX_OVERALL_AXES; iAxis++) {
    const FLOAT fReading = Abs(_pInput->GetAxisValue(iAxis));
    BOOL bDetected = FALSE;

    if (iAxis < FIRST_JOYAXIS) {
      bDetected = fReading >= 5.0f;
    } else {
      bDetected = fReading >= Clamp(inp_fAxisPressThreshold, 0.01f, 1.0f);
    }

    if (bDetected) {
      // Finish axis detection
      FinishDetection(iAxis);
      return;
    }
  }
};

// Render axis detection icon
void CMGAxisDetector::Render(CDrawPort *pdp) {
  CTextureObject &to = _pGUIM->gmCustomizeAxisMenu.gm_toAxisIcons;

  // Fallback for when there are no icons
  if (to.GetData() == NULL) {
    CMGButton::Render(pdp);
    return;
  }

  const PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  const FLOAT fSize = box.Size()(2) * 1.25f;
  FLOAT fX = box.Center()(1) - fSize / 2.00f;
  FLOAT fY = box.Center()(2) - fSize / 1.75f;

  const FLOAT fUSize = 1.0f / 8.0f;
  const FLOAT fVSize = 1.0f / 4.0f;

  // Third row
  FLOAT fU = 0.0f;
  FLOAT fV = fVSize * 2.0f;

  COLOR col = GetCurrentColor();

  pdp->InitTexture(&to, TRUE);
  pdp->AddTexture(fX, fY, fX + fSize, fY + fSize, fU, fV, fU + fUSize, fV + fVSize, col);
  pdp->FlushRenderingQueue();
};

// Select detected axis
void CMGAxisDetector::FinishDetection(INDEX iSetAxis) {
  mg_bDetecting = FALSE;
  _bDefiningKey = FALSE;

  // Set detected axis
  if (iSetAxis != -1) {
    CMGTrigger &mg = _pGUIM->gmCustomizeAxisMenu.gm_amgMounted[mg_iIndex];

    mg.mg_iSelected = iSetAxis;
    mg.ApplyCurrentSelection();
  }

  _pInput->ClearInput();
};
