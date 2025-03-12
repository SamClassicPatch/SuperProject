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

// Define world events for the plugin

void IWorldEvents_OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld)
{
  // Any changes to the world here will affect its calculated CRC and the default game state, meaning
  // that even if you change something before anyone connects, like via IGameEvents::OnGameStart(),
  // the clients still won't be able to connect and will receive a CRC or DIFF error upon trying to do so.

  // On the contrary, if all clients have the plugin that changes the world via this method, it will be
  // executed every time a new world is loaded (upon connecting or switching levels) and its changes will
  // stay, compared to IGameEvents::OnGameStart(), which only affects the world that the server started on.
};
