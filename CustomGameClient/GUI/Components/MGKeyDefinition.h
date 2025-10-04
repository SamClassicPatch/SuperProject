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

#ifndef SE_INCL_MENU_GADGET_KEYDEFINITION_H
#define SE_INCL_MENU_GADGET_KEYDEFINITION_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "MenuGadget.h"

class CMGKeyDefinition : public CMenuGadget {
  public:
    EGadgetKeyState mg_iState;
    INDEX mg_iControlNumber;

    // [Cecil] Controls for editing passed from CCustomizeKeyboardMenu
    CControls *mg_pControls;

  // [Cecil] Private strings
  private:
    CTString mg_strLabel;
    CTString mg_strBinding;

  public:
    CMGKeyDefinition(void);
    void Appear(void);
    void Disappear(void);
    void OnActivate(void);
    // return TRUE if handled
    BOOL OnKeyDown(PressedMenuButton pmb);
    void Think(void);
    // set names for both key bindings
    void SetBindingNames(BOOL bDefining);
    void DefineKey(INDEX iDik);
    void Render(CDrawPort *pdp);

    // [Cecil] Get node name
    virtual const CTString &GetName(void) const {
      return mg_strLabel;
    };
    
    // [Cecil] Set node name
    virtual void SetName(const CTString &strNew) {
      mg_strLabel = strNew;
    };

    // [Cecil] Get key binding
    virtual const CTString &GetText(void) const {
      return mg_strBinding;
    };
    
    // [Cecil] Set key binding
    virtual void SetText(const CTString &strNew) {
      mg_strBinding = strNew;
    };
};

#endif /* include-once check. */