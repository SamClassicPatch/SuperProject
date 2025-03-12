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

bool CExtEntityPosition::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  BOOL bRotation = props["bRotation"].IsTrue();
  nm.WriteBits(&bRotation, 1);

  const FLOAT3D &vSet = props["vSet"].GetVector();

  if (bRotation) {
    INetCompress::Angle3D(nm, vSet);
  } else {
    INetCompress::Float3D(nm, vSet);
  }

  BOOL bRelative = props["bRelative"].IsTrue();
  nm.WriteBits(&bRelative, 1);
  return true;
};

void CExtEntityPosition::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  BOOL bRotation = FALSE;
  nm.ReadBits(&bRotation, 1);
  props["bRotation"].GetIndex() = bRotation;

  FLOAT3D &vSet = props["vSet"].GetVector();

  if (bRotation) {
    INetDecompress::Angle3D(nm, vSet);
  } else {
    INetDecompress::Float3D(nm, vSet);
  }

  BOOL bRelative = FALSE;
  nm.ReadBits(&bRelative, 1);
  props["bRelative"].GetIndex() = bRelative;
};

void CExtEntityPosition::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CPlacement3D pl = pen->GetPlacement();

  const FLOAT3D &vSet = props["vSet"].GetVector();
  const BOOL bRotation = props["bRotation"].IsTrue();
  const BOOL bRelative = props["bRelative"].IsTrue();

  // Relative to absolute axes
  if (bRelative) {
    if (bRotation) {
      pl.pl_OrientationAngle += vSet;
    } else {
      pl.pl_PositionVector += vSet;
    }

  // Absolute position or rotation
  } else {
    if (bRotation) {
      pl.pl_OrientationAngle = vSet;
    } else {
      pl.pl_PositionVector = vSet;
    }
  }

  pen->Teleport(pl, FALSE);

  CAnyValue val(pl);
  ClassicsPackets_ServerReport(this, TRANS("Teleported %u entity to %s\n"), pen->en_ulID, val.ToString());
};

#endif // _PATCHCONFIG_EXT_PACKETS
