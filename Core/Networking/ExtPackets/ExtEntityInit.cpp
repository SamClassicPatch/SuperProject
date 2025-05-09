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

void CExtEntityInit::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  // Use received event for initialization instead of sending
  if (pen->GetRenderType() == CEntity::RT_NONE) {
    pen->Initialize(eEvent);
    ClassicsPackets_ServerReport(this, TRANS("Initialized %u entity\n"), pen->en_ulID);

  } else {
    // Not using CEntity::Reinitialize() because it doesn't take initialization events
    pen->End_internal();
    pen->Initialize_internal(eEvent);

    ClassicsPackets_ServerReport(this, TRANS("Reinitialized %u entity\n"), pen->en_ulID);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
