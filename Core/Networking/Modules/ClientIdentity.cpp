/* Copyright (c) 2022-2026 Dreamy Cecil
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

#include "ClientIdentity.h"

// Identities of every logged client
CDynamicStackArray<CClientIdentity> _aClientIdentities;

// Find address index of some client
INDEX CClientIdentity::FindAddress(const SClientAddress &addr) const {
  const INDEX ctAddresses = aAddresses.Count();

  // Go through the client's addresses
  for (INDEX i = 0; i < ctAddresses; i++) {
    // Found matching address
    if (aAddresses[i] == addr) {
      return i;
    }
  }

  // Client doesn't have such address
  return -1;
};

// Find character index of some client
INDEX CClientIdentity::FindCharacter(const CPlayerCharacter &pc) const {
  const INDEX ctChars = aCharacters.Count();

  // Go through the client's characters
  for (INDEX i = 0; i < ctChars; i++) {
    // Found matching character
    if (aCharacters[i] == pc) {
      return i;
    }
  }

  // Client doesn't have such character
  return -1;
};

// Add a new character to the client
BOOL CClientIdentity::AddNewCharacter(const CPlayerCharacter &pc) {
  // Find this character
  INDEX iChar = FindCharacter(pc);

  // Add it if it's not found
  if (iChar == -1) {
    aCharacters.Push() = pc;

    // Added a new character
    return TRUE;
  }

  // Character already exists
  return FALSE;
};

// Write client identity data
void CClientIdentity::Write(CTStream *strm) {
  INDEX i, ct;

  // Addresses
  ct = aAddresses.Count();
  *strm << ct;

  for (i = 0; i < ct; i++) {
    aAddresses[i].Write(strm);
  }

  // Characters
  ct = aCharacters.Count();
  *strm << ct;

  for (i = 0; i < ct; i++) {
    *strm << aCharacters[i];
  }
};

// Read client identity data
void CClientIdentity::Read(CTStream *strm) {
  INDEX i, ct;

  // Addresses
  *strm >> ct;

  if (ct != 0) {
    SClientAddress *aNewAddresses = aAddresses.Push(ct);

    for (i = 0; i < ct; i++) {
      aNewAddresses[i].Read(strm);
    }
  }

  // Characters
  *strm >> ct;

  if (ct != 0) {
    CPlayerCharacter *aNewChars = aCharacters.Push(ct);

    for (i = 0; i < ct; i++) {
      *strm >> aNewChars[i];
    }
  }
};
