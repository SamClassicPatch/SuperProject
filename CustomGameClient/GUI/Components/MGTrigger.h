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

#ifndef SE_INCL_MENU_GADGET_TRIGGER_H
#define SE_INCL_MENU_GADGET_TRIGGER_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "MenuGadget.h"

class CMGTrigger : public CMenuGadget {
  public:
    // [Cecil] Callback for rendering visual values
    typedef void (*CVisualRenderFunc)(CMGTrigger *pmg, CDrawPort *pdp);

  // [Cecil] Private strings
  private:
    CTString mg_strLabel;
    CTString mg_strValue;

  public:
    CTString *mg_astrTexts;
    INDEX mg_ctTexts;
    INDEX mg_iSelected;
    INDEX mg_iCenterI, mg_iCenterJ; // [Cecil] Vertical centering
    BOOL mg_bVisual;

    // [Cecil] For custom rendering of visual values
    CVisualRenderFunc mg_pRenderCallback;

    CMGTrigger(void);

    void ApplyCurrentSelection(void);
    void OnSetNextInList(PressedMenuButton pmb);
    void (*mg_pPreTriggerChange)(INDEX iCurrentlySelected);
    void (*mg_pOnTriggerChange)(INDEX iCurrentlySelected);

    // return TRUE if handled
    BOOL OnKeyDown(PressedMenuButton pmb);
    void Render(CDrawPort *pdp);

    // [Cecil] Get node name
    virtual const CTString &GetName(void) const {
      return mg_strLabel;
    };
    
    // [Cecil] Set node name
    virtual void SetName(const CTString &strNew) {
      mg_strLabel = strNew;
    };

    // [Cecil] Get value
    virtual const CTString &GetText(void) const {
      return mg_strValue;
    };
    
    // [Cecil] Set value
    virtual void SetText(const CTString &strNew) {
      mg_strValue = strNew;
    };
};

#endif /* include-once check. */