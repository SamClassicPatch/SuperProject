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

#include "StdH.h"

#include "Unzip.h"

// Other game directories
CTString _astrGameDirs[GAME_DIRECTORIES_CT];

// Include/exclude lists for base directory writing/reading
CFileList _aBaseWriteInc;
CFileList _aBaseWriteExc;
CFileList _aBaseBrowseInc;
CFileList _aBaseBrowseExc;

// Check if a filename is under a specified game directory
BOOL IsFileFromDir(const CTString &strGameDir, const CTFileName &fnm) {
  if (strGameDir == "") return FALSE;

  CTFileName fnmFull;

  // Try checking the archive path
  if (ExpandFilePath(EFP_READ, fnm, fnmFull) == EFP_BASEZIP) {
    fnmFull = IUnzip::GetFileArchive(fnm);
  }

  return fnmFull.HasPrefix(strGameDir);
};

// List files from a specific directory on a disk
void ListInDir(const CTFileName &fnmBaseDir, CFileList &afnm,
  const CTString &strDir, const CTString &strPattern, BOOL bRecursive, CFileList *paInclude, CFileList *paExclude)
{
  struct DirToRead {
    CListNode dr_lnNode;
    CTString dr_strDir;
  };

  // Add the directory to search list
  DirToRead *pdrFirst = new DirToRead;
  pdrFirst->dr_strDir = strDir;

  CListHead lhDirs;
  lhDirs.AddTail(pdrFirst->dr_lnNode);

  // While the list of directories is not empty
  while (!lhDirs.IsEmpty())
  {
    // Take the first one
    DirToRead *pdr = LIST_HEAD(lhDirs, DirToRead, dr_lnNode);

    const CTString strCurDir = pdr->dr_strDir;
    delete pdr;

    // Skip if the directory is not allowed
    if (paInclude != NULL && paExclude != NULL
    && (IFiles::MatchesList(*paInclude, strCurDir) == -1 || IFiles::MatchesList(*paExclude, strCurDir) != -1)) {
      continue;
    }

    // Start listing the directory
    _finddata_t fdFile;
    long hFile = _findfirst((fnmBaseDir + strCurDir + "*").str_String, &fdFile);

    // Keep going through the files in the directory
    BOOL bFileExists = (hFile != -1);

    while (bFileExists)
    {
      // Skip dummy directories
      if (fdFile.name[0] == '.') {
        bFileExists = (_findnext(hFile, &fdFile) == 0);
        continue;
      }

      // Get path to the file
      CTString strFile = strCurDir + fdFile.name;

      // If it's a directory
      if (fdFile.attrib & _A_SUBDIR) {
        // If reading recursively
        if (bRecursive) {
          // Add this directory to search list
          DirToRead *pdrNew = new DirToRead;
          pdrNew->dr_strDir = strFile + "\\";

          lhDirs.AddTail(pdrNew->dr_lnNode);
        }

      // If it matches the pattern
      } else if (strPattern == "" || strFile.Matches(strPattern)) {
        // Add the file
        CTFileName &fnmPush = afnm.Push();
        fnmPush = strFile;
      }

      // Try to find the next file
      bFileExists = (_findnext(hFile, &fdFile) == 0);
    }
  }
};

