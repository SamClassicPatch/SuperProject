/* Copyright (c) 2023-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_GAMETHEMES_H
#define CECIL_INCL_GAMETHEMES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Game theme settings
class CGameTheme {
  public:
    static CTString strTheme; // Path to the theme config to set

    CTString strConfig; // Config filename

    // Elements
    BOOL bColumn;
    BOOL bSam;

    CTextureObject toBackdrop;
    CTextureObject toClouds;
    CTextureObject toGrid;
    CTextureObject toPointer;

    BOOL bTFEClouds1;
    BOOL bTFEClouds2;
    BOOL bTFEGrid;
    BOOL bConClouds;
    BOOL bCompRawColor;

    // Colors
    COLOR colOrangeLight;
    COLOR colOrangeNeutral;
    COLOR colOrangeDark;
    COLOR colOrangeDarkLT;
    COLOR colBlueDarkHV;
    COLOR colBlueDark;
    COLOR colBlueDarkLT;
    COLOR colBlueNeutral;
    COLOR colBlueLight;
    COLOR colBlueGreenLT;

    // New colors
    COLOR colLight;
    COLOR colNeutral;
    COLOR colBorder;
    COLOR colConBorder;
    COLOR colConText;
    COLOR colCompText;
    COLOR colCompClouds;
    COLOR colCompFloor;

  public:
    // Default constructor
    CGameTheme() {
      Reset();
    };

    // Reset theme to TSE
    void Reset(void) {
      bColumn = TRUE;
      bSam = TRUE;
      bTFEClouds1 = FALSE;
      bTFEClouds2 = FALSE;
      bTFEGrid = FALSE;
      bConClouds = TRUE;
      bCompRawColor = FALSE;

      colOrangeLight   = 0xFFD70000;
      colOrangeNeutral = 0xEE9C0000;
      colOrangeDark    = 0x9B4B0000;
      colOrangeDarkLT  = 0xBC6A0000;
      colBlueDarkHV    = 0x151C2300;
      colBlueDark      = 0x2A384600;
      colBlueDarkLT    = 0x43596F00;
      colBlueNeutral   = 0x5C7A9900;
      colBlueLight     = 0x64B4FF00;
      colBlueGreenLT   = 0x6CFF6C00;

      colLight      = 0xFFD70000;
      colNeutral    = 0xEE9C0000;
      colBorder     = 0x5C7A9900;
      colConBorder  = 0x5C7A9900;
      colConText    = 0x64B4FF00;
      colCompText   = 0x64B4FF00;
      colCompClouds = 0x5C7A9900;
      colCompFloor  = 0x5A8EC200;
    };

    // Reload textures ("*" for default TSE)
    void ReloadTextures(CTString strBackdrop, CTString strClouds, CTString strGrid, CTString strPointer);

    // Load game theme from a config file
    void Load(const CTString &strFile, BOOL bNewTheme);

    // Load new game themes when needed
    void Update(void);
};

// Main theme structure
extern CGameTheme _gmtTheme;

#endif
