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

// [Cecil] Definitions of unexported texture effects methods from the engine

#ifndef CECIL_INCL_TEXTUREEFFECTS_DEFS_H
#define CECIL_INCL_TEXTUREEFFECTS_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Graphics/TextureEffects.h>

#if _PATCHCONFIG_FUNC_DEFINITIONS

// Initialize the texture effect source
void CTextureEffectSource::Initialize(CTextureEffectGlobal *ptegGlobalEffect, ULONG ulEffectSourceType,
                                      PIX pixU0, PIX pixV0, PIX pixU1, PIX pixV1)
{
  // Remember global effect for cross linking
  tes_ptegGlobalEffect = ptegGlobalEffect;
  tes_ulEffectSourceType = ulEffectSourceType;

  // Obtain effect source table for the current effect class
  const ULONG ulType = ptegGlobalEffect->teg_ulEffectType;
  TextureEffectSourceType *paSourceTypes = _ategtTextureEffectGlobalPresets[ulType].tet_atestEffectSourceTypes;

  // Initialize for animating
  paSourceTypes[ulEffectSourceType].test_Initialize(this, pixU0, pixV0, pixU1, pixV1);
};

// Animate the texture effect source
void CTextureEffectSource::Animate(void)
{
  // Obtain effect source table for the current effect class
  const ULONG ulType = tes_ptegGlobalEffect->teg_ulEffectType;
  TextureEffectSourceType *paSourceTypes = _ategtTextureEffectGlobalPresets[ulType].tet_atestEffectSourceTypes;

  // Animate it
  paSourceTypes[tes_ulEffectSourceType].test_Animate(this);
};

// Default constructor
CTextureEffectGlobal::CTextureEffectGlobal(CTextureData *ptdTexture, ULONG ulGlobalEffect)
{
  // Remember global effect's texture data for cross linking
  teg_ptdTexture = ptdTexture;
  teg_ulEffectType = ulGlobalEffect;

  // Initialize for animating
  _ategtTextureEffectGlobalPresets[teg_ulEffectType].tegt_Initialize();

  // Make sure the texture is updated when it's used next time
  teg_updTexture.Invalidate();
};

// Check if it's a water effect type
BOOL CTextureEffectGlobal::IsWater(void) {
  // [Cecil] NOTE: 0-3 animation types in standard Serious Engine are all water
  return teg_ulEffectType < 4;
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
