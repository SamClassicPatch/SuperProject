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

#include <Extras/XGizmo/Vanilla/EntityEvents.h>

#if _PATCHCONFIG_EXT_PACKETS

bool CExtChangeLevel::Write(CNetworkMessage &nm) {
  CTString &strWorld = props["strWorld"].GetString();

  // Store up to 255 characters
  UBYTE ct = (UBYTE)ClampUp(strWorld.Length(), (INDEX)255);
  nm << ct;

  if (ct != 0) {
    nm.Write(strWorld.str_String, ct);
    return true;
  }

  return false;
};

void CExtChangeLevel::Read(CNetworkMessage &nm) {
  CTString &strWorld = props["strWorld"].GetString();
  strWorld = "";

  UBYTE ct;
  nm >> ct;

  if (ct != 0) {
    // Read characters into a nullified buffer
    char strRead[256] = { 0 };
    nm.Read(strRead, ct);
    strWorld = strRead;
  }
};

void CExtChangeLevel::Process(void) {
  const CTString &strWorld = props["strWorld"].GetString();

  if (!FileExists(strWorld)) {
    ClassicsPackets_ServerReport(this, TRANS("Cannot change world to '%s': World file does not exist\n"), strWorld);
    return;
  }

  try {
    CEntity *pen = IWorld::GetWorld()->CreateEntity_t(CPlacement3D(), CTFILENAME("Classes\\WorldLink.ecl"));

    // Retrieve CWorldLink::m_strWorld and CWorldLink::m_EwltType
    static CPropertyPtr pptrWorld(pen);
    static CPropertyPtr pptrType(pen);

    // Change type
    if (pptrType.ByVariable("CWorldLink", "m_EwltType")) {
      INDEX &iType = ENTITYPROPERTY(pen, pptrType.Offset(), INDEX);
      iType = 1; // WLT_FIXED
    }

    // Set world and change to it
    if (pptrWorld.ByVariable("CWorldLink", "m_strWorld")) {
      CTFileNameNoDep &strProp = ENTITYPROPERTY(pen, pptrWorld.Offset(), CTFileNameNoDep);
      strProp = strWorld;

      pen->Initialize();
      pen->SendEvent(VNL_ETrigger());

    // Discard entity
    } else {
      pen->Initialize();
      pen->Destroy();
    }

  } catch (char *strError) {
    ClassicsPackets_ServerReport(this, TRANS("Cannot load %s class:\n%s"), "WorldLink", strError);
  }
};

void CExtChangeWorld::Process(void) {
  const CTString &strWorld = props["strWorld"].GetString();

  if (!FileExists(strWorld)) {
    ClassicsPackets_ServerReport(this, TRANS("Cannot change world to '%s': World file does not exist\n"), strWorld);
    return;
  }

  _pNetwork->ChangeLevel(strWorld, FALSE, 0);
};

#endif // _PATCHCONFIG_EXT_PACKETS
