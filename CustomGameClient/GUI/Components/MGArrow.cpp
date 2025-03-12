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
#include "MGArrow.h"

// [Cecil] Setup arrow gadget for some menu
void CMGArrow::SetupForMenu(CGameMenu *pgmMenu, ArrowDir eDir, CMenuGadget *pmgLeave) {
  mg_adDirection = eDir;

  if (eDir == AD_DOWN) {
    mg_pmgLeft = mg_pmgRight = mg_pmgUp = pmgLeave;
  } else {
    mg_pmgLeft = mg_pmgRight = mg_pmgDown = pmgLeave;
  }

  pgmMenu->AddChild(this);
};

// [Cecil] Update the arrow upon filling list items
void CMGArrow::UpdateArrow(BOOL bEnable) {
  mg_bEnabled = bEnable;
  mg_bModern = sam_bModernScrollbars;

  // Set appropriate area
  if (mg_bModern) {
    mg_boxOnScreen = BoxScrollbarArrow(mg_adDirection);
  } else {
    mg_boxOnScreen = BoxArrow(mg_adDirection);
  }
};

void CMGArrow::Render(CDrawPort *pdp) {
  SetFontMedium(pdp, mg_fTextScale);

  PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  COLOR col = GetCurrentColor();

  // [Cecil] Render arrow icons in boxes on the right for the modern scrollbar
  if (mg_bModern) {
    const PIX2D vSize = box.Size();
    const PIX pixW = Min(vSize(1), vSize(2)) * 0.5f;

    // Upside down for AD_DOWN
    const PIX2D vOffset(pixW, (mg_adDirection == AD_DOWN) ? -pixW : +pixW);

    const PIX2D boxArrow0 = box.Center() - vOffset;
    const PIX2D boxArrow1 = box.Center() + vOffset;

    extern CTextureObject _toMenuArrow;

    pdp->InitTexture(&_toMenuArrow, TRUE);
    pdp->AddTexture(boxArrow0(1), boxArrow0(2), boxArrow1(1), boxArrow1(2), col);
    pdp->FlushRenderingQueue();

    pdp->DrawBorder(box.Min()(1), box.Min()(2), vSize(1), vSize(2), col);
    return;
  }

  // Render regular "Page Up" and "Page Down" buttons on the left
  CTString str;
  if (mg_adDirection == AD_NONE) {
    str = "???";
  } else if (mg_adDirection == AD_UP) {
    str = LOCALIZE("Page Up");
  } else if (mg_adDirection == AD_DOWN) {
    str = LOCALIZE("Page Down");
  } else {
    ASSERT(FALSE);
  }

  PIX pixI = box.Min()(1);
  PIX pixJ = box.Min()(2);
  pdp->PutText(str, pixI, pixJ, col);
}

void CMGArrow::OnActivate(void) {
  if (mg_adDirection == AD_UP) {
    pgmCurrentMenu->ScrollList(-3);
  } else if (mg_adDirection == AD_DOWN) {
    pgmCurrentMenu->ScrollList(+3);
  }
}