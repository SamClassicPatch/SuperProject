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

bool CExtEntityHealth::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Float(nm, props["fHealth"].GetFloat());
  return true;
};

void CExtEntityHealth::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Float(nm, props["fHealth"].GetFloat());
};

void CExtEntityHealth::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsLiveEntity(pen)) {
    FLOAT fHealth = props["fHealth"].GetFloat();

    ((CLiveEntity *)pen)->SetHealth(fHealth);
    ClassicsPackets_ServerReport(this, TRANS("Set health of %u entity to %.2f\n"), pen->en_ulID, fHealth);

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Cannot set health for %u entity: not a live entity\n"), pen->en_ulID);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
