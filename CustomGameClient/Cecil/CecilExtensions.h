/* Copyright (c) 2022-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_EXTENSIONS_H
#define CECIL_INCL_EXTENSIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// General
extern INDEX sam_bPatchVersionLabel;
extern INDEX sam_bBackgroundGameRender;
extern INDEX sam_bOptionTabs;
extern INDEX sam_bConfigValueLists;
extern INDEX sam_bModernScrollbars;
extern INDEX sam_bLevelCategories;
extern INDEX sam_bLevelFiltering;
extern INDEX sam_bExtrasMenu;
extern INDEX sam_bDecoratedServerNames;

extern INDEX sam_iUpdateReminder;
extern CTString sam_strLastVersionCheck;

extern INDEX sam_iLastSeasonCheck;

// Level filtering
extern INDEX sam_iShowLevelFormat;
extern CTString sam_strLevelTitleFilter;

// Command line arguments for reloading the current game
extern CTString _strRestartCommandLine;
extern BOOL _bRestartGameClient;

// Restart the game client
void RestartGame(void);

// Start some level immediately (like on '+level' argument)
void StartMap(SHELL_FUNC_ARGS);

// Custom initialization
void ClassicsPatch_InitExt(void);

#endif
