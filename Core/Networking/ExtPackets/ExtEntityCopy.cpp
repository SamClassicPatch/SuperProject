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

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

bool CExtEntityCopy::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  INDEX iCopies = props["iCopies"].GetIndex();
  nm.WriteBits(&iCopies, 5); // Up to 31
  return true;
};

void CExtEntityCopy::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  INDEX iCopies = 0;
  nm.ReadBits(&iCopies, 5);
  props["iCopies"].GetIndex() = iCopies;
};

void CExtEntityCopy::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CTString strReport(0, TRANS("Copied %u entity: "), pen->en_ulID);
  const INDEX iCopies = props["iCopies"].GetIndex();

  for (INDEX i = 0; i < iCopies; i++) {
    // Update last created entity
    CExtEntityCreate::penLast = IWorld::GetWorld()->CopyEntityInWorld(*pen, pen->GetPlacement(), TRUE);

    strReport += CTString(0, (i == 0) ? "%u" : ", %u", CExtEntityCreate::penLast->en_ulID);
  }

  ClassicsPackets_ServerReport(this, "%s\n", strReport.str_String);
};

#endif // _PATCHCONFIG_EXT_PACKETS
