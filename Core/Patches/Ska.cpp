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

#if _PATCHCONFIG_ENGINEPATCHES

#include "Ska.h"

#if _PATCHCONFIG_FIX_SKA

// [Cecil] TODO: Make SKA patches work in Debug
// SKA models are only usable since 1.07
#if SE1_VER >= SE1_107 && defined(NDEBUG)

CSkaPatch _SkaPatch;

// Original function pointers
void (CModelInstance::*pModelInstanceCopyFunc)(CModelInstance &) = NULL;
void (*pShaEnd)(void) = NULL;
void (*pShaSetLightColor)(COLOR, COLOR) = NULL;

void CModelInstancePatch::P_Copy(CModelInstance &miOther) {
  // Proceed to the original function
  (this->*pModelInstanceCopyFunc)(miOther);

  // Copy collision box for all frames
  mi_cbAllFramesBBox.Min() = miOther.mi_cbAllFramesBBox.Min();
  mi_cbAllFramesBBox.Max() = miOther.mi_cbAllFramesBBox.Max();
  mi_cbAllFramesBBox.SetName(miOther.mi_cbAllFramesBBox.GetName());
};

// Update model for fog and haze
void P_DoFogAndHaze(BOOL bOpaqueSurface) {
  // Get current surface vertex array
  GFXVertex4 *aVertices = shaGetVertexArray();
  GFXColor *aColors = shaGetColorArray();
  GFXColor *aHazeColors = shaGetNewColorArray();
  INDEX ctVertices = shaGetVertexCount();

  // Opaque surface
  if (bOpaqueSurface) {
    if (PrepareFog()) {
      _SkaPatch.aTexMipFoggy.PopAll();
      _SkaPatch.aTexMipFoggy.Push(ctVertices);

      // Setup texture coordinates only
      for (INDEX i = 0; i < ctVertices; i++) {
        GetFogMapInVertex(aVertices[i], _SkaPatch.aTexMipFoggy[i]);
      }

      // Don't use shaSetFogUVMap()
      _SkaPatch.aFogUVMap = &_SkaPatch.aTexMipFoggy[0];
    }

    if (PrepareHaze()) {
      _SkaPatch.aTexMipHazy.PopAll();
      _SkaPatch.aTexMipHazy.Push(ctVertices);

      const COLOR colH = AdjustColor(IGfx::Haze::Params().hp_colColor, IGfx::TexHueShift(), IGfx::TexSaturation());
      GFXColor colHaze(colH);

      // Setup texture coordinates and color
      for (INDEX i = 0; i < ctVertices; i++) {
        GetHazeMapInVertex(aVertices[i], _SkaPatch.aTexMipHazy[i].s);
        _SkaPatch.aTexMipHazy[i].t = 0.0f;
        aHazeColors[i] = colHaze;
      }

      // Don't use shaSetHazeUVMap() and shaSetHazeColorArray()
      _SkaPatch.aHazeUVMap = &_SkaPatch.aTexMipHazy[0];
      _SkaPatch.acolVtxHaze = &aHazeColors[0];
    }

  // Translucent surface
  } else {
    if (PrepareFog()) {
      GFXTexCoord tex;

      for (INDEX i = 0; i < ctVertices; i++) {
        GetFogMapInVertex(aVertices[i], tex);

        UBYTE ub = IGfx::Fog::GetAlpha(tex) ^ 255;
        aColors[i].AttenuateA(ub);
      }
    }

    if (PrepareHaze()) {
      FLOAT tx1;

      for (INDEX i = 0; i < ctVertices; i++) {
        GetHazeMapInVertex(aVertices[i], tx1);

        UBYTE ub = IGfx::Haze::GetAlpha(tx1) ^ 255;
        aHazeColors[i] = aColors[i];
        aHazeColors[i].AttenuateA(ub);
      }

      // Don't use shaSetHazeColorArray()
      _SkaPatch.acolVtxHaze = &aHazeColors[0];
    }
  }
};

// Render additional pass for fog and haze
void P_shaDoFogPass(void) {
  // Reset temporary arrays
  _SkaPatch.ResetArrays();

  // Calculate fog and haze UV-map for this opaque surface
  P_DoFogAndHaze(TRUE);

  // Access to '_paIndices' and '_ctIndices'
  INDEX *paIndices = shaGetIndexArray();
  INDEX ctIndices = shaGetIndexCount();

  // Fog UV-map has been given
  if (_SkaPatch.aFogUVMap != NULL)
  {
    IGfx::SetTextureWrapping(GFX_CLAMP, GFX_CLAMP);
    IGfx::SetTexture(IGfx::Fog::Texture(), IGfx::Fog::TexParams());
    IGfx::SetTexCoordArray(_SkaPatch.aFogUVMap, FALSE);
    // [Cecil] Adjust fog color
    const COLOR colF = AdjustColor(IGfx::Fog::Params().fp_colColor, IGfx::TexHueShift(), IGfx::TexSaturation());
    IGfx::SetConstantColor(colF);
    IGfx::BlendFunc(GFX_SRC_ALPHA, GFX_INV_SRC_ALPHA);
    IGfx::EnableBlend();

    // Render fog pass
    IGfx::DrawElements(ctIndices, paIndices);
  }

  // Haze UV-map has been given
  if (_SkaPatch.aHazeUVMap != NULL)
  {
    IGfx::SetTextureWrapping(GFX_CLAMP, GFX_CLAMP);
    IGfx::SetTexture(IGfx::Haze::Texture(), IGfx::Haze::TexParams());
    IGfx::SetTexCoordArray(_SkaPatch.aHazeUVMap, FALSE);
    IGfx::BlendFunc(GFX_SRC_ALPHA, GFX_INV_SRC_ALPHA);
    IGfx::EnableBlend();

    // Set vertex color array for haze
    if (_SkaPatch.acolVtxHaze != NULL) {
      IGfx::SetColorArray(_SkaPatch.acolVtxHaze);
    }

    // Render haze pass
    IGfx::DrawElements(ctIndices, paIndices);
  }
};

// Wrap up shader rendering
void P_shaEnd(void) {
  // Reset texture wrapping prior to rendering with any of the default shaders,
  // which erroneously set the textures before the needed texture wrapping
  IGfx::SetTextureWrapping(GFX_REPEAT, GFX_REPEAT);

  // Proceed to the original function
  pShaEnd();
};

// Set light colors
void P_shaSetLightColor(COLOR colAmbient, COLOR colLight) {
  // Adjust light colors
  colAmbient = AdjustColor(colAmbient, IGfx::ShdHueShift(), IGfx::ShdSaturation());
  colLight   = AdjustColor(colLight,   IGfx::ShdHueShift(), IGfx::ShdSaturation());

  // Proceed to the original function
  pShaSetLightColor(colAmbient, colLight);
};

#endif

#endif // _PATCHCONFIG_FIX_SKA

#endif // _PATCHCONFIG_ENGINEPATCHES
