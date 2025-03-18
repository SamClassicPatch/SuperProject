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

#ifndef CECIL_INCL_MAPCONVERSION_H
#define CECIL_INCL_MAPCONVERSION_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "UnknownProperty.h"
#include <Extras/XGizmo/Objects/MapStructure.h>

// Common data for converters
#include <Extras/XGizmo/Vanilla/EnumTypes.h>

// Dummy placement
const CPlacement3D _plWorldCenter(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

// Abstract base for different format converters
class IWorldConverter {
  public:
    int m_iID; // Unique identifier instead of a name (-1 for invalid)

    FWorldConverterDestructor m_pDestructor;
    FWorldConverterReset m_pReset;
    FWorldConverterHandleProperty m_pHandleProperty;
    FWorldConverterConvert m_pConvertWorld;

  public:
    // Constructor with nullified methods
    IWorldConverter() {
      Clear();
    };

    // Copy constructor
    IWorldConverter(const IWorldConverter &convOther) {
      operator=(convOther);
    };

    // Optional destructor
    __forceinline ~IWorldConverter() {
      if (m_pDestructor != NULL) m_pDestructor();
    };

    // Clear the converter
    inline void Clear(void) {
      m_iID = -1;

      m_pDestructor     = NULL;
      m_pReset          = NULL;
      m_pHandleProperty = NULL;
      m_pConvertWorld   = NULL;
    };

    // Assignment operator
    IWorldConverter &operator=(const IWorldConverter &convOther) {
      m_pDestructor     = convOther.m_pDestructor;
      m_pReset          = convOther.m_pReset;
      m_pHandleProperty = convOther.m_pHandleProperty;
      m_pConvertWorld   = convOther.m_pConvertWorld;
      return *this;
    };

  public:
    // Add new world converter with a specific name
    // Returns NULL if new converter could not be created (name already exists or it's empty)
    static IWorldConverter *Add(const CTString &strName);

    // Remove a world converter with a specific name
    // Returns false if nothing was removed
    static bool Remove(const CTString &strName);

    // Try to find a converter by its name
    static IWorldConverter *Find(const CTString &strName);

    // Try to find a converter by its identifier
    static IWorldConverter *Find(int iID);

  public:
    // Signals for setting up a new converter
    static int CreateConverter(void *strName);
    static int SetMethodDestructor(void *pConverterData);
    static int SetMethodReset(void *pConverterData);
    static int SetMethodHandleProperty(void *pConverterData);
    static int SetMethodConvertWorld(void *pConverterData);

    // Get world converter for a specific level format
    static int GetConverterForFormat(void *pFormat);

    // Get world converter by its name
    static int GetConverterByName(void *strName);

    // Reset a specific world converter before using it
    static int ResetConverter(void *pConverterData);

    // Convert the world using a specific converter
    static int ConvertWorld(void *pConverterData);

    // Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
    // It uses a method from the current converter that's set by calling ConvertWorld()
    static int HandleUnknownProperty(void *pPropData);
};

// Common methods related to world conversion

// Check if the entity state doesn't match
BOOL CheckEntityState(CRationalEntity *pen, SLONG slState, INDEX iClassID);

// Create a global light entity to fix shadow issues with brush polygon layers
void CreateGlobalLight(void);

// Get weapon flag from type
__forceinline INDEX WeaponFlag(INDEX iWeapon) {
  return (1 << (iWeapon - 1));
};

// Replace nonexistent vanilla classes upon loading them from ECL classes
int ReplaceMissingClasses(void *pEclData);

// Replace nonexistent Revolution classes before loading them from ECL files
int ReplaceRevolutionClasses(void *pfnmCopy);

#endif
