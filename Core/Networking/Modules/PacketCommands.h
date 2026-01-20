/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_PACKETCOMMANDS_H
#define CECIL_INCL_PACKETCOMMANDS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_EXT_PACKETS

// Interface for commands for sending extension packets
namespace IPacketCommands {

// Event setup
void SetupEvent(SHELL_FUNC_ARGS);
void EventFieldIndex(SHELL_FUNC_ARGS);
void EventFieldFloat(SHELL_FUNC_ARGS);
void EventFieldVector(SHELL_FUNC_ARGS);

// Entity instantiation
void EntityCreate(SHELL_FUNC_ARGS);
void EntityDelete(SHELL_FUNC_ARGS);
void EntityCopy(SHELL_FUNC_ARGS);

// Entity logic
void EntityEvent(SHELL_FUNC_ARGS);
void EntityInit(SHELL_FUNC_ARGS);
void EntityInitEvent(SHELL_FUNC_ARGS);
void EntityItem(SHELL_FUNC_ARGS);

// Entity placement
void EntityTeleport(SHELL_FUNC_ARGS);
void EntitySetPos(SHELL_FUNC_ARGS);
void EntitySetRot(SHELL_FUNC_ARGS);

// Entity properties
void EntityParent(SHELL_FUNC_ARGS);
void EntityNumberProp(SHELL_FUNC_ARGS);
void EntityStringProp(SHELL_FUNC_ARGS);
void EntityHealth(SHELL_FUNC_ARGS);

// Entity flags
void EntityFlags(SHELL_FUNC_ARGS);
void EntityPhysicalFlags(SHELL_FUNC_ARGS);
void EntityCollisionFlags(SHELL_FUNC_ARGS);

// Entity movement
void EntityMove(SHELL_FUNC_ARGS);
void EntityRotate(SHELL_FUNC_ARGS);
void EntityImpulse(SHELL_FUNC_ARGS);

// Entity damage
void SetupDamage(SHELL_FUNC_ARGS);
void SetDirectDamage(SHELL_FUNC_ARGS);
void SetRangeDamage(SHELL_FUNC_ARGS);
void SetBoxDamage(SHELL_FUNC_ARGS);

void EntityDamage(void);

// World change
void ChangeLevel(SHELL_FUNC_ARGS);
void ChangeWorld(SHELL_FUNC_ARGS);

// Session properties
void StartSesProps(SHELL_FUNC_ARGS);
void SesPropIndex(SHELL_FUNC_ARGS);
void SesPropFloat(SHELL_FUNC_ARGS);
void SesPropString(SHELL_FUNC_ARGS);
void SeekSesProp(SHELL_FUNC_ARGS);
void SendSesProps(void);

// Gameplay extensions
void GameplayExtNumber(SHELL_FUNC_ARGS);
void GameplayExtString(SHELL_FUNC_ARGS);

// Sound playback
void PlayQuickSound(SHELL_FUNC_ARGS);
void PlayGlobalSound(SHELL_FUNC_ARGS);

}; // namespace

#endif // _PATCHCONFIG_EXT_PACKETS

#endif
