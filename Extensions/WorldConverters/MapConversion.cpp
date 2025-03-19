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

// Storage of all possible converters under specific identifiers
typedef se1::map<CTString, IWorldConverter> CMapConverters;
static CMapConverters _mapConverters;

// Add new world converter with a specific name
IWorldConverter *IWorldConverter::Add(const CTString &strName) {
  CMapConverters::const_iterator it = _mapConverters.find(strName);

  // Doesn't exist yet
  if (it == _mapConverters.end()) {
    IWorldConverter &conv = _mapConverters[strName];

    // Set a unique identifier
    static int _iConverterID = 0;
    conv.m_iID = _iConverterID++;

    return &conv;
  }

  return NULL;
};

// Remove a world converter with a specific name
bool IWorldConverter::Remove(const CTString &strName) {
  CMapConverters::iterator it = _mapConverters.find(strName);

  if (it != _mapConverters.end()) {
    _mapConverters.erase(it);
    return true;
  }

  return false;
};

// Try to find a converter by its name
IWorldConverter *IWorldConverter::Find(const CTString &strName) {
  CMapConverters::iterator it = _mapConverters.find(strName);

  if (it != _mapConverters.end()) {
    return &it->second;
  }

  return NULL;
};

// Try to find a converter by its identifier
IWorldConverter *IWorldConverter::Find(int iID) {
  // Invalid converter
  if (iID == -1) return NULL;

  CMapConverters::iterator it;

  for (it = _mapConverters.begin(); it != _mapConverters.end(); it++) {
    if (it->second.m_iID == iID) return &it->second;
  }

  return NULL;
};

// Get level format from the converter name (internal)
inline INDEX ConverterNameToLevelFormat(const CTString &strName) {
  if (strName == "tfe") return E_LF_TFE;
  else
  if (strName == "tse") return E_LF_TSE;
  else
  if (strName == "ssr") return E_LF_SSR;
  else
  if (strName == "150") return E_LF_150;

  return -1;
};

// List available converters
void IWorldConverter::ListConverters(void) {
  CMapConverters::const_iterator it;

  for (it = _mapConverters.begin(); it != _mapConverters.end(); it++) {
    const CTString &str = it->first;
    CPrintF("%d - \"%s\" ", it->second.m_iID, str.str_String);

    // Specify which level format this converter is for
    const INDEX iFormat = ConverterNameToLevelFormat(str);

    if (iFormat != -1) {
      CPrintF(TRANS("(for level format %d)\n"), iFormat);
    } else {
      CPutString("\n");
    }
  }
};

int IWorldConverter::CreateConverter(void *strName) {
  if (strName == NULL) return -1;

  IWorldConverter *pConv = Add((const char *)strName);
  if (pConv == NULL) return -1;

  return pConv->m_iID;
};

int IWorldConverter::SetMethodDestructor(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;
  IWorldConverter *pConv = Find(data.iID);

  if (pConv == NULL) return FALSE;

  pConv->m_pDestructor = (FWorldConverterDestructor)data.pData;
  return TRUE;
};

int IWorldConverter::SetMethodPrepare(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;
  IWorldConverter *pConv = Find(data.iID);

  if (pConv == NULL) return FALSE;

  pConv->m_pPrepare = (FWorldConverterPrepare)data.pData;
  return TRUE;
};

int IWorldConverter::SetMethodReplaceClass(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;
  IWorldConverter *pConv = Find(data.iID);

  if (pConv == NULL) return FALSE;

  pConv->m_pReplaceClass = (FWorldConverterReplaceClass)data.pData;
  return TRUE;
};

int IWorldConverter::SetMethodHandleProperty(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;
  IWorldConverter *pConv = Find(data.iID);

  if (pConv == NULL) return FALSE;

  pConv->m_pHandleProperty = (FWorldConverterHandleProperty)data.pData;
  return TRUE;
};

int IWorldConverter::SetMethodConvertWorld(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;
  IWorldConverter *pConv = Find(data.iID);

  if (pConv == NULL) return FALSE;

  pConv->m_pConvertWorld = (FWorldConverterConvert)data.pData;
  return TRUE;
};

// Currently used converters
static CDynamicContainer<IWorldConverter> _cCurrentConverters;

// Get world converter for a specific level format
int IWorldConverter::GetConverterForFormat(void *pFormat)
{
  ELevelFormat eFormat = *(ELevelFormat *)pFormat;

  // Find world converter for any supported level format
  IWorldConverter *pConv = NULL;

  switch (eFormat) {
    case E_LF_TFE: pConv = Find("tfe"); break;
    case E_LF_TSE: pConv = Find("tse"); break;
    case E_LF_SSR: pConv = Find("ssr"); break;
    case E_LF_150: pConv = Find("150"); break;
  }

  return (pConv != NULL) ? pConv->m_iID : -1;
};

