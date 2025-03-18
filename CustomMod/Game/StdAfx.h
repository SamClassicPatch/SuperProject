/* Copyright (c) 2002-2012 Croteam Ltd. 
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

// [Cecil] Include the core library and patches
#define CORE_NO_GAME_HEADER
#include <Core/Core.h>

// [Cecil] Custom mod is disabled
#if !_PATCHCONFIG_CUSTOM_MOD
  #error Custom mod has been disabled in this build!
#endif

#include "Game.h"

// [Cecil] Include new features
#include "Cecil/GameColors.h"
#include "Cecil/GameThemes.h"

// [Cecil] Link vanilla Entities if custom mod ones are disabled
#if !_PATCHCONFIG_CUSTOM_MOD || !_PATCHCONFIG_CUSTOM_MOD_ENTITIES
  #include <EntitiesV/StdH/StdH.h>
  #include <EntitiesV/Player.h>

  #ifdef NDEBUG
    #pragma comment(lib, "EntitiesV.lib")
  #else
    #pragma comment(lib, "EntitiesVD.lib")
  #endif

// [Cecil] Include common header for Entities
#elif SE1_GAME != SS_TFE
  #include <EntitiesTSE/EntitiesAPI.h>
  #include <EntitiesTSE/Players/Player.h>
#else
  #include <EntitiesTFE/EntitiesAPI.h>
  #include <EntitiesTFE/Players/Player.h>
#endif
