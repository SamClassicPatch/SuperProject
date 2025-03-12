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

#ifndef SE_INCL_MENU_STUFF_H
#define SE_INCL_MENU_STUFF_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "FileInfo.h"

#define TRIGGER_MG(mg, y, up, down, text, astr) \
  mg.mg_pmgUp = &up; \
  mg.mg_pmgDown = &down; \
  mg.mg_boxOnScreen = BoxMediumRow(y); \
  AddChild(&mg); \
  mg.mg_astrTexts = astr; \
  mg.mg_ctTexts = sizeof(astr) / sizeof(astr[0]); \
  mg.mg_iSelected = 0; \
  mg.SetName(text); \
  mg.SetText(astr[0]);

extern INDEX ctGameTypeRadioTexts;

extern CTString astrNoYes[2];
extern CTString astrWeapon[4];
extern CTString astrComputerInvoke[2];
extern CTString astrCrosshair[8];
extern CTString *astrMaxPlayersRadioTexts; // [Cecil] Dynamic array
extern CTString astrGameTypeRadioTexts[16];
extern CTString astrDifficultyRadioTexts[16]; // [Cecil] 16 difficulties
extern CTString astrSplitScreenRadioTexts[4];
extern CTString astrDisplayPrefsRadioTexts[5]; // [Cecil] 4 -> 5

// [Cecil] Last option in video preferences (Custom)
static const INDEX _iDisplayPrefsLastOpt = ARRAYCOUNT(astrDisplayPrefsRadioTexts) - 1;

void InitGameTypes(void);

int qsort_CompareFileInfos_NameUp(const void *elem1, const void *elem2);
int qsort_CompareFileInfos_NameDn(const void *elem1, const void *elem2);
int qsort_CompareFileInfos_FileUp(const void *elem1, const void *elem2);
int qsort_CompareFileInfos_FileDn(const void *elem1, const void *elem2);

GfxAPIType NormalizeGfxAPI(INDEX i);
DisplayDepth NormalizeDepth(INDEX i);

void ControlsMenuOn();
void ControlsMenuOff();

#endif /* include-once check. */