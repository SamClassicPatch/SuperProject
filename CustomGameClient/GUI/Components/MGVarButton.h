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

#ifndef SE_INCL_MENU_GADGET_VARBUTTON_H
#define SE_INCL_MENU_GADGET_VARBUTTON_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "MGButton.h"

// [Cecil] Derive from CMGEdit, which derives from CMGButton
class CMGVarButton : public CMGEdit {
  public:
    class CVarSetting *mg_pvsVar;

    // [Cecil] Active value list
    INDEX mg_ctListValuesAtOnce; // Amount of rendered values in the defined list area (<= total values)
    INDEX mg_iValueListOffset; // First value shown in the defined list area (always 0 if able to display all values at once)
    PIXaabbox2D mg_boxList; // Defined list area during rendering

    // [Cecil] Values during list scrollbar rendering
    INDEX mg_iListValue; // Currently selected list value
    INDEX mg_iLastListValue; // For playing selection sound
    PIX2D mg_vMouseInScrollbarArea; // Precise mouse position when it's inside the scroll area, otherwise [-1, -1]
    PIXaabbox2D mg_boxScrollbarArea; // Scrollbar area inside the overall scroll area

    // [Cecil] Values in the beginning of scrolling
    PIX2D mg_vMouseScrollbarDrag; // Mouse position
    BOOL mg_bMouseOnScrollbar; // Whether the mouse was specifically on the scrollbar
    INDEX mg_iLastValueListOffset; // Value of mg_iValueListOffset

    EGadgetKeyState mg_iState; // [Cecil]

    CMGVarButton(); // [Cecil]
    PIXaabbox2D GetSliderBox(CDrawPort *pdp, INDEX iSliderType);
    BOOL OnKeyDown(PressedMenuButton pmb);
    BOOL OnKeyUp(PressedMenuButton pmb); // [Cecil]
    BOOL OnChar(MSG msg); // [Cecil]
    BOOL OnMouseHeld(PressedMenuButton pmb); // [Cecil]
    void Render(CDrawPort *pdp);
    BOOL IsSeparator(void);
    BOOL IsEnabled(void);

    // [Cecil] Thinking logic
    void Think(void);

    // [Cecil] Define new key bind
    void DefineKey(INDEX iKID);

    // [Cecil] Signal that some variable has been changed
    void OnVarChanged(void);

    // [Cecil] Change strings
    void OnStringChanged(void);
    void OnStringCanceled(void);

    // [Cecil] Activate value list
    void ListActivate(void);

    // [Cecil] Deactivate value list
    void ListDeactivate(INDEX iSelectValue);

    // [Cecil] Value list interactions
    BOOL ListOnKeyDown(PressedMenuButton pmb);
    BOOL ListDragScrollbar(void);

    // [Cecil] Render value list
    void ListRender(CDrawPort *pdp, PIX2D vListBox, CTString strValue);
};

#endif /* include-once check. */