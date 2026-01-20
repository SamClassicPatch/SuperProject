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

#include "StdH.h"

#include "Converters/TFEMaps.h"

#if CLASSIC_TSE_FUSION_MODE

// Classes that need to be converted
#include <EntitiesV/StdH/StdH.h>
#include <EntitiesV/AmmoPack.h>
#include <EntitiesV/Camera.h>
#include <EntitiesV/Devil.h>
#include <EntitiesV/DoorController.h>
#include <EntitiesV/EnemyBase.h>
#include <EntitiesV/EnemySpawner.h>
#include <EntitiesV/KeyItem.h>
#include <EntitiesV/Lightning.h>
#include <EntitiesV/MovingBrush.h>
#include <EntitiesV/PlayerMarker.h>
#include <EntitiesV/PyramidSpaceShip.h>
#include <EntitiesV/StormController.h>
#include <EntitiesV/Trigger.h>
#include <EntitiesV/Woman.h>
#include <EntitiesV/WorldSettingsController.h>

namespace IConvertTFE {

// Prepare the converter before loading a new world
void Prepare(void) {
  // Clear the rain
  ClearRainVariables();
};

// Handle some unknown property
void HandleProperty(const ExtArgUnknownProp_t &propData)
{
  const SWorldConverterUnknownProp &prop = (const SWorldConverterUnknownProp &)propData;

  if (IsOfClassID(prop.pen, CWorldSettingsController_ClassID)) {
    RememberWSC(prop);
  }
};

// Convert invalid weapon flag in a mask
void ConvertWeapon(INDEX &iFlags, INDEX iWeapon) {
  switch (iWeapon) {
    case IMapsTFE::WEAPON_LASER: iFlags |= WeaponFlag(IMapsTSE::WEAPON_LASER); break;
    case IMapsTFE::WEAPON_IRONCANNON: iFlags |= WeaponFlag(IMapsTSE::WEAPON_IRONCANNON); break;

    // Nonexistent weapons
    case 10: case 11: case 12: case 13: case 15: case 17: break;

    // Others
    default: iFlags |= WeaponFlag(iWeapon);
  }
};

// Convert invalid key types
void ConvertKeyType(INDEX &eKey) {
#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  // Shift TFE keys from 0 to 16
  if (ClassicsCore_IsCustomModActive()) {
    eKey += 16;
    return;
  }
#endif

  switch (eKey) {
    // Dummy keys
    case IMapsTFE::KIT_ANKHGOLDDUMMY: eKey = IMapsTSE::KIT_TABLESDUMMY; break;
    case IMapsTFE::KIT_SCARABDUMMY:   eKey = IMapsTSE::KIT_TABLESDUMMY; break;

    // Element keys
    case IMapsTFE::KIT_ELEMENTEARTH: eKey = IMapsTSE::KIT_CROSSWOODEN; break;
    case IMapsTFE::KIT_ELEMENTWATER: eKey = IMapsTSE::KIT_CROSSMETAL; break;
    case IMapsTFE::KIT_ELEMENTAIR:   eKey = IMapsTSE::KIT_CRYSTALSKULL; break;
    case IMapsTFE::KIT_ELEMENTFIRE:  eKey = IMapsTSE::KIT_CROSSGOLD; break;

    // Other keys
    case IMapsTFE::KIT_ANKHWOOD: eKey = IMapsTSE::KIT_CROSSWOODEN; break;
    case IMapsTFE::KIT_ANKHROCK: eKey = IMapsTSE::KIT_CROSSMETAL; break;
    case IMapsTFE::KIT_ANKHGOLD: eKey = IMapsTSE::KIT_CROSSGOLD; break;
    case IMapsTFE::KIT_AMONGOLD: eKey = IMapsTSE::KIT_KINGSTATUE; break;
    case IMapsTFE::KIT_RAKEY:    eKey = IMapsTSE::KIT_HOLYGRAIL; break;
    case IMapsTFE::KIT_MOONKEY:  eKey = IMapsTSE::KIT_BOOKOFWISDOM; break;
    case IMapsTFE::KIT_EYEOFRA:  eKey = IMapsTSE::KIT_BOOKOFWISDOM; break;
    case IMapsTFE::KIT_SCARAB:   eKey = IMapsTSE::KIT_STATUEHEAD03; break;
    case IMapsTFE::KIT_COBRA:    eKey = IMapsTSE::KIT_HOLYGRAIL; break;
    case IMapsTFE::KIT_HEART:    eKey = IMapsTSE::KIT_STATUEHEAD01; break;
    case IMapsTFE::KIT_FEATHER:  eKey = IMapsTSE::KIT_STATUEHEAD02; break;
    case IMapsTFE::KIT_SPHINX1:  eKey = IMapsTSE::KIT_WINGEDLION; break;
    case IMapsTFE::KIT_SPHINX2:  eKey = IMapsTSE::KIT_ELEPHANTGOLD; break;

    // Edge case
    default: eKey = IMapsTSE::KIT_KINGSTATUE; break;
  }
};

// Convert one specific entity without reinitializing it
BOOL ConvertEntity(CEntity *pen) {
  // Remove napalm and sniper bullets
  if (IsOfClassID(pen, CAmmoPack_ClassID)) {
    // Retrieve CAmmoPack::m_iNapalm and CAmmoPack::m_iSniperBullets
    static CPropertyPtr pptrNapalm(pen);
    static CPropertyPtr pptrSniper(pen);

    if (pptrNapalm.ByVariable("CAmmoPack", "m_iNapalm")) {
      ENTITYPROPERTY(pen, pptrNapalm.Offset(), INDEX) = 0;
    }

    if (pptrSniper.ByVariable("CAmmoPack", "m_iSniperBullets")) {
      ENTITYPROPERTY(pen, pptrSniper.Offset(), INDEX) = 0;
    }

    return TRUE;

  // Adjust weapon masks
  } else if (IsOfClassID(pen, CPlayerMarker_ClassID)) {
    // Retrieve CPlayerMarker::m_iGiveWeapons and CPlayerMarker::m_iTakeWeapons
    static CPropertyPtr pptrGive(pen);
    static CPropertyPtr pptrTake(pen);

    // Convert weapon flags
    if (pptrGive.ByVariable("CPlayerMarker", "m_iGiveWeapons")
     && pptrTake.ByVariable("CPlayerMarker", "m_iTakeWeapons"))
    {
      INDEX &iGiveWeapons = ENTITYPROPERTY(pen, pptrGive.Offset(), INDEX);
      INDEX &iTakeWeapons = ENTITYPROPERTY(pen, pptrTake.Offset(), INDEX);

      INDEX iNewGive = 0x03; // Knife and Colt
      INDEX iNewTake = 0;

      for (INDEX i = IMapsTFE::WEAPON_NONE + 1; i < IMapsTFE::WEAPON_LAST + 1; i++) {
        // Replace the weapon if it exists
        if (iGiveWeapons & WeaponFlag(i)) {
          ConvertWeapon(iNewGive, i);
        }

        if (iTakeWeapons & WeaponFlag(i)) {
          ConvertWeapon(iNewTake, i);
        }
      }

      iGiveWeapons = iNewGive;
      iTakeWeapons = iNewTake;
    }

    return TRUE;

  // Adjust keys
  } else if (IsOfClassID(pen, CKeyItem_ClassID)) {
    // Retrieve CKeyItem::m_kitType and CKeyItem::m_iSoundComponent
    static CPropertyPtr pptrType(pen);
    static CPropertyPtr pptrSound(pen);

    // Convert key type
    if (pptrType.ByVariable("CKeyItem", "m_kitType")) {
      ConvertKeyType(ENTITYPROPERTY(pen, pptrType.Offset(), INDEX));
    }

    // Fix sound component index (301 -> 300)
    if (pptrSound.ByVariable("CKeyItem", "m_iSoundComponent")) {
      ENTITYPROPERTY(pen, pptrSound.Offset(), INDEX) = (CKeyItem_ClassID << 8) + 300;
    }

    return TRUE;

  // Adjust keys
  } else if (IsOfClassID(pen, CDoorController_ClassID)) {
    // Retrieve CDoorController::m_dtType and CDoorController::m_kitKey
    static CPropertyPtr pptrType(pen);
    static CPropertyPtr pptrKey(pen);

    if (pptrType.ByVariable("CDoorController", "m_dtType")
     && pptrKey.ByVariable("CDoorController", "m_kitKey"))
    {
      // Convert key type only for locked doors (DT_LOCKED)
      if (ENTITYPROPERTY(pen, pptrType.Offset(), INDEX) == 2) {
        ConvertKeyType(ENTITYPROPERTY(pen, pptrKey.Offset(), INDEX));
      }
    }

    return TRUE;

  // Adjust storm shade color
  } else if (IsOfClassID(pen, CStormController_ClassID)) {
    // Retrieve CStormController::m_colShadeStart and CStormController::m_colShadeStop
    static CPropertyPtr pptrShadeStart(pen);
    static CPropertyPtr pptrShadeStop(pen);

    // Properties don't exist
    if (!pptrShadeStart.ByVariable("CStormController", "m_colShadeStart")) {
      return FALSE;
    }

    if (!pptrShadeStop.ByVariable("CStormController", "m_colShadeStop")) {
      return FALSE;
    }

    // Matches gray scale in the 64-255 brightness range
    ENTITYPROPERTY(pen, pptrShadeStart.Offset(), COLOR) = C_WHITE | CT_OPAQUE;
    ENTITYPROPERTY(pen, pptrShadeStop.Offset(), COLOR) = C_dGRAY | CT_OPAQUE;

    // Add to the list of storm controllers
    extern CEntities TFE_cenStorms;
    TFE_cenStorms.Add(pen);

    // Proceed with reinitialization
    return FALSE;
  }

  // Invalid entity
  return FALSE;
};

// Convert the entire world with possible entity reinitializations
void ConvertWorld(CWorld *pwo) {
  CEntities cEntities;

  FOREACHINDYNAMICCONTAINER(pwo->wo_cenEntities, CEntity, iten) {
    CEntity *pen = iten;

    // Convert specific entities regardless of state first
    if (ConvertEntity(pen)) continue;

    // Remember triggers for future use
    if (IsOfClassID(pen, CTrigger_ClassID)) {
      extern CEntities TFE_cenTriggers;
      TFE_cenTriggers.Add(pen);

      cEntities.Add(pen);
      continue;
    }

    // Reinitialize all spawners
    if (IsOfClassID(pen, CEnemySpawner_ClassID)) {
      cEntities.Add(pen);
      continue;
    }

    // Ignore entities without states
    if (!IsRationalEntity(pen)) continue;

    // Check TFE states of logical entities
    CRationalEntity *penRE = (CRationalEntity *)pen;

    if (CheckEntityState(penRE, 0x00DC000A, CCamera_ClassID)
     || CheckEntityState(penRE, 0x00DC000D, CCamera_ClassID)
     || CheckEntityState(penRE, 0x01300043, CEnemySpawner_ClassID)
     || CheckEntityState(penRE, 0x025F0009, CLightning_ClassID)
     || CheckEntityState(penRE, 0x00650014, CMovingBrush_ClassID)
     || CheckEntityState(penRE, 0x0261002E, CPyramidSpaceShip_ClassID)
     || CheckEntityState(penRE, 0x025E000C, CStormController_ClassID)
     || CheckEntityState(penRE, 0x014C013B, CDevil_ClassID)
     || CheckEntityState(penRE, 0x0140001B, CWoman_ClassID)) {
      cEntities.Add(pen);
      continue;
    }

    // Other TFE enemies
    if (IsDerivedFromID(pen, CEnemyBase_ClassID)) {
      if (penRE->en_stslStateStack.Count() > 0
       && penRE->en_stslStateStack[0] != 0x01360070) {
        cEntities.Add(pen);
        continue;
      }
    }
  }

  // Restore rain
  ApplyRainProperties();

  FOREACHINDYNAMICCONTAINER(cEntities, CEntity, itenReinit) {
    itenReinit->Reinitialize();
  }

  // Fix shadow issues
  CreateGlobalLight();
};

}; // namespace

#endif // CLASSIC_TSE_FUSION_MODE
