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

inline void CopyStringIntoField(ULONG &ulField, const char *strValue) {
  size_t ctLen = strlen(strValue);
  char *strField = (char *)AllocMemory(ctLen + 1);
  strcpy(strField, strValue);

  ulField = reinterpret_cast<ULONG>(strField);
};

// Copy event bytes
void EExtEntityEvent::SetEvent(const CEntityEvent &ee, const ExtEventFields &fields) {
  Reset();

  // Copy event type
  ee_slEvent = ee.ee_slEvent;

  // No data to copy
  if (fields.aeTypes[0] == EXTEF_NONE) return;

  // Skip bytes before event data
  const size_t iSkip = offsetof(CEntityEvent, ee_slEvent) + sizeof(ee.ee_slEvent);

  // Copy all event data as generic number fields
  const ULONG *pEventData = reinterpret_cast<const ULONG *>(((const UBYTE *)&ee) + iSkip);

  for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
    // No more fields to copy
    if (fields.aeTypes[i] == EXTEF_NONE) break;

    switch (fields.aeTypes[i]) {
      case EXTEF_ENTITY: {
        CEntity *pen = reinterpret_cast<CEntity *>(pEventData[i]);
        SetEntity(i, pen);
      } break;

      case EXTEF_STRING: {
        const char *str = reinterpret_cast<const char *>(pEventData[i]);
        SetString(i, str);
      } break;

      default: SetInt(i, pEventData[i]); break;
    }
  }
};

// Copy event data from another event
void EExtEntityEvent::Copy(const EExtEntityEvent &eeOther) {
  Reset();

  ee_slEvent = eeOther.ee_slEvent;
  ee_ctFields = eeOther.ee_ctFields;

  for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
    ee_aeFieldType[i] = eeOther.ee_aeFieldType[i];

    // Copy strings
    if (ee_aeFieldType[i] == EXTEF_STRING) {
      const char *str = reinterpret_cast<const char *>(eeOther.ee_aulFields[i]);
      CopyStringIntoField(ee_aulFields[i], str);

    // Copy data as is
    } else {
      ee_aulFields[i] = eeOther.ee_aulFields[i];
    }
  }
};

ULONG EExtEntityEvent::SetInt(INDEX i, ULONG iValue) {
  ASSERT(i >= 0 && i < EXT_ENTITY_EVENT_FIELDS);
  FreeField(i);

  ee_aulFields[i] = iValue;
  ee_aeFieldType[i] = EXTEF_NUMERIC;

  ee_ctFields = Max(ee_ctFields, ULONG(i + 1));
  return ee_ctFields;
};

ULONG EExtEntityEvent::SetFloat(INDEX i, FLOAT fValue) {
  ASSERT(i >= 0 && i < EXT_ENTITY_EVENT_FIELDS);
  FreeField(i);

  ee_aulFields[i] = reinterpret_cast<ULONG &>(fValue);
  ee_aeFieldType[i] = EXTEF_NUMERIC;

  ee_ctFields = Max(ee_ctFields, ULONG(i + 1));
  return ee_ctFields;
};

ULONG EExtEntityEvent::SetVector(INDEX i, const FLOAT3D &vValue) {
  ASSERT(i >= 0 && i < EXT_ENTITY_EVENT_FIELDS - 2);
  FreeField(i + 0);
  FreeField(i + 1);
  FreeField(i + 2);

  FLOAT3D &vField = reinterpret_cast<FLOAT3D &>(ee_aulFields[i]);
  vField = vValue;
  ee_aeFieldType[i + 0] = EXTEF_NUMERIC;
  ee_aeFieldType[i + 1] = EXTEF_NUMERIC;
  ee_aeFieldType[i + 2] = EXTEF_NUMERIC;

  ee_ctFields = Max(ee_ctFields, ULONG(i + 3));
  return ee_ctFields;
};

