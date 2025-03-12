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

// Include zlib
#include <Extras/zlib/zlib.h>
#pragma comment(lib, "zlib.lib")

// [Cecil] Pointer to 'zip_csLock' in the engine
static CTCriticalSection *_pcsZipLock = (CTCriticalSection *)ADDR_UNZIP_CRITSEC;

#pragma pack(push, 1)

// Before each file in the zip
#define SIGNATURE_LFH 0x04034b50

struct LocalFileHeader {
  SWORD lfh_swVersionToExtract;
  SWORD lfh_swGPBFlag;
  SWORD lfh_swCompressionMethod;
  SWORD lfh_swModFileTime;
  SWORD lfh_swModFileDate;
  SLONG lfh_slCRC32;
  SLONG lfh_slCompressedSize;
  SLONG lfh_slUncompressedSize;
  SWORD lfh_swFileNameLen;
  SWORD lfh_swExtraFieldLen;
  // Follows:
  // - filename (variable size)
  // - extra field (variable size)
};

// After file data, only if compressed from a non-seekable stream
// this exists only if bit 3 in GPB flag is set
#define SIGNATURE_DD 0x08074b50

struct DataDescriptor {
  SLONG dd_slCRC32;
  SLONG dd_slCompressedSize;
  SLONG dd_slUncompressedSize;
};

// One file in central dir
#define SIGNATURE_FH 0x02014b50

struct FileHeader {
  SWORD fh_swVersionMadeBy;
  SWORD fh_swVersionToExtract;
  SWORD fh_swGPBFlag;
  SWORD fh_swCompressionMethod;
  SWORD fh_swModFileTime;
  SWORD fh_swModFileDate;
  SLONG fh_slCRC32;
  SLONG fh_slCompressedSize;
  SLONG fh_slUncompressedSize;
  SWORD fh_swFileNameLen;
  SWORD fh_swExtraFieldLen;
  SWORD fh_swFileCommentLen;
  SWORD fh_swDiskNoStart;
  SWORD fh_swInternalFileAttributes;
  SLONG fh_swExternalFileAttributes;
  SLONG fh_slLocalHeaderOffset;
  // Follows:
  // - filename (variable size)
  // - extra field (variable size)
  // - file comment (variable size)
};

// At the end of entire zip file
#define SIGNATURE_EOD 0x06054b50

struct EndOfDir {
  SWORD eod_swDiskNo;
  SWORD eod_swDirStartDiskNo;
  SWORD eod_swEntriesInDirOnThisDisk;
  SWORD eod_swEntriesInDir;
  SLONG eod_slSizeOfDir;
  SLONG eod_slDirOffsetInFile;
  SWORD eod_swCommentLenght;
  // Follows:
  // - zipfile comment (variable size)
};

#pragma pack(pop)

#define BUF_SIZE 1024

// An open instance of a file inside a zip
class CZipHandle {
  public:
    BOOL zh_bOpen;        // Set if the handle is used
    CZipEntry zh_zeEntry; // The entry itself
    z_stream zh_zstream;  // zlib filestream for decompression
    FILE *zh_fFile;       // Open handle of the archive
    UBYTE *zh_pubBufIn;   // Input buffer
    
  public:
    CZipHandle();
    void Clear(void);
    void Throw_t(int iErr, const CTString &strDescription);
};

CZipHandle::CZipHandle(void)
{
  zh_bOpen = FALSE;
  zh_fFile = NULL;
  zh_pubBufIn = NULL;

  memset(&zh_zstream, 0, sizeof(zh_zstream));
};

void CZipHandle::Clear(void)
{
  zh_bOpen = FALSE;
  zh_zeEntry.Clear();

  // Clear the zlib stream
  CTSingleLock slZip(_pcsZipLock, TRUE);

  inflateEnd(&zh_zstream);
  memset(&zh_zstream, 0, sizeof(zh_zstream));

  // Free buffers
  if (zh_pubBufIn != NULL) {
    FreeMemory(zh_pubBufIn);
    zh_pubBufIn = NULL;
  }

  // Close the zip archive file
  if (zh_fFile != NULL) {
    fclose(zh_fFile);
    zh_fFile = NULL;
  }
};

