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

#include "StdH.h"

#include "PropertyPtr.h"

// [Cecil] Rev: No property tables needed for Revolution
#if SE1_GAME != SS_REV
  // Define table of entity property references
  #include <EngineEx/PropertyTables.h>

  static CPropertyRefTable _mapPropRefs;

  // Define static references that get added into the reference table immediately
  #define ENTITYPROPERTYREF_DECL static
  #define ENTITYPROPERTYREF_ENTRY(Class, Refs, RefsCount) \
    struct Class##_PropRefsEntryInit { \
      int iDummy; \
      Class##_PropRefsEntryInit() { \
        _mapPropRefs.FillPropertyReferences(#Class, Refs, RefsCount); \
      }; \
    } Class##_proprefsentry;

  #include <EccExtras/EntityProperties.h>
  #include <EntitiesV/_DefinePropertyRefLists.inl>
#endif

// Find entity property data by a variable name of a specific class
const CEntityProperty *FindPropertyByVariable(const CTString &strClass, const CTString &strVariable) {
#if SE1_GAME != SS_REV
  // Find property in the table
  const CEntityProperty *pep = _mapPropRefs.FindProperty(strClass, strVariable);
  ASSERTMSG(pep != NULL, "Cannot find entity property data in the reference table!");

  return pep;

#else
  ASSERTALWAYS("Please use CDLLEntityClass::PropertyForVariable() instead of table lookup!");
  return NULL;
#endif
};
