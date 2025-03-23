/* Copyright (c) 2022-2025 Dreamy Cecil
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

// Define extension
CLASSICSPATCH_DEFINE_EXTENSION("PATCH_TEST_sample", k_EPluginFlagEngine | k_EPluginFlagGame, MakeVersion(1, 0, 0),
  "Dreamy Cecil", "Extension Sample", "This is an example extension that comes with the Serious Sam Classics Patch source code.");

// Define handle to this extension, which is then accessible via EXTENSIONMODULE_LOCALHANDLE
CLASSICSPATCH_DEFINE_EXTENSION_HANDLE;

// Define some extension properties
CLASSICSPATCH_EXTENSION_PROPS_BEGIN {
  ExtensionProp_t("packets",      true), // For toggling received packet counter
  ExtensionProp_t("tps",          20), // Ticks per second for decreasing health each second
  ExtensionProp_t("kill_health",  10.0f), // Health to receive for each enemy kill
  ExtensionProp_t("player_model", (const char *)NULL), // Player model name to enforce on all clients
} CLASSICSPATCH_EXTENSION_PROPS_END;

static int PATCH_CALLTYPE ListSignal(void *);
static int PATCH_CALLTYPE MultiplySignal(void *piNumber);
static int PATCH_CALLTYPE RandomHealthSignal(void *);

// Define some extension signals
CLASSICSPATCH_EXTENSION_SIGNALS_BEGIN {
  { "List",         &ListSignal },         // Arg ptr : NULL
  { "Multiply",     &MultiplySignal },     // Arg ptr : int
  { "RandomHealth", &RandomHealthSignal }, // Arg ptr : NULL
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// List available extension signals
int ListSignal(void *) {
  CPutString("Available extension signals:\n");

  for (size_t i = 0; i < EXTENSIONMODULE_SIGNALCOUNT; i++) {
    CPrintF("- %s\n", EXTENSIONMODULE_SIGNALARRAY[i].m_strSignal);
  }

  return 0;
};

// Multiply specified number by 4 and print the result
int MultiplySignal(void *piNumber) {
  if (piNumber == NULL) {
    CPutString("No number specified\n");
    return FALSE;
  }

  int iNumber = *(int *)piNumber;
  CPrintF("%d x 4 = %d\n", iNumber, iNumber * 4);
  return TRUE;
};

// Randomize health of local players
int RandomHealthSignal(void *) {
  if (!_pNetwork->IsServer()) {
    CPutString("Only the server may execute this signal!\n");
    return -1;
  }

  int iRnd = (rand() % 10000) + 1;

  CPlayerEntities cenPlayers;
  IWorld::GetLocalPlayers(cenPlayers);

  FOREACHINDYNAMICCONTAINER(cenPlayers, CPlayerEntity, iten) {
    #if _PATCHCONFIG_EXT_PACKETS
      // Send packet to change entity's health
      CExtEntityHealth pck;
      pck("ulEntity", (int)iten->en_ulID);
      pck("fHealth", (FLOAT)iRnd);
      pck.SendToClients();

    #else
      // Change health directly
      iten->SetHealth(iRnd);
    #endif
  }

  // Return set health
  return iRnd;
};

static void SampleSetNumber(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strProp = *NEXT_ARG(CTString *);
  const FLOAT fValue = NEXT_ARG(FLOAT);

  // Find any property under this name and try to set a number value to it
  ExtensionProp_t *pProp = ClassicsExtensions_FindProperty(EXTENSIONMODULE_LOCALHANDLE, strProp);

  if (pProp != NULL) {
    switch (pProp->m_eType) {
      case ExtensionProp_t::k_EType_Bool:   pProp->m_value._bool = !!fValue; return;
      case ExtensionProp_t::k_EType_Int:    pProp->m_value._int = fValue; return;
      case ExtensionProp_t::k_EType_Float:  pProp->m_value._float = fValue; return;
      case ExtensionProp_t::k_EType_Double: pProp->m_value._double = fValue; return;
    }
  }

  CPrintF("No number property found under the name '%s'!\n", strProp.str_String);
};

static void SampleSetString(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strProp = *NEXT_ARG(CTString *);
  const CTString &strValue = *NEXT_ARG(CTString *);

  // Find a string property under this name and replace the pointer to an array of characters in it
  ExtensionProp_t *pProp = ClassicsExtensions_FindPropertyOfType(EXTENSIONMODULE_LOCALHANDLE, strProp, ExtensionProp_t::k_EType_String);

  if (pProp != NULL) {
    // Destroy the last string
    if (pProp->m_value._string != NULL) {
      FreeMemory((char *)pProp->m_value._string);
      pProp->m_value._string = NULL;
    }

    // Create a copy of the string
    char *strCopy = (char *)AllocMemory(strValue.Length() + 1);
    strcpy(strCopy, strValue.str_String);

    pProp->m_value._string = strCopy;
    return;
  }

  CPrintF("No string property found under the name '%s'!\n", strProp.str_String);
};

static void SampleCallSignal(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strSignal = *NEXT_ARG(CTString *);
  int iOpt = NEXT_ARG(INDEX);

  FExtensionSignal pSignal = ClassicsExtensions_FindSignal(EXTENSIONMODULE_LOCALHANDLE, strSignal.str_String);

  if (pSignal != NULL) {
    int iResult = pSignal(&iOpt);
    CPrintF("\nSignal result: %d\n", iResult);

  } else {
    CPrintF("'%s' signal doesn't exist!\n", strSignal.str_String);
  }
};

// Dummy chat command
static BOOL DummyChatCommand(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  strResult.PrintF("Dummy command arguments: '%s'", strArguments);
  return TRUE;
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
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

  // Add custom shell functions for modifying extension properties and calling extension signals for testing
  ClassicsPlugins()->RegisterMethod(TRUE, "void", "SampleSetNumber", "CTString, FLOAT",    &SampleSetNumber);
  ClassicsPlugins()->RegisterMethod(TRUE, "void", "SampleSetString", "CTString, CTString", &SampleSetString);
  ClassicsPlugins()->RegisterMethod(TRUE, "void", "SampleCallSignal", "CTString, INDEX",   &SampleCallSignal);

  // Add custom chat command
  ClassicsChat_RegisterCommand("dummy", &DummyChatCommand);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
  // Remove custom chat command
  ClassicsChat_UnregisterCommand("dummy");
};
