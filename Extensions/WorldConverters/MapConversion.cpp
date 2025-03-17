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

// [Cecil] TEMP: _EnginePatches._eWorldFormat
#include <CorePatches/Patches.h>

// Currently used converter
static IMapConverter *_pconvCurrent = NULL;

// Get map converter for a specific format
int IMapConverter::SetConverterForFormat(void *pFormat)
{
  ELevelFormat eFormat = *(ELevelFormat *)pFormat;

  switch (eFormat) {
  #if CLASSIC_TSE_FUSION_MODE
    case E_LF_TFE: _pconvCurrent = &_convTFE; break;
    case E_LF_SSR: _pconvCurrent = &_convSSR; break;
  #endif
    default: _pconvCurrent = NULL;
  }

  //ASSERTMSG(_pconvCurrent != NULL, "No converter available for the desired level format!");
  return (_pconvCurrent != NULL);
};

// Reset a specific map converter before using it
int IMapConverter::ResetConverter(void *) {
  if (_pconvCurrent != NULL) {
    _pconvCurrent->Reset();
  }

  return 0;
};

// Convert the world using the current converter
int IMapConverter::ConvertWorld(void *pWorld) {
  if (_pconvCurrent != NULL) {
    _pconvCurrent->ConvertWorld((CWorld *)pWorld);
  }

  return 0;
};

// Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
int IMapConverter::HandleUnknownProperty(void *pPropData)
{
  if (_pconvCurrent != NULL) {
    struct SignalUnknownProp {
      CEntity *pen;
      ULONG ulType;
      ULONG ulID;
      void *pValue;
    } *pArgProp = (SignalUnknownProp *)pPropData;

    UnknownProp prop(pArgProp->ulType, pArgProp->ulID, pArgProp->pValue);
    _pconvCurrent->HandleProperty(pArgProp->pen, prop);
  }

  return 0;
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
    CEntity *penLight = IWorld::GetWorld()->CreateEntity_t(_plWorldCenter, strLightClass);

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

// Load some class from patch's ExtraEntities library instead of vanilla entities, if required
static BOOL LoadClassFromExtras(CTString &strClassName, CTFileName &fnmDLL, ClassReplacementPair *aTable) {
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
static BOOL ReplaceClassFromTable(CTString &strClassName, ClassReplacementPair *aTable) {
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

// Replace nonexistent vanilla classes upon loading them from ECL classes
int ReplaceMissingClasses(void *pEclData) {
  struct EclData {
    CTFileName fnmDLL;
    CTString strClassName;
  } *pData = (EclData *)pEclData;

  CTFileName &fnmDLL = pData->fnmDLL;
  CTString &strClassName = pData->strClassName;

  // Classes available in ExtraEntities library
  static ClassReplacementPair aExtras[] = {
    // Alpha enemies
    { "CAirWave",       NULL },
    { "CCatman",        NULL },
    { "CCyborg",        NULL },
    { "CCyborgBike",    NULL },
    { "CDragonman",     NULL },
    { "CFishman",       NULL },
    { "CHuanman",       NULL },
    { "CMamut",         NULL },
    { "CMamutman",      NULL },
    { "CMantaman",      NULL },
    { "CRobotDriving",  NULL },
    { "CRobotFixed",    NULL },
    { "CRobotFlying",   NULL },
    { "CTerrainEntity", NULL },

    // Revolution entities (commented ones aren't finished)
    { "CAchievementEntity",    NULL },
    { "CControlZoneEntity",    NULL },
    { "CDestroyer",            NULL },
    { "CFlagItem",             NULL },
    //{ "CModelHolder2Movable",  NULL },
    //{ "CPostProcessingEffect", NULL },
    { "CSpectatorCamera",      NULL },
    { "CUghzy",                NULL },
    //{ "CVehicle",              NULL },
    { "CWorldInfoEntity",      NULL },
    { NULL, NULL }
  };

  // Replace classes with something from ExtraEntities
  if (LoadClassFromExtras(strClassName, fnmDLL, aExtras)) return TRUE;

  // It should only reach this point when custom mod is disabled,
  // which means that in Revolution these entities already exist
#if SE1_GAME != SS_REV

  // Not a Revolution map
  if (_EnginePatches._eWorldFormat != E_LF_SSR) return FALSE;

  // Replace some vanilla entities with those from ExtraEntities library
  static ClassReplacementPair aRevEntities[] = {
    { "CDamager",     NULL },
    { "CElemental",   NULL },
    { "CHeadman",     NULL },
    { "CSoundHolder", "CSoundHolderRev" },
    { "CWalker",      NULL },
    { NULL, NULL },
  };

  if (LoadClassFromExtras(strClassName, fnmDLL, aRevEntities)) return TRUE;

  // Replace classes from Revolution
  static ClassReplacementPair aRevReplace[] = {
    // Alpha enemies
    { "CCatman",               "CGrunt" },
    { "CCyborg",               "CWalker" },
    { "CDragonman",            "CWoman" },
    { "CFishman",              "CHeadman" },
    { "CHuanman",              "CGrunt" },
    { "CMamut",                "CWerebull" },
    { "CMamutman",             "CHeadman" },
    { "CMantaman",             "CFish" },
    { "CRobotDriving",         "CGrunt" },
    { "CRobotFlying",          "CWoman" },

    // Revolution entities
    { "CAchievementEntity",    "CTrigger" },
    { "CControlZoneEntity",    "CTrigger" },
    { "CDestroyer",            "CDemon" },
    { "CFlagItem",             "CHealthItem" },
    { "CPostProcessingEffect", "CMarker" },
    { "CSpectatorCamera",      "CMarker" },
    { "CUghzy",                "CGuffy" },
    { "CWorldInfoEntity",      "CMarker" },
    { NULL, NULL },
  };

  return ReplaceClassFromTable(strClassName, aRevReplace);

#endif // SE1_GAME != SS_REV

  // No replacement
  return FALSE;
};

// Replace nonexistent Revolution classes before loading them from ECL files
int ReplaceRevolutionClasses(void *pfnmCopy) {
  CTFileName &fnmCopy = *(CTFileName *)pfnmCopy;

  static ClassReplacementPair aRevReplace[] = {
    { "Classes\\PostProcessingEffect.ecl", "Classes\\Marker.ecl" },
    { NULL, NULL },
  };

  return ReplaceClassFromTable(fnmCopy, aRevReplace);
};
