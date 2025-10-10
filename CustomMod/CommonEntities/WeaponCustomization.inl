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

// Global viewmodel customization
static FLOAT wpn_fViewPosX = 1.0f;
static FLOAT wpn_fViewPosY = 1.0f;
static FLOAT wpn_fViewPosZ = 1.0f;
static FLOAT wpn_fViewRotH = 1.0f;
static FLOAT wpn_fViewRotP = 1.0f;
static FLOAT wpn_fViewRotB = 1.0f;
static FLOAT wpn_fViewFOV = 1.0f;

// Per weapon viewmodel customization
static FLOAT wpn_afAddPosX[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddPosY[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddPosZ[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddPosH[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddPosP[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddPosB[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static FLOAT wpn_afAddFOV [20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static INDEX wpn_bTSECannon = CHOOSE_FOR_GAME(FALSE, TRUE, TRUE);

INDEX wpn_bViewMirrored = FALSE;

#if SE1_GAME != SS_TFE
INDEX wpn_bPowerUpParticles = FALSE;
#endif

// Reset weapon position
static void ResetWeaponPosition(void) {
  wpn_fViewPosX = 1.0f;
  wpn_fViewPosY = 1.0f;
  wpn_fViewPosZ = 1.0f;
  wpn_fViewRotH = 1.0f;
  wpn_fViewRotP = 1.0f;
  wpn_fViewRotB = 1.0f;
  wpn_fViewFOV  = 1.0f;
};

static void ResetPerWeaponPositions(void) {
  for (INDEX i = 0; i < 20; i++) {
    wpn_afAddPosX[i] = 0.0f;
    wpn_afAddPosY[i] = 0.0f;
    wpn_afAddPosZ[i] = 0.0f;
    wpn_afAddPosH[i] = 0.0f;
    wpn_afAddPosP[i] = 0.0f;
    wpn_afAddPosB[i] = 0.0f;
    wpn_afAddFOV [i] = 0.0f;
  }
};

static void DeclareWeaponCustomizationSymbols(void) {
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewPosX;", &wpn_fViewPosX);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewPosY;", &wpn_fViewPosY);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewPosZ;", &wpn_fViewPosZ);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewRotH;", &wpn_fViewRotH);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewRotP;", &wpn_fViewRotP);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewRotB;", &wpn_fViewRotB);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_fViewFOV;", &wpn_fViewFOV);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosX[20];", &wpn_afAddPosX);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosY[20];", &wpn_afAddPosY);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosZ[20];", &wpn_afAddPosZ);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosH[20];", &wpn_afAddPosH);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosP[20];", &wpn_afAddPosP);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddPosB[20];", &wpn_afAddPosB);
  _pShell->DeclareSymbol("persistent user FLOAT wpn_afAddFOV[20];", &wpn_afAddFOV);

  _pShell->DeclareSymbol("user void ResetWeaponPosition(void);", &ResetWeaponPosition);
  _pShell->DeclareSymbol("user void ResetPerWeaponPositions(void);", &ResetPerWeaponPositions);

  _pShell->DeclareSymbol("persistent user INDEX wpn_bTSECannon;", &wpn_bTSECannon);
  _pShell->DeclareSymbol("persistent user INDEX wpn_bViewMirrored;", &wpn_bViewMirrored);

#if SE1_GAME != SS_TFE
  _pShell->DeclareSymbol("persistent user INDEX wpn_bPowerUpParticles;", &wpn_bPowerUpParticles);
#endif
};

// Get weapon position for rendering
void RenderPos(INDEX iWeapon, FLOAT3D &vPos, FLOAT3D &vRot, FLOAT3D &vFire, FLOAT &fFOV) {
  ASSERT(iWeapon >= 0 && iWeapon < 20);
  iWeapon = Clamp(iWeapon, (INDEX)0, (INDEX)19);

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
  vPos(1) *= wpn_fViewPosX;
  vPos(2) *= wpn_fViewPosY;
  vPos(3) *= wpn_fViewPosZ;
  vPos(1) += wpn_afAddPosX[iWeapon];
  vPos(2) += wpn_afAddPosY[iWeapon];
  vPos(3) += wpn_afAddPosZ[iWeapon];

  vRot(1) *= wpn_fViewRotH;
  vRot(2) *= wpn_fViewRotP;
  vRot(3) *= wpn_fViewRotB;
  vRot(1) += wpn_afAddPosH[iWeapon];
  vRot(2) += wpn_afAddPosP[iWeapon];
  vRot(3) += wpn_afAddPosB[iWeapon];

  vFire(1) *= wpn_fViewPosX;
  vFire(2) *= wpn_fViewPosY;
  vFire(3) *= wpn_fViewPosZ;
  vFire(1) += wpn_afAddPosX[iWeapon];
  vFire(2) += wpn_afAddPosY[iWeapon];
  vFire(3) += wpn_afAddPosZ[iWeapon];

  fFOV = Clamp(fFOV * wpn_fViewFOV + wpn_afAddFOV[iWeapon], 1.0f, 170.0f);
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
