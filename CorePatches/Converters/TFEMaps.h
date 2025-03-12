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

#ifndef CECIL_INCL_CONVERTERS_TFEMAPS_H
#define CECIL_INCL_CONVERTERS_TFEMAPS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <EnginePatches/MapConversion.h>

#if _PATCHCONFIG_CONVERT_MAPS && TSE_FUSION_MODE

// Interface for converting worlds from The First Encounter
class IConvertTFE : public IMapConverter {
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
    // List of rain properties for each controller in the world
    CStaticStackArray<SRainProps> aRainProps;

    // List of triggers and storm controllers in the world
    CEntities cenTriggers;
    CEntities cenStorms;

    // First and last created environment particles holders
    CEntity *penFirstEPH;
    CEntity *penLastEPH;

  public:

    // Constructor
    IConvertTFE() : penFirstEPH(NULL), penLastEPH(NULL)
    {
    };

    // Clear rain variables
    void ClearRainVariables(void);

    // Remember rain properties of CWorldSettingsController
    void RememberWSC(CEntity *penWSC, const UnknownProp &prop);

    // Apply remembered rain properties from controllers
    void ApplyRainProperties(void);

  // Converter methods
  public:

    // Reset the converter before loading a new world
    virtual void Reset(void);

    // Handle some unknown property
    virtual void HandleProperty(CEntity *pen, const UnknownProp &prop);

    // Convert invalid weapon flag in a mask
    virtual void ConvertWeapon(INDEX &iFlags, INDEX iWeapon);

    // Convert invalid key types
    virtual void ConvertKeyType(INDEX &eKey);

    // Convert one specific entity without reinitializing it
    virtual BOOL ConvertEntity(CEntity *pen);

    // Convert the entire world with possible entity reinitializations
    virtual void ConvertWorld(CWorld *pwo);
};

// Converter instance
extern IConvertTFE _convTFE;

#endif // _PATCHCONFIG_CONVERT_MAPS && TSE_FUSION_MODE

#endif
