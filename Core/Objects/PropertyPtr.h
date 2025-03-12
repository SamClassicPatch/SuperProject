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

#ifndef CECIL_INCL_PROPERTYPTR_H
#define CECIL_INCL_PROPERTYPTR_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Rename XGizmo class in order to reuse it
#define CPropertyPtr CPropertyPtr_XGizmoBase
#include <Extras/XGizmo/Objects/PropertyPtr.h>
#undef CPropertyPtr

// Find entity property data by a variable name of a specific class
CORE_API const CEntityProperty *FindPropertyByVariable(const CTString &strClass, const CTString &strVariable);

class CPropertyPtr : public CPropertyPtr_XGizmoBase {
  public:
    // Default constructor
    __forceinline CPropertyPtr() : CPropertyPtr_XGizmoBase()
    {
    };

    // Constructor from a holder
    __forceinline CPropertyPtr(LibClassHolder lchSet) : CPropertyPtr_XGizmoBase(lchSet)
    {
    };

  public:
    // Get property by variable
    BOOL ByVariable(const char *strClass, const char *strVariable) {
      if (_pep == NULL)
      {
      #if SE1_GAME != SS_REV
        // Find property data and try to find it by name or ID
        const CEntityProperty *pep = FindPropertyByVariable(strClass, strVariable);
        _pep = IWorld::PropertyForNameOrId(_lch, pep->ep_eptType, pep->ep_strName, pep->ep_ulID);
      #else
        _pep = _lch->PropertyForVariable(strVariable);
      #endif

        ASSERTMSG(_pep != NULL, "Cannot find property by variable for CPropertyPtr!");
      }

      return (_pep != NULL);
    };
};

#endif
