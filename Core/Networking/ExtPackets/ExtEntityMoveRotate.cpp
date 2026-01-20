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

#include "StdH.h"

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

bool CExtEntityMove::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  INetCompress::Float3D(nm, props["vSpeed"].GetVector());
  return true;
};

void CExtEntityMove::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  INetDecompress::Float3D(nm, props["vSpeed"].GetVector());
};

#define REPORT_NOT_MOVABLE TRANS("not a movable entity")

void CExtEntityMove::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromID(pen, CMovableEntity_ClassID)) {
    CAnyValue &val = props["vSpeed"];

    ((CMovableEntity *)pen)->SetDesiredTranslation(val.GetVector());
    ClassicsPackets_ServerReport(this, TRANS("Changed movement speed of %u entity to %s\n"), pen->en_ulID, val.ToString());

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Cannot change movement speed for %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

void CExtEntityRotate::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromID(pen, CMovableEntity_ClassID)) {
    CAnyValue &val = props["vSpeed"];

    ((CMovableEntity *)pen)->SetDesiredRotation(val.GetVector());
    ClassicsPackets_ServerReport(this, TRANS("Changed rotation speed of %u entity to %s\n"), pen->en_ulID, val.ToString());

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Cannot change rotation speed for %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

void CExtEntityImpulse::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  if (IsDerivedFromID(pen, CMovableEntity_ClassID)) {
    CAnyValue &val = props["vSpeed"];

    ((CMovableEntity *)pen)->GiveImpulseTranslationAbsolute(val.GetVector());
    ClassicsPackets_ServerReport(this, TRANS("Gave impulse to %u entity: %s\n"), pen->en_ulID, val.ToString());

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Cannot give impulse to %u entity: %s\n"), pen->en_ulID, REPORT_NOT_MOVABLE);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
