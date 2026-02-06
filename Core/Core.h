/* Copyright (c) 2022-2026 Dreamy Cecil
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

// Don't warn about identifier truncation
#pragma warning(disable: 4786)

// Main engine components
#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>

// Classics Patch API
#include <API/include/classicspatch_api.h>

// Dummy variables for Revolution support
#if SE1_GAME == SS_REV
  static CTFileName _fnmCDPath = CTString("");
  static CTFileName _fnmMod = CTString("");
#endif

// Extras
#include <Extras/XGizmo/Base/IniConfig.h>
#include <Extras/XGizmo/Interfaces/Data.h>
#include <Extras/XGizmo/Interfaces/Directories.h>
#include <Extras/XGizmo/Interfaces/Files.h>
#include <Extras/XGizmo/Interfaces/Libraries.h>
#include <Extras/XGizmo/Interfaces/Properties.h>
#include <Extras/XGizmo/Interfaces/Render.h>
#include <Extras/XGizmo/Interfaces/World.h>
#include <Extras/XGizmo/Objects/AnyValue.h>
#include <Extras/XGizmo/Objects/MapStructure.h>
#include <Extras/XGizmo/Objects/Node.h>
#include <Extras/XGizmo/Objects/StructPtr.h>
#include <Extras/XGizmo/Objects/SymbolPtr.h>

// Classics Patch configuration
#include <Core/Config.h>
#include <Core/GameSpecific.h>

// Check if "fusion mode" is available (only for TSE)
#define CLASSIC_TSE_FUSION_MODE (SE1_GAME == SS_TSE)

#if SE1_VER != SE1_110
  // Declare shell function arguments
  #define SHELL_FUNC_ARGS void *__pFirstArg

  // Begin shell function code
  #define BEGIN_SHELL_FUNC void *pArgs = (void *)&__pFirstArg

#else
  // Declare shell function arguments
  #define SHELL_FUNC_ARGS void *pArgs

  // Begin shell function code
  #define BEGIN_SHELL_FUNC NOTHING
#endif

// Next argument in the symbol function call
#define NEXT_ARG(Type) (*((Type *&)pArgs)++)

// Translate a string that has already been translated in vanilla localizations
#define LOCALIZE(ConstString) ((char *)TranslateConst(ConstString, 0))

// Import library for use
#ifndef CORELIB_EXPORTS
  #pragma comment(lib, "ClassicsCore.lib")
  #pragma comment(lib, "Ws2_32.lib")

  #define CORE_API __declspec(dllimport)
#else
  #define CORE_API __declspec(dllexport)
#endif

// C++ API
#include <Core/API/IChat.h>
#include <Core/API/IConfig.h>
#include <Core/API/ICore.h>
#include <Core/API/IGame.h>
#include <Core/API/IHooks.h>
#include <Core/API/IPatches.h>
#include <Core/API/IPlugins.h>
#include <Core/API/ISteam.h>

// Common components
#include <Core/Base/GameDirectories.h>
#include <Core/Objects/PropertyPtr.h>
