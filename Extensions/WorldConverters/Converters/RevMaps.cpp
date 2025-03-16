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

#include "../MapConversion.h"

// Classes that need to be converted
#include <EntitiesV/StdH/StdH.h>
#include <EntitiesV/AmmoItem.h>
#include <EntitiesV/AmmoPack.h>
#include <EntitiesV/BlendController.h>
#include <EntitiesV/Damager.h>
#include <EntitiesV/DoorController.h>
#include <EntitiesV/Elemental.h>
#include <EntitiesV/Eruptor.h>
#include <EntitiesV/Headman.h>
#include <EntitiesV/KeyItem.h>
#include <EntitiesV/PlayerMarker.h>
#include <EntitiesV/PowerUpItem.h>
#include <EntitiesV/SoundHolder.h>
#include <EntitiesV/Teleport.h>
#include <EntitiesV/VoiceHolder.h>
#include <EntitiesV/Walker.h>
#include <EntitiesV/WeaponItem.h>
#include <EntitiesV/WorldBase.h>
#include <EntitiesV/WorldLink.h>
#include <EntitiesV/WorldSettingsController.h>

// Converter instance
IConvertSSR _convSSR;

// Reset the converter before loading a new world
void IConvertSSR::Reset(void) {
  // Clear entities to destroy
  cenDestroy.Clear();
};

// Handle some unknown property
void IConvertSSR::HandleProperty(CEntity *pen, const UnknownProp &prop) {
  // Replace new ammo
  if (IsOfClassID(pen, CAmmoPack_ClassID)) {
    switch (prop.ulID) {
      // Plasma packs
      case (CAmmoPack_ClassID << 8) + 18: {
        static CPropertyPtr pptrAmmo(pen);

        // Give electricity instead
        if (pptrAmmo.ByVariable("CAmmoPack", "m_iElectricity")) {
          INDEX &iAmmo = ENTITYPROPERTY(pen, pptrAmmo.Offset(), INDEX);
          iAmmo = Max(prop.Index(), iAmmo);
        }
      } break;

      // Mine packs
      case (CAmmoPack_ClassID << 8) + 19: {
        static CPropertyPtr pptrAmmo(pen);

        // Give grenades instead
        if (pptrAmmo.ByVariable("CAmmoPack", "m_iGrenades")) {
          INDEX &iAmmo = ENTITYPROPERTY(pen, pptrAmmo.Offset(), INDEX);
          iAmmo = Max(prop.Index(), iAmmo);
        }
      } break;
    }

  // New damager functionality
  } else if (IsOfClassID(pen, CDamager_ClassID)) {
    switch (prop.ulID) {
      // Damage in range
      case (CDamager_ClassID << 8) + 20: {
        // Remember for later destruction
        if (prop.Bool()) {
          cenDestroy.Add(pen);
        }
      } break;

      // Heal
      case (CDamager_ClassID << 8) + 30: {
        static CPropertyPtr pptrAmount(pen);

        // Remove all damage
        if (prop.Bool() && pptrAmount.ByVariable("CDamager", "m_fAmmount")) {
          ENTITYPROPERTY(pen, pptrAmount.Offset(), FLOAT) = 0.0f;
        }
      } break;
    }

  // Anti-speedrun teleporters
  } else if (IsOfClassID(pen, CTeleport_ClassID)) {
    // Remember for later destruction if "Stops speedrunners" is enabled
    if (prop.ulID == (CTeleport_ClassID << 8) + 9 && prop.Bool()) {
      cenDestroy.Add(pen);
    }

  // Read value into another field
  } else if (IsOfClassID(pen, CWorldSettingsController_ClassID)) {
    // Check if the replacement field can be used
    if (prop.ulID == (CWorldSettingsController_ClassID << 8) + 90) {
      static CSymbolPtr piOffset("ent_iWSCBlendSpeedOffset");

      if (piOffset.Exists()) {
        ENTITYPROPERTY(pen, piOffset.GetIndex(), FLOAT) = prop.Float();
      }
    }
  }
};

// Convert invalid weapon flag in a mask
void IConvertSSR::ConvertWeapon(INDEX &iFlags, INDEX iWeapon) {
  switch (iWeapon) {
    case IMapsSSR::WEAPON_GHOSTBUSTER:
    case IMapsSSR::WEAPON_PLASMATHROWER:
      iFlags |= WeaponFlag(IMapsTSE::WEAPON_LASER);
      break;

    case IMapsSSR::WEAPON_MINELAYER:
      iFlags |= WeaponFlag(IMapsTSE::WEAPON_GRENADELAUNCHER);
      break;

    // Others
    default: iFlags |= WeaponFlag(iWeapon);
  }
};

