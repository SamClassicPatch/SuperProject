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

// [Cecil] Include the core library and patches
#define CORE_NO_GAME_HEADER
#include <Core/Core.h>

// [Cecil] Custom mod entities are disabled
#if !_PATCHCONFIG_CUSTOM_MOD || !_PATCHCONFIG_CUSTOM_MOD_ENTITIES
  #error Custom mod entities have been disabled in this build!
#endif

// [Cecil] Include common header for the library itself
#define ENTITIES_EXPORTS
#include "EntitiesAPI.h"
