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

#ifndef CECIL_INCL_PATCHES_SKA_H
#define CECIL_INCL_PATCHES_SKA_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Core/Interfaces/GfxFunctions.h>

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_FIX_SKA

// [Cecil] TODO: Make SKA patches work in Debug
// SKA models are only usable since 1.07
#if SE1_VER >= SE1_107 && defined(NDEBUG)

// SKA patch variables
class CSkaPatch {
  public:
    CStaticStackArray<GFXTexCoord> aTexMipFoggy;
    CStaticStackArray<GFXTexCoord> aTexMipHazy;

    // For remembering fog and haze parameters
    FLOAT3D vDirView;
    FLOAT fFogAdd;
    FLOAT fHazeAdd;

    // Temporary arrays
    GFXTexCoord *aFogUVMap;
    GFXTexCoord *aHazeUVMap;
    GFXColor *acolVtxHaze;

    // Texture wrapping set by the shader
    GfxWrap aTexWrap[2];

  public:
    // Constructor
    CSkaPatch() {
      ResetArrays();

      aTexWrap[0] = GFX_REPEAT;
      aTexWrap[1] = GFX_REPEAT;
    };

    // Reset temporary arrays
    void ResetArrays(void) {
      aFogUVMap = NULL;
      aHazeUVMap = NULL;
      acolVtxHaze = NULL;
    };
};

PATCHES_API extern CSkaPatch _SkaPatch;

class CModelInstancePatch : public CModelInstance {
  public:
    void P_Copy(CModelInstance &miOther);
};

// Check vertex against fog
__forceinline void GetFogMapInVertex(GFXVertex4 &vtx, GFXTexCoord &tex) {
  const FLOAT3D &vDirView = _SkaPatch.vDirView;
  const FLOAT fH = vtx.x * vDirView(1) + vtx.y * vDirView(2) + vtx.z * vDirView(3);

  tex.s = -vtx.z * IGfx::Fog::MulZ();
  tex.t = (fH + _SkaPatch.fFogAdd) * IGfx::Fog::MulH();
};

// Prepare fog values
__forceinline BOOL PrepareFog(void) {
  if (RM_GetRenderFlags() & SRMF_FOG) {
    // Get fog offset and direction in viewer space
    _SkaPatch.vDirView = IGfx::Fog::DirView();
    _SkaPatch.fFogAdd = IGfx::Fog::AddH();
    return TRUE;
  }

  return FALSE;
};

// Check vertex against haze
__forceinline void GetHazeMapInVertex(GFXVertex4 &vtx, FLOAT &tx1) {
  tx1 = (-_SkaPatch.fHazeAdd - vtx.z) * IGfx::Haze::Mul();
};

// Prepare haze values
__forceinline BOOL PrepareHaze(void) {
  if (RM_GetRenderFlags() & SRMF_HAZE) {
    _SkaPatch.fHazeAdd = IGfx::Haze::Params().hp_fNear;
    return TRUE;
  }

  return FALSE;
};

// Update model for fog and haze
PATCHES_API void P_DoFogAndHaze(BOOL bOpaqueSurface);

// Render additional pass for fog and haze
PATCHES_API void P_shaDoFogPass(void);

// Set texture wrapping
PATCHES_API void P_shaSetTextureWrapping(GfxWrap eWrapU, GfxWrap eWrapV);

#endif

#endif // _PATCHCONFIG_FIX_SKA

#endif
