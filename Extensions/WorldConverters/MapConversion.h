/* Copyright (c) 2023-2026 Dreamy Cecil
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
    FWorldConverterPrepare m_pPrepare;
    FWorldConverterReplaceClass m_pReplaceClass;
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
      m_pPrepare        = NULL;
      m_pReplaceClass   = NULL;
      m_pHandleProperty = NULL;
      m_pConvertWorld   = NULL;
    };

    // Assignment operator
    IWorldConverter &operator=(const IWorldConverter &convOther) {
      m_pDestructor     = convOther.m_pDestructor;
      m_pPrepare        = convOther.m_pPrepare;
      m_pReplaceClass   = convOther.m_pReplaceClass;
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

    // List available converters
    static void ListConverters(void);

  public:
    // Signals for setting up a new converter
    static int PATCH_CALLTYPE CreateConverter(void *strName);
    static int PATCH_CALLTYPE SetMethodDestructor(void *pConverterData);
    static int PATCH_CALLTYPE SetMethodPrepare(void *pConverterData);
    static int PATCH_CALLTYPE SetMethodReplaceClass(void *pConverterData);
    static int PATCH_CALLTYPE SetMethodHandleProperty(void *pConverterData);
    static int PATCH_CALLTYPE SetMethodConvertWorld(void *pConverterData);

    // Get world converter for a specific level format
    static int PATCH_CALLTYPE GetConverterForFormat(void *pFormat);

    // Get world converter by its name
    static int PATCH_CALLTYPE GetConverterByName(void *strName);

    // Get level format from the converter name
    static int PATCH_CALLTYPE GetFormatFromConverter(void *strName);

    // Prepare a specific world converter before using it
    static int PATCH_CALLTYPE PrepareConverter(void *pConverterData);

    // Replace some class from some library upon loading it from an ECL file (e.g. Revolution class that doesn't exist in vanilla TSE)
    // Returns true if the class (or the library it's in) in the input has been replaced
    static int PATCH_CALLTYPE ReplaceClass(void *pEclData);

    // Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
    // It uses a method from the current converter that's set by calling ConvertWorld()
    static int PATCH_CALLTYPE HandleUnknownProperty(void *pPropData);

    // Convert the world using a specific converter
    static int PATCH_CALLTYPE ConvertWorld(void *pWorld);
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

#endif
