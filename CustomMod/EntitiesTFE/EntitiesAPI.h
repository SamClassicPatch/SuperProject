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

// [Cecil] This header can be used to include all the necessary parts of the Entities library at once, either for exporting or importing

#include <Game/SessionProperties.h>

#ifndef ENTITIES_EXPORTS
  // [Cecil] Link appropriate library
  #ifdef NDEBUG
    #pragma comment(lib, "Entities" CLASSICSPATCH_SUFFIX ".lib")
  #else
    #pragma comment(lib, "Entities" CLASSICSPATCH_SUFFIX "D.lib")
  #endif

  #define DECL_DLL __declspec(dllimport)

#else
  #define DECL_DLL __declspec(dllexport)
#endif

#include "Global.h"
#include "Common/Flags.h"
#include "Common/Common.h"
#include <CommonEntities/Common/Particles.h>
#include <CommonEntities/Common/EmanatingParticles.h> // [Cecil]
#include "Common/GameInterface.h"
