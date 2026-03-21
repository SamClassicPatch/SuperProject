/* Copyright (c) 2026 Dreamy Cecil
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

BOOL INetworkEvents_OnServerPacket(CNetworkMessage &nmMessage, const ULONG ulType)
{
  // Don't use this function in custom scripts
  // Reenable this function in plugin startup before adding any code here
  return FALSE;
};

BOOL INetworkEvents_OnClientPacket(CNetworkMessage &nmMessage, const ULONG ulType)
{
  // Don't use this function in custom scripts
  // Reenable this function in plugin startup before adding any code here
  return FALSE;
};

static CPlayerEntity *_penPlayer = NULL;
static BOOL _bLocal;

inline SQRESULT PushPlayer(sq::VM &vm) {
  sq_pushroottable(vm);
  PushNewPointer(vm.Root(), "CEntityPointer", _penPlayer);
  sq_pushbool(vm, _bLocal);
  return SQ_OK;
};

void INetworkEvents_OnAddPlayer(CPlayerTarget &plt, BOOL bLocal) {
  _penPlayer = plt.plt_penPlayerEntity;
  _bLocal = bLocal;
  RunCustomScripts("OnAddPlayer", &PushPlayer);
};

void INetworkEvents_OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal) {
  _penPlayer = plt.plt_penPlayerEntity;
  _bLocal = bLocal;
  RunCustomScripts("OnRemovePlayer", &PushPlayer);
};
