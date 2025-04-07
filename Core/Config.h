/* Copyright (c) 2023-2025 Dreamy Cecil
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

// [Cecil] NOTE: This file contains global switches for Classics Patch features that can be toggled
// in order to disable building of specific code. Useful for porting code to another engine (e.g. SSR or 1.50).

#ifndef CECIL_INCL_CLASSICSPATCH_CONFIG_H
#define CECIL_INCL_CLASSICSPATCH_CONFIG_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

//================================================================================================//
// Classics Patch constants
//
// These constants define information about the current release and project repository.
//================================================================================================//

// Current Classics Patch version
#define CORE_PATCH_VERSION MakeVersion(1, 9, 2)

// Indication of a specific pre-release build (set to 0 for full releases)
#define CORE_PRERELEASE_BUILD 0

// Suffix for custom binaries (a.k.a. mod extension)
#define CLASSICSPATCH_SUFFIX "_Custom"

// Dedicated directory name for the Classics Patch "mod"
#define CLASSICSPATCH_MODDIRNAME "ClassicsPatchMod"

// Relevant links to different webpages of the project
#define CLASSICSPATCH_URL_SHORT   "github.com/SamClassicPatch"
#define CLASSICSPATCH_URL_FULL    "https://" CLASSICSPATCH_URL_SHORT
#define CLASSICSPATCH_URL_PROJECT CLASSICSPATCH_URL_FULL "/SuperProject"

// URL to the latest patch release
#define CLASSICSPATCH_URL_LATESTRELEASE CLASSICSPATCH_URL_PROJECT "/releases/latest"

// URL to a patch release under a specific tag
#define CLASSICSPATCH_URL_TAGRELEASE(_TagName) (CTString(CLASSICSPATCH_URL_PROJECT "/releases/tag/") + _TagName)

// URL for the latest release request via HttpRequest()
#define CLASSICSPATCH_URL_HTTPREQUEST L"/repos/SamClassicPatch/SuperProject/releases/latest"

//================================================================================================//
// Classics Patch feature toggles
//
// These switches can be used to disable specific features of the patch either for compatibility,
// testing or any other reason. They can be toggled by switching '1' to '0' and vice versa.
//
// Some features can also be disabled automatically depending on the selected build configuration
// by modifying the macro value to rely on another macro switch, like it's done with Revolution
// and its automatic _APCT_NREV switch that's added to some of the features that aren't compatible
// with the Revolution engine.
//================================================================================================//

// [Cecil] TEMP: "APCT" - "Automatic Patch Config Toggle"
#define _APCT_NREV (SE1_GAME != SS_REV) // Feature should be disabled for Revolution
#define _APCT_N110 (SE1_VER != SE1_110 || !_APCT_NREV) // Feature should be disabled for 1.10 forks (but not Revolution)

// API
#define _PATCHCONFIG_STEAM_API (1 && _APCT_NREV) // Enable interactions with Steam

// Custom mod
#define _PATCHCONFIG_CUSTOM_MOD          (1) // Allow usage of custom mod libraries (Entities, Game & GameGUI)
#define _PATCHCONFIG_CUSTOM_MOD_ENTITIES (1) // Allow usage of the custom Entities library (only valid when _PATCHCONFIG_CUSTOM_MOD is enabled!)

// Definitions
#define _PATCHCONFIG_CLASS_DEFINITIONS (1 && _APCT_N110) // Define classes that aren't exported from the engine
#define _PATCHCONFIG_FUNC_DEFINITIONS  (1 && _APCT_N110) // Define methods that aren't exported from the engine

// Networking
#define _PATCHCONFIG_EXT_PACKETS  (1 && _APCT_NREV) // Support for custom extension packets
#define _PATCHCONFIG_GAMEPLAY_EXT (1 && _APCT_NREV) // Implement gameplay extensions
#define _PATCHCONFIG_GUID_MASKING (1 && _APCT_NREV) // System for masking GUIDs of player characters separately for each server client

// Query
#define _PATCHCONFIG_NEW_QUERY (1 && _APCT_NREV && _APCT_N110) // Utilize new query manager and switch master servers

// Engine patches
#define _PATCHCONFIG_ENGINEPATCHES (1) // Patch any engine functions at all or not

// Patch modules
#define _PATCHCONFIG_EXTEND_ENTITIES   (1) // Extend entities functionality by patching their methods
#define _PATCHCONFIG_ENTITY_FORCE      (1) // Hook force methods of some vanilla entities for modifying the gravity
#define _PATCHCONFIG_EXTEND_FILESYSTEM (1) // Extend file system functionality by patching its methods
#define _PATCHCONFIG_EXTEND_NETWORK    (1) // Extend networking functionality by patching its methods
#define _PATCHCONFIG_FIX_RENDERING     (1) // Fix FOV and other rendering issues by patching methods
#define _PATCHCONFIG_FIX_SKA           (1 && _APCT_NREV && _APCT_N110) // Fix SKA issues by patching methods
#define _PATCHCONFIG_FIX_STRINGS       (1) // Fix CTString methods by patching them
#define _PATCHCONFIG_EXTEND_TEXTURES   (1 && _APCT_N110) // Extend texture functionality by patching its methods
#define _PATCHCONFIG_FIX_LOGICTIMERS   (1 && _APCT_NREV) // Fix imprecise timers for entity logic
#define _PATCHCONFIG_FIX_STREAMPAGING  (1 && _APCT_NREV && _APCT_N110) // Fix streams by patching their paging methods (no need for 1.10)

#endif