void CZipHandle::Throw_t(int iErr, const CTString &strDescription) {
  CTString strError;

  switch (iErr) {
    case Z_OK:            strError = "Z_OK           "; break;
    case Z_STREAM_END:    strError = "Z_STREAM_END   "; break;
    case Z_NEED_DICT:     strError = "Z_NEED_DICT    "; break;
    case Z_STREAM_ERROR:  strError = "Z_STREAM_ERROR "; break;
    case Z_DATA_ERROR:    strError = "Z_DATA_ERROR   "; break;
    case Z_MEM_ERROR:     strError = "Z_MEM_ERROR    "; break;
    case Z_BUF_ERROR:     strError = "Z_BUF_ERROR    "; break;
    case Z_VERSION_ERROR: strError = "Z_VERSION_ERROR"; break;
    case Z_ERRNO: strError.PrintF("Z_ERRNO: %s", strerror(errno)); break;
    default: strError.PrintF(LOCALIZE("Unknown ZLIB error: %d"), iErr);
  }

  ThrowF_t(LOCALIZE("(%s/%s) %s - ZLIB error: %s - %s"), zh_zeEntry.ze_pfnmArchive->str_String,
    zh_zeEntry.ze_fnm.str_String, strDescription, strError.str_String, zh_zstream.msg);
};

// [Cecil] Pointer to '_azhHandles' in the engine
static CStaticStackArray<CZipHandle> &_aZipHandles = *(CStaticStackArray<CZipHandle> *)ADDR_UNZIP_HANDLES;

// [Cecil] Pointer to '_azeFiles' in the engine
static CStaticStackArray<CZipEntry> &_aZipFiles = *(CStaticStackArray<CZipEntry> *)ADDR_UNZIP_ENTRIES;

// [Cecil] Pointer to '_afnmArchives' in the engine
static CStaticStackArray<CTFileName> &_aZipArchives = *(CStaticStackArray<CTFileName> *)ADDR_UNZIP_ARCHIVES;

// [Cecil] Handle verification (to avoid code duplication)
static inline BOOL VerifyHandle(INDEX iHandle) {
  // Check handle number
  if (iHandle < 0 || iHandle >= _aZipHandles.Count()) {
    ASSERT(FALSE);
    return FALSE;
  }

  // Check the handle
  CZipHandle &zh = _aZipHandles[iHandle];

  if (!zh.zh_bOpen) {
    ASSERT(FALSE);
    return FALSE;
  }

  return TRUE;
};

