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

// Custom initialization for other tools
void ClassicsPatch_InitTools(void) {
  // Initialize the core
  ClassicsPatch_Init();

  // Function patches
  CPutString("--- Tools: Intercepting Engine functions ---\n");
  {
    extern void ClassicsPatch_ModelRenderPatches(void);
    ClassicsPatch_ModelRenderPatches();
  }
  CPutString("--- Done! ---\n");

  // Load needed plugins
  GetPluginAPI()->LoadPlugins(k_EPluginFlagTools);
};

// Original function pointer
static CGame *(*pGameCreateFunc)(void) = NULL;

// Patched GAME_Create() method
static CGame *P_GameCreate(void) {
  // Call original method and retrieve a pointer to created CGame
  _pGame = (*pGameCreateFunc)();

  // Hook default fields
  GetGameAPI()->HookFields();

  // Load needed plugins after the Game library
  GetPluginAPI()->LoadPlugins(k_EPluginFlagEditor);

  // Return it
  return _pGame;
};

// Custom initialization for Serious Editor
void ClassicsPatch_InitEditor(void) {
  // Initialize the core
  ClassicsPatch_Init();

#if _PATCHCONFIG_ENGINEPATCHES

  // Function patches
  CPutString("--- Editor: Intercepting Engine functions ---\n");
  _EnginePatches.CorePatches();
  CPutString("--- Done! ---\n");

#endif // _PATCHCONFIG_ENGINEPATCHES

  // Load Game library in advance
  CPluginModule *pGameLib = GetPluginAPI()->LoadGamePlugin();

  // Patch game creation method to avoid creation of multiple instances of CGame
  pGameLib->GetSymbol_t(&pGameCreateFunc, "GAME_Create");
  CreatePatch(pGameCreateFunc, &P_GameCreate, "GAME_Create()");
};
