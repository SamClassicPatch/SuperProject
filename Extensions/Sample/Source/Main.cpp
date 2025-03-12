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

// Define plugin
CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagEngine | k_EPluginFlagGame, MakeVersion(1, 0, 0),
  "Dreamy Cecil", "Example Plugin", "This is an example plugin that comes with the source code for Serious Sam Classics Patch.");

// Dummy chat command
static BOOL DummyChatCommand(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  strResult.PrintF("Dummy command arguments: '%s'", strArguments);
  return TRUE;
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(CIniConfig &props, PluginEvents_t &events)
{
  // Register plugin events
  events.m_processing->OnStep  = &IProcessingEvents_OnStep;
  events.m_processing->OnFrame = &IProcessingEvents_OnFrame;

  events.m_rendering->OnPreDraw    = &IRenderingEvents_OnPreDraw;
  events.m_rendering->OnPostDraw   = &IRenderingEvents_OnPostDraw;
  events.m_rendering->OnRenderView = &IRenderingEvents_OnRenderView;

  events.m_network->OnServerPacket = &INetworkEvents_OnServerPacket;
  events.m_network->OnClientPacket = &INetworkEvents_OnClientPacket;
  events.m_network->OnAddPlayer    = &INetworkEvents_OnAddPlayer;
  events.m_network->OnRemovePlayer = &INetworkEvents_OnRemovePlayer;

  events.m_packet->OnCharacterConnect = &IPacketEvents_OnCharacterConnect;
  events.m_packet->OnCharacterChange  = &IPacketEvents_OnCharacterChange;
  events.m_packet->OnPlayerAction     = &IPacketEvents_OnPlayerAction;
  events.m_packet->OnChatMessage      = &IPacketEvents_OnChatMessage;

  events.m_game->OnGameStart   = &IGameEvents_OnGameStart;
  events.m_game->OnChangeLevel = &IGameEvents_OnChangeLevel;
  events.m_game->OnGameStop    = &IGameEvents_OnGameStop;
  events.m_game->OnGameSave    = &IGameEvents_OnGameSave;
  events.m_game->OnGameLoad    = &IGameEvents_OnGameLoad;

  events.m_demo->OnDemoPlay  = &IDemoEvents_OnDemoPlay;
  events.m_demo->OnDemoStart = &IDemoEvents_OnDemoStart;
  events.m_demo->OnDemoStop  = &IDemoEvents_OnDemoStop;

  events.m_world->OnWorldLoad = &IWorldEvents_OnWorldLoad;

  events.m_listener->OnSendEvent     = &IListenerEvents_OnSendEvent;
  events.m_listener->OnReceiveItem   = &IListenerEvents_OnReceiveItem;
  events.m_listener->OnCallProcedure = &IListenerEvents_OnCallProcedure;

  events.m_timer->OnTick   = &ITimerEvents_OnTick;
  events.m_timer->OnSecond = &ITimerEvents_OnSecond;

  // Add custom chat command
  ClassicsChat_RegisterCommand("dummy", &DummyChatCommand);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(CIniConfig &props)
{
  // Remove custom chat command
  ClassicsChat_UnregisterCommand("dummy");
};
