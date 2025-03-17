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

#ifndef CECIL_INCL_UNKNOWNPROPERTY_H
#define CECIL_INCL_UNKNOWNPROPERTY_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

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

#endif
