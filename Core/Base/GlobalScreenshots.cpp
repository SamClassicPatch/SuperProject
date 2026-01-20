/* Copyright (c) 2025-2026 Dreamy Cecil
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

#include "GlobalScreenshots.h"

#include <Extras/zlib/zlib.h>

// Possible formats for saving screenshots
enum EScreenshotFormat {
  E_SHOT_PNG,
  E_SHOT_JPG,
  E_SHOT_TGA,
};

INDEX sam_kidScreenshot = KID_F12; // Rebindable key ID for taking global screenshots
static INDEX sam_iScreenshotFormat = E_SHOT_PNG; // File format to save the screenshot in

static CTCriticalSection _csScreenshots;

// Screenshots to save
struct SBufferedScreenshot {
  BOOL bWrite;
  INDEX iFormat;
  CTString fnm;
  CImageInfo ii;

  SBufferedScreenshot() : bWrite(FALSE), iFormat(E_SHOT_PNG) {};

  inline void Clear(void) {
    ii.Clear();
    fnm = "";
    bWrite = FALSE;
  }
};

#define BUFFERED_SHOTS_COUNT 10
static SBufferedScreenshot _aBufferedScreenshots[BUFFERED_SHOTS_COUNT];

// The zlib version that the engine and the patch use does not have compressBound()
static inline size_t CustomCompressBound(size_t iLen) {
  return iLen + (iLen >> 12) + (iLen >> 14) + 11;
};

static UBYTE *CustomZlibCompress(const UBYTE *pData, int iDataLen, int *piOutLen, int iQuality) {
  // Estimate upper bound for compressed data
  uLongf iBufLen = CustomCompressBound(iDataLen);
  UBYTE *pOutBuf = (UBYTE *)AllocMemory(iBufLen);

  if (pOutBuf == NULL) {
    *piOutLen = 0;
    return NULL;
  }

  if (compress2(pOutBuf, &iBufLen, (const Bytef *)pData, (uLong)iDataLen, iQuality) != Z_OK) {
    FreeMemory(pOutBuf);
    *piOutLen = 0;
    return NULL;
  }

  *piOutLen = (int)iBufLen;
  return pOutBuf;
};

static void *CustomRealloc(void *pData, size_t iSize) {
  ResizeMemory(&pData, (SLONG)iSize);
  return pData;
};

// Configure stb_image_write before implementing it
#define STBIW_ASSERT(x)         ASSERT(x)
#define STBIW_MALLOC(sz)        AllocMemory(sz)
#define STBIW_REALLOC(p, newsz) CustomRealloc(p, newsz)
#define STBIW_FREE(p)           FreeMemory(p)
#define STBIW_ZLIB_COMPRESS     CustomZlibCompress

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Extras/stb/stb_image_write.h>

namespace IScreenshots {

// Drawport for making global screenshots from within the game
static CDrawPort *_pdpScreenshot = NULL;

// Initialize the interface
void Initialize(void) {
  _csScreenshots.cs_iIndex = -1;

  _pShell->DeclareSymbol("persistent INDEX sam_kidScreenshot;", &sam_kidScreenshot);
  _pShell->DeclareSymbol("persistent user INDEX sam_iScreenshotFormat;", &sam_iScreenshotFormat);
};

// Set drawport that will be used for making global screenshots from within the game
void SetHook(CDrawPort *pdpScreenshotSurface) {
  _pdpScreenshot = pdpScreenshotSurface;

  // Hook Steam screenshots
  GetSteamAPI()->SetScreenshotHook(pdpScreenshotSurface != NULL);
};

// Capture screenshot from the current drawport
BOOL Capture(CImageInfo &iiScreenshot) {
  if (_pdpScreenshot == NULL) return FALSE;

  // Take a new screenshot
  _pdpScreenshot->GrabScreen(iiScreenshot, 0);
  return TRUE;
};

// Create name for a new screenshot
static CTString MakeScreenshotName(INDEX iFormat) {
  CTString strBase = "ScreenShots\\";

  // Create base name from the current world
  if (GetGameAPI()->IsHooked() && !GetGameAPI()->IsMenuOn() && GetGameAPI()->IsGameOn()) {
    strBase += _pNetwork->GetCurrentWorld().FileName();

  // Otherwise use a generic name for non-world screenshots
  } else {
    strBase += "SeriousSam";
  }

  // Determine extension from the format
  const char *strExt = "";

  switch (iFormat) {
    case E_SHOT_PNG: strExt = ".png"; break;
    case E_SHOT_JPG: strExt = ".jpg"; break;
    case E_SHOT_TGA: strExt = ".tga"; break;
  }

  INDEX iShot = 0;

  FOREVER {
    // Create full filename with the number
    CTString fnmFull = CTString(0, "%s_shot%04d%s", strBase, iShot, strExt);

    // Make sure this filename isn't buffered yet
    BOOL bFree = TRUE;

    for (INDEX iCheckBuffer = 0; iCheckBuffer < BUFFERED_SHOTS_COUNT; iCheckBuffer++) {
      if (_aBufferedScreenshots[iCheckBuffer].fnm == fnmFull) {
        bFree = FALSE;
        break;
      }
    }

    // File doesn't exist yet, so it can be used
    if (bFree && !FileExistsForWriting(fnmFull)) {
      return fnmFull;
    }

    // Try the next number
    iShot++;
  }
};

// Write screenshot to disk in the specified format
static void WriteScreenshot_t(CImageInfo &ii, const CTString &fnmScreenshot, INDEX iFormat) {
  CTString fnmAbsolute = IDir::AppPath() + fnmScreenshot;
  int iBPP;

  if (ii.ii_BitsPerPixel == 32) {
    iBPP = 4; // RGBA
  } else if (ii.ii_BitsPerPixel == 24) {
    iBPP = 3; // RGB
  } else {
    ThrowF_t(LOCALIZE("Unsupported BitsPerPixel in ImageInfo header."));
  }

  switch (iFormat) {
    case E_SHOT_PNG:
      stbi_write_png(fnmAbsolute.str_String, ii.ii_Width, ii.ii_Height, iBPP, ii.ii_Picture, ii.ii_Width * iBPP);
      break;

    case E_SHOT_JPG:
      stbi_write_jpg(fnmAbsolute.str_String, ii.ii_Width, ii.ii_Height, iBPP, ii.ii_Picture, 100);
      break;

    case E_SHOT_TGA:
      stbi_write_tga(fnmAbsolute.str_String, ii.ii_Width, ii.ii_Height, iBPP, ii.ii_Picture);
      break;

    // Use default engine method
    default: ii.SaveTGA_t(fnmScreenshot);
  }
};

// Process one buffered screenshot
static DWORD __stdcall ProcessBufferedScreenshot(LPVOID pData) {
  SBufferedScreenshot *pbs = (SBufferedScreenshot *)pData;

  // If the current screenshot can be written
  if (pbs->bWrite && pbs->ii.ii_Picture != NULL) {
    // Save it to disk and then free it
    try {
      WriteScreenshot_t(pbs->ii, pbs->fnm, pbs->iFormat);
      CPrintF(LOCALIZE("screen shot: %s\n"), pbs->fnm.str_String);

    } catch (char *strError) {
      CPrintF(LOCALIZE("Cannot save screenshot:\n%s\n"), strError);
    }

    pbs->Clear();
  }

  return 0;
};

// Try to take a new screenshot and request to save it
BOOL Request(void) {
  CTSingleLock slShot(&_csScreenshots, TRUE);

  // Find a free buffer
  SBufferedScreenshot *pbs = NULL;

  for (INDEX i = 0; i < BUFFERED_SHOTS_COUNT; i++) {
    if (!_aBufferedScreenshots[i].bWrite) {
      pbs = &_aBufferedScreenshots[i];
      break;
    }
  }

  if (pbs == NULL) return FALSE;

  // Clear the slot just in case
  pbs->Clear();

  BOOL bResult;
  const BOOL bInMenu = (GetGameAPI()->IsHooked() ? GetGameAPI()->IsMenuOn() : FALSE);

  // Take a screenshot using the observer camera (if not in the menu)
  if (GetGameAPI()->GetCamera().IsActive() && !bInMenu) {
    bResult = GetGameAPI()->GetCamera().TakeScreenshot(pbs->ii);

  // Take a regular screenshot
  } else {
    bResult = Capture(pbs->ii);
  }

  // If the screenshot has been captured
  if (bResult) {
    CPutString(TRANS("Saving a new screenshot...\n"));

    // Ask Steam to save it on its own
    GetSteamAPI()->WriteScreenshot(pbs->ii);

    // Then signal to write it to disk under a specific format
    pbs->iFormat = sam_iScreenshotFormat;
    pbs->fnm = MakeScreenshotName(sam_iScreenshotFormat);
    pbs->bWrite = TRUE;

    // Create a separate thread for writing it to disk
    DWORD dwThreadID;
    HANDLE hThread = CreateThread(NULL, 0, &ProcessBufferedScreenshot, (LPVOID)pbs, 0, &dwThreadID);
    if (hThread != NULL) CloseHandle(hThread);
  }

  return bResult;
};

}; // namespace
