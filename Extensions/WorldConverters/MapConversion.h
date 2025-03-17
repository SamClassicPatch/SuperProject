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

// Common data for converters
#include <Extras/XGizmo/Vanilla/EnumTypes.h>

// Dummy placement
const CPlacement3D _plWorldCenter(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

// Abstract base for different format converters
class IWorldFormatConverter {
  // Conversion method prototypes
  public:

    typedef void (*FDestructor)(void);
    typedef void (*FReset)(void);
    typedef void (*FHandleProperty)(CEntity *pen, const UnknownProp &prop);
    typedef void (*FConvertWorld)(CWorld *pwo);

  // Pointers to specific conversion methods
  public:

    // Class destructor
    FDestructor m_pDestructor;

    // Reset the converter before loading a new world
    FReset m_pReset;

    // Handle some unknown property
    FHandleProperty m_pHandleProperty;

    // Convert the entire world with possible entity reinitializations
    FConvertWorld m_pConvertWorld;

  public:

    // Constructor with nullified methods
    IWorldFormatConverter() {
      m_pDestructor = NULL;
      m_pReset = NULL;
      m_pHandleProperty = NULL;
      m_pConvertWorld = NULL;
    };

    // Optional destructor
    __forceinline ~IWorldFormatConverter() {
      if (m_pDestructor != NULL) m_pDestructor();
    };
};

// Specific converters
#include "Converters/TFEMaps.h"
#include "Converters/RevMaps.h"

// Common methods related to world conversion

// Set current map converter for a specific format
int SetConverterForFormat(void *pFormat);

// Reset a specific map converter before using it
int ResetConverter(void *);

// Convert the world using the current converter
int ConvertWorld(void *pWorld);

// Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
int HandleUnknownProperty(void *pPropData);

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
