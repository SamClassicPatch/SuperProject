/* Copyright (c) 2024-2026 Dreamy Cecil
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

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

// [Cecil] NOTE: Assume that NET_MAXSESSIONPROPERTIES is 2048
#define SESPROPS_BITFIT 11

// Set new data at the current end and expand session properties size
BOOL CExtSessionProps::AddData(const void *pData, size_t ctBytes) {
  INDEX &ctSize = GetSize();

  // Not enough space
  if (ctSize + ctBytes > sizeof(sp)) return FALSE;

  // Copy data to the current offset and add its size
  memcpy(sp + ctSize, pData, ctBytes);
  ctSize += ctBytes;
  return TRUE;
};

bool CExtSessionProps::Write(CNetworkMessage &nm) {
  // From first to last byte
  INDEX iOffset = Clamp(GetOffset(), (INDEX)0, INDEX(sizeof(sp) - 1));
  nm.WriteBits(&iOffset, SESPROPS_BITFIT);

  // Fit as much data as there's space after offset
  INDEX ctSize = Clamp(GetSize(), (INDEX)0, INDEX(sizeof(sp) - iOffset));
  nm.WriteBits(&ctSize, SESPROPS_BITFIT);

  for (INDEX i = 0; i < ctSize; i++) {
    nm << sp[i];
  }

  return true;
};

void CExtSessionProps::Read(CNetworkMessage &nm) {
  INDEX iOffset = 0;
  nm.ReadBits(&iOffset, SESPROPS_BITFIT);

  INDEX ctSize = 0;
  nm.ReadBits(&ctSize, SESPROPS_BITFIT);

  for (INDEX i = 0; i < ctSize; i++) {
    nm >> sp[i];
  }

  GetSize() = ctSize;
  GetOffset() = iOffset;
};

void CExtSessionProps::Process(void) {
  INDEX ctSize = GetSize();
  INDEX iOffset = GetOffset();

  // No data or past the limits
  if (ctSize == 0 || iOffset < 0 || iOffset >= sizeof(sp)) return;

  // Fit as much data as there's space after offset
  ctSize = Clamp(ctSize, (SLONG)0, SLONG(sizeof(sp) - iOffset));

  // Copy received data from the beginning into current session properties with some offset
  if (ctSize != 0) {
    UBYTE *pubProps = (UBYTE *)_pNetwork->GetSessionProperties();
    memcpy(pubProps + iOffset, sp, ctSize);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
