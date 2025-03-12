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

#ifndef CECIL_INCL_PATCHES_LOGICTIMERS_H
#define CECIL_INCL_PATCHES_LOGICTIMERS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_FIX_LOGICTIMERS && _PATCHCONFIG_ENGINEPATCHES

class CRationalEntityTimerPatch : public CRationalEntity {
  public:
    // Set next timer event to occur after some time
    void P_SetTimerAfter(TIME tmDelta);
};

#endif // _PATCHCONFIG_FIX_LOGICTIMERS

#endif
