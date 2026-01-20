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

#ifndef CECIL_INCL_GFXFUNCTIONS_H
#define CECIL_INCL_GFXFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Graphics/GfxLibrary.h>

// Interface of public methods from the engine's graphical API
namespace IGfx {

#if SE1_VER >= SE1_107
  // Pointers to function pointers
  #define GFX_FUNC_POINTER **
#else
  // Pointers to functions
  #define GFX_FUNC_POINTER *

  typedef GfxCull GfxFace;
#endif

typedef void (GFX_FUNC_POINTER CVoidFunc   )(void);
typedef void (GFX_FUNC_POINTER CBlendFunc  )(GfxBlend, GfxBlend);
typedef void (GFX_FUNC_POINTER CDepthFunc  )(GfxComp);
typedef void (GFX_FUNC_POINTER CFaceFunc   )(GfxFace);
typedef void (GFX_FUNC_POINTER COrthoFunc  )(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, BOOL);
typedef void (GFX_FUNC_POINTER CFrustumFunc)(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);
typedef void (GFX_FUNC_POINTER CPolModeFunc)(GfxPolyMode);
typedef void (GFX_FUNC_POINTER CColorsFunc )(ULONG);
typedef void (GFX_FUNC_POINTER CTexRefFunc )(ULONG &);
typedef void (GFX_FUNC_POINTER CMatrixFunc )(const FLOAT *);
typedef void (GFX_FUNC_POINTER CClipPlFunc )(const DOUBLE *);
typedef void (GFX_FUNC_POINTER CSetVtxFunc )(GFXVertex4 *, INDEX);
typedef void (GFX_FUNC_POINTER CSetNorFunc )(GFXNormal *);
typedef void (GFX_FUNC_POINTER CSetCrdFunc )(GFXTexCoord *, BOOL);
typedef void (GFX_FUNC_POINTER CSetColFunc )(GFXColor *);
typedef void (GFX_FUNC_POINTER CDrawElFunc )(INDEX, INDEX *);
typedef void (GFX_FUNC_POINTER CWrapUVFunc )(GfxWrap, GfxWrap);
typedef void (GFX_FUNC_POINTER CTexModFunc )(INDEX);
typedef void (GFX_FUNC_POINTER CMinMaxFunc )(FLOAT, FLOAT);

#define GFX_FUNC __forceinline

#define ARGS_GfxSetOrtho   (FLOAT fL, FLOAT fR, FLOAT fT, FLOAT fB, FLOAT fNear, FLOAT fFar, BOOL bSubPixelAdjust)
#define ARGS_GfxSetFrustum (FLOAT fL, FLOAT fR, FLOAT fT, FLOAT fB, FLOAT fNear, FLOAT fFar)

// [Cecil] TODO: Find addresses of methods in Debug and both 1.05 games
#if SE1_VER == SE1_107 && defined(NDEBUG)

// Fog interface
namespace Fog {
  GFX_FUNC CFogParameters &Params(void) { return *(CFogParameters *)0x602B8690; }; // _fog_fp
  GFX_FUNC CTexParams &TexParams(void)  { return *(CTexParams     *)0x602B8678; }; // _fog_tpLocal

  GFX_FUNC FLOAT3D &DirView(void) { return *(FLOAT3D *)0x602B8638; }; // _fog_vHDirView
  GFX_FUNC FLOAT   &MulZ(void)    { return *(FLOAT   *)0x602B86D4; }; // _fog_fMulZ
  GFX_FUNC FLOAT   &MulH(void)    { return *(FLOAT   *)0x602B86D8; }; // _fog_fMulH
  GFX_FUNC FLOAT   &AddH(void)    { return *(FLOAT   *)0x602B86DC; }; // _fog_fAddH
  GFX_FUNC ULONG   &Texture(void) { return *(ULONG   *)0x602B86E4; }; // _fog_ulTexture

