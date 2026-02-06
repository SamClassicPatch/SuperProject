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

#ifndef SE_INCL_GAME_MENU_H
#define SE_INCL_GAME_MENU_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

class CGameMenu : public CNode {
  public:
    FLOAT gm_fPopupSize; // [Cecil] Considered a popup if the size is bigger than 0 (replacing 'gm_bPopup')
    CTString gm_strName; // menu name (for mod interface only)
    class CMenuGadget *gm_pmgSelectedByDefault;
    class CMenuGadget *gm_pmgArrowUp;
    class CMenuGadget *gm_pmgArrowDn;
    class CMenuGadget *gm_pmgListTop;
    class CMenuGadget *gm_pmgListBottom;
    INDEX gm_iListOffset;
    INDEX gm_iListWantedItem; // item you want to focus initially
    INDEX gm_ctListVisible;
    INDEX gm_ctListTotal;
    CGameMenu(void);
    void ScrollList(INDEX iDir);
    void KillAllFocuses(void);
    virtual void Initialize_t(void);
    virtual void Destroy(void);
    virtual void StartMenu(void);
    virtual void FillListItems(void);
    virtual void EndMenu(void);
    // return TRUE if handled
    virtual BOOL OnKeyDown(PressedMenuButton pmb);
    virtual BOOL OnKeyUp(PressedMenuButton pmb); // [Cecil]
    virtual BOOL OnMouseHeld(PressedMenuButton pmb); // [Cecil]
    virtual BOOL OnChar(MSG msg);
    virtual void Think(void);

    // [Cecil] Render extras on top of all gadgets
    virtual void PostRender(CDrawPort *pdp) {};

    // [Cecil] Render popup box in a separate method
    static void RenderPopup(CDrawPort *pdp, FLOAT fPopupSize);

    // Wrapper method for compatibility
    __forceinline void AddChild(class CMenuGadget *pNode) {
      AddTail((CNode *)pNode);
    };
};

#endif /* include-once check. */