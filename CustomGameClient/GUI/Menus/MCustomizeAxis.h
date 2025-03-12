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

#ifndef SE_INCL_GAME_MENU_CUSTOMIZEAXIS_H
#define SE_INCL_GAME_MENU_CUSTOMIZEAXIS_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "GameMenu.h"
#include "GUI/Components/MGSlider.h"
#include "GUI/Components/MGTitle.h"
#include "GUI/Components/MGTrigger.h"

#include "Cecil/MGAxisDetector.h"

class CCustomizeAxisMenu : public CGameMenu {
  public:
    CMGTitle gm_mgTitle;

    // [Cecil] Labels for each gadget kind
    CMGButton gm_amgLabels[3];

    // [Cecil] Tab buttons
    CMGButton gm_mgSenseTab;
    CMGButton gm_mgFlagsTab;

    // [Cecil] One gadget of each kind for each axis
    CMGAxisDetector gm_amgDetect[AXIS_ACTIONS_CT];
    CMGTrigger gm_amgMounted    [AXIS_ACTIONS_CT];
    CMGSlider  gm_amgSensitivity[AXIS_ACTIONS_CT];
    CMGSlider  gm_amgDeadzone   [AXIS_ACTIONS_CT];
    CMGTrigger gm_amgInvert     [AXIS_ACTIONS_CT];
    CMGTrigger gm_amgRelative   [AXIS_ACTIONS_CT];
    CMGTrigger gm_amgSmooth     [AXIS_ACTIONS_CT];

    // [Cecil] Currently selected tab
    INDEX gm_iTab;

    // [Cecil] Sprite sheet with all axis icons
    CTextureObject gm_toAxisIcons;

    ~CCustomizeAxisMenu(void);
    void Initialize_t(void);
    void StartMenu(void);
    void EndMenu(void);
    void ObtainActionSettings(void);
    void ApplyActionSettings(void);

    // [Cecil] Extra rendering
    virtual void PostRender(CDrawPort *pdp);

    // [Cecil] Change to the menu
    static void ChangeTo(void);
};

#endif /* include-once check. */