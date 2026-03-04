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

bool CExtEntityEvent::Write(CNetworkMessage &nm) {
  WriteEntity(nm);
  eEvent.Write(nm);
  return true;
};

void CExtEntityEvent::Read(CNetworkMessage &nm) {
  ReadEntity(nm);
  eEvent.Read(nm);
};

void CExtEntityEvent::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->SendEvent(eEvent);
};

void CExtEntityItem::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  pen->ReceiveItem(eEvent);
};

#endif // _PATCHCONFIG_EXT_PACKETS