namespace IUnzip {

// [Cecil] Get priority for a specific archive
static INDEX ArchiveDirPriority(CTFileName fnm)
{
  #define PRI_MOD   103
  #define PRI_CDMOD 102
  #define PRI_EXTRA 101
  #define PRI_GAME  100
  #define PRI_CD   -100 // Absolute last resort

  // Current game (overrides other games and CD)
  if (fnm.RemovePrefix(IDir::AppPath())) {
    // Check for mod (overrides everything)
    return fnm.HasPrefix("Mods\\") ? PRI_MOD : PRI_GAME;

  // CD path
  } else if (_fnmCDPath != "" && fnm.RemovePrefix(_fnmCDPath)) {
    // Check for CD mod (overrides everything but normal mod)
    return fnm.HasPrefix("Mods\\") ? PRI_CDMOD : PRI_CD;
  }

  // Other game paths
  for (INDEX iDir = 0; iDir < GAME_DIRECTORIES_CT; iDir++) {
    const CTString &strDir = _astrGameDirs[iDir];

    if (strDir != "" && fnm.HasPrefix(strDir)) {
      // Sort by list index (doesn't override other files)
      return iDir;
    }
  }

  // None of the above - extra content directory (overrides current game)
  return PRI_EXTRA;
};

// [Cecil] Compare two ZIP file entries
static int qsort_CompareContentDir(const void *pElement1, const void *pElement2)
{
  // Get the entries
  const CZipEntry &ze1 = *(const CZipEntry *)pElement1;
  const CZipEntry &ze2 = *(const CZipEntry *)pElement2;

  // Sort archive directories with priority
  INDEX iPriority1 = ArchiveDirPriority(*ze1.ze_pfnmArchive);
  INDEX iPriority2 = ArchiveDirPriority(*ze2.ze_pfnmArchive);

  if (iPriority1 < iPriority2) {
    return +1;
  } else if (iPriority1 > iPriority2) {
    return -1;
  }

  // Sort archives in reverse alphabetical order
  return -stricmp(ze1.ze_pfnmArchive->str_String, ze2.ze_pfnmArchive->str_String);
};

// [Cecil] Sort files in ZIP archives by content directory
void SortEntries(void) {
  // Order of archives after sorting:
  // 1. From mod
  // 2. From mod on CD
  // 3. From extra content directories
  // 4. From the game itself
  // 5. From other game directories
  // 6. From CD
  qsort(&_aZipFiles[0], _aZipFiles.Count(), sizeof(CZipEntry), qsort_CompareContentDir);
};

// Add one zip archive to the currently active set
void AddArchive(const CTFileName &fnm)
{
  _aZipArchives.Push() = fnm;
};

// Read directory of a zip archive and add all files in it to active set
static void ReadZIPDirectory_t(CTFileName *pfnmZip)
{
  char *strZip = pfnmZip->str_String;

  // Open the archive
  FILE *f = fopen(strZip, "rb");

  if (f == NULL) {
    ThrowF_t(LOCALIZE("%s: Cannot open file (%s)"), strZip, strerror(errno));
  }

  // Start at the end of file, minus expected minimum overhead
  fseek(f, 0, SEEK_END);

  int iPos = ftell(f) - sizeof(long) - sizeof(EndOfDir) + 2;

  // Do not search more than 128k (should be around 65k at most)
  int iMinPos = iPos - 128 * 1024;

  if (iMinPos < 0) {
    iMinPos = 0;
  }

  EndOfDir eod;
  BOOL bEODFound = FALSE;

  // While not at the beginning
  for (; iPos > iMinPos; iPos--) {
    // Read signature
    fseek(f, iPos, SEEK_SET);

    int iSignature;
    fread(&iSignature, sizeof(iSignature), 1, f);

    // Found the signature
    if (iSignature == SIGNATURE_EOD) {
      // Read directory end
      fread(&eod, sizeof(eod), 1, f);

      // Cannot have a multi-volume zip
      if (eod.eod_swDiskNo != 0 || eod.eod_swDirStartDiskNo != 0
       || eod.eod_swEntriesInDirOnThisDisk != eod.eod_swEntriesInDir) {
        ThrowF_t(LOCALIZE("%s: Multi-volume zips are not supported"), strZip);
      }

      // Cannot have an empty zip
      if (eod.eod_swEntriesInDir <= 0) {
        ThrowF_t(LOCALIZE("%s: Empty zip"), strZip);
      }

      bEODFound = TRUE;
      break;
    }
  }

  // EOD is not found
  if (!bEODFound) {
    ThrowF_t(LOCALIZE("%s: Cannot find 'end of central directory'"), strZip);
  }

  // Check if the zip is from a mod
  BOOL bMod = pfnmZip->HasPrefix(IDir::AppPath() + "Mods\\")
           || pfnmZip->HasPrefix(_fnmCDPath + "Mods\\");

  // Go to the beginning of the central dir
  fseek(f, eod.eod_slDirOffsetInFile, SEEK_SET);

  INDEX ctFiles = 0;

  // For each file
  for (INDEX iFile = 0; iFile < eod.eod_swEntriesInDir; iFile++) {
    // Read the signature
    int slSig;
    fread(&slSig, sizeof(slSig), 1, f);

    // Unexpected signature
    if (slSig != SIGNATURE_FH) {
      ThrowF_t(LOCALIZE("%s: Wrong signature for 'file header' number %d'"), strZip, iFile);
    }

    // Read its header
    FileHeader fh;
    fread(&fh, sizeof(fh), 1, f);

    // Check the filename
    const SLONG slMaxFileName = 512;

    if (fh.fh_swFileNameLen > slMaxFileName) {
      ThrowF_t(LOCALIZE("%s: Too long filepath in zip"), strZip);
    }

    if (fh.fh_swFileNameLen <= 0) {
      ThrowF_t(LOCALIZE("%s: Invalid filepath length in zip"), strZip);
    }
    
    // Read the filename
    char strBuffer[slMaxFileName + 1];
    memset(strBuffer, 0, sizeof(strBuffer));

    fread(strBuffer, fh.fh_swFileNameLen, 1, f);

    // Skip eventual comment and extra fields
    if (fh.fh_swFileCommentLen + fh.fh_swExtraFieldLen > 0) {
      fseek(f, fh.fh_swFileCommentLen+fh.fh_swExtraFieldLen, SEEK_CUR);
    }

    // If it's a directory
    if (strBuffer[strlen(strBuffer) - 1] == '/') {
      // Check the size
      if (fh.fh_slUncompressedSize != 0 || fh.fh_slCompressedSize != 0) {
        ThrowF_t(LOCALIZE("%s/%s: Invalid directory"), strZip, strBuffer);
      }

    // If it's a file
    } else {
      ctFiles++;

      // Convert slashes in the filename
      IData::ReplaceChar(strBuffer, '/', '\\');

      // Create a new entry
      CZipEntry &ze = _aZipFiles.Push();

      // Remember file data
      ze.ze_fnm = CTString(strBuffer);
      ze.ze_pfnmArchive = pfnmZip;
      ze.ze_slCompressedSize = fh.fh_slCompressedSize;
      ze.ze_slUncompressedSize = fh.fh_slUncompressedSize;
      ze.ze_slDataOffset = fh.fh_slLocalHeaderOffset;
      ze.ze_ulCRC = fh.fh_slCRC32;

      #if SE1_GAME != SS_REV
        ze.ze_bMod = bMod;
      #endif

      // Check for compressopn
      if (fh.fh_swCompressionMethod == 0) {
        ze.ze_bStored = TRUE;

      } else if (fh.fh_swCompressionMethod == 8) {
        ze.ze_bStored = FALSE;

      } else {
        ThrowF_t(LOCALIZE("%s/%s: Only 'deflate' compression is supported"),
          ze.ze_pfnmArchive->str_String, ze.ze_fnm.str_String);
      }
    }
  }

  // Some error has occurred
  if (ferror(f)) {
    ThrowF_t(LOCALIZE("%s: Error reading central directory"), strZip);
  }

  // Report that the file has been read
  CPrintF(LOCALIZE("  %s: %d files\n"), strZip, ctFiles++);
};

// Read one directory of an archive
static void ReadOneArchiveDir_t(CTFileName &fnm)
{
  // Remember current number of files
  INDEX ctOrgFiles = _aZipFiles.Count();

  // Try to read the directory and add all files
  try {
    ReadZIPDirectory_t(&fnm);

  } catch (char *) {
    // Remove added files
    if (ctOrgFiles < _aZipFiles.Count())
    {
      if (ctOrgFiles == 0) {
        _aZipFiles.PopAll();
      } else {
        _aZipFiles.PopUntil(ctOrgFiles - 1);
      }
    }

    // Pass the error further up
    throw;
  }
};

static int qsort_ArchiveCTFileName_reverse(const void *pElement1, const void *pElement2)
{
  // Get the filenames
  const CTFileName &fnm1 = *(const CTFileName *)pElement1;
  const CTFileName &fnm2 = *(const CTFileName *)pElement2;

  // Calculate priorities based on the location of a GRO file
  BOOL bMod1 = fnm1.HasPrefix(IDir::AppPath() + "Mods\\");
  BOOL bCD1 = fnm1.HasPrefix(_fnmCDPath);
  BOOL bModCD1 = fnm1.HasPrefix(_fnmCDPath + "Mods\\");

  INDEX iPriority1 = 1;

  if (bMod1) {
    iPriority1 = 3;

  } else if (bModCD1) {
    iPriority1 = 2;

  } else if (bCD1) {
    iPriority1 = 0;
  }

  BOOL bMod2 = fnm2.HasPrefix(IDir::AppPath() + "Mods\\");
  BOOL bCD2 = fnm2.HasPrefix(_fnmCDPath);
  BOOL bModCD2 = fnm2.HasPrefix(_fnmCDPath + "Mods\\");

  INDEX iPriority2 = 1;

  if (bMod2) {
    iPriority2 = 3;

  } else if (bModCD2) {
    iPriority2 = 2;

  } else if (bCD2) {
    iPriority2 = 0;
  }

  // Find sorting order
  if (iPriority1 < iPriority2) {
    return +1;
  } else if (iPriority1 > iPriority2) {
    return -1;
  }

  return -stricmp(fnm1.str_String, fnm2.str_String);
};

// Read directories of all currently added archives in reverse alphabetical order
void ReadDirectoriesReverse_t(void)
{
  // No archives
  if (_aZipArchives.Count() == 0) return;

  // Sort the archive filenames reversely
  qsort(&_aZipArchives[0], _aZipArchives.Count(), sizeof(CTFileName), qsort_ArchiveCTFileName_reverse);

  CTString strAllErrors = "";

  // Go through the archives
  for (INDEX iArchive = 0; iArchive < _aZipArchives.Count(); iArchive++) {
    // Try to read the archive directory
    try {
      ReadOneArchiveDir_t(_aZipArchives[iArchive]);

    // Write the error
    } catch (char *strError) {
      strAllErrors += strError;
      strAllErrors += "\n";
    }
  }

  // Report any errors
  if (strAllErrors != "") {
    ThrowF_t(strAllErrors.str_String);
  }
};

// Enumeration for all files in all zips
INDEX GetFileCount(void) {
  return _aZipFiles.Count();
};

// [Cecil] Get ZIP file entry at a specific position
const CZipEntry &GetEntry(INDEX i) {
  return _aZipFiles[i];
};

// Get file at a specific position
const CTFileName &GetFileAtIndex(INDEX i) {
  return _aZipFiles[i].ze_fnm;
};

// Check if specific file is from a mod
BOOL IsFileAtIndexMod(INDEX i) {
#if SE1_GAME != SS_REV
  return _aZipFiles[i].ze_bMod;
#else
  return FALSE;
#endif
};

// Get index of a specific file (-1 if no file)
INDEX GetFileIndex(const CTFileName &fnm)
{
  for (INDEX iFile = 0; iFile < _aZipFiles.Count(); iFile++) {
    // Filename matches
    if (_aZipFiles[iFile].ze_fnm == fnm) {
      return iFile;
    }
  }

  return -1;
};

// [Cecil] Get path to the archive with the file
const CTFileName &GetFileArchive(const CTFileName &fnm) {
  for (INDEX iFile = 0; iFile < _aZipFiles.Count(); iFile++) {
    // Filename matches
    if (_aZipFiles[iFile].ze_fnm == fnm) {
      return *_aZipFiles[iFile].ze_pfnmArchive;
    }
  }

  static CTFileName fnmNoFile = CTString("");
  return fnmNoFile;
};

// Get info of a zip file entry
void GetFileInfo(INDEX iHandle, CTFileName &fnmZip, 
  SLONG &slOffset, SLONG &slSizeCompressed, SLONG &slSizeUncompressed, BOOL &bCompressed)
{
  if (!VerifyHandle(iHandle)) return;
  
  // Get parameters of the entry
  const CZipEntry &ze = _aZipHandles[iHandle].zh_zeEntry;

  fnmZip = *ze.ze_pfnmArchive;
  bCompressed = !ze.ze_bStored;
  slOffset = ze.ze_slDataOffset;
  slSizeCompressed = ze.ze_slCompressedSize;
  slSizeUncompressed = ze.ze_slUncompressedSize;
};

// Open a zip file entry for reading
INDEX Open_t(const CTFileName &fnm)
{
  CZipEntry *pze = NULL;

  for (INDEX iFile = 0; iFile < _aZipFiles.Count(); iFile++)
  {
    // Stop searching if it's that one
    if (_aZipFiles[iFile].ze_fnm == fnm) {
      pze = &_aZipFiles[iFile];
      break;
    }
  }

  // Not found
  if (pze == NULL) {
    ThrowF_t(LOCALIZE("File not found: %s"), fnm.str_String);
  }

  // Go through each existing handle
  BOOL bHandleFound = FALSE;
  INDEX iHandle = 1;

  for (; iHandle < _aZipHandles.Count(); iHandle++) {
    // Found unused one
    if (!_aZipHandles[iHandle].zh_bOpen) {
      bHandleFound = TRUE;
      break;
    }
  }

  // If no free handle found
  if (!bHandleFound) {
    // Create a new one
    iHandle = _aZipHandles.Count();
    _aZipHandles.Push(1);
  }
  
  // Get the handle
  CZipHandle &zh = _aZipHandles[iHandle];

  ASSERT(!zh.zh_bOpen);
  zh.zh_zeEntry = *pze;

  // Open zip archive for reading
  zh.zh_fFile = fopen(pze->ze_pfnmArchive->str_String, "rb");

  // If failed to open it
  if (zh.zh_fFile == NULL) {
    // Clear the handle
    zh.Clear();

    // Report error
    ThrowF_t(LOCALIZE("Cannot open '%s': %s"), pze->ze_pfnmArchive->str_String, strerror(errno));
  }

  // Seek to the local header of the entry
  fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);

