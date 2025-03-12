/* Copyright (c) 2023-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_LEVELCATEGORIES_MENU_H
#define CECIL_INCL_LEVELCATEGORIES_MENU_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "GUI/Menus/GameMenu.h"
#include "GUI/Components/MGButton.h"
#include "GUI/Components/MGTitle.h"

// Level category button
class CMGLevelCategory : public CMGButton {
  public:
    INDEX mg_iCategory;

  public:
    // Default constructor
    CMGLevelCategory() : CMGButton(), mg_iCategory(-1)
    {
    };

    // Select levels from this category
    void OnActivate(void);
};

// Menu with all level categories
class CLevelCategoriesMenu : public CGameMenu {
  public:
    CMGTitle gm_mgTitle;
    CStaticArray<CMGLevelCategory> gm_amgCategories;

    // Initialize categories
    void Initialize_t(void);

    // Add category under a specific index
    void AddCategory(INDEX i, const CTString &strName, const CTString &strTip);
};

#endif