ULONG EExtEntityEvent::SetEntity(INDEX i, CEntity *penValue) {
  ASSERT(i >= 0 && i < EXT_ENTITY_EVENT_FIELDS);
  FreeField(i);

  ee_aulFields[i] = reinterpret_cast<ULONG>(penValue);
  ee_aeFieldType[i] = EXTEF_ENTITY;

  ee_ctFields = Max(ee_ctFields, ULONG(i + 1));
  return ee_ctFields;
};

ULONG EExtEntityEvent::SetString(INDEX i, const char *strValue) {
  ASSERT(i >= 0 && i < EXT_ENTITY_EVENT_FIELDS);
  FreeField(i);

  CopyStringIntoField(ee_aulFields[i], strValue);
  ee_aeFieldType[i] = EXTEF_STRING;

  ee_ctFields = Max(ee_ctFields, ULONG(i + 1));
  return ee_ctFields;
};

// Write event into a network packet
void EExtEntityEvent::Write(CNetworkMessage &nm) {
  nm << SLONG(0xFFFFFFFF); // Magic number for new event data
  nm << ee_slEvent;

  // Write data
  UBYTE ubData = (ee_ctFields != 0);
  nm.WriteBits(&ubData, 1);

  if (!ubData) return;

  // Fit 64 fields by writing the 0-63 range
  ubData = UBYTE(ee_ctFields - 1);
  nm.WriteBits(&ubData, 6);

  INDEX i;

  // Write field types first for space optimization
  for (i = 0; i < ee_ctFields; i++) {
    nm.WriteBits(&ee_aeFieldType[i], 2);
  }

  // Then write data according to the types
  for (i = 0; i < ee_ctFields; i++) {
    switch (ee_aeFieldType[i]) {
      // Write entity ID instead of a pointer
      case EXTEF_ENTITY: {
        CEntity *pen = reinterpret_cast<CEntity *>(ee_aulFields[i]);
        nm << ULONG(pen != NULL ? pen->en_ulID : 0);
      } break;

      // Write string
      case EXTEF_STRING: {
        char *str = reinterpret_cast<char *>(ee_aulFields[i]);
        nm << CTString(str);
      } break;

      // Write as is
      default: nm << ee_aulFields[i]; break;
    }
  }
};

// Read event from a network packet
void EExtEntityEvent::Read(CNetworkMessage &nm) {
  Reset();

  SLONG slMagic;
  nm >> slMagic;

  UBYTE ubData = 0;

  // Legacy data
  if (slMagic != 0xFFFFFFFF) {
    ee_slEvent = slMagic;

    // Read data
    nm.ReadBits(&ubData, 1);
    if (!ubData) return;

    // Interpret read size as being in the 1-64 range
    nm.ReadBits(&ee_ctFields, 6);
    ee_ctFields++;

    nm.Read(&ee_aulFields[0], ee_ctFields * sizeof(ULONG));

    // Convert fields of a specific event after reading them
    ConvertTypes();
    return;
  }

  // New data
  nm >> ee_slEvent;

  // Read data
  nm.ReadBits(&ubData, 1);
  if (!ubData) return;

  // Interpret read size as being in the 1-64 range
  nm.ReadBits(&ee_ctFields, 6);
  ee_ctFields++;

  INDEX i;

  // Read field types first
  for (i = 0; i < ee_ctFields; i++) {
    nm.ReadBits(&ee_aeFieldType[i], 2);
  }

  // Then read data according to the types
  for (i = 0; i < ee_ctFields; i++) {
    switch (ee_aeFieldType[i]) {
      // Read entity ID and convert it into a pointer
      case EXTEF_ENTITY: {
        nm >> ee_aulFields[i];
        ee_aulFields[i] = EntityFromID(i);
      } break;

      // Read string
      case EXTEF_STRING: {
        CTString str;
        nm >> str;
        CopyStringIntoField(ee_aulFields[i], str.str_String);
      } break;

      // Read as is
      default: nm >> ee_aulFields[i]; break;
    }
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
