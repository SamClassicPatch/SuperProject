/* Copyright (c) 2024 Dreamy Cecil
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

// Include some base vanilla entities
#if SE1_GAME == SS_TFE
  #include <EntitiesTFE/EntitiesAPI.h>
  #include <EntitiesTFE/Enemies/EnemyBase.h>
  #include <EntitiesTFE/Enemies/EnemyDive.h>
  #include <EntitiesTFE/Enemies/EnemyFly.h>
  #include <EntitiesTFE/Items/Item.h>
#else
  #include <EntitiesTSE/EntitiesAPI.h>
  #include <EntitiesTSE/Enemies/EnemyBase.h>
  #include <EntitiesTSE/Enemies/EnemyDive.h>
  #include <EntitiesTSE/Enemies/EnemyFly.h>
  #include <EntitiesTSE/Items/Item.h>
#endif

#undef DECL_DLL

#ifndef EXTRAENTITIES_EXPORTS
  // Link appropriate library
  #ifdef NDEBUG
    #pragma comment(lib, "ClassicsExtras" CLASSICSPATCH_SUFFIX ".lib")
  #else
    #pragma comment(lib, "ClassicsExtras" CLASSICSPATCH_SUFFIX "D.lib")
  #endif

  #define DECL_DLL __declspec(dllimport)

#else
  #define DECL_DLL __declspec(dllexport)
#endif
