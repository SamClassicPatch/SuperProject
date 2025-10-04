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

#ifndef SE_INCL_MENU_GADGET_H
#define SE_INCL_MENU_GADGET_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "MenuPrinting.h"

enum EGadgetKeyState {
  GKS_DOING_NOTHING = 0,
  GKS_PRESS_KEY_WAITING = 1,
  GKS_RELEASE_RETURN_WAITING = 2,
};

#define EMPTYSLOTSTRING LOCALIZE("<save a new one>")

// [Cecil] Side size ratios for when one gadget has two elements (e.g. label and a button)
extern FLOAT _fGadgetSideRatioL;
extern FLOAT _fGadgetSideRatioR;

class CMenuGadget : public CLinkedNode {
  public:
    FLOATaabbox2D mg_boxOnScreen;
    BOOL mg_bVisible;
    BOOL mg_bEnabled;
    BOOL mg_bLabel;
    BOOL mg_bFocused;
    INDEX mg_iInList; // for scrollable gadget lists

    CTString mg_strTip;
    CMenuGadget *mg_pmgLeft;
    CMenuGadget *mg_pmgRight;
    CMenuGadget *mg_pmgUp;
    CMenuGadget *mg_pmgDown;

    CMenuGadget(void);
    // return TRUE if handled
    virtual BOOL OnKeyDown(PressedMenuButton pmb);
    virtual BOOL OnKeyUp(PressedMenuButton pmb); // [Cecil]
    virtual BOOL OnMouseHeld(PressedMenuButton pmb); // [Cecil]
    virtual BOOL OnChar(MSG msg);
    virtual void OnActivate(void);
    virtual void OnSetFocus(void);
    virtual void OnKillFocus(void);
    virtual void Appear(void);
    virtual void Disappear(void);
    virtual void Think(void);
    virtual void OnMouseOver(PIX pixI, PIX pixJ);

    virtual COLOR GetCurrentColor(void);
    virtual void Render(CDrawPort *pdp);

    virtual BOOL IsSeparator(void) {
      return FALSE;
    };
};

enum ButtonFontSize {
  BFS_SMALL = 0,
  BFS_MEDIUM = 1,
  BFS_LARGE = 2,
};

// [Cecil] Value editing state
enum EValueEdit {
  VED_NONE = 0,
  VED_STRING, // Equivalent of vanilla '_bEditingString'
  VED_LIST,
  VED_KEYBIND, // Equivalent of vanilla '_bDefiningKey'
};

// [Cecil] Declared here
extern EValueEdit _eEditingValue;
extern CMenuGadget *_pmgLastActivatedGadget;

// [Cecil] Last pressed gadget
extern CMenuGadget *_pmgLastPressedGadget;
extern PressedMenuButton _pmbLastPressedButton;

#endif /* include-once check. */