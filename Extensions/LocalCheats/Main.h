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

extern CPluginSymbol _psAutoKill;
extern CPluginSymbol _psAutoKillRange;

// Container of local player entities
extern CDynamicContainer<CPlayerEntity> _cenPlayers;

// Iterate through each local player
#define FOREACHPLAYER(_PlayerIter) \
  _cenPlayers.Clear(); \
  IWorld::GetLocalPlayers(_cenPlayers); \
  FOREACHINDYNAMICCONTAINER(_cenPlayers, CPlayerEntity, _PlayerIter)

// Protection against running commands on someone else's server
#define SERVER_CLIENT_ONLY if (!_pNetwork->IsServer()) { \
  CPutString(TRANS("Local cheats can only be used in single player or on locally hosted servers!\n")); return; \
}
