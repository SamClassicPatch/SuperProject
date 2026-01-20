/* Copyright (c) 2025-2026 Dreamy Cecil
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

#include "MGScrollbar.h"

extern PIX _pixCursorPosI;
extern PIX _pixCursorPosJ;

inline void SetGadgetOffset(INDEX iOffset) {
  pgmCurrentMenu->gm_iListOffset = iOffset;
  pgmCurrentMenu->FillListItems();
};

inline INDEX GetGadgetOffset(void) {
  return pgmCurrentMenu->gm_iListOffset;
};

inline INDEX GetGadgetVisible(void) {
  return pgmCurrentMenu->gm_ctListVisible;
};

inline INDEX GetGadgetCount(void) {
  return pgmCurrentMenu->gm_ctListTotal;
};

// Constructor
CMGScrollbar::CMGScrollbar() : CMGButton() {
  mg_vMouseInScrollbarArea = PIX2D(-1, -1);
  mg_boxScrollbarArea = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));

  mg_vMouseScrollbarDrag = PIX2D(-1, -1);
  mg_bMouseOnScrollbar = FALSE;
  mg_iLastGadgetOffset = -1;
};

// Update the scrollbar upon filling list items
void CMGScrollbar::UpdateScrollbar(BOOL bEnable) {
  // Add the scrollbar
  if (sam_bModernScrollbars) {
    mg_bEnabled = bEnable;
    Appear();

    // Set appropriate area
    mg_boxOnScreen = BoxScrollbar();

  // Remove the scrollbar
  } else {
    Disappear();
  }
};

BOOL CMGScrollbar::OnKeyDown(PressedMenuButton pmb) {
  // Start dragging with left mouse button
  if (mg_bEnabled && pmb.iKey == VK_LBUTTON && mg_vMouseInScrollbarArea != PIX2D(-1, -1)) {
    mg_vMouseScrollbarDrag = mg_vMouseInScrollbarArea;
    mg_bMouseOnScrollbar = (mg_boxScrollbarArea >= mg_vMouseInScrollbarArea);
    mg_iLastGadgetOffset = GetGadgetOffset();
    return TRUE;
  }

  return CMenuGadget::OnKeyDown(pmb);
};

// Stop dragging the scrollbar
BOOL CMGScrollbar::OnKeyUp(PressedMenuButton pmb) {
  if (mg_bEnabled && pmb.iKey == VK_LBUTTON) {
    mg_vMouseScrollbarDrag = PIX2D(-1, -1);
    return TRUE;
  }

  return CMenuGadget::OnKeyUp(pmb);
};

// Scroll the list
BOOL CMGScrollbar::OnMouseHeld(PressedMenuButton pmb)
{
  if (mg_bEnabled && pmb.iKey == VK_LBUTTON && mg_vMouseScrollbarDrag != PIX2D(-1, -1)) {
    return DragScrollbar();
  }

  return CMenuGadget::OnMouseHeld(pmb);
};

BOOL CMGScrollbar::DragScrollbar(void) {
  const INDEX ctList = GetGadgetCount();

  // All gadgets are already visible
  if (GetGadgetVisible() >= ctList) return FALSE;

  // Can't grab the scrollbar
  if (mg_vMouseScrollbarDrag == PIX2D(-1, -1)) return FALSE;

  const INDEX iLastOffset = (ctList - GetGadgetVisible());

  // Move scrollbar to a specific position
  if (!mg_bMouseOnScrollbar) {
    const FLOAT fRatio = Clamp(FLOAT(_pixCursorPosJ - mg_boxScrollArea.Min()(2)) / mg_boxScrollArea.Size()(2), 0.0f, 1.0f);

    // Round to the nearest value
    SetGadgetOffset((INDEX)floorf((FLOAT)iLastOffset * fRatio + 0.5f));
    return TRUE;
  }

  // Drag the scrollbar relative to the last offset
  const PIX pixDelta = _pixCursorPosJ - mg_vMouseScrollbarDrag(2);
  const INDEX iWantedValue = mg_iLastGadgetOffset + (ctList * pixDelta) / mg_boxScrollArea.Size()(2);

  SetGadgetOffset(Clamp(iWantedValue, (INDEX)0, iLastOffset));
  return TRUE;
};

void CMGScrollbar::Render(CDrawPort *pdp) {
  mg_vMouseInScrollbarArea = PIX2D(-1, -1);
  mg_boxScrollbarArea = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));

  const INDEX ctList = GetGadgetCount();

  const PIX2D vCursorPos(_pixCursorPosI, _pixCursorPosJ);
  mg_boxScrollArea = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  COLOR col = GetCurrentColor();

  // Render scrollbar
  PIX2D vScrollStart = mg_boxScrollArea.Min();
  PIX2D vScrollSize = mg_boxScrollArea.Size();
  pdp->DrawBorder(vScrollStart(1), vScrollStart(2), vScrollSize(1), vScrollSize(2), col);

  vScrollStart += PIX2D(2, 2);
  vScrollSize  -= PIX2D(4, 4);

  // Adjust slider for the current values on screen, if it can be scrolled
  if (mg_bEnabled) {
    const FLOAT fScrollH = (FLOAT)vScrollSize(2) / (FLOAT)ctList;
    vScrollStart(2) += fScrollH * GetGadgetOffset();

    vScrollSize(2) *= Clamp((FLOAT)GetGadgetVisible() / (FLOAT)ctList, 0.0f, 1.0f);
    vScrollSize(2) = ClampDn(vScrollSize(2), (PIX)1);
  }

  // Grab the scrollbar
  if (mg_boxScrollArea >= vCursorPos) {
    mg_vMouseInScrollbarArea = vCursorPos;
    mg_boxScrollbarArea = PIXaabbox2D(vScrollStart, vScrollStart + vScrollSize);
  }

  pdp->Fill(vScrollStart(1), vScrollStart(2), vScrollSize(1), vScrollSize(2), col);
};
