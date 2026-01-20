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

#ifndef CECIL_INCL_GAMEDIRECTORIES_H
#define CECIL_INCL_GAMEDIRECTORIES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Different level formats
enum ELevelFormat {
  E_LF_TFE = 0, // Serious Sam: The First Encounter
  E_LF_TSE = 1, // Serious Sam: The Second Encounter
  E_LF_SSR = 2, // Serious Sam Revolution
  E_LF_150 = 3, // Games on 1.50

  E_LF_FORMATCOUNT,

  // Current format
#if SE1_GAME == SS_REV
  E_LF_CURRENT = E_LF_SSR,
#elif SE1_GAME == SS_TFE
  E_LF_CURRENT = E_LF_TFE,
#else
  E_LF_CURRENT = E_LF_TSE,
#endif
};

enum EFileListFlags {
  FLF_RECURSIVE   = DLI_RECURSIVE, // Look into subdirectories
  FLF_SEARCHCD    = DLI_SEARCHCD,  // List extras from the CD
  FLF_SEARCHMOD   = (1 << 2),      // List extras from the mod (always enabled in SE1's MakeDirList)
  FLF_ONLYCD      = (1 << 3),      // List exclusively from the CD directory (incompatible with FLF_ONLYMOD)
  FLF_ONLYMOD     = (1 << 4),      // List exclusively from the mod directory (incompatible with FLF_ONLYCD)
  FLF_IGNORELISTS = (1 << 5),      // Ignore include/exclude lists if playing a mod
  FLF_IGNOREGRO   = (1 << 6),      // Ignore contents of loaded GRO packages
  FLF_SEARCHGAMES = (1 << 7),      // Search directories of other games (TFE, SSR etc.)
  FLF_REUSELIST   = (1 << 8),      // Reuse existing entries in the provided list
};

// Other game directories
#define GAME_DIRECTORIES_CT 2
CORE_API extern CTString _astrGameDirs[GAME_DIRECTORIES_CT];

#define GAME_DIR_TFE (_astrGameDirs[0])
#define GAME_DIR_SSR (_astrGameDirs[1])

// Include/exclude lists for base directory writing/reading
CORE_API extern CFileList _aBaseWriteInc;
CORE_API extern CFileList _aBaseWriteExc;
CORE_API extern CFileList _aBaseBrowseInc;
CORE_API extern CFileList _aBaseBrowseExc;

// Check if a filename is under a specified game directory
CORE_API BOOL IsFileFromDir(const CTString &strGameDir, const CTFileName &fnm);

// List files from a specific directory on a disk
CORE_API void ListInDir(const CTFileName &fnmBaseDir, CFileList &afnm,
  const CTString &strDir, const CTString &strPattern, BOOL bRecursive, CFileList *paInclude, CFileList *paExclude);

// List files from a specific game directory
CORE_API void ListGameFiles(CFileList &afnmFiles, const CTString &strDir, const CTString &strPattern, ULONG ulFlags);

#endif
