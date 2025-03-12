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

#ifndef CECIL_INCL_UNZIP_H
#define CECIL_INCL_UNZIP_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// A file entry in a ZIP archive
class CZipEntry {
  public:
    const CTFileName *ze_pfnmArchive; // Path of the archive
    CTFileName ze_fnm;           // File name with path inside archive
    SLONG ze_slCompressedSize;   // Size of file in the archive
    SLONG ze_slUncompressedSize; // Size when uncompressed
    SLONG ze_slDataOffset;       // Position of compressed data inside archive
    ULONG ze_ulCRC;              // Checksum of the file
    BOOL ze_bStored;             // Set if file is not compressed, but stored

    // [Cecil] Rev: Mod archives cannot exist
  #if SE1_GAME != SS_REV
    BOOL ze_bMod; // Set if from a mod's archive
  #endif

  public:
    void Clear(void) {
      ze_pfnmArchive = NULL;
      ze_fnm.Clear();
    };
};

// Interface with functions for getting files out of ZIP archives
namespace IUnzip {

// [Cecil] Sort files in ZIP archives by content directory
CORE_API void SortEntries(void);

// Add one zip archive to the currently active set
CORE_API void AddArchive(const CTFileName &fnm);

// Read directories of all currently added archives in reverse alphabetical order
CORE_API void ReadDirectoriesReverse_t(void);

// Enumeration for all files in all zips
CORE_API INDEX GetFileCount(void);

// [Cecil] Get ZIP file entry at a specific position
CORE_API const CZipEntry &GetEntry(INDEX i);

// Get file at a specific position
CORE_API const CTFileName &GetFileAtIndex(INDEX i);

// Check if specific file is from a mod
CORE_API BOOL IsFileAtIndexMod(INDEX i);

// Get index of a specific file (-1 if no file)
CORE_API INDEX GetFileIndex(const CTFileName &fnm);

// [Cecil] Get path to the archive with the file
CORE_API const CTFileName &GetFileArchive(const CTFileName &fnm);

// Get info of a zip file entry
CORE_API void GetFileInfo(INDEX iHandle, CTFileName &fnmZip,
  SLONG &slOffset, SLONG &slSizeCompressed, SLONG &slSizeUncompressed, BOOL &bCompressed);

// Check if a file entry exists
inline BOOL FileExists(const CTFileName &fnm) {
  return (GetFileIndex(fnm) != -1);
};

// Open a zip file entry for reading
CORE_API INDEX Open_t(const CTFileName &fnm);

// Get uncompressed size of a file
CORE_API SLONG GetSize(INDEX iHandle);

// Get CRC of a file
CORE_API ULONG GetCRC(INDEX iHandle);

// Read a block from ZIP file
CORE_API void ReadBlock_t(INDEX iHandle, UBYTE *pub, SLONG slStart, SLONG slLen);

// Close a ZIP file entry
CORE_API void Close(INDEX iHandle);

}; // namespace

#endif
