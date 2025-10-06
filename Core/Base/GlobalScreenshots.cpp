/* Copyright (c) 2025 Dreamy Cecil
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

// Rebindable key ID for taking global screenshots
INDEX sam_kidScreenshot = KID_F12;

namespace IScreenshots {

// Drawport for making global screenshots from within the game
static CDrawPort *_pdpScreenshot = NULL;

// Initialize the interface
void Initialize(void) {
  _pShell->DeclareSymbol("persistent INDEX sam_kidScreenshot;", &sam_kidScreenshot);
};

// Set drawport that will be used for making global screenshots from within the game
void SetHook(CDrawPort *pdpScreenshotSurface) {
  _pdpScreenshot = pdpScreenshotSurface;

  // Hook Steam screenshots
  GetSteamAPI()->SetScreenshotHook(pdpScreenshotSurface);
};

// Capture screenshot from the current drawport
BOOL Capture(CImageInfo &iiScreenshot) {
  if (_pdpScreenshot == NULL) return FALSE;

  // Take a new screenshot
  _pdpScreenshot->GrabScreen(iiScreenshot, 0);
  return TRUE;
};

// Create name for a new screenshot
static CTFileName MakeScreenShotName(void) {
  CTString strBase = "ScreenShots\\";

  // Create base name from the current world
  if (GetGameAPI()->IsHooked() && !GetGameAPI()->IsMenuOn() && GetGameAPI()->IsGameOn()) {
    strBase += _pNetwork->GetCurrentWorld().FileName();

  // Otherwise use a generic name for non-world screenshots
  } else {
    strBase += "SeriousSam";
  }

  INDEX iShot = 0;

  FOREVER {
    // Create full filename with the number
    CTFileName fnmFull = CTString(0, "%s_shot%04d.tga", strBase, iShot);

    // File doesn't exist yet, so it can be used
    if (!FileExistsForWriting(fnmFull)) {
      return fnmFull;
    }

    // Try the next number
    iShot++;
  }
};

// Save local screenshot to disk
void SaveLocal(CImageInfo &iiScreenshot) {
  try {
    CTFileName fnmScreenshot = MakeScreenShotName();
    iiScreenshot.SaveTGA_t(fnmScreenshot);
    CPrintF(LOCALIZE("screen shot: %s\n"), fnmScreenshot.str_String);

  } catch (char *strError) {
    CPrintF(LOCALIZE("Cannot save screenshot:\n%s\n"), strError);
  }
};

// Try to take a new screenshot and request to save it
BOOL Request(CImageInfo &iiScreenshot) {
  // Take a screenshot using the observer camera (if not in the menu)
  const BOOL bInMenu = (GetGameAPI()->IsHooked() ? GetGameAPI()->IsMenuOn() : FALSE);

  if (GetGameAPI()->GetCamera().IsActive() && !bInMenu) {
    return GetGameAPI()->GetCamera().TakeScreenshot(iiScreenshot);
  }

  // Take a regular screenshot
  return Capture(iiScreenshot);
};

}; // namespace
