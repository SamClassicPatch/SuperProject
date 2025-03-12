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

// Library entry point
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReason, LPVOID lpReserved) {
#if _PATCHCONFIG_ENGINEPATCHES

  // Apply patches upon loading the library
  if (ulReason == DLL_PROCESS_ATTACH) {
  // [Cecil] TODO: Make SKA patches work in Debug
  #ifdef NDEBUG
    _EnginePatches.Ska();
  #endif
  }

#endif // _PATCHCONFIG_ENGINEPATCHES

  return TRUE;
};
