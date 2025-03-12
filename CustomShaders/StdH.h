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
#include <Core/Core.h>
#include <CorePatches/Patches.h>

#include <Engine/Graphics/Shader.h>

// [Cecil] SKA models aren't usable prior to 1.07
#if SE1_VER < SE1_107
  #error Shaders cannot be built for Serious Engine versions before 1.07!
#endif

// [Cecil] Common includes
#include "Common.h"

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_FIX_SKA

// [Cecil] Replace engine methods with patches
#include <CorePatches/Patches/Ska.h>

// [Cecil] TODO: Make SKA patches work in Debug
#ifdef NDEBUG
#define shaDoFogPass          P_shaDoFogPass
#define shaSetTextureWrapping P_shaSetTextureWrapping
#endif

#endif // _PATCHCONFIG_ENGINEPATCHES