  // Read the signature
  int slSig;
  fread(&slSig, sizeof(slSig), 1, zh.zh_fFile);

  // Unexpected signature
  if (slSig != SIGNATURE_LFH) {
    ThrowF_t(LOCALIZE("%s/%s: Wrong signature for 'local file header'"), 
      zh.zh_zeEntry.ze_pfnmArchive->str_String, zh.zh_zeEntry.ze_fnm.str_String);
  }

  // Read the header
  LocalFileHeader lfh;
  fread(&lfh, sizeof(lfh), 1, zh.zh_fFile);

  // Determine exact compressed data position
  zh.zh_zeEntry.ze_slDataOffset = ftell(zh.zh_fFile) + lfh.lfh_swFileNameLen + lfh.lfh_swExtraFieldLen;

  // Seek there
  fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);

  // Allocate buffers
  zh.zh_pubBufIn = (UBYTE *)AllocMemory(BUF_SIZE);

  // Initialize zlib stream
  CTSingleLock slZip(_pcsZipLock, TRUE);

  zh.zh_zstream.next_out  = NULL;
  zh.zh_zstream.avail_out = 0;
  zh.zh_zstream.next_in   = NULL;
  zh.zh_zstream.avail_in  = 0;
  zh.zh_zstream.zalloc = (alloc_func)Z_NULL;
  zh.zh_zstream.zfree = (free_func)Z_NULL;

  int iErr = inflateInit2(&zh.zh_zstream, -15);

  // If failed
  if (iErr != Z_OK) {
    // Clean up what is possible
    FreeMemory(zh.zh_pubBufIn);
    zh.zh_pubBufIn = NULL;

    fclose(zh.zh_fFile);
    zh.zh_fFile = NULL;

    zh.Throw_t(iErr, LOCALIZE("Cannot init inflation"));
  }

  // Return the handle successfully
  zh.zh_bOpen = TRUE;

  return iHandle;
};

