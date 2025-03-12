/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_ENTITIES_BLOODTHEMES_INL
#define CECIL_INCL_ENTITIES_BLOODTHEMES_INL

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "BloodThemes.h"

// Custom blood themes
static INDEX gam_iCustomBloodColor = 0xFF0000FF;
static INDEX gam_iCustomBloodTheme = BloodTheme::E_AUTO;

static void DeclareCustomBloodSymbols(void) {
  _pShell->DeclareSymbol("persistent user INDEX gam_iCustomBloodColor;", &gam_iCustomBloodColor);
  _pShell->DeclareSymbol("persistent user INDEX gam_iCustomBloodTheme;", &gam_iCustomBloodTheme);
};

// Get dynamic blood color
COLOR BloodTheme::GetColor(INDEX iRandom, UBYTE ubFactor, UBYTE ubAlpha) const {
  // Default color
  const COLOR colDefault = RGBAToColor(ubFactor, ubFactor, ubFactor, ubAlpha);

  switch (eType) {
    // Override custom color
    case E_FORCED: return MulColors(colForced, colDefault);

    // No blending for colorful textures
    case E_HIPPIE: return colDefault;

    // Birthday colors
    case E_PARTY: {
      const FLOAT fNorm = NormByteToFloat(ubFactor);

      switch (iRandom % 6) {
        case 0: return RGBAToColor(0xEF * fNorm, 0x7E * fNorm, 0x7A * fNorm, ubAlpha); // 0xEF7E7A
        case 1: return RGBAToColor(0xF2 * fNorm, 0xA8 * fNorm, 0x6B * fNorm, ubAlpha); // 0xF2A86B
        case 2: return RGBAToColor(0xD6 * fNorm, 0xE3 * fNorm, 0x8A * fNorm, ubAlpha); // 0xD6E38A
        case 3: return RGBAToColor(0x6C * fNorm, 0xC4 * fNorm, 0xC1 * fNorm, ubAlpha); // 0x6CC4C1
        case 4: return RGBAToColor(0x7E * fNorm, 0x8E * fNorm, 0xD9 * fNorm, ubAlpha); // 0x7E8ED9
        case 5: return RGBAToColor(0xC7 * fNorm, 0xA6 * fNorm, 0xE3 * fNorm, ubAlpha); // 0xC7A6E3
      }
    } return colDefault;

    // Festive colors
    case E_CHRISTMAS: {
      switch (iRandom % 3) {
        case 0: return RGBAToColor(ubFactor, ubFactor * 0.15f, ubFactor * 0.15f, ubAlpha); // Red (255, 38, 38)
        case 1: return RGBAToColor(ubFactor * 0.15f, ubFactor, ubFactor * 0.15f, ubAlpha); // Green (38, 255, 38)
        case 2: return colDefault; // White
      }
    } return colDefault;
  }

  // Simply colored
  return MulColors(gam_iCustomBloodColor, colDefault);
};

// Get current blood theme
BloodTheme GetBloodTheme(void) {
  BloodTheme::Type eType = (BloodTheme::Type)gam_iCustomBloodTheme;

  // Based on the current event
  if (eType == BloodTheme::E_AUTO) {
    switch (ClassicsCore_GetSeason()) {
      case k_EClassicsPatchSeason_Valentine: return BloodTheme(BloodTheme::E_FORCED, 0xFF00AFFF);
      case k_EClassicsPatchSeason_Anniversary:
      case k_EClassicsPatchSeason_Birthday:  return BloodTheme(BloodTheme::E_PARTY);
      case k_EClassicsPatchSeason_Halloween: return BloodTheme(BloodTheme::E_FORCED, 0xFF7F00FF);
      case k_EClassicsPatchSeason_Christmas: return BloodTheme(BloodTheme::E_CHRISTMAS);
    }

    // Reset to simple coloring
    eType = BloodTheme::E_COLOR;
  }

  // Pick a specific type and blend it
  return BloodTheme(eType, gam_iCustomBloodColor);
};

#endif
