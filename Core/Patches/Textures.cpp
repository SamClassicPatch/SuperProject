/* Copyright (c) 2022-2025 Dreamy Cecil
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

#include "Textures.h"

#include <Engine/Graphics/TextureEffects.h>

#include <Core/Definitions/BaseDefs.h>

#if _PATCHCONFIG_EXTEND_TEXTURES

// Calculate size of one effect buffer
static ULONG GetEffectBufferSize(CTextureData *ptd)
{
  ASSERT(ptd->td_ptegEffect != NULL);
  PIX pixW = ptd->td_pixBufferWidth; 
  PIX pixH = ptd->td_pixBufferHeight;

  ULONG ulSize = pixW * pixH * sizeof(UBYTE);

  // Eventual adjustment for water effect type
  if (ptd->td_ptegEffect->IsWater()) {
    ulSize = pixW * (pixH + 2) * sizeof(SWORD);
  }

  return ulSize;
};

// Create texture with specific flags
void CTexDataPatch::P_Create(const CImageInfo *pII, MEX mexWanted, INDEX ctFineMips, int ulFlags)
{
  // Check for supported image formats
  if (pII->ii_BitsPerPixel != 24 && pII->ii_BitsPerPixel != 32) {
    ASSERTALWAYS("Invalid bits per pixel for the texture creation!");

    ThrowF_t(LOCALIZE("Only 24-bit and 32-bit pictures can be processed."));
  }

  // Get picture data
  PIX pixSizeU = pII->ii_Width;
  PIX pixSizeV = pII->ii_Height;

  // Check maximum supported texture dimension
  if (pixSizeU > MAX_MEX || pixSizeV > MAX_MEX) {
    ThrowF_t(LOCALIZE("At least one of texture dimensions is too large."));
  }

  // Make sure picture sizes are powers of 2
  #ifndef NDEBUG
    ULONG ulCheckSizeU = 1UL << FastLog2(pixSizeU);
    ULONG ulCheckSizeV = 1UL << FastLog2(pixSizeV);

    ASSERT(ulCheckSizeU == pixSizeU && ulCheckSizeV == pixSizeV);
  #endif

  // Dimension in mexels must not be smaller than the one in pixels
  ASSERT(pixSizeU <= mexWanted);

  // Determine mip index from the mex size
  td_iFirstMipLevel = FastLog2(mexWanted / pixSizeU);

  // Set flags
  td_ulFlags = NONE;

  if (pII->ii_BitsPerPixel == 32) {
    td_ulFlags |= TEX_ALPHACHANNEL;
  }

  if (ulFlags & TEX_32BIT) {
    td_ulFlags |= TEX_32BIT;
  }

  // Initialize general properties
  td_ctFrames = 0;
  td_mexWidth = pixSizeU << td_iFirstMipLevel;
  td_mexHeight = pixSizeV << td_iFirstMipLevel;

  // Create all mipmaps (either bilinear or downsampled)
  INDEX ctMipMaps = GetNoOfMipmaps(pixSizeU, pixSizeV);
  td_ctFineMipLevels = Min(ctFineMips, ctMipMaps);

  // Determine total size of one frame
  td_slFrameSize = GetMipmapOffset(15, pixSizeU, pixSizeV) * BYTES_PER_TEXEL;

  // Allocate small amount of memory just for reallocation
  td_pulFrames = (ULONG *)AllocMemory(16);

  // Add one frame from the image
  AddFrame_t(pII);
};

// Write texture data into a stream
void CTexDataPatch::P_Write(CTStream *strm)
{
  // Cannot write textures that have been mangled somehow
  if (td_ptegEffect == NULL && IsModified()) {
    ThrowF_t(LOCALIZE("Cannot write texture that has modified frames."));
  }

  // Should not have the same name as the base texture
  if (td_ptdBaseTexture != NULL) {
    CTFileName fnTex = strm->GetDescription();

    if (fnTex == td_ptdBaseTexture->GetName()) {
      ThrowF_t(LOCALIZE("Texture \"%s\" has same name as its base texture."), (CTString &)fnTex);
    }
  }

  // Write version
  INDEX iVersion = 4;
  strm->WriteID_t("TVER");
  *strm << iVersion;

  // Isolate required flags
  const BOOL bAlphaChannel = td_ulFlags & TEX_ALPHACHANNEL;

  #if SE1_VER >= SE1_150
    const BOOL bCompress      = td_ulFlags & TEX_COMPRESS;
    const BOOL bCompressAlpha = td_ulFlags & TEX_COMPRESSALPHA;

    ULONG ulFlags = td_ulFlags & (TEX_ALPHACHANNEL | TEX_32BIT | TEX_EQUALIZED | TEX_TRANSPARENT
                                | TEX_STATIC | TEX_CONSTANT | TEX_COMPRESSED | TEX_COMPRESSEDALPHA);

    if (bCompress) {
      ulFlags |= TEX_COMPRESSED;
    }

    if (bCompressAlpha) {
      ulFlags |= TEX_COMPRESSEDALPHA;
    }

  #else
    ULONG ulFlags = td_ulFlags & (TEX_ALPHACHANNEL | TEX_32BIT | TEX_EQUALIZED
                                | TEX_TRANSPARENT | TEX_STATIC | TEX_CONSTANT);
  #endif

  // Write chunk containing texture data
  strm->WriteID_t(CChunkID("TDAT"));

  // Write data describing texture
  *strm << ulFlags;
  *strm << td_mexWidth;
  *strm << td_mexHeight;
  *strm << td_ctFineMipLevels;
  *strm << td_iFirstMipLevel;
  *strm << td_ctFrames;

  // Save frames if texture has no global effect
  if (td_ptegEffect == NULL) {
    ASSERT(td_ctFrames > 0);

  #if SE1_VER >= SE1_150
    if (bCompress) {
      // Write chunk containing compressed frames
      strm->WriteID_t(CChunkID("FRMC"));

      SLONG slCompressedFrameSize;
      UBYTE *pubCompressed = Compress(slCompressedFrameSize, bCompressAlpha);

      if (pubCompressed == NULL) {
        ASSERTALWAYS("Cannot compress texture!");

        ThrowF_t(TRANS("Cannot compress texture."));
      }

      // Write size of a compressed frame
      *strm << slCompressedFrameSize;

      // Write all compressed frames at once
      strm->Write_t(pubCompressed, td_ctFrames * slCompressedFrameSize);
      FreeMemory(pubCompressed);

    } else
  #endif
    {
      ASSERT(td_pulFrames != NULL);

      // Write chunk containing raw frames
      strm->WriteID_t(CChunkID("FRMS"));

      const PIX pixFrSize = GetPixWidth() * GetPixHeight();

      // Eventually prepare temp buffer in case of frames without alpha channel
      UBYTE *pubTemp = NULL;

      if (!bAlphaChannel) {
        pubTemp = (UBYTE *)AllocMemory(pixFrSize * 3);
      }

      // Write highest mipmap of each individual frame
      for (INDEX iFrame = 0; iFrame < td_ctFrames; iFrame++) {
        // Get first pixel of the current frame
        ULONG *pulCurrentFrame = td_pulFrames + (iFrame * td_slFrameSize / BYTES_PER_TEXEL);

        // Write frame with the alpha channel
        if (bAlphaChannel) {
          strm->Write_t(pulCurrentFrame, pixFrSize * 4);

        // Write frame without the alpha channel
        } else {
          RemoveAlphaChannel(pulCurrentFrame, pubTemp, pixFrSize);
          strm->Write_t(pubTemp, pixFrSize * 3);
        }
      }

      // Delete temporary buffer
      if (pubTemp != NULL) {
        FreeMemory(pubTemp);
      }
    }

  // Save effect texture data
  } else {
    // Write chunk containing effect data
    strm->WriteID_t(CChunkID("FXDT"));

    // Write effect type and buffer dimensions
    *strm << td_ptegEffect->teg_ulEffectType;
    *strm << td_pixBufferWidth;
    *strm << td_pixBufferHeight;

    // Write amount of effect sources
    *strm << td_ptegEffect->teg_atesEffectSources.Count();

    // Write entire array of effect sources
    FOREACHINDYNAMICARRAY(td_ptegEffect->teg_atesEffectSources, CTextureEffectSource, itEffectSource)
    {
      // Write effect source type
      *strm << itEffectSource->tes_ulEffectSourceType;

      // Write effect source properties
      strm->Write_t(&itEffectSource->tes_tespEffectSourceProperties, sizeof(TextureEffectSourceProperties));

      // Write amount of effect pixels
      INDEX ctEffectSourcePixels = itEffectSource->tes_atepPixels.Count();
      *strm << ctEffectSourcePixels;

      // If there are any effect pixels
      if (ctEffectSourcePixels > 0) {
        // Write all effect pixels at once
        strm->Write_t(&itEffectSource->tes_atepPixels[0], ctEffectSourcePixels * sizeof(TextureEffectPixel));
      }
    }

    // If effect buffers are valid
    if (td_pubBuffer1 != NULL && td_pubBuffer2 != NULL)
    {
      // Write chunk containing effect buffers
      strm->WriteID_t(CChunkID("FXB2"));
      ULONG ulSize = GetEffectBufferSize(this);

      // Write effect buffers
      strm->Write_t(td_pubBuffer1, ulSize);
      strm->Write_t(td_pubBuffer2, ulSize);
    }
  }

  // Write animation data
  strm->WriteID_t(CChunkID("ANIM"));

  // [Cecil] CAnimData::Write_t inline code
  {
    strm->WriteID_t(CChunkID("ADAT"));

    // Write amount of animations
    *strm << ad_NumberOfAnims;

    for (INDEX i = 0; i < ad_NumberOfAnims; i++)
    {
      // Write data about one animation
      strm->Write_t(&ad_Anims[i].oa_Name, sizeof(NAME));
      strm->Write_t(&ad_Anims[i].oa_SecsPerFrame, sizeof(TIME));
      strm->Write_t(&ad_Anims[i].oa_NumberOfFrames, sizeof(INDEX));
      strm->Write_t(ad_Anims[i].oa_FrameIndices, ad_Anims[i].oa_NumberOfFrames * sizeof(INDEX));
    }
  }

  // Write filename of the base texture, if it exists
  if (td_ptdBaseTexture != NULL) {
    strm->WriteID_t(CChunkID("BAST"));
    *strm << td_ptdBaseTexture->GetName();
  }

  #if SE1_VER >= SE1_150
    // Don't need to compress again
    td_ulFlags &= ~TEX_COMPRESS;
  #endif
};

// Create new animated texture from a script
void P_ProcessTextureScript(const CTFileName &fnInput)
{
  // Open the script file
  CTFileStream strmScript;
  strmScript.Open_t(fnInput);

  // Texture properties
  FLOAT fTextureWidthMeters = 2.0f;
  INDEX ctTexMipmaps = (MAX_MEX_LOG2 - 2);
  ULONG ulFlags = NONE;

  // [Cecil] Use patched class
  CTexDataPatch td;
  CListHead lhFrameNames;

  // Significant data counter
  INDEX ctFoundData = 0;

  // Parse script lines
  CTString strLine;

  FOREVER {
    // Get a proper line
    do {
      strmScript.GetLine_t(strLine);
    } while (strLine.Length() == 0 || strLine[0] == ';');

    // Make the line uppercase
    _strupr(strLine.str_String);

    // Specified texture width
    if (strLine.HasPrefix("TEXTURE_WIDTH")) {
      strLine.ScanF("TEXTURE_WIDTH %g", &fTextureWidthMeters);
      ctFoundData++;

    // Amount of texture mipmaps
    } else if (strLine.HasPrefix("TEXTURE_MIPMAPS")) {
      strLine.ScanF("TEXTURE_MIPMAPS %d", &ctTexMipmaps);

    // Force 32-bit quality on the texture
    } else if (strLine.HasPrefix("TEXTURE_32BIT")) {
      ulFlags |= TEX_32BIT;

#if SE1_VER >= SE1_150
    // Compress the entire texture
    } else if (strLine.HasPrefix("TEXTURE_COMPRESSED")) {
      ulFlags |= TEX_COMPRESS;

    // Compress the alpha channel
    } else if (strLine.HasPrefix("TEXTURE_COMPRESSALPHA")) {
      ulFlags |= TEX_COMPRESSALPHA;
#endif

    // Load animations from the script
    } else if (strLine.HasPrefix("ANIM_START")) {
      td.LoadFromScript_t(&strmScript, &lhFrameNames);
      ctFoundData++;

    // End script loading
    } else if (strLine.HasPrefix("END")) {
      break;

    // Unrecognized keyword
    } else {
      ThrowF_t(LOCALIZE("Unidentified key-word found (line: \"%s\") or unexpected end of file reached."), strLine);
    }
  }

  // Unusual amount of data
  if (ctFoundData != 2) {
    ThrowF_t(LOCALIZE("Required key-word(s) has not been specified in script file:\nTEXTURE_WIDTH and/or ANIM_START"));
  }

  // Now we will create texture file form read script data
  CImageInfo iiFrame;
  CTFileStream strm;

  // Load the first picture
  CFileNameNode *pFirstFNN = LIST_HEAD(lhFrameNames, CFileNameNode, cfnn_Node);
  iiFrame.LoadAnyGfxFormat_t(CTString(pFirstFNN->cfnn_FileName));

  // Create texture with one frame
  td.P_Create(&iiFrame, MEX_METERS(fTextureWidthMeters), ctTexMipmaps, (int)ulFlags);
  iiFrame.Clear();

  // Add the rest of the frames, if any
  BOOL bFirst = TRUE;

  FOREACHINLIST(CFileNameNode, cfnn_Node, lhFrameNames, it1) {
    // Skip the first frame because it's been already added
    if (bFirst) {
      bFirst = FALSE;
      continue;
    }

    // Add new picture as the next animation frame
    iiFrame.LoadAnyGfxFormat_t(CTString(it1->cfnn_FileName));
    td.AddFrame_t(&iiFrame);
    iiFrame.Clear();
  }

  // Save the texture
  td.Save_t(fnInput.NoExt() + ".TEX");

  // Clear the list
  FORDELETELIST(CFileNameNode, cfnn_Node, lhFrameNames, itDel) {
    delete &itDel.Current();
  }
};

// Create new texture from a picture and save it into a specific file
void P_CreateTextureOut(const CTFileName &fnInput, const CTFileName &fnOutput, MEX mexInput, INDEX ctMipmaps, int ulFlags)
{
  // Process a script file
  if (fnInput.FileExt() == ".SCR") {
    ProcessScript_t(fnInput);

  // Process a picture file
  } else {
    // Invalid mexel units
    if (mexInput <= 0) {
      ThrowF_t(LOCALIZE("Invalid or unspecified mexel units."));
    }

    // [Cecil] Use patched class
    CTexDataPatch td;
    CImageInfo iiPicture;

    // Load the picture
    iiPicture.LoadAnyGfxFormat_t(fnInput);

    // Create texture and save it into a file
    td.P_Create(&iiPicture, mexInput, ctMipmaps, ulFlags);
    td.Save_t(fnOutput);

    // Clear the picture
    iiPicture.Clear();
  }
};

// Create new texture from a picture
void P_CreateTexture(const CTFileName &fnInput, MEX mexInput, INDEX ctMipmaps, int ulFlags) {
  const CTFileName fnOutput = fnInput.NoExt() + ".TEX";
  P_CreateTextureOut(fnInput, fnOutput, mexInput, ctMipmaps, ulFlags);
};

#endif // _PATCHCONFIG_EXTEND_TEXTURES

#endif // _PATCHCONFIG_ENGINEPATCHES
