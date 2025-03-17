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

#ifndef CECIL_INCL_CONVERTERS_TFEMAPS_H
#define CECIL_INCL_CONVERTERS_TFEMAPS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface for converting worlds from The First Encounter
class IConvertTFE : public IWorldFormatConverter {
  public:
    // Structure with rain properties for a specific CWorldSettingsController
    struct SRainProps {
      CEntity *penWSC; // Pointer to CWorldSettingsController

      CTFileName fnm; // CWorldSettingsController::m_fnHeightMap
      FLOATaabbox3D box; // CWorldSettingsController::m_boxHeightMap

      // Constructor
      SRainProps() : penWSC(NULL)
      {
      };
    };

  public:

    // [Cecil] TODO: Rework this class into a simple namespace and register this converter on plugin startup
    IConvertTFE() : IWorldFormatConverter()
    {
      m_pReset          = &Reset;
      m_pHandleProperty = &HandleProperty;
      m_pConvertWorld   = &ConvertWorld;
    };

    // Clear rain variables
    static void ClearRainVariables(void);

    // Remember rain properties of CWorldSettingsController
    static void RememberWSC(CEntity *penWSC, const UnknownProp &prop);

    // Apply remembered rain properties from controllers
    static void ApplyRainProperties(void);

  // Converter methods
  public:

    // Reset the converter before loading a new world
    static void Reset(void);

    // Handle some unknown property
    static void HandleProperty(CEntity *pen, const UnknownProp &prop);

    // Convert invalid weapon flag in a mask
    static void ConvertWeapon(INDEX &iFlags, INDEX iWeapon);

    // Convert invalid key types
    static void ConvertKeyType(INDEX &eKey);

    // Convert one specific entity without reinitializing it
    static BOOL ConvertEntity(CEntity *pen);

    // Convert the entire world with possible entity reinitializations
    static void ConvertWorld(CWorld *pwo);
};

// Converter instance
extern IConvertTFE _convTFE;

#endif
