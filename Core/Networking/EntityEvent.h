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

#ifndef CECIL_INCL_EXTENTITYEVENT_H
#define CECIL_INCL_EXTENTITYEVENT_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// If this amount is changed, make sure the amount of used fields is written into packets
// properly because it is only using 6 bits for the counter (1-64 used fields) right now!
#define EXT_ENTITY_EVENT_FIELDS 64

enum EExtEventFieldType {
  EXTEF_NONE = -1, // Internal type; DON'T USE!

  EXTEF_NUMERIC = 0, // 4-byte data as is: integers, floats, bools, vector axes etc.
  EXTEF_ENTITY  = 1, // Direct pointer to an entity in the world (serialized as entity ID)
  EXTEF_STRING  = 2, // Pointer to a raw string of characters that needs to be manually freed on destruction
};

// Container of field types to pass into EExtEntityEvent::SetEvent()
struct ExtEventFields {
  EExtEventFieldType aeTypes[EXT_ENTITY_EVENT_FIELDS];

  // Constructor for no fields
  __forceinline ExtEventFields() {
    for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
      aeTypes[i] = EXTEF_NONE;
    }
  };

  // Constructor for the first 15 fields for convenience
  __forceinline ExtEventFields(
    EExtEventFieldType e1              , EExtEventFieldType e2  = EXTEF_NONE, EExtEventFieldType e3  = EXTEF_NONE,
    EExtEventFieldType e4  = EXTEF_NONE, EExtEventFieldType e5  = EXTEF_NONE, EExtEventFieldType e6  = EXTEF_NONE,
    EExtEventFieldType e7  = EXTEF_NONE, EExtEventFieldType e8  = EXTEF_NONE, EExtEventFieldType e9  = EXTEF_NONE,
    EExtEventFieldType e10 = EXTEF_NONE, EExtEventFieldType e11 = EXTEF_NONE, EExtEventFieldType e12 = EXTEF_NONE,
    EExtEventFieldType e13 = EXTEF_NONE, EExtEventFieldType e14 = EXTEF_NONE, EExtEventFieldType e15 = EXTEF_NONE)
  {
    for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
      aeTypes[i] = EXTEF_NONE;
    }

    if (e1  != EXTEF_NONE) aeTypes[ 0] = e1;
    if (e2  != EXTEF_NONE) aeTypes[ 1] = e2;
    if (e3  != EXTEF_NONE) aeTypes[ 2] = e3;
    if (e4  != EXTEF_NONE) aeTypes[ 3] = e4;
    if (e5  != EXTEF_NONE) aeTypes[ 4] = e5;
    if (e6  != EXTEF_NONE) aeTypes[ 5] = e6;
    if (e7  != EXTEF_NONE) aeTypes[ 6] = e7;
    if (e8  != EXTEF_NONE) aeTypes[ 7] = e8;
    if (e9  != EXTEF_NONE) aeTypes[ 8] = e9;
    if (e10 != EXTEF_NONE) aeTypes[ 9] = e10;
    if (e11 != EXTEF_NONE) aeTypes[10] = e11;
    if (e12 != EXTEF_NONE) aeTypes[11] = e12;
    if (e13 != EXTEF_NONE) aeTypes[12] = e13;
    if (e14 != EXTEF_NONE) aeTypes[13] = e14;
    if (e15 != EXTEF_NONE) aeTypes[14] = e15;
  };

  EExtEventFieldType &operator[](int i) { return aeTypes[i]; };
};

// Holder for event fields
class CORE_API EExtEntityEvent : public CEntityEvent {
  private:
    // Accommodate for multiple fields of varying data
    ULONG ee_aulFields[EXT_ENTITY_EVENT_FIELDS];

    // Data contained within each field
    EExtEventFieldType ee_aeFieldType[EXT_ENTITY_EVENT_FIELDS];

  public:
    ULONG ee_ctFields; // Amount of used fields

  public:
    EExtEntityEvent() : CEntityEvent(EVENTCODE_EVoid), ee_ctFields(0) {
      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
        ee_aulFields[i] = 0;
        ee_aeFieldType[i] = EXTEF_NUMERIC;
      }
    };

    ~EExtEntityEvent() {
      Free();
    };

    CEntityEvent *MakeCopy(void) {
      EExtEntityEvent *pCopy = new EExtEntityEvent;
      pCopy->Copy(*this);
      return pCopy;
    };

  private:
    // Free internal memory used by a specific field
    inline void FreeField(INDEX i) {
      // Free string fields
      if (ee_aeFieldType[i] == EXTEF_STRING) {
        char *str = reinterpret_cast<char *>(ee_aulFields[i]);
        FreeMemory(str);
      }
    };

    // Free internal memory used by the event
    inline void Free(void) {
      for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
        FreeField(i);
      }
    };

    // Convert entity ID into a pointer
    inline ULONG EntityFromID(INDEX i) {
      CEntity *pen = IWorld::FindEntityByID(IWorld::GetWorld(), ee_aulFields[i]);
      return reinterpret_cast<ULONG>(pen);
    };

    // Convert entity ID into a pointer, if possible
    inline ULONG MaybeEntity(INDEX i) {
      // Return entity if found any
      ULONG ulPtr = EntityFromID(i);
      if (ulPtr != NULL) return ulPtr;

      // Return value as is
      return ee_aulFields[i];
    };

    // Convert fields according to the event type
    void ConvertTypes(void);

  public:
    // Reset event fields
    inline void Reset(void) {
      Free(); // Free previous memory
      ee_slEvent = EVENTCODE_EVoid;
      ee_ctFields = 0;

      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      for (INDEX i = 0; i < EXT_ENTITY_EVENT_FIELDS; i++) {
        ee_aulFields[i] = 0;
        ee_aeFieldType[i] = EXTEF_NUMERIC;
      }
    };

    // Copy event bytes
    // aFields should contain types for each field within the event that need to be copied over:
    // - EXTEF_NUMERIC should be used for 4-byte integers, floats and numeric containers (e.g. FLOAT3D that needs three types in a row instead of just one)
    // - EXTEF_ENTITY should be used for entity pointers, i.e. only CEntityPointer or CEntity * (and NEVER entity IDs!!!)
    // - EXTEF_STRING should be used for CTString fields. CTFileName are fine too but only if they're followed by E_NUMERIC (for an extra field inside the class)
    void SetEvent(const CEntityEvent &ee, const ExtEventFields &fields = ExtEventFields());

    // Copy event data from another event
    void Copy(const EExtEntityEvent &eeOther);

    // Write event into a network packet
    void Write(CNetworkMessage &nm);

    // Read event from a network packet
    void Read(CNetworkMessage &nm);

  // Manual data setup
  public:

    // Set integer field
    // Returns maximum amount of used event fields
    ULONG SetInt(INDEX i, ULONG iValue);

    // Set float field
    // Returns maximum amount of used event fields
    ULONG SetFloat(INDEX i, FLOAT fValue);

    // Set vector field
    // Returns maximum amount of used event fields
    ULONG SetVector(INDEX i, const FLOAT3D &vValue);

    // Set entity field
    // Returns maximum amount of used event fields
    ULONG SetEntity(INDEX i, CEntity *penValue);

    // Set string field
    // Returns maximum amount of used event fields
    ULONG SetString(INDEX i, const char *strValue);
};

#endif
