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

#define VANILLA_EVENTS_ENTITY_ID
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

// Define listener events for the plugin

void IListenerEvents_OnSendEvent(CEntity *pen, const CEntityEvent &ee)
{
  // This function is executed every time CEntity::SendEvent() is called

  // EXAMPLE: Heal a player a bit every time an enemy is killed
  if (!IsDerivedFromID(pen, CPlayerEntity_ClassID)) return;

  if (ee.ee_slEvent == EVENTCODE_VNL_EKilledEnemy) {
    VNL_EHealth eeHealth;
    eeHealth.fHealth = 10.0f;
    eeHealth.bOverTopHealth = TRUE;

    #if _PATCHCONFIG_EXT_PACKETS
      // Send packet to give item to an entity
      CExtEntityItem pck;
      pck("ulEntity", (int)pen->en_ulID);
      pck.SetEvent(eeHealth, sizeof(eeHealth));
      pck.SendToClients();

    #else
      // Give item directly
      pen->ReceiveItem(ee);
    #endif
  }
};

void IListenerEvents_OnReceiveItem(CEntity *penPlayer, const CEntityEvent &ee, BOOL bPickedUp)
{
  // This function is executed every time CPlayer::ReceiveItem() from the Entities library is called.
  // If it isn't defined in the Entities library for the Player class, this function is never called.
};

void IListenerEvents_OnCallProcedure(CEntity *pen, const CEntityEvent &ee)
{
  // This function is executed every time CRationalEntity::Call() is called,
  // which is happening during procedure logic defined by ES files.

  // EXAMPLE: Kick the player in the hit direction upon receiving damage from enemies
  if (!IsDerivedFromID(pen, CPlayerEntity_ClassID)) return;

  if (ee.ee_slEvent == EVENTCODE_EDamage) {
    const EDamage &eeDamage = (const EDamage &)ee;

    // CEnemyBase_ClassID
    if (IsDerivedFromID(eeDamage.penInflictor, 310)) {
      #if _PATCHCONFIG_EXT_PACKETS
        // Send packet to give impulse to an entity
        CExtEntityImpulse pck;
        pck("ulEntity", (int)pen->en_ulID);
        pck("vSpeed", eeDamage.vDirection * 20.0f);
        pck.SendToClients();

      #else
        // Give impulse directly
        ((CMovableEntity *)pen)->GiveImpulseTranslationAbsolute(eeDamage.vDirection * 20.0f);
      #endif
    }
  }
};