  GFX_FUNC PIX    &SizeH(void) { return *(PIX    *)0x602B86EC; }; // _fog_pixSizeH
  GFX_FUNC PIX    &SizeL(void) { return *(PIX    *)0x602B86F0; }; // _fog_pixSizeL
  GFX_FUNC UBYTE *&Table(void) { return *(UBYTE **)0x602B86FC; }; // _fog_pubTable

  // Return lineary interpolated fog strength in fog texture
  GFX_FUNC ULONG GetAlpha(const GFXTexCoord &tex)
  {
    // Point sampling of height
    PIX pixT = FloatToInt(tex.t * SizeH());
    pixT = Clamp(pixT, 0L, SizeH() - 1L) * SizeL();

    // Linear interpolation of depth
    const PIX pixSF = FloatToInt(tex.s * (FLOAT)SizeL() * 255.499f);
    const PIX pixS1 = Clamp((PIX)((pixSF >> 8) + 0), 0L, SizeL() - 1L);
    const PIX pixS2 = Clamp((PIX)((pixSF >> 8) + 1), 0L, SizeL() - 1L);

    const ULONG ulF  = pixSF & 255;
    const ULONG ulA1 = Table()[pixT + pixS1];
    const ULONG ulA2 = Table()[pixT + pixS2];

    // Recalculate '_fog_ulAlpha' instead of pulling it
    ULONG ulAlpha = (Params().fp_colColor & CT_AMASK) >> CT_ASHIFT;
    return ((ulA1 * (ulF ^ 255) + ulA2 * ulF) * ulAlpha) >> 16;
  };
};

// Haze interface
namespace Haze {
  GFX_FUNC CHazeParameters &Params(void) { return *(CHazeParameters *)0x602B8618; }; // _haze_hp
  GFX_FUNC CTexParams &TexParams(void)   { return *(CTexParams      *)0x602B8600; }; // _haze_tpLocal

  GFX_FUNC FLOAT &Mul(void)     { return *(FLOAT *)0x602B8710; }; // _haze_fMul
  GFX_FUNC ULONG &Texture(void) { return *(ULONG *)0x602B871C; }; // _haze_ulTexture

  GFX_FUNC PIX &Size(void)     { return *(PIX    *)0x602B8704; }; // _haze_pixSize
  GFX_FUNC UBYTE *&Table(void) { return *(UBYTE **)0x602B870C; }; // _haze_pubTable

