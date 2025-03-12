/* Copyright (c) 2022-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_CLIENTIDENTITY_H
#define CECIL_INCL_CLIENTIDENTITY_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ClientLogging.h"

// Clear method for CDynamicStackArray
inline void Clear(CPlayerCharacter &pc) {
  pc = CPlayerCharacter();
};

// A unique client
class CORE_API CClientIdentity {
  public:
    // Addresses the client has played from (if detected the same characters)
    CDynamicStackArray<SClientAddress> aAddresses;

    // Characters the client has played as (playing in split screen or from the same addresses)
    CDynamicStackArray<CPlayerCharacter> aCharacters;

  public:
    // Find address index of this client
    INDEX FindAddress(const SClientAddress &addr) const;

    // Find character index of this client
    INDEX FindCharacter(const CPlayerCharacter &pc) const;

    // Add a new character to the client
    BOOL AddNewCharacter(const CPlayerCharacter &pc);

  public:
    // Clear method for CDynamicStackArray
    inline void Clear(void) {
      aAddresses.Clear();
      aCharacters.Clear();
    };

    // Write client identity data
    void Write(CTStream *strm);

    // Read client identity data
    void Read(CTStream *strm);
};

// Identities of every logged client
CORE_API extern CDynamicStackArray<CClientIdentity> _aClientIdentities;

#endif
