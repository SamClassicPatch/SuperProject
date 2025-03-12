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

#if _PATCHCONFIG_CONVERT_MAPS

// Common data for converters
#include <Extras/XGizmo/Vanilla/EnumTypes.h>

// Abstract base for different map converters
class IMapConverter {
  public:
    // Property data for handling unknown entity properties
    struct UnknownProp {
      ULONG ulType; // Property type, i.e. CEntityProperty::PropertyType
      ULONG ulID;   // Property ID, i.e. what should've been in CEntityProperty::ep_ulID
      void *pValue; // Pointer to any value type

      // Default constructor
      __forceinline UnknownProp(ULONG ulSetType, ULONG ulSetID, void *pSetValue)
        : ulType(ulSetType), ulID(ulSetID), pValue(pSetValue)
      {
      };

      // Define methods for converting any pointer to a typed reference
      #define DEFINE_VALUE_REF(_Type, _Method) \
        __forceinline _Type &_Method(void) const { return *static_cast<_Type *>(pValue); };

      DEFINE_VALUE_REF(BOOL, Bool);
      DEFINE_VALUE_REF(COLOR, Color);
      DEFINE_VALUE_REF(CEntityPointer, Entity);
      DEFINE_VALUE_REF(ULONG, Flags); // flags
      DEFINE_VALUE_REF(INDEX, Index); // enum, INDEX, ANIMATION, ILLUMINATIONTYPE
      DEFINE_VALUE_REF(FLOAT, Float); // FLOAT, ANGLE, RANGE
      DEFINE_VALUE_REF(CTString, String); // CTString, CTStringTrans, CTFileNameNoDep

      // Resources
      DEFINE_VALUE_REF(CTFileName, Filename);
      DEFINE_VALUE_REF(CModelObject, ModelObject);
    #if SE1_VER >= SE1_107
      DEFINE_VALUE_REF(CModelInstance, ModelInstance);
    #endif
      DEFINE_VALUE_REF(CAnimObject, AnimObject);
      DEFINE_VALUE_REF(CSoundObject, SoundObject);

      // 3D environment
      DEFINE_VALUE_REF(FLOAT3D, Float3D);
      DEFINE_VALUE_REF(ANGLE3D, Angle3D);
      DEFINE_VALUE_REF(CPlacement3D, Placement);
      DEFINE_VALUE_REF(FLOATaabbox3D, Box);
      DEFINE_VALUE_REF(FLOATplane3D, Plane);
      DEFINE_VALUE_REF(FLOATquat3D, Quat);
      DEFINE_VALUE_REF(FLOATmatrix3D, Matrix);

      #undef DEFINE_VALUE_REF
    };

  // Common methods
  public:

    // Set current map converter for a specific format
    static IMapConverter *SetConverter(ELevelFormat eFormat);

    // Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
    static void HandleUnknownProperty(CEntity *pen, ULONG ulType, ULONG ulID, void *pValue);

    // Check if the entity state doesn't match
    static BOOL CheckEntityState(CRationalEntity *pen, SLONG slState, INDEX iClassID);

    // Get weapon flag from type
    static __forceinline INDEX WeaponFlag(INDEX iWeapon) {
      return (1 << (iWeapon - 1));
    };

    // Create a global light entity to fix shadow issues with brush polygon layers
    static void CreateGlobalLight(void);

  // Converter methods
  public:

    // Destructor
    virtual ~IMapConverter() {};

    // Reset the converter before loading a new world
    virtual void Reset(void) = 0;

    // Handle some unknown property
    virtual void HandleProperty(CEntity *pen, const UnknownProp &prop) = 0;

    // Convert invalid weapon flag in a mask
    virtual void ConvertWeapon(INDEX &iFlags, INDEX iWeapon) = 0;

    // Convert invalid key types
    virtual void ConvertKeyType(INDEX &eKey) = 0;

    // Convert one specific entity without reinitializing it
    virtual BOOL ConvertEntity(CEntity *pen) = 0;

    // Convert the entire world with possible entity reinitializations
    virtual void ConvertWorld(CWorld *pwo) = 0;
};

// Specific converters
#include <CorePatches/Converters/TFEMaps.h>
#include <CorePatches/Converters/RevMaps.h>

#endif // _PATCHCONFIG_CONVERT_MAPS

// Pair of class names for a replacement table
struct ClassReplacementPair {
  const char *strOld;
  const char *strNew;
};

// Load some class from patch's ExtraEntities library instead of vanilla entities, if required
BOOL LoadClassFromExtras(CTString &strClassName, CTFileName &fnmDLL, ClassReplacementPair *aTable);

// Load another class in place of the current one, if it's found in the replacement table
BOOL ReplaceClassFromTable(CTString &strClassName, ClassReplacementPair *aTable);

#endif
