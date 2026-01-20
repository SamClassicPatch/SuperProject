/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_EXTRAS_MENU_H
#define CECIL_INCL_EXTRAS_MENU_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "GUI/Menus/GameMenu.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGTitle.h"

class CExtrasMenu : public CGameMenu {
  public:
    CMGTitle gm_mgTitle;
    CMGButton gm_mgMods;
    CMGButton gm_mgPatchCredits;
    CMGButton gm_mgGameCredits;
    CMGButton gm_mgDocs;
    CMGButton gm_mgRestart;

    // Initialize extras
    void Initialize_t(void);
};

#endif
