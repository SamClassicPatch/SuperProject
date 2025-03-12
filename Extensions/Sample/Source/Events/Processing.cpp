/* Copyright (c) 2022-2024 Dreamy Cecil
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

// Define processing events for the plugin

void IProcessingEvents_OnStep(void)
{
  // This function is executed each simulation tick and is synchronized
  // with the simulation itself, just like regular entity logic.

  // EXAMPLE: Decrease health of each player if it's higher than 50 every second
  if (ULONG(_pTimer->CurrentTick() * 20) % 20 != 0) return;

  // Find all player entities
  CEntities cen;
  IWorld::FindClassesByID(IWorld::GetWorld()->wo_cenEntities, cen, CPlayerEntity_ClassID);

  FOREACHINDYNAMICCONTAINER(cen, CEntity, iten) {
    CPlayerEntity *pen = (CPlayerEntity *)&*iten;

    if (pen->GetHealth() <= 50) continue;

    #if _PATCHCONFIG_EXT_PACKETS
      // Send packet to change entity's health
      CExtEntityHealth pck;
      pck("ulEntity", (int)pen->en_ulID);
      pck("fHealth", pen->GetHealth() - 1);
      pck.SendToClients();

    #else
      // Change health directly
      pen->SetHealth(pen->GetHealth() - 1);
    #endif
  }
};

void IProcessingEvents_OnFrame(CDrawPort *pdp)
{
  // This function is executed after everything in the game is
  // drawn, so it can draw on top of the menu and the console.
};
