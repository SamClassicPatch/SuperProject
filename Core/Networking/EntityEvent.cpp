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

#include "EntityEvent.h"

#define VANILLA_EVENTS_ENTITY_ID
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

// Copy event bytes (iEventSize = sizeof(ee))
void EExtEntityEvent::SetEvent(const CEntityEvent &ee, size_t iEventSize) {
  ASSERT(iEventSize <= sizeof(CEntityEvent) + sizeof(ee_aulFields));
  Reset();

  // Copy event type
  ee_slEvent = ee.ee_slEvent;

  // Skip bytes before event data
  const size_t iSkip = offsetof(CEntityEvent, ee_slEvent) + sizeof(ee.ee_slEvent);
  iEventSize -= iSkip;

  // No data to copy
  if (iEventSize == 0) return;

  // Count 4-byte fields
  ee_ctFields = (iEventSize + 3) / 4;

  const void *pEventData = ((const UBYTE *)&ee) + iSkip;
  memcpy(ee_aulFields, pEventData, iEventSize);
};

// Write event into a network packet
void EExtEntityEvent::Write(CNetworkMessage &nm) {
  nm << ee_slEvent;

  // Write data
  UBYTE ubData = (ee_ctFields != 0);
  nm.WriteBits(&ubData, 1);

  if (ubData) {
    // Fit 64 fields by writing the 0-63 range
    ubData = UBYTE(ee_ctFields - 1);
    nm.WriteBits(&ubData, 6);

    nm.Write(&ee_aulFields[0], ee_ctFields * sizeof(ULONG));
  }
};

// Read event from a network packet
void EExtEntityEvent::Read(CNetworkMessage &nm) {
  Reset();
  nm >> ee_slEvent;

  // Read data
  UBYTE ubData = 0;
  nm.ReadBits(&ubData, 1);

  if (ubData) {
    // Interpret read size as being in the 1-64 range
    nm.ReadBits(&ee_ctFields, 6);
    ee_ctFields++;

    nm.Read(&ee_aulFields[0], ee_ctFields * sizeof(ULONG));

    // Convert fields of a specific event after reading them
    ConvertTypes();
  }
};

// Convert fields according to the event type
void EExtEntityEvent::ConvertTypes(void)
{
  // Convert entity IDs into pointers
  switch (ee_slEvent) {
    // First field is an entity
    case EVENTCODE_VNL_EStart:
    case EVENTCODE_VNL_ETrigger:
    case EVENTCODE_VNL_EAirShockwave:
    case EVENTCODE_VNL_EAirWave:
    case EVENTCODE_VNL_EBulletInit:
    case EVENTCODE_VNL_ELaunchCannonBall:
    case EVENTCODE_VNL_ELaunchLarvaOffspring:
    case EVENTCODE_VNL_EAnimatorInit:
    case EVENTCODE_VNL_EWeaponsInit:
    case EVENTCODE_VNL_EWeaponEffectInit:
    case EVENTCODE_VNL_ELaunchProjectile:
    case EVENTCODE_VNL_EReminderInit:
    case EVENTCODE_VNL_ESeriousBomb:
    case EVENTCODE_VNL_EWatcherInit:
    case EVENTCODE_VNL_EWater: {
      VNL_EStart &ee = (VNL_EStart &)*this;
      ee.penCaused = EntityFromID(0);
    } break;

    // Second field is an entity
    case EVENTCODE_VNL_ESound:
    case EVENTCODE_VNL_EScroll:
    case EVENTCODE_VNL_ETextFX:
    case EVENTCODE_VNL_EHudPicFX:
    case EVENTCODE_VNL_ECredits: {
      VNL_ESound &ee = (VNL_ESound &)*this;
      ee.penTarget = EntityFromID(1);
    } break;

    // Two first fields are entities
    case EVENTCODE_VNL_EDevilProjectile:
    case EVENTCODE_VNL_EFlame:
    case EVENTCODE_VNL_EViewInit:
    case EVENTCODE_VNL_ESpinnerInit: {
      VNL_EFlame &ee = (VNL_EFlame &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penAttach = EntityFromID(1);
    } break;

    // Same ID as ETwister
    case EVENTCODE_VNL_ESpawnerProjectile: {
      VNL_ESpawnerProjectile &ee = (VNL_ESpawnerProjectile &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penTemplate = MaybeEntity(1); // Preserves 'ETwister::fSize'
    } break;

    // Skips: sptType, fDamagePower, fSizeMultiplier, vDirection
    case EVENTCODE_VNL_ESpawnSpray: {
      VNL_ESpawnSpray &ee = (VNL_ESpawnSpray &)*this;
      ee.penOwner = EntityFromID(6);
    } break;

    case EVENTCODE_VNL_ESpawnDebris: {
      VNL_ESpawnDebris &ee = (VNL_ESpawnDebris &)*this;
      ee.penFallFXPapa = EntityFromID(22);

      if (ee.penFallFXPapa != NULL) {
        CEntity *pen = (CEntity *)ee.penFallFXPapa;

        ee.pmd = pen->GetModelObject()->GetData(); // 1
        ee.ptd = (CTextureData *)pen->GetModelObject()->mo_toTexture.GetData(); // 3
        ee.ptdRefl = (CTextureData *)pen->GetModelObject()->mo_toReflection.GetData(); // 4
        ee.ptdSpec = (CTextureData *)pen->GetModelObject()->mo_toSpecular.GetData(); // 5
        ee.ptdBump = (CTextureData *)pen->GetModelObject()->mo_toBump.GetData(); // 6
      }
    } break;

    // Skips: eetType, vDamageDir, vDestination, tmLifeTime, fSize, ctCount
    case EVENTCODE_VNL_ESpawnEffector: {
      VNL_ESpawnEffector &ee = (VNL_ESpawnEffector &)*this;
      ee.penModel  = EntityFromID(10);
      ee.penModel2 = EntityFromID(11);
    } break;
  }
};