// Get world converter by its name
int IWorldConverter::GetConverterByName(void *strName) {
  if (strName == NULL) return -1;

  IWorldConverter *pConv = Find((const char *)strName);
  return (pConv != NULL) ? pConv->m_iID : -1;
};

// Get level format from the converter name
int IWorldConverter::GetFormatFromConverter(void *strName) {
  if (strName == NULL) return -1;
  return ConverterNameToLevelFormat((const char *)strName);
};

// Prepare a specific world converter before using it
int IWorldConverter::PrepareConverter(void *pConverterData) {
  if (pConverterData == NULL) return FALSE;

  // Clear previous converters
  _cCurrentConverters.Clear();

  ExtArgWorldConverter_t &data = *(ExtArgWorldConverter_t *)pConverterData;

  // Determine order of converters instead of using a single one
  bool bMultiple = false;

  if (data.pData != NULL) {
    const char *pchBeg = (const char *)data.pData;
    const char *pchCur = pchBeg;

    FOREVER {
      const bool bEnd = (*pchCur == '\0');

      // If encountered a separator or the end
      if (*pchCur == ';' || bEnd) {
        // And it's not the beginning of the string (i.e. empty string)
        if (pchCur != pchBeg) {
          // Try to find a converter with this name
          const size_t ctLen = pchCur - pchBeg;

          char *strTemp = (char *)AllocMemory(ctLen + 1);
          strTemp[ctLen] = '\0';

          memcpy(strTemp, pchBeg, ctLen);

          IWorldConverter *pConv = Find(strTemp);

          if (pConv != NULL) {
            _cCurrentConverters.Add(pConv);
            bMultiple = true;
          }

          FreeMemory(strTemp);
        }

        // Remember beginning of the next string
        pchBeg = pchCur + 1;
      }

      // Quit on string end
      if (bEnd) break;

      // Check the next character
      pchCur++;
    }
  }

  // Try using a single converter from the specified ID if none have been added in order
  if (!bMultiple) {
    IWorldConverter *pConv = Find(data.iID);
    if (pConv == NULL) return FALSE;

    _cCurrentConverters.Add(pConv);
  }

  // Prepare all converters in the specified order
  FOREACHINDYNAMICCONTAINER(_cCurrentConverters, IWorldConverter, itConv) {
    if (itConv->m_pPrepare != NULL) {
      itConv->m_pPrepare();
    }
  }

  return TRUE;
};

// Convert the world using a specific converter
int IWorldConverter::ConvertWorld(void *pWorld) {
  // No world
  if (pWorld == NULL) return FALSE;

  // Convert the world in the specified order
  FOREACHINDYNAMICCONTAINER(_cCurrentConverters, IWorldConverter, itConv) {
    if (itConv->m_pConvertWorld != NULL) {
      itConv->m_pConvertWorld((CWorld *)pWorld);
    }
  }

  return TRUE;
};

// Handle unknown entity property upon reading it via CEntity::ReadProperties_t()
int IWorldConverter::HandleUnknownProperty(void *pPropData) {
  // No property data
  if (pPropData == NULL) return FALSE;

  // Handle unknown property in the specified order
  FOREACHINDYNAMICCONTAINER(_cCurrentConverters, IWorldConverter, itConv) {
    if (itConv->m_pHandleProperty != NULL) {
      ExtArgUnknownProp_t &prop = *(ExtArgUnknownProp_t *)pPropData;
      itConv->m_pHandleProperty(prop);
    }
  }

  return TRUE;
};

// Check if the entity state doesn't match
BOOL CheckEntityState(CRationalEntity *pen, SLONG slState, INDEX iClassID) {
  // Wrong entity class
  if (!IsOfClassID(pen, iClassID)) return FALSE;

  // No states at all, doesn't matter
  if (pen->en_stslStateStack.Count() <= 0) return FALSE;

  return pen->en_stslStateStack[0] != slState;
};

// Create a global light entity to fix shadow issues with brush polygon layers
void CreateGlobalLight(void) {
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

// Pair of class names for a replacement table
struct ClassReplacementPair {
  const char *strOld;
  const char *strNew;
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

// Replace some class from some library upon loading it from an ECL file
int IWorldConverter::ReplaceClass(void *pEclData) {
  ExtArgEclData_t &eclData = *(ExtArgEclData_t *)pEclData;

  // See if any converter can replace certain classes in the specified order
  FOREACHINDYNAMICCONTAINER(_cCurrentConverters, IWorldConverter, itConv) {
    if (itConv->m_pReplaceClass != NULL) {
      // Quit if it can
      if (itConv->m_pReplaceClass(eclData)) return TRUE;
    }
  }

  CTFileName &fnmDLL = *eclData.pfnmDLL;
  CTString &strClassName = *eclData.pstrClassName;

  // Not vanilla entities
  if (fnmDLL != "Bin\\Entities.dll") return FALSE;

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