  // Return lineary interpolated haze strength in haze texture
  GFX_FUNC ULONG GetAlpha(const FLOAT fS)
  {
    // Linear interpolation of depth
    const PIX pixSH = FloatToInt(fS * (FLOAT)Size() * 255.4999f);
    const PIX pixS1 = Clamp((PIX)((pixSH >> 8) + 0), 0L, Size() - 1L);
    const PIX pixS2 = Clamp((PIX)((pixSH >> 8) + 1), 0L, Size() - 1L);

    const ULONG ulH  = pixSH & 255;
    const ULONG ulA1 = Table()[pixS1];
    const ULONG ulA2 = Table()[pixS2];

    // Recalculate '_haze_ulAlpha' instead of pulling it
    ULONG ulAlpha = (Params().hp_colColor & CT_AMASK) >> CT_ASHIFT;
    return ((ulA1 * (ulH ^ 255) + ulA2 * ulH) * ulAlpha) >> 16;
  };
};

GFX_FUNC SLONG &TexSaturation(void) { return *(SLONG *)0x602A08B4; }; // _slTexSaturation
GFX_FUNC SLONG &TexHueShift(void)   { return *(SLONG *)0x602C7294; }; // _slTexHueShift

// The same order as in GFX_SetGraphicsPointers()

GFX_FUNC void EnableDepthWrite(void)                     { (*(CVoidFunc   )0x602BBB64)(); };
GFX_FUNC void EnableDepthBias(void)                      { (*(CVoidFunc   )0x602BBB68)(); };
GFX_FUNC void EnableDepthTest(void)                      { (*(CVoidFunc   )0x602BBB6C)(); };
GFX_FUNC void EnableAlphaTest(void)                      { (*(CVoidFunc   )0x602BBB70)(); };
GFX_FUNC void EnableBlend(void)                          { (*(CVoidFunc   )0x602BBB74)(); };
GFX_FUNC void EnableDither(void)                         { (*(CVoidFunc   )0x602BBB78)(); };
GFX_FUNC void EnableTexture(void)                        { (*(CVoidFunc   )0x602BBB7C)(); };
GFX_FUNC void EnableClipping(void)                       { (*(CVoidFunc   )0x602BBB80)(); };
GFX_FUNC void EnableClipPlane(void)                      { (*(CVoidFunc   )0x602BBB84)(); };
GFX_FUNC void EnableTruform(void)                        { (*(CVoidFunc   )0x602BBC10)(); };
GFX_FUNC void DisableDepthWrite(void)                    { (*(CVoidFunc   )0x602BBB88)(); };
GFX_FUNC void DisableDepthBias(void)                     { (*(CVoidFunc   )0x602BBB8C)(); };
GFX_FUNC void DisableDepthTest(void)                     { (*(CVoidFunc   )0x602BBB90)(); };
GFX_FUNC void DisableAlphaTest(void)                     { (*(CVoidFunc   )0x602BBB94)(); };
GFX_FUNC void DisableBlend(void)                         { (*(CVoidFunc   )0x602BBB98)(); };
GFX_FUNC void DisableDither(void)                        { (*(CVoidFunc   )0x602BBB9C)(); };
GFX_FUNC void DisableTexture(void)                       { (*(CVoidFunc   )0x602BBBA0)(); };
GFX_FUNC void DisableClipping(void)                      { (*(CVoidFunc   )0x602BBBA4)(); };
GFX_FUNC void DisableClipPlane(void)                     { (*(CVoidFunc   )0x602BBBA8)(); };
GFX_FUNC void DisableTruform(void)                       { (*(CVoidFunc   )0x602BBC14)(); };
GFX_FUNC void BlendFunc(GfxBlend eSrc, GfxBlend eDst)    { (*(CBlendFunc  )0x602BBBAC)(eSrc, eDst); };
GFX_FUNC void DepthFunc(GfxComp eComp)                   { (*(CDepthFunc  )0x602BBBB0)(eComp); };
GFX_FUNC void DepthRange(FLOAT fMin, FLOAT fMax)         { (*(CMinMaxFunc )0x602BBBB4)(fMin, fMax); };
GFX_FUNC void CullFace(GfxFace eFace)                    { (*(CFaceFunc   )0x602BBBB8)(eFace); };
GFX_FUNC void FrontFace(GfxFace eFace)                   { (*(CFaceFunc   )0x602BBBBC)(eFace); };
GFX_FUNC void ClipPlane(const DOUBLE *pdViewPlane)       { (*(CClipPlFunc )0x602BBBC0)(pdViewPlane); };
GFX_FUNC void SetOrtho   ARGS_GfxSetOrtho                { (*(COrthoFunc  )0x602BBBC4)(fL, fR, fT, fB, fNear, fFar, bSubPixelAdjust); };
GFX_FUNC void SetFrustum ARGS_GfxSetFrustum              { (*(CFrustumFunc)0x602BBBC8)(fL, fR, fT, fB, fNear, fFar); };
GFX_FUNC void SetTextureMatrix(const FLOAT *pfMatrix)    { (*(CMatrixFunc )0x602BBBCC)(pfMatrix); };
GFX_FUNC void SetViewMatrix(const FLOAT *pfMatrix)       { (*(CMatrixFunc )0x602BBBD0)(pfMatrix); };
GFX_FUNC void PolygonMode(GfxPolyMode eMode)             { (*(CPolModeFunc)0x602BBBD4)(eMode); };
GFX_FUNC void SetTextureWrapping(GfxWrap eU, GfxWrap eV) { (*(CWrapUVFunc )0x602BBBD8)(eU, eV); };
GFX_FUNC void SetTextureModulation(INDEX i)              { (*(CTexModFunc )0x602BBBDC)(i); };
GFX_FUNC void GenerateTexture(ULONG &ulTex)              { (*(CTexRefFunc )0x602BBBE0)(ulTex); };
GFX_FUNC void DeleteTexture(ULONG &ulTex)                { (*(CTexRefFunc )0x602BBBE4)(ulTex); };
GFX_FUNC void SetVertexArray(GFXVertex4 *aVtx, INDEX ct) { (*(CSetVtxFunc )0x602BBBE8)(aVtx, ct); };
GFX_FUNC void SetNormalArray(GFXNormal *aNot)            { (*(CSetNorFunc )0x602BBBEC)(aNot); };
GFX_FUNC void SetTexCoordArray(GFXTexCoord *a, BOOL b4)  { (*(CSetCrdFunc )0x602BBBF0)(a, b4); };
GFX_FUNC void SetColorArray(GFXColor *aCol)              { (*(CSetColFunc )0x602BBBF4)(aCol); };
GFX_FUNC void DrawElements(INDEX ct, INDEX *aElements)   { (*(CDrawElFunc )0x602BBBF8)(ct, aElements); };
GFX_FUNC void SetConstantColor(ULONG ulCol)              { (*(CColorsFunc )0x602BBBFC)(ulCol); };
GFX_FUNC void EnableColorArray(void)                     { (*(CVoidFunc   )0x602BBC00)(); };
GFX_FUNC void DisableColorArray(void)                    { (*(CVoidFunc   )0x602BBC04)(); };
GFX_FUNC void Finish(void)                               { (*(CVoidFunc   )0x602BBC08)(); };
GFX_FUNC void LockArrays(void)                           { (*(CVoidFunc   )0x602BBC0C)(); };
GFX_FUNC void SetColorMask(ULONG ulCol)                  { (*(CColorsFunc )0x602BBC18)(ulCol); };

// Non-pointer functions

GFX_FUNC void GetTextureFiltering(INDEX &iFilterType, INDEX &iAnisotropyDegree) {
  ((void (*)(INDEX &, INDEX &))0x6011B0F0)(iFilterType, iAnisotropyDegree);
};

GFX_FUNC void SetTextureFiltering(INDEX &iFilterType, INDEX &iAnisotropyDegree) {
  ((void (*)(INDEX &, INDEX &))0x6011B110)(iFilterType, iAnisotropyDegree);
};

GFX_FUNC void SetTextureBiasing(FLOAT &fLODBias) {
  ((void (*)(FLOAT &))0x6011B300)(fLODBias);
};

GFX_FUNC void SetTextureUnit(INDEX iUnit) {
  ((void (*)(INDEX))0x6011B380)(iUnit);
};

GFX_FUNC void SetTexture(ULONG &ulTex, CTexParams &tpLocal) {
  ((void (*)(ULONG &, CTexParams &))0x6011B420)(ulTex, tpLocal);
};

GFX_FUNC void UploadTexture(ULONG *pulTex, PIX pixW, PIX pixH, ULONG ulFormat, BOOL bNoDiscard) {
  ((void (*)(ULONG *, PIX, PIX, ULONG, BOOL))0x6011B5F0)(pulTex, pixW, pixH, ulFormat, bNoDiscard);
};

GFX_FUNC void UnlockArrays(void)  { ((void (*)(void))0x6011C080)(); };
GFX_FUNC void FlushElements(void) { ((void (*)(void))0x6011BEC0)(); };
GFX_FUNC void FlushQuads(void)    { ((void (*)(void))0x6011BD90)(); };

#endif

}; // namespace

#endif