// Convert invalid key types
void IConvertSSR::ConvertKeyType(INDEX &eKey) {
  // Shift TSE keys from 19 to 0
  if (eKey >= IMapsSSR::KIT_BOOKOFWISDOM && eKey <= IMapsSSR::KIT_CRYSTALSKULL) {
    eKey -= IMapsSSR::KIT_BOOKOFWISDOM;
    return;
  }

#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  // TFE keys
  if (ClassicsCore_IsCustomModActive()) {
    // Shift some keys by 16
    if (eKey < IMapsSSR::KIT_EYEOFRA) {
      eKey += 16;

    // And the rest by 17 because there of a gap that was removed in SSR
    } else {
      eKey += 17;
    }

    return;
  }
#endif

  switch (eKey) {
    // Dummy keys
    case IMapsSSR::KIT_ANKHGOLDDUMMY: eKey = IMapsTSE::KIT_TABLESDUMMY; break;
    case IMapsSSR::KIT_SCARABDUMMY:   eKey = IMapsTSE::KIT_TABLESDUMMY; break;

    // Element keys
    case IMapsSSR::KIT_ELEMENTEARTH: eKey = IMapsTSE::KIT_CROSSWOODEN; break;
    case IMapsSSR::KIT_ELEMENTWATER: eKey = IMapsTSE::KIT_CROSSMETAL; break;
    case IMapsSSR::KIT_ELEMENTAIR:   eKey = IMapsTSE::KIT_CRYSTALSKULL; break;
    case IMapsSSR::KIT_ELEMENTFIRE:  eKey = IMapsTSE::KIT_CROSSGOLD; break;

    // Other keys
    case IMapsSSR::KIT_ANKHWOOD: eKey = IMapsTSE::KIT_CROSSWOODEN; break;
    case IMapsSSR::KIT_ANKHROCK: eKey = IMapsTSE::KIT_CROSSMETAL; break;
    case IMapsSSR::KIT_ANKHGOLD: eKey = IMapsTSE::KIT_CROSSGOLD; break;
    case IMapsSSR::KIT_AMONGOLD: eKey = IMapsTSE::KIT_KINGSTATUE; break;
    case IMapsSSR::KIT_RAKEY:    eKey = IMapsTSE::KIT_HOLYGRAIL; break;
    case IMapsSSR::KIT_MOONKEY:  eKey = IMapsTSE::KIT_BOOKOFWISDOM; break;
    case IMapsSSR::KIT_EYEOFRA:  eKey = IMapsTSE::KIT_BOOKOFWISDOM; break;
    case IMapsSSR::KIT_SCARAB:   eKey = IMapsTSE::KIT_STATUEHEAD03; break;
    case IMapsSSR::KIT_COBRA:    eKey = IMapsTSE::KIT_HOLYGRAIL; break;
    case IMapsSSR::KIT_HEART:    eKey = IMapsTSE::KIT_STATUEHEAD01; break;
    case IMapsSSR::KIT_FEATHER:  eKey = IMapsTSE::KIT_STATUEHEAD02; break;
    case IMapsSSR::KIT_SPHINX1:  eKey = IMapsTSE::KIT_WINGEDLION; break;
    case IMapsSSR::KIT_SPHINX2:  eKey = IMapsTSE::KIT_ELEPHANTGOLD; break;

    // Edge case
    default: eKey = IMapsTSE::KIT_KINGSTATUE; break;
  }
};

// Convert new blend modes
static INDEX ConvertBlendMode(INDEX iMode, INDEX iFirstNew, INDEX iFirstConverted) {
  // Toggle lights 1 and Toggle controlled lights 1
  if (iMode == iFirstNew + 0 || iMode == iFirstNew + 4) {
    return iFirstConverted + 0;
  }

  // Toggle lights 2 and Toggle controlled lights 2
  if (iMode == iFirstNew + 1 || iMode == iFirstNew + 5) {
    return iFirstConverted + 1;
  }

  // Toggle lights 3 and Toggle controlled lights 3
  if (iMode == iFirstNew + 2 || iMode == iFirstNew + 6) {
    return iFirstConverted + 2;
  }

  // Toggle lights 4 and Toggle controlled lights 4
  if (iMode == iFirstNew + 3 || iMode == iFirstNew + 7) {
    return iFirstConverted + 3;
  }

  return iMode;
};

