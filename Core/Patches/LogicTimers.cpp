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

#include "StdH.h"

#if _PATCHCONFIG_FIX_LOGICTIMERS && _PATCHCONFIG_ENGINEPATCHES

#include "LogicTimers.h"

// Set next timer event to occur after some time
void CRationalEntityTimerPatch::P_SetTimerAfter(TIME tmDelta) {
#if _PATCHCONFIG_GAMEPLAY_EXT

  // [Cecil] Fix timers as a gameplay extension
  if (IConfig::gex[k_EGameplayExt_FixTimers]) {
    // [Cecil] NOTE: This can cause unexpected behavior if timers are set to unusual delays, e.g. 0.075s (1.5 ticks)
    // Minus 2/5 of a tick for TIME_EPSILON == 0.0001 (as if TIME_EPSILON became 0.0201)
    tmDelta -= 0.02f;
  }

#endif // _PATCHCONFIG_GAMEPLAY_EXT

  // Set to execute some time after of the current tick
  SetTimerAt(_pTimer->CurrentTick() + tmDelta);
};

#endif // _PATCHCONFIG_FIX_LOGICTIMERS
