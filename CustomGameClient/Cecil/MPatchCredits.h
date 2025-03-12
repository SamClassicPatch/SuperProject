/* Copyright (c) 2023-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_CREDITS_MENU_H
#define CECIL_INCL_CREDITS_MENU_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "GUI/Menus/GameMenu.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGTitle.h"

// Credits names
enum EPatchCreditsName {
  PCN_CECIL,

  PCN_FREEKNIK,
  PCN_MAX512,
  PCN_NIKC75,
  PCN_SUPERSNIPER98,
  PCN_TIN,
  PCN_ZDZICHU,

  PCN_FIRST_THIRDPARTY,

  PCN_KENNEY = PCN_FIRST_THIRDPARTY,
  PCN_SDL,
  PCN_STEAMAPI,
  PCN_ZLIB,

  PCN_LAST,
};

class CPatchCreditsMenu : public CGameMenu {
  public:
    CMGButton gm_mgPatchName;
    CMGButton gm_mgPatchLink;

    CMGButton gm_mgLead;
    CMGButton gm_mgThanks;
    CMGButton gm_mgThirdParty;
    CMGButton gm_amgNames[PCN_LAST];

    // Initialize credits
    void Initialize_t(void);

    // Start credits menu
    void StartMenu(void);
};

#endif