// Convert brush entity
static void ConvertBrushEntity(CEntity *penBrush) {
  // Go through each polygon in each brush mip
  FOREACHINLIST(CBrushMip, bm_lnInBrush, penBrush->GetBrush()->br_lhBrushMips, itbm) {
    FOREACHINDYNAMICARRAY(itbm->bm_abscSectors, CBrushSector, itbsc) {
      FOREACHINSTATICARRAY(itbsc->bsc_abpoPolygons, CBrushPolygon, itbpo) {
        // Convert surface type
        UBYTE &ubSurface = itbpo->bpo_bppProperties.bpp_ubSurfaceType;

        switch (ubSurface) {
          case 22: // Sliding weeee slope
            ubSurface = 5; // Ice sliding slope
            break;

          case 23: // Gravel
            ubSurface = 0; // Standard
            break;

          case 24: // Gravel no impact
            ubSurface = 11; // Standard - no impact
            break;

          case 25: // Sand no impact
            ubSurface = SURFACE_SAND; // Sand
            break;

          case 26: // Red sand no impact
            ubSurface = SURFACE_RED_SAND; // Red sand
            break;

          case 27: // Wood no impact
            ubSurface = SURFACE_WOOD; // Wood
            break;

          case 28: // Snow no impact
            ubSurface = SURFACE_SNOW; // Snow
            break;
        }

        // Convert blend modes
        UBYTE &ub1 = itbpo->bpo_abptTextures[0].s.bpt_ubBlend;
        UBYTE &ub2 = itbpo->bpo_abptTextures[1].s.bpt_ubBlend;
        UBYTE &ub3 = itbpo->bpo_abptTextures[2].s.bpt_ubBlend;

        ub1 = ConvertBlendMode(ub1, 16, 11);
        ub2 = ConvertBlendMode(ub2, 16, 11);
        ub3 = ConvertBlendMode(ub3, 16, 11);
      }
    }
  }
};

