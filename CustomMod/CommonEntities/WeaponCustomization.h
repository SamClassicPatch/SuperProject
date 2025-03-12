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

#ifndef CECIL_INCL_ENTITIES_WEAPONCUSTOMIZATION_H
#define CECIL_INCL_ENTITIES_WEAPONCUSTOMIZATION_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Weapon viewmodel customization
extern INDEX wpn_bViewMirrored;

#if SE1_GAME != SS_TFE
extern INDEX wpn_bPowerUpParticles;
#endif

// Get mirroring state (rendering only)
inline BOOL MirrorState(void) {
  return wpn_bViewMirrored;
};

// Get weapon position for rendering
void RenderPos(FLOAT3D &vPos, FLOAT3D &vRot, FLOAT3D &vFire, FLOAT &fFOV);

// Get weapon position for Cannon depending on the game
void GetCannonPos(FLOAT &fX, FLOAT &fY, FLOAT &fZ, FLOAT &fFOV);

#endif
