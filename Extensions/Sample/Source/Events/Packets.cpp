/* Copyright (c) 2024 Dreamy Cecil
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

// Define packet events for the plugin

void IPacketEvents_OnCharacterConnect(INDEX iClient, CPlayerCharacter &pc)
{
  // This function is executed every time a new player connects to the server with a specific character
  // that can be modified here in order to force specific customization onto them (e.g. name or skin).

  // EXAMPLE: Force all players to have Boxer Barry skin
  CPlayerSettings *pps = (CPlayerSettings *)pc.pc_aubAppearance;
  strncpy(pps->ps_achModelFile, "BoxerBarry", sizeof(pps->ps_achModelFile));
};

BOOL IPacketEvents_OnCharacterChange(INDEX iClient, INDEX iPlayer, CPlayerCharacter &pc)
{
  // This function is executed every time a player wishes to change their player character mid-game
  // that can be modified here in order to force specific customization onto them (e.g. name or skin).

  // If it returns FALSE, all changes are discarded and the new player character is ignored
  // instead of being changed, which can be used to forbid character customization mid-game.

  // EXAMPLE: Force all players to have Boxer Barry skin
  CPlayerSettings *pps = (CPlayerSettings *)pc.pc_aubAppearance;
  strncpy(pps->ps_achModelFile, "BoxerBarry", sizeof(pps->ps_achModelFile));

  return TRUE;
};

void IPacketEvents_OnPlayerAction(INDEX iClient, INDEX iPlayer, CPlayerAction &pa, INDEX iResent)
{
  // This function is executed every time a player sends new button actions to be applied
  // that can be modified here in order to force a specific action (e.g. knife selection)
  // or restrict certain actions (e.g. toggle third person view if it's currently on).
};

BOOL IPacketEvents_OnChatMessage(INDEX iClient, ULONG ulFrom, ULONG ulTo, CTString &strMessage)
{
  // This function is executed every time the server receives a new chat message, addressed from certain
  // players to other players. It can be used to modify the message or be parsed as a specific chat command,
  // though this is discouraged in favor of the built-in system using IChatCommands::Register().

  // If it returns FALSE, it means that the message has been processed as some command
  // and it shouldn't be displayed in chat as a regular message for all clients to see.
  return TRUE;
};