// List files from a specific game directory
void ListGameFiles(CFileList &afnmFiles, const CTString &strDir, const CTString &strPattern, ULONG ulFlags)
{
  const BOOL bRecursive = ulFlags & FLF_RECURSIVE;

  // Don't allow multiple exclusive flags at once
  #ifndef NDEBUG
    BOOL bAssertOnlyCD = (ulFlags & FLF_ONLYCD) != 0;
    BOOL bAssertOnlyMod = (ulFlags & FLF_ONLYMOD) != 0;
    ASSERTMSG((!bAssertOnlyCD || !bAssertOnlyMod), "Cannot have multiple exclusive flags in ListGameFiles()!");
  #endif

  // Make a temporary list
  CFileList afnmTemp;

  // Reuse the file list
  if (ulFlags & FLF_REUSELIST) {
    afnmTemp = afnmFiles;
  }

  // Clear the final list
  afnmFiles.PopAll();

  BOOL bMod = (_fnmMod != "");
  BOOL bCD = (_fnmCDPath != "");
  BOOL bLists = bMod && !(ulFlags & FLF_IGNORELISTS);

  // List files exclusively from the mod
  if (ulFlags & FLF_ONLYMOD) {
    if (bMod) {
      ListInDir(IDir::AppPath() + _fnmMod, afnmTemp, strDir, strPattern, bRecursive, NULL, NULL);
    }

  // List files exclusively from the CD
  } else if (ulFlags & FLF_ONLYCD) {
    if (bCD) {
      ListInDir(_fnmCDPath, afnmTemp, strDir, strPattern, bRecursive,
                bLists ? &_aBaseBrowseInc : NULL, bLists ? &_aBaseBrowseExc : NULL);
    }

  } else {
    // List files from the game directory
    ListInDir(IDir::AppPath(), afnmTemp, strDir, strPattern, bRecursive,
              bLists ? &_aBaseBrowseInc : NULL, bLists ? &_aBaseBrowseExc : NULL);

    // [Cecil] List files from other game directories
    if (ulFlags & FLF_SEARCHGAMES) {
      for (INDEX iDir = 0; iDir < GAME_DIRECTORIES_CT; iDir++)
      {
        if (_astrGameDirs[iDir] == "") continue;

        ListInDir(_astrGameDirs[iDir], afnmTemp, strDir, strPattern, bRecursive,
                  bLists ? &_aBaseBrowseInc : NULL, bLists ? &_aBaseBrowseExc : NULL);
      }
    }

    // List extra files from the CD
    if (ulFlags & FLF_SEARCHCD && bCD) {
      ListInDir(_fnmCDPath, afnmTemp, strDir, strPattern, bRecursive,
                bLists ? &_aBaseBrowseInc : NULL, bLists ? &_aBaseBrowseExc : NULL);
    }

    // List extra files from the mod directory
    if (ulFlags & FLF_SEARCHMOD && bMod) {
      ListInDir(IDir::AppPath() + _fnmMod, afnmTemp, strDir, strPattern, bRecursive, NULL, NULL);
    }
  }

  // Search for files in the archives
  if (!(ulFlags & FLF_IGNOREGRO)) {
    INDEX ctFilesInZips = IUnzip::GetFileCount();

    for (INDEX iFileInZip = 0; iFileInZip < ctFilesInZips; iFileInZip++) {
      // Get ZIP entry
      const CZipEntry &ze = IUnzip::GetEntry(iFileInZip);
      const CTFileName &fnm = ze.ze_fnm;

      // Skip if not under this directory
      if (bRecursive) {
        if (!fnm.HasPrefix(strDir)) continue;

      // Skip if not the same directory
      } else if (fnm.FileDir() != strDir) {
        continue;
      }

      // Doesn't match the pattern
      if (strPattern != "" && !fnm.Matches(strPattern)) continue;

      // [Cecil] Ignore archive files from other game directories
      BOOL bSkipFromOtherGames = FALSE;

      if (!(ulFlags & FLF_SEARCHGAMES)) {
        for (INDEX iDir = 0; iDir < GAME_DIRECTORIES_CT; iDir++)
        {
          if (_astrGameDirs[iDir] == "") continue;

          if (ze.ze_pfnmArchive->HasPrefix(_astrGameDirs[iDir])) {
            bSkipFromOtherGames = TRUE;
            break;
          }
        }
      }

      if (bSkipFromOtherGames) continue;

      BOOL bFileFromMod = IUnzip::IsFileAtIndexMod(iFileInZip);

      // List files exclusively from the mod
      if (ulFlags & FLF_ONLYMOD) {
        if (bMod && bFileFromMod) {
          afnmTemp.Push() = fnm;
        }

      // List files from the game
      } else if (!bFileFromMod) {
        // Not a mod file or shouldn't match mod's browse paths
        if (!bLists) {
          afnmTemp.Push() = fnm;

        // Matches mod's browse paths
        } else if (IFiles::MatchesList(_aBaseBrowseInc, fnm) != -1 && IFiles::MatchesList(_aBaseBrowseExc, fnm) == -1) {
          afnmTemp.Push() = fnm;
        }

      // List extras from the mod
      } else if (ulFlags & FLF_SEARCHMOD && bMod) {
        afnmTemp.Push() = fnm;
      }
    }
  }

  const INDEX ctFiles = afnmTemp.Count();

  // Don't check for duplicates if no files
  if (ctFiles == 0) return;

  // Sort the file list
  qsort(afnmTemp.da_Pointers, afnmTemp.Count(), sizeof(CTFileName *), IFiles::CompareFileNames);

  // Copy the first file into the final list
  afnmFiles.Push() = afnmTemp[0];

  // Copy the rest of the files if they aren't matching previous files
  for (INDEX iFile = 1; iFile < ctFiles; iFile++)
  {
    if (afnmTemp[iFile] != afnmTemp[iFile - 1]) {
      afnmFiles.Push() = afnmTemp[iFile];
    }
  }
};
