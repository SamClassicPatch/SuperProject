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

#ifndef CECIL_INCL_PATCHES_TEXTURES_H
#define CECIL_INCL_PATCHES_TEXTURES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_EXTEND_TEXTURES

class CTexDataPatch : public CTextureData {
  public:
    // Create texture with specific flags
    PATCHES_API void P_Create(const CImageInfo *pII, MEX mexWanted, INDEX ctFineMips, int ulFlags);

    // Write texture data into a stream
    void P_Write(CTStream *strm);
};

// Create new animated texture from a script
PATCHES_API void P_ProcessTextureScript(const CTFileName &fnInput);

// Create new texture from a picture and save it into a specific file
PATCHES_API void P_CreateTextureOut(const CTFileName &fnInput, const CTFileName &fnOutput, MEX mexInput, INDEX ctMipmaps, int ulFlags);

// Create new texture from a picture
PATCHES_API void P_CreateTexture(const CTFileName &fnInput, MEX mexInput, INDEX ctMipmaps, int ulFlags);

#else

class CTexDataPatch : public CTextureData {
  public:
    __forceinline void P_Create(const CImageInfo *pII, MEX mexWanted, INDEX ctFineMips, int ulFlags) {
      Create_t(pII, mexWanted, ctFineMips, (ulFlags & TEX_32BIT) != 0);
    };
};

#define P_ProcessTextureScript ProcessScript_t

__forceinline void P_CreateTextureOut(const CTFileName &fnInput, const CTFileName &fnOutput, MEX mexInput, INDEX ctMipmaps, int ulFlags) {
  CreateTexture_t(fnInput, fnOutput, mexInput, ctMipmaps, (ulFlags & TEX_32BIT) != 0);
};

__forceinline void P_CreateTexture(const CTFileName &fnInput, MEX mexInput, INDEX ctMipmaps, int ulFlags) {
  CreateTexture_t(fnInput, mexInput, ctMipmaps, (ulFlags & TEX_32BIT) != 0);
};

#endif // _PATCHCONFIG_EXTEND_TEXTURES

#endif
