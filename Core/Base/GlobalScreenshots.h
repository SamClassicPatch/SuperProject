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

#ifndef CECIL_INCL_GLOBALSCREENSHOTS_H
#define CECIL_INCL_GLOBALSCREENSHOTS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Rebindable key ID for taking global screenshots
CORE_API extern INDEX sam_kidScreenshot;

// Interface with functions for working with global game screenshots
namespace IScreenshots {

// Initialize the interface
void Initialize(void);

// Set drawport that will be used for making global screenshots from within the game
CORE_API void SetHook(CDrawPort *pdpScreenshotSurface);

// Capture screenshot from the current drawport
CORE_API BOOL Capture(CImageInfo &iiScreenshot);

// Save local screenshot to disk
CORE_API void SaveLocal(CImageInfo &iiScreenshot);

// Try to take a new screenshot and request to save it
CORE_API BOOL Request(CImageInfo &iiScreenshot);

}; // namespace

#endif
