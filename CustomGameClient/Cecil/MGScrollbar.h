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

#ifndef CECIL_INCL_SCROLLBAR_GADGET_H
#define CECIL_INCL_SCROLLBAR_GADGET_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "GUI/Components/MGButton.h"

class CMGScrollbar : public CMGButton {
  public:
    // Values during scrollbar rendering
    PIX2D mg_vMouseInScrollbarArea; // Precise mouse position when it's inside the scroll area, otherwise [-1, -1]
    PIXaabbox2D mg_boxScrollArea; // Overall scroll area
    PIXaabbox2D mg_boxScrollbarArea; // Scrollbar area inside the overall scroll area

    // Values in the beginning of scrolling
    PIX2D mg_vMouseScrollbarDrag; // Mouse position
    BOOL mg_bMouseOnScrollbar; // Whether the mouse was specifically on the scrollbar
    INDEX mg_iLastGadgetOffset; // Visible gadget in the list

    CMGScrollbar();

    // Update the scrollbar upon filling list items
    void UpdateScrollbar(BOOL bEnable);

    BOOL OnKeyDown(PressedMenuButton pmb);
    BOOL OnKeyUp(PressedMenuButton pmb);
    BOOL OnMouseHeld(PressedMenuButton pmb);

    BOOL DragScrollbar(void);
    void Render(CDrawPort *pdp);
};

#endif
