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

// Holder for event fields
class CORE_API EExtEntityEvent : public CEntityEvent {
  public:
    // Accommodate for multiple fields of varying data
    ULONG ee_aulFields[64];

    ULONG ee_ctFields; // Amount of used fields

  public:
    EExtEntityEvent() : CEntityEvent(EVENTCODE_EVoid) {
      Reset();
    };

    CEntityEvent *MakeCopy(void) {
      EExtEntityEvent *pCopy = new EExtEntityEvent;
      pCopy->Copy(*this);
      return pCopy;
    };

  public:
    // Reset event fields
    inline void Reset(void) {
      ee_slEvent = EVENTCODE_EVoid;
      ee_ctFields = 0;

      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      memset(ee_aulFields, 0, sizeof(ee_aulFields));
    };

    // Copy event bytes (iEventSize = sizeof(ee))
    // If some event has CEntityPointer fields, they need to contain entity IDs as 4-byte integers instead
    // of pointers to entities directly. When setting entity IDs for CEntityPointer fields, do it like this:
    //   (ULONG &)ee.pen = iEntityID;
    // DO NOT FORGET to do this at the end of the function to avoid crashes upon calling the pointer destructor:
    //   (ULONG &)ee.pen = NULL;
    void SetEvent(const CEntityEvent &ee, size_t iEventSize);

    // Copy event data from another event
    inline void Copy(const EExtEntityEvent &eeOther) {
      ee_slEvent = eeOther.ee_slEvent;
      ee_ctFields = eeOther.ee_ctFields;
      memcpy(ee_aulFields, eeOther.ee_aulFields, sizeof(ee_aulFields));
    };

  private:
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
    // Write event into a network packet
    void Write(CNetworkMessage &nm);

    // Read event from a network packet
    void Read(CNetworkMessage &nm);
};

#endif
