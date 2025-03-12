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

#ifndef CECIL_INCL_ENTITIES_WEAPONCUSTOMIZATION_INL
#define CECIL_INCL_ENTITIES_WEAPONCUSTOMIZATION_INL

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "WeaponCustomization.h"

// Weapon viewmodel customization
static FLOAT wpn_afViewPos[3] = { 1.0f, 1.0f, 1.0f };
static FLOAT wpn_afViewRot[3] = { 1.0f, 1.0f, 1.0f };
static FLOAT wpn_fViewFOV = 1.0f;

static INDEX wpn_bTSECannon = CHOOSE_FOR_GAME(FALSE, TRUE, TRUE);

INDEX wpn_bViewMirrored = FALSE;

#if SE1_GAME != SS_TFE
INDEX wpn_bPowerUpParticles = FALSE;
#endif

// Reset weapon position
static void ResetWeaponPosition(void) {
  for (INDEX i = 0; i < 3; i++) {
    wpn_afViewPos[i] = 1.0f;
    wpn_afViewRot[i] = 1.0f;
  }
  wpn_fViewFOV = 1.0f;
};

static void DeclareWeaponCustomizationSymbols(void) {
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afViewPos[3];", &wpn_afViewPos);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afViewRot[3];", &wpn_afViewRot);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewFOV;", &wpn_fViewFOV);
  _pShell->DeclareSymbol("user void ResetWeaponPosition(void);", &ResetWeaponPosition);

  _pShell->DeclareSymbol("persistent user INDEX wpn_bTSECannon;", &wpn_bTSECannon);
  _pShell->DeclareSymbol("persistent user INDEX wpn_bViewMirrored;", &wpn_bViewMirrored);

#if SE1_GAME != SS_TFE
  _pShell->DeclareSymbol("persistent user INDEX wpn_bPowerUpParticles;", &wpn_bPowerUpParticles);
#endif
};

// Get weapon position for rendering
void RenderPos(FLOAT3D &vPos, FLOAT3D &vRot, FLOAT3D &vFire, FLOAT &fFOV) {
  // Mirror the position
  if (MirrorState()) {
    FLOATmatrix3D mRot;
    MakeRotationMatrix(mRot, vRot);

    // Mirror the rotation
    mRot(1, 2) *= -1.0f;
    mRot(1, 3) *= -1.0f;
    DecomposeRotationMatrix(vRot, mRot);

    vPos(1) *= -1.0f;
    vRot(3) *= -1.0f;
    vFire(1) *= -1.0f;
  }

  // Customizable position
  vPos(1) *= wpn_afViewPos[0];
  vPos(2) *= wpn_afViewPos[1];
  vPos(3) *= wpn_afViewPos[2];

  vRot(1) *= wpn_afViewRot[0];
  vRot(2) *= wpn_afViewRot[1];
  vRot(3) *= wpn_afViewRot[2];

  vFire(1) *= wpn_afViewPos[0];
  vFire(2) *= wpn_afViewPos[1];
  vFire(3) *= wpn_afViewPos[2];

  fFOV = Clamp(fFOV * wpn_fViewFOV, 1.0f, 170.0f);
};

// Get weapon position for Cannon depending on the game
void GetCannonPos(FLOAT &fX, FLOAT &fY, FLOAT &fZ, FLOAT &fFOV) {
  static const FLOAT afTFE[4] = { 0.225f, -0.345f, -0.570f, 57.0f };
  static const FLOAT afTSE[4] = { 0.170f, -0.300f, -0.625f, 50.0f };
  
  if (!wpn_bTSECannon) {
    fX   = afTFE[0];
    fY   = afTFE[1];
    fZ   = afTFE[2];
    fFOV = afTFE[3];
  } else {
    fX   = afTSE[0];
    fY   = afTSE[1];
    fZ   = afTSE[2];
    fFOV = afTSE[3];
  }
};

#endif
