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

// Define networking events for the plugin

INDEX _ctPacketsReceived = 0;

BOOL INetworkEvents_OnServerPacket(CNetworkMessage &nmMessage, const ULONG ulType)
{
  // This function is executed every time a server receives a packet of a new PCK_EXTENSION type with
  // a specific type that can be handled by the plugin. The packet data starts with the first byte after
  // the type integer that's already being read by INetDecompress::Integer() and passed as 'ulType'.

  // If this function returns TRUE, it means that this plugin has handled this extension packet and
  // it won't be processed by other plugins down the list.
  // If all plugins return FALSE for this type, an error about an invalid packet is shown in console.
  return FALSE;
};

BOOL INetworkEvents_OnClientPacket(CNetworkMessage &nmMessage, const ULONG ulType)
{
  // This function is executed every time a client receives a packet of a new PCK_EXTENSION type with
  // a specific type that can be handled by the plugin. The packet data starts with the first byte after
  // the type integer that's already being read by INetDecompress::Integer() and passed as 'ulType'.

  // If this function returns TRUE, it means that this plugin has handled this extension packet and
  // it won't be processed by other plugins down the list.
  // If all plugins return FALSE for this type, an error about an invalid packet is shown in console.

  // EXAMPLE: Count all extension packets sent by the server
  _ctPacketsReceived++;

  return FALSE;
};

void INetworkEvents_OnAddPlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Executed each time a new client joins the game and adds their player entities
  // 'bLocal' is TRUE only if it's a player of a local (connecting) client
};

void INetworkEvents_OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Executed each time a client leaves the game and removes their player entities
  // 'bLocal' is TRUE only if it's a player of a local (disconnecting) client
};