// Convert one specific entity without reinitializing it
BOOL IConvertSSR::ConvertEntity(CEntity *pen) {
  // Replace plasma packs and mine packs
  if (IsOfClassID(pen, CAmmoItem_ClassID)) {
    // Retrieve CAmmoItem::m_EaitType
    static CPropertyPtr pptrType(pen);

    if (pptrType.ByVariable("CAmmoItem", "m_EaitType")) {
      INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

      switch (iType) {
        // Plasma packs
        case 12: iType = AIT_ELECTRICITY; break;

        // Mine packs
        case 13: iType = AIT_GRENADES; break;
      }
    }

    // Reinitialize
    return FALSE;

  // Replace new weapons
  } else if (IsOfClassID(pen, CWeaponItem_ClassID)) {
    // Retrieve CWeaponItem::m_EwitType
    static CPropertyPtr pptrType(pen);

    if (pptrType.ByVariable("CWeaponItem", "m_EwitType")) {
      INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

      switch (iType) {
        // Ghostbuster and Plasma Thrower
        case 13: case 14: iType = WIT_LASER; break;

        // Minelayer
        case 15: iType = WIT_GRENADELAUNCHER; break;
      }
    }

    // Reinitialize
    return FALSE;

  // Adjust weapon masks
  } else if (IsOfClassID(pen, CPlayerMarker_ClassID)) {
    // Retrieve CPlayerMarker::m_iGiveWeapons, CPlayerMarker::m_iTakeWeapons and CPlayerMarker::m_iTakeAmmo
    static CPropertyPtr pptrGive(pen);
    static CPropertyPtr pptrTake(pen);
    static CPropertyPtr pptrAmmo(pen);

    // Convert weapon flags
    if (pptrGive.ByVariable("CPlayerMarker", "m_iGiveWeapons")
     && pptrTake.ByVariable("CPlayerMarker", "m_iTakeWeapons"))
    {
      INDEX &iGiveWeapons = ENTITYPROPERTY(pen, pptrGive.Offset(), INDEX);
      INDEX &iTakeWeapons = ENTITYPROPERTY(pen, pptrTake.Offset(), INDEX);

      INDEX iNewGive = 0x03; // Knife and Colt
      INDEX iNewTake = 0;

      for (INDEX i = IMapsSSR::WEAPON_NONE + 1; i < IMapsSSR::WEAPON_LAST + 1; i++) {
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

    // Take new ammo types
    if (pptrAmmo.ByVariable("CPlayerMarker", "m_iTakeAmmo")) {
      INDEX &iTakeAmmo = ENTITYPROPERTY(pen, pptrAmmo.Offset(), INDEX);

      const INDEX iTakePlasma = (1 << 9);
      const INDEX iTakeMines = (1 << 10);

      if (iTakeAmmo & iTakePlasma) {
        iTakeAmmo |= (1 << AMMO_ELECTRICITY);
        iTakeAmmo &= ~iTakePlasma;
      }

      if (iTakeAmmo & iTakeMines) {
        iTakeAmmo |= (1 << AMMO_GRENADES);
        iTakeAmmo &= ~iTakeMines;
      }
    }

  // Adjust keys
  } else if (IsOfClassID(pen, CKeyItem_ClassID)) {
    // Retrieve CKeyItem::m_kitType
    static CPropertyPtr pptrType(pen);

    // Convert key type
    if (pptrType.ByVariable("CKeyItem", "m_kitType")) {
      ConvertKeyType(ENTITYPROPERTY(pen, pptrType.Offset(), INDEX));
    }

  // Adjust keys
  } else if (IsOfClassID(pen, CDoorController_ClassID)) {
    // Retrieve CDoorController::m_kitKey
    static CPropertyPtr pptrKey(pen);

    // Convert key type
    if (pptrKey.ByVariable("CDoorController", "m_kitKey")) {
      ConvertKeyType(ENTITYPROPERTY(pen, pptrKey.Offset(), INDEX));
    }

  // Adjust damage types
  } else if (IsOfClassID(pen, CDamager_ClassID)) {
    // Retrieve CDamager::m_dmtType
    static CPropertyPtr pptrType(pen);

    if (pptrType.ByVariable("CDamager", "m_dmtType")) {
      INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

      switch (iType) {
        case 20: // TommyGun
        case 21: // MiniGun
        case 22: // Single Shotgun
        case 23: // Double Shotgun
        case 24: // Sniper
        case 26: // Colt
        case 27: // Double Colt
          iType = DMT_BULLET;
          break;

        case 19: // Laser
        case 25: // Ghostbuster
          iType = DMT_PROJECTILE;
          break;

        case 28: // Rocket Launcher
        case 29: // Grenade Launcher
        case 31: // Mines
          iType = DMT_EXPLOSION;
          break;

        case 30: // Flamer
          iType = DMT_BURNING;
          break;
      }
    }

  // Make soundless sound holders silent
  } else if (IsOfClassID(pen, CSoundHolder_ClassID)) {
    // Retrieve CSoundHolder::m_fnSound and CSoundHolder::m_fVolume
    static CPropertyPtr pptrSound(pen);
    static CPropertyPtr pptrVolume(pen);

    if (pptrSound.ByVariable("CSoundHolder", "m_fnSound")) {
      CTFileName &fnmSound = ENTITYPROPERTY(pen, pptrSound.Offset(), CTFileName);

      if (fnmSound == "" || fnmSound == "Sounds\\Default.wav") {
        fnmSound = CTString("Sounds\\Misc\\Silence.wav");

        if (pptrVolume.ByVariable("CSoundHolder", "m_fVolume")) {
          ENTITYPROPERTY(pen, pptrVolume.Offset(), FLOAT) = 0.0f;
        }
      }
    }

  // Make soundless voice holders silent
  } else if (IsOfClassID(pen, CVoiceHolder_ClassID)) {
    // Retrieve CVoiceHolder::m_fnmMessage
    static CPropertyPtr pptrSound(pen);

    if (pptrSound.ByVariable("CVoiceHolder", "m_fnmMessage")) {
      CTFileName &fnmSound = ENTITYPROPERTY(pen, pptrSound.Offset(), CTFileName);

      if (fnmSound == "" || fnmSound == "Sounds\\Default.wav") {
        fnmSound = CTString("Sounds\\Misc\\Silence.wav");
      }
    }

  // Adjust world paths
  } else if (IsOfClassID(pen, CWorldLink_ClassID)) {
    // Retrieve CWorldLink::m_strWorld
    static CPropertyPtr pptrWorld(pen);

    // Convert mission pack paths
    if (pptrWorld.ByVariable("CWorldLink", "m_strWorld")) {
      CTFileNameNoDep &strWorld = ENTITYPROPERTY(pen, pptrWorld.Offset(), CTFileNameNoDep);
      IFiles::FixRevPath(strWorld);

      if (strWorld.RemovePrefix("Levels\\LevelsMP\\")) {
        strWorld = "Levels\\" + strWorld;
      }
    }

  // [Cecil] NOTE: Extra conversions for when custom mod is disabled
#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  } else if (!ClassicsCore_IsCustomModActive()) {
#else
  } else {
#endif
    // Adjust blend modes
    if (IsOfClassID(pen, CBlendController_ClassID)) {
      // Retrieve CBlendController::m_bctType
      static CPropertyPtr pptrType(pen);

      // Convert blend type
      if (pptrType.ByVariable("CBlendController", "m_bctType")) {
        INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);
        iType = ConvertBlendMode(iType, 7, BCT_ACTIVATE_PLATE_1);
      }

    // Replace new elemental types
    } else if (IsOfClassID(pen, CElemental_ClassID)) {
      // Reinitialize
      return FALSE;

    // Replace new projectile types
    } else if (IsOfClassID(pen, CEruptor_ClassID)) {
      // Retrieve CEruptor::m_ptType
      static CPropertyPtr pptrType(pen);

      if (pptrType.ByVariable("CEruptor", "m_ptType")) {
        INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

        switch (iType) {
          case 78: // Plasma
            iType = PRT_CYBORG_LASER;
            break;

          case 79: // Railbolt
          case 80: // Railbolt spray
            iType = PRT_ROCKET;
            break;

          case 30: // Waterman (old)
          case 33: // Waterman small
          case 83: // Airman small
          case 92: // Earthman small
            iType = PRT_LAVAMAN_STONE;
            break;

          case 31: // Waterman big
          case 82: // Airman big
          case 91: // Earthman big
            iType = PRT_LAVAMAN_BOMB;
            break;

          case 32: // Waterman large
          case 81: // Airman large
          case 90: // Earthman big bomb
            iType = PRT_LAVAMAN_BIG_BOMB;
            break;
        }
      }

    // Replace new headman types
    } else if (IsOfClassID(pen, CHeadman_ClassID)) {
      // Retrieve CHeadman::m_hdtType
      static CPropertyPtr pptrType(pen);

      if (pptrType.ByVariable("CHeadman", "m_hdtType")) {
        INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

        switch (iType) {
          case 4: iType = HDT_BOMBERMAN; break; // Commando
          case 5: iType = HDT_FIRECRACKER; break; // Vaporwave
        }
      }

      // Reinitialize
      return FALSE;

    // Replace new powerups
    } else if (IsOfClassID(pen, CPowerUpItem_ClassID)) {
      // Retrieve CPowerUpItem::m_puitType
      static CPropertyPtr pptrType(pen);

      if (pptrType.ByVariable("CPowerUpItem", "m_puitType")) {
        INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

        // Serious Jump
        if (iType == 5) iType = PUIT_SPEED;
      }

      // Reinitialize
      return FALSE;

    // Replace new walker types
    } else if (IsOfClassID(pen, CWalker_ClassID)) {
      // Retrieve CWalker::m_EwcChar
      static CPropertyPtr pptrType(pen);

      if (pptrType.ByVariable("CWalker", "m_EwcChar")) {
        INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);

        switch (iType) {
          case 2: iType = WLC_SERGEANT; break; // Artillery
          case 3: iType = WLC_SOLDIER; break; // Spawner
        }
      }

      // Reinitialize
      return FALSE;

    // Adjust brushes
    } else if (IsOfClassID(pen, CWorldBase_ClassID)) {
      ConvertBrushEntity(pen);
    }
  }

  // [Cecil] NOTE: This normally isn't required but since some Revolution enemies are being
  // replaced with TSE ones, all placed enemies need to be reinitialized to reset their models
#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  if (!ClassicsCore_IsCustomModActive())
#endif
  {
    return !IsDerivedFromID(pen, CEnemyBase_ClassID);
  }

  // Converted without having to reinitialize
  return TRUE;
};

// Convert the entire world with possible entity reinitializations
void IConvertSSR::ConvertWorld(CWorld *pwo) {
  CEntities cEntities;

  FOREACHINDYNAMICCONTAINER(pwo->wo_cenEntities, CEntity, iten) {
    // Convert specific entities
    if (ConvertEntity(iten)) continue;

    // Reinitialize entity
    cEntities.Add(iten);
  }

  // Destroy certain entities
  FOREACHINDYNAMICCONTAINER(cenDestroy, CEntity, itenDestroy) {
    cEntities.Remove(itenDestroy);
    itenDestroy->Destroy();
  }

  FOREACHINDYNAMICCONTAINER(cEntities, CEntity, itenReinit) {
    itenReinit->Reinitialize();
  }

  // Fix shadow issues
  CreateGlobalLight();
};
