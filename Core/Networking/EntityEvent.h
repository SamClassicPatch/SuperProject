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
    ULONG aulFields[64];

  public:
    EExtEntityEvent() : CEntityEvent(EVENTCODE_EVoid) {
      Reset();
    };

    CEntityEvent *MakeCopy(void) {
      return new EExtEntityEvent(*this);
    };

  public:
    // Reset event fields
    inline void Reset(void) {
      ee_slEvent = EVENTCODE_EVoid;

      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      memset(aulFields, 0, sizeof(aulFields));
    };

    // Copy event data from another event
    inline void Copy(const EExtEntityEvent &eeOther) {
      ee_slEvent = eeOther.ee_slEvent;
      memcpy(aulFields, eeOther.aulFields, sizeof(aulFields));
    };

    // Convert entity ID into a pointer
    inline ULONG EntityFromID(INDEX i) {
      return (ULONG)IWorld::FindEntityByID(IWorld::GetWorld(), aulFields[i]);
    };

    // Convert entity ID into a pointer, if possible
    inline ULONG MaybeEntity(INDEX i) {
      // Return entity if found any
      ULONG ulPtr = EntityFromID(i);
      if (ulPtr != NULL) return ulPtr;

      // Return value as is
      return aulFields[i];
    };

    // Write event into a network packet
    void Write(CNetworkMessage &nm, ULONG ctFields);

    // Read event from a network packet
    ULONG Read(CNetworkMessage &nm);

    // Convert fields according to the event type
    void ConvertTypes(void);
};

#endif
