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

#ifndef CECIL_INCL_ENTITIES_BLOODTHEMES_H
#define CECIL_INCL_ENTITIES_BLOODTHEMES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Moved from Bullet.es (TFE) and Common.h (TSE)
#define BLOOD_SPILL_RED RGBAToColor(250, 20, 20, 255)
#define BLOOD_SPILL_GREEN RGBAToColor(0, 250, 0, 255)

// Custom blood theme
struct BloodTheme {
  enum Type {
    E_AUTO      = 0, // Based on the seasonal event
    E_COLOR     = 1, // Simple coloring
    E_FORCED    = 2, // Color override

    // Custom models and different colors
    E_HIPPIE    = 3,
    E_PARTY     = 4,
    E_CHRISTMAS = 5,
  };

  Type eType;
  COLOR colForced;

  // Constructor
  BloodTheme(Type eSetType, COLOR colSet = 0xFFFFFFFF) :
    eType(eSetType), colForced(colSet)
  {
    // This shouldn't be a real type
    ASSERT(eSetType != E_AUTO);
  };

  // Get dynamic blood color
  COLOR GetColor(INDEX iRandom = 0, UBYTE ubFactor = 0xFF, UBYTE ubAlpha = 0xFF) const;
};

// Get current blood theme
BloodTheme GetBloodTheme(void);

#endif
