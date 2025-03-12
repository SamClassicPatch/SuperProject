/* Copyright (c) 2023-2024 Dreamy Cecil
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

bool CExtEntityParent::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  ULONG ulParent = props["ulParent"].GetIndex();
  INetCompress::Integer(nm, ulParent);
  return true;
};

void CExtEntityParent::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  ULONG ulParent;
  INetDecompress::Integer(nm, ulParent);
  props["ulParent"].GetIndex() = ulParent;
};

void CExtEntityParent::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  const ULONG ulParent = props["ulParent"].GetIndex();
  CEntity *penParent = FindExtEntity(ulParent);

  pen->SetParent(penParent);

  if (pen->GetParent() == NULL) {
    ClassicsPackets_ServerReport(this, TRANS("Unparented %u entity\n"), pen->en_ulID);

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Parented %u entity to %u\n"), pen->en_ulID, penParent->en_ulID);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
