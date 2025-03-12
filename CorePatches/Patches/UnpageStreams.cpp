/* Copyright (c) 2022-2024 Dreamy Cecil
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

#if _PATCHCONFIG_ENGINEPATCHES

#include "UnpageStreams.h"
#include "FileSystem.h"

#include <Engine/Base/Unzip.h>

#include <CoreLib/Base/Unzip.h>
#include <CoreLib/Interfaces/ResourceFunctions.h>

#if _PATCHCONFIG_FIX_STREAMPAGING

// Define CNameTable_CTFileName
#define TYPE CTFileName
#define CNameTable_TYPE CNameTable_CTFileName
#define CNameTableSlot_TYPE CNameTableSlot_CTFileName

#define NAMETABLE_CASESENSITIVE 0
#include <Engine/Templates/NameTable.h>
#include <Engine/Templates/NameTable.cpp>

#undef CNameTableSlot_TYPE
#undef CNameTable_TYPE
#undef TYPE

// Allocate memory normally
void CUnpageStreamPatch::P_AllocVirtualMemory(ULONG ulBytesToAllocate)
{
  // Allocate at least 128 bytes and align them to blocks of 64
  ULONG ulAlloc = (ulBytesToAllocate / 64 + 2) * 64;

  strm_pubBufferBegin = (UBYTE *)calloc(ulAlloc, 1);
  strm_pubBufferEnd = strm_pubBufferBegin + ulAlloc;

  strm_pubCurrentPos = strm_pubBufferBegin;
  strm_pubMaxPos = strm_pubBufferBegin;

  strm_pubEOF = strm_pubBufferBegin + ulBytesToAllocate;
};

// Free memory normally
void CUnpageStreamPatch::P_FreeBuffer(void)
{
  if (strm_pubBufferBegin != NULL) {
    free(strm_pubBufferBegin);

    strm_pubBufferBegin = NULL;
    strm_pubBufferEnd   = NULL;
    strm_pubCurrentPos  = NULL;
    strm_pubEOF         = NULL;
    strm_pubMaxPos      = NULL;
  }
};

// Create a new file
void CFileStreamPatch::P_Create(const CTFileName &fnFileName, CTStream::CreateMode cm)
{
  CTFileName fnmFullFileName;
  INDEX iFile = ExpandFilePath(EFP_WRITE, fnFileName, fnmFullFileName);

  ASSERT(fnFileName.Length() > 0);
  ASSERT(fstrm_pFile == NULL);

  fstrm_pFile = fopen(fnmFullFileName, "wb+");

  if (fstrm_pFile == NULL) {
    Throw_t(LOCALIZE("Cannot create file `%s' (%s)"), fnmFullFileName.str_String, strerror(errno));
  }

  // Allocate enough memory for writing
  P_AllocVirtualMemory(_EnginePatches._ulMaxWriteMemory);

  strm_strStreamDescription = fnFileName;
  fstrm_bReadOnly = FALSE;
};

// Open a file
void CFileStreamPatch::P_Open(const CTFileName &fnFileName, CTStream::OpenMode om)
{
  ASSERT(fnFileName.Length() > 0);
  ASSERT(fstrm_pFile == NULL && fstrm_iZipHandle == -1);

  const ULONG ulOpenFlags = (om == OM_READ) ? EFP_READ : EFP_WRITE;
  CTFileName fnmFullFileName;
  INDEX iFile = ExpandFilePath(ulOpenFlags, fnFileName, fnmFullFileName);

  // [Cecil] Substitute missing resources with placeholders
  INDEX iPlaceholders = _EnginePatches._bUsePlaceholderResources;

  // [Cecil] Replace every resource for fun or only the ones that weren't found
  if (iPlaceholders >= 2 || (iPlaceholders > 0 && iFile == EFP_NONE)) {
    CTFileName fnmReplacement = fnFileName;
    CTString strExt = fnmReplacement.FileExt();

    if (strExt == ".mdl") {
      fnmReplacement = CTString("ModelsPatch\\Placeholder.mdl");
    } else if (strExt == ".tex") {
      fnmReplacement = CTString("TexturesPatch\\Placeholder.tex");
    } else if (strExt == ".wav" || strExt == ".mp3" || strExt == ".ogg") {
      fnmReplacement = CTString("Sounds\\Misc\\Silence.wav");
    }

    iFile = ExpandFilePath(ulOpenFlags, fnmReplacement, fnmFullFileName);
  }

  if (om == OM_READ) {
    fstrm_pFile = NULL;

    if (iFile == EFP_MODZIP || iFile == EFP_BASEZIP) {
      // Retrieve ZIP handle to the file
      fstrm_iZipHandle = IUnzip::Open_t(fnmFullFileName);

      // Allocate as much memory as the decompressed file size
      const SLONG slFileSize = IUnzip::GetSize(fstrm_iZipHandle);

      P_AllocVirtualMemory(slFileSize);

      // Read file contents into the stream
      IUnzip::ReadBlock_t(fstrm_iZipHandle, strm_pubBufferBegin, 0, slFileSize);

    } else if (iFile == EFP_FILE) {
      // Open file for reading
      fstrm_pFile = fopen(fnmFullFileName, "rb");

      // Allocate as much memory as the file size
      fseek(fstrm_pFile, 0, SEEK_END);
      const SLONG slFileSize = ftell(fstrm_pFile);
      fseek(fstrm_pFile, 0, SEEK_SET);

      P_AllocVirtualMemory(slFileSize);

      // Read file contents into the stream
      fread(strm_pubBufferBegin, slFileSize, 1, fstrm_pFile);

    } else {
      Throw_t(LOCALIZE("Cannot open file `%s' (%s)"), fnmFullFileName.str_String, LOCALIZE("File not found"));
    }

    fstrm_bReadOnly = TRUE;

  } else if (om == OM_WRITE) {
    // Open file for updating
    fstrm_pFile = fopen(fnmFullFileName, "rb+");
    fstrm_bReadOnly = FALSE;

    // Allocate enough memory for writing
    P_AllocVirtualMemory(_EnginePatches._ulMaxWriteMemory);

  } else {
    FatalError(LOCALIZE("File stream opening requested with unknown open mode: %d\n"), om);
  }

  if (fstrm_pFile == NULL && fstrm_iZipHandle == -1) {
    Throw_t(LOCALIZE("Cannot open file `%s' (%s)"), fnmFullFileName.str_String, strerror(errno));
  }

  strm_strStreamDescription = fnmFullFileName;
};

// Close opened file
void CFileStreamPatch::P_Close(void)
{
  if (fstrm_pFile == NULL && fstrm_iZipHandle == -1) {
    ASSERT(FALSE);
    return;
  }

  strm_strStreamDescription = "";

  if (fstrm_pFile != NULL) {
    // Flush written data back into the file
    if (!fstrm_bReadOnly) {
      fseek(fstrm_pFile, 0, SEEK_SET);
      fwrite(strm_pubBufferBegin, GetStreamSize(), 1, fstrm_pFile);
      fflush(fstrm_pFile);
    }

    fclose(fstrm_pFile);
    fstrm_pFile = NULL;

  } else if (fstrm_iZipHandle >= 0) {
    IUnzip::Close(fstrm_iZipHandle);

    fstrm_iZipHandle = -1;
  }

  // Clear allocated memory
  P_FreeBuffer();

  // Clear file dictionary
  strm_dmDictionaryMode = DM_NONE;
  strm_ntDictionary.Clear();
  strm_afnmDictionary.Clear();
  strm_slDictionaryPos = 0;
};

// Constructor
CRemLevel::CRemLevel(void) {
  strm_strStreamDescription = "dynamic memory stream";

  // Allocate enough memory for writing
  P_AllocVirtualMemory(_EnginePatches._ulMaxWriteMemory);
};

// Destructor
CRemLevel::~CRemLevel(void) {
  // Clear allocated memory
  P_FreeBuffer();
};

// Remember current level by its filename
void CRemLevelPatch::P_RememberCurrentLevel(const CTString &strFileName)
{
  for (;;) {
    CRemLevel *prlOld = P_FindRememberedLevel(strFileName);

    if (prlOld == NULL) {
      break;
    }

    prlOld->rl_lnInSessionState.Remove();
    delete prlOld;
  }

  CRemLevel *prlNew = new CRemLevel;
  ses_lhRememberedLevels.AddTail(prlNew->rl_lnInSessionState);

  prlNew->rl_strFileName = strFileName;
  WriteWorldAndState_t(prlNew);
};

// Fine remembered level by its filename
CRemLevel *CRemLevelPatch::P_FindRememberedLevel(const CTString &strFileName)
{
  FOREACHINLIST(CRemLevel, rl_lnInSessionState, ses_lhRememberedLevels, itrl) {
    CRemLevel &rl = *itrl;

    if (rl.rl_strFileName == strFileName) {
      return &rl;
    }
  }

  return NULL;
};

// Restore old level by its filename
void CRemLevelPatch::P_RestoreOldLevel(const CTString &strFileName)
{
  CRemLevel *prlOld = P_FindRememberedLevel(strFileName);
  ASSERT(prlOld != NULL);

  try {
    prlOld->SetPos_t(0);
    _pTimer->SetCurrentTick(0.0f);

    ReadWorldAndState_t(prlOld);
    _pTimer->SetCurrentTick(ses_tmLastProcessedTick);

  } catch (char *strError) {
    FatalError(LOCALIZE("Cannot restore old level '%s':\n%s"), prlOld->rl_strFileName, strError);
  }

  delete prlOld;
};

// Forget all remembered levels
void CRemLevelPatch::P_ForgetOldLevels(void)
{
  FORDELETELIST(CRemLevel, rl_lnInSessionState, ses_lhRememberedLevels, itrl) {
    delete &*itrl;
  }
};

#endif // _PATCHCONFIG_FIX_STREAMPAGING

#endif // _PATCHCONFIG_ENGINEPATCHES
