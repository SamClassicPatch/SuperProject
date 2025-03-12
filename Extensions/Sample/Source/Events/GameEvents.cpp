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

// Define game events for the plugin

void IGameEvents_OnGameStart(void)
{
  // This function is executed after starting the simulation for the first time after loading
  // a level but before adding any of the players (even local server ones). It can be used to
  // safely modify the level as if it was already played on a little bit and even vanilla clients
  // will see the changes after connecting to the server (e.g. item replacements).

  // This only works on the very first level started by the game/server and won't work after
  // changing levels. For that, you have to either use IGameEvents::OnChangeLevel() or replace
  // both of these functions with code inside IWorldEvents::OnWorldLoad(). But NEITHER of these
  // methods will work for clients that don't have this plugin downloaded and installed locally,
  // unless you decide to send built-in extension packets (commands prefixed with "pck_").

  // EXAMPLE: Find all beheads and replace them with kamikazes
  CEntities cenHeadmen;
  IWorld::FindClassesByID(IWorld::GetWorld()->wo_cenEntities, cenHeadmen, 303); // CHeadman_ClassID

  FOREACHINDYNAMICCONTAINER(cenHeadmen, CEntity, iten) {
    CEntity *pen = iten;

    // Find property offset within CHeadman class by name
    static CPropertyPtr pptrType(pen);

    if (pptrType.ByName(CEntityProperty::EPT_ENUM, "Type"))
    {
      ENTITYPROPERTY(pen, pptrType.Offset(), INDEX) = 3;
      iten->Reinitialize();
    }
  }
};

void IGameEvents_OnChangeLevel(void)
{
  // This function is executed after loading the next level
};

void IGameEvents_OnGameStop(void)
{
  // This function is executed after stopping the game simulation
};

void IGameEvents_OnGameSave(const CTFileName &fnmSave)
{
  // This function is executed right after making a save file
};

void IGameEvents_OnGameLoad(const CTFileName &fnmSave)
{
  // This function is executed right after loading a save file
};
