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

#include "StdH.h"

#include "MapConversion.h"

#if _PATCHCONFIG_CONVERT_MAPS

// Currently used converter
static IMapConverter *_pconvCurrent = NULL;

// Get map converter for a specific format
IMapConverter *IMapConverter::SetConverter(ELevelFormat eFormat)
{
  switch (eFormat) {
  #if TSE_FUSION_MODE
    case E_LF_TFE: _pconvCurrent = &_convTFE; break;
    case E_LF_SSR: _pconvCurrent = &_convSSR; break;
  #endif
    default: _pconvCurrent = NULL;
  }

  //ASSERTMSG(_pconvCurrent != NULL, "No converter available for the desired level format!");
  return _pconvCurrent;
};

// Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
void IMapConverter::HandleUnknownProperty(CEntity *pen, ULONG ulType, ULONG ulID, void *pValue)
{
  UnknownProp prop(ulType, ulID, pValue);

  if (_pconvCurrent != NULL) {
    _pconvCurrent->HandleProperty(pen, prop);
  }
};

// Check if the entity state doesn't match
BOOL IMapConverter::CheckEntityState(CRationalEntity *pen, SLONG slState, INDEX iClassID) {
  // Wrong entity class
  if (!IsOfClassID(pen, iClassID)) return FALSE;

  // No states at all, doesn't matter
  if (pen->en_stslStateStack.Count() <= 0) return FALSE;

  return pen->en_stslStateStack[0] != slState;
};

// Create a global light entity to fix shadow issues with brush polygon layers
void IMapConverter::CreateGlobalLight(void) {
#if SE1_VER >= SE1_107
  // Create an invisible light that covers the whole map
  try {
    static const CTString strLightClass = "Classes\\Light.ecl";
    CEntity *penLight = IWorld::GetWorld()->CreateEntity_t(IDummy::plCenter, strLightClass);

    // Retrieve light properties
    static CPropertyPtr pptrType(penLight); // CLight::m_ltType
    static CPropertyPtr pptrFallOff(penLight); // CLight::m_rFallOffRange
    static CPropertyPtr pptrColor(penLight); // CLight::m_colColor

    if (pptrType   .ByVariable("CLight", "m_ltType")
     && pptrFallOff.ByVariable("CLight", "m_rFallOffRange")
     && pptrColor  .ByVariable("CLight", "m_colColor"))
    {
      ENTITYPROPERTY(penLight, pptrType.Offset(), INDEX) = 2; // LightType::LT_STRONG_AMBIENT
      ENTITYPROPERTY(penLight, pptrFallOff.Offset(), RANGE) = 10000.0f;
      ENTITYPROPERTY(penLight, pptrColor.Offset(), COLOR) = 0; // Black
    }

    penLight->Initialize();
    penLight->GetLightSource()->FindShadowLayers(FALSE);

  } catch (char *strError) {
    FatalError(TRANS("Cannot load %s class:\n%s"), "Light", strError);
  }
#endif
};

#endif // _PATCHCONFIG_CONVERT_MAPS

// Load some class from patch's ExtraEntities library instead of vanilla entities, if required
BOOL LoadClassFromExtras(CTString &strClassName, CTFileName &fnmDLL, ClassReplacementPair *aTable) {
  // ExtraEntities library is part of the custom mod
#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  if (!ClassicsCore_IsCustomModActive()) return FALSE;
#else
  return FALSE;
#endif

  INDEX i = 0;

  while (aTable[i].strOld != NULL) {
    // If found the class in the table
    if (strClassName == aTable[i].strOld) {
      // Optionally replace it with another one
      if (aTable[i].strNew != NULL) strClassName = aTable[i].strNew;

      // Replace the library and exit
      fnmDLL = CTString("Bin\\ClassicsExtras.dll");
      return TRUE;
    }

    i++;
  }

  // Class not found
  return FALSE;
};

// Load another class in place of the current one, if it's found in the replacement table
BOOL ReplaceClassFromTable(CTString &strClassName, ClassReplacementPair *aTable) {
  INDEX i = 0;

  while (aTable[i].strOld != NULL) {
    // If found the class in the table
    if (strClassName == aTable[i].strOld) {
      // Replace it with another one and exit
      strClassName = aTable[i].strNew;
      return TRUE;
    }

    i++;
  }

  // Class not found
  return FALSE;
};