// Get uncompressed size of a file
SLONG GetSize(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return 0;

  return _aZipHandles[iHandle].zh_zeEntry.ze_slUncompressedSize;
};

// Get CRC of a file
ULONG GetCRC(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return 0;

  return _aZipHandles[iHandle].zh_zeEntry.ze_ulCRC;
};

// Read a block from ZIP file
void ReadBlock_t(INDEX iHandle, UBYTE *pub, SLONG slStart, SLONG slLen)
{
  if (!VerifyHandle(iHandle)) return;

  CZipHandle &zh = _aZipHandles[iHandle];

  // Over the end of file
  if (slStart >= zh.zh_zeEntry.ze_slUncompressedSize) {
    return;
  }

  // Clamp length to end of the entry data
  slLen = Min(slLen, zh.zh_zeEntry.ze_slUncompressedSize - slStart);

  // If not compressed
  if (zh.zh_zeEntry.ze_bStored) {
    // Just read from file
    fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset+slStart, SEEK_SET);
    fread(pub, 1, slLen, zh.zh_fFile);
    return;
  }

  CTSingleLock slZip(_pcsZipLock, TRUE);

  // If behind the current pointer
  if (slStart < zh.zh_zstream.total_out) {
    // Reset the zlib stream to beginning
    inflateReset(&zh.zh_zstream);
    zh.zh_zstream.avail_in = 0;
    zh.zh_zstream.next_in = NULL;

    // Seek to start of zip entry data inside archive
    fseek(zh.zh_fFile, zh.zh_zeEntry.ze_slDataOffset, SEEK_SET);
  }

  // While ahead of the current pointer
  while (slStart > zh.zh_zstream.total_out)
  {
    // If zlib has no more input
    while (zh.zh_zstream.avail_in == 0) {
      // Read more to it
      SLONG slRead = fread(zh.zh_pubBufIn, 1, BUF_SIZE, zh.zh_fFile);

      if (slRead <= 0) {
        return;
      }

      // Tell zlib that there is more to read
      zh.zh_zstream.next_in = zh.zh_pubBufIn;
      zh.zh_zstream.avail_in = slRead;
    }

    // Read dummy data from the output
    #define DUMMY_SIZE 256
    UBYTE aubDummy[DUMMY_SIZE];

    // Decode to output
    zh.zh_zstream.avail_out = Min(SLONG(slStart-zh.zh_zstream.total_out), SLONG(DUMMY_SIZE));
    zh.zh_zstream.next_out = aubDummy;

    int iErr = inflate(&zh.zh_zstream, Z_SYNC_FLUSH);

    if (iErr != Z_OK && iErr != Z_STREAM_END) {
      zh.Throw_t(iErr, LOCALIZE("Error seeking in zip"));
    }
  }

  // If not streaming continuously
  if (slStart != zh.zh_zstream.total_out) {
    // This should not happen
    ASSERT(FALSE);

    // Read empty
    memset(pub, 0, slLen);

    return;
  }

  // Set zlib for writing to the block
  zh.zh_zstream.avail_out = slLen;
  zh.zh_zstream.next_out = pub;

  // While there is something to write to given block
  while (zh.zh_zstream.avail_out > 0)
  {
    // If zlib has no more input
    while (zh.zh_zstream.avail_in == 0) {
      // Read more to it
      SLONG slRead = fread(zh.zh_pubBufIn, 1, BUF_SIZE, zh.zh_fFile);

      if (slRead <= 0) {
        return;
      }

      // Tell zlib that there is more to read
      zh.zh_zstream.next_in = zh.zh_pubBufIn;
      zh.zh_zstream.avail_in = slRead;
    }

    // Decode to output
    int iErr = inflate(&zh.zh_zstream, Z_SYNC_FLUSH);

    if (iErr != Z_OK && iErr != Z_STREAM_END) {
      zh.Throw_t(iErr, LOCALIZE("Error reading from zip"));
    }
  }
};

// Close a ZIP file entry
void Close(INDEX iHandle)
{
  if (!VerifyHandle(iHandle)) return;

  // Clear it
  _aZipHandles[iHandle].Clear();
};

}; // namespace
