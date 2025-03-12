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

#include "StartActions.h"
#include "Sandbox.h"

#include <CoreLib/Networking/MessageProcessing.h>

#include <EntitiesV/StdH/StdH.h>
#include <EntitiesV/AmmoItem.h>
#include <EntitiesV/ArmorItem.h>
#include <EntitiesV/HealthItem.h>
#if SE1_GAME != SS_TFE
  #include <EntitiesV/PowerUpItem.h>
#endif
#include <EntitiesV/WeaponItem.h>
#include <EntitiesV/EnemySpawner.h>
#include <EntitiesV/PlayerMarker.h>

// Affect entities at the beginning of the game
static void AffectEntities(void) {
  FOREACHINDYNAMICCONTAINER(IWorld::GetWorld()->wo_cenEntities, CEntity, iten) {
    CEntity *pen = iten;

    if (IsOfClassID(pen, CWeaponItem_ClassID)) {
      AffectWeaponItem(pen);

    } else if (IsOfClassID(pen, CAmmoItem_ClassID)) {
      AffectAmmoItem(pen);

    } else if (IsOfClassID(pen, CHealthItem_ClassID)) {
      AffectHealthItem(pen);

    } else if (IsOfClassID(pen, CArmorItem_ClassID)) {
      AffectArmorItem(pen);

  #if SE1_GAME != SS_TFE
    } else if (IsOfClassID(pen, CPowerUpItem_ClassID)) {
      AffectPowerUpItem(pen);
  #endif

    } else if (IsOfClassID(pen, CPlayerMarker_ClassID)) {
      AffectPlayerMarker(pen);

    } else if (IsOfClassID(pen, CEnemySpawner_ClassID)) {
      AffectEnemySpawner(pen);
    }
  }
};

void IGameEvents_OnGameStart(void)
{
  AffectEntities();

  // Execute all scheduled commands
  CStringStack &astrCommands = IServerSandbox::astrScheduled;

  if (astrCommands.Count() > 0) {
    CPrintF(TRANS("Executing scheduled sandbox commands...\n"));

    CTString strExecute = "";

    for (INDEX iCommand = 0; iCommand < astrCommands.Count(); iCommand++) {
      strExecute += astrCommands[iCommand];
    }

    _pShell->Execute(strExecute);
  }
};

void IGameEvents_OnChangeLevel(void)
{
  // Affect entities on a new level (if singleplayer/split screen or patch-exclusive)
  const BOOL bLocal = !_pNetwork->IsNetworkEnabled();
  const BOOL bExclusive = (IProcessPacket::_bForbidVanilla || IProcessPacket::GameplayExtEnabled());

  if (_pNetwork->IsServer() && (bLocal || bExclusive)) {
    AffectEntities();
  }
};
