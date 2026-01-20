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

#include "StdH.h"

#include "MLevelCategories.h"
#include "GUI/Menus/MenuPrinting.h"
#include "GUI/Menus/MenuStuff.h"
#include "LevelInfo.h"

// Select levels from this category
void CMGLevelCategory::OnActivate(void) {
  CLevelsMenu &gmCurrent = _pGUIM->gmLevelsMenu;
  gmCurrent.gm_iCategory = mg_iCategory;

  ChangeToMenu(&gmCurrent);

  extern CSoundData *_psdPress;
  PlayMenuSound(_psdPress);

  CMGButton::OnActivate();
};

// Categories with level lists
static INDEX _ctCats = 0;

#define MAX_CUSTOM_CATEGORIES 17
#define MAX_BIG_CATEGORIES    10

// Initialize categories
void CLevelCategoriesMenu::Initialize_t(void) {
  gm_strName = "LevelCategories";

  gm_mgTitle.SetName(TRANS("CHOOSE CATEGORY"));
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  // Load category lists
  CFileList aCategories;
  BOOL bLoadFromGame = TRUE;

  // Load from the mod
  if (_fnmMod != "") {
    ListGameFiles(aCategories, "Data\\ClassicsPatch\\LevelCategories\\", "*.lst", FLF_ONLYMOD);

    // Don't load from the game if there are any mod categories
    if (aCategories.Count() != 0) {
      bLoadFromGame = FALSE;
    }
  }

  // Load from the game
  if (bLoadFromGame) {
    ListGameFiles(aCategories, "Data\\ClassicsPatch\\LevelCategories\\", "*.lst", 0);
  }

  // Create new categories
  _ctCats = Min(aCategories.Count(), INDEX(MAX_CUSTOM_CATEGORIES));

  // No categories
  if (_ctCats == 0) return;

  _aLevelCategories.New(_ctCats);
  gm_amgCategories.New(_ctCats + 1); // One more for other levels

  for (INDEX i = 0; i < _ctCats; i++) {
    const CTFileName &fnm = aCategories[i];

    // Load level list from this category file
    IFiles::LoadStringList(_aLevelCategories[i], fnm);

    // Get category name and description
    CTString strName = "???";
    CTString strDesc = "";

    try {
      // Try loading text from the description file nearby
      strName.Load_t(fnm.NoExt() + ".des");

      // Separate text into name and description
      ULONG ulLineBreak = IData::FindChar(strName, '\n');

      if (ulLineBreak != -1) {
        strName.Split(ulLineBreak + 1, strName, strDesc);
      }

    } catch (char *strError) {
      // Just set text to the filename
      (void)strError;
      strName = fnm.FileName();
    }

    // Use first line from the list as the category name
    AddCategory(i, strName, strDesc);
  }

  // Add extra category with all other levels at the end
  AddCategory(_ctCats, TRANS("OTHER LEVELS"), TRANS("unsorted user made levels"));

  gm_pmgSelectedByDefault = &gm_amgCategories[0];
};

// Add category under a specific index
void CLevelCategoriesMenu::AddCategory(INDEX i, const CTString &strName, const CTString &strTip) {
  CMGLevelCategory &mg = gm_amgCategories[i];
  mg.mg_iCategory = i;

  mg.SetText(strName);
  mg.mg_strTip = strTip;

  const BOOL bBig = (_ctCats <= MAX_BIG_CATEGORIES);
  mg.mg_bfsFontSize = (bBig ? BFS_LARGE : BFS_MEDIUM);
  mg.mg_boxOnScreen = (bBig ? BoxBigRow(i - 1) : BoxMediumRow(i - 2));
  mg.mg_pActivatedFunction = NULL;

  const INDEX ct = _ctCats + 1;
  mg.mg_pmgUp = &gm_amgCategories[(i + ct - 1) % ct];
  mg.mg_pmgDown = &gm_amgCategories[(i + 1) % ct];

  AddChild(&mg);
};
