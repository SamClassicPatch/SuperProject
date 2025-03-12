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

// [Cecil] Definitions of unexported graphical methods from the engine

#ifndef CECIL_INCL_GRAPHICS_DEFS_H
#define CECIL_INCL_GRAPHICS_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_FUNC_DEFINITIONS

// Retrive memory offset of a specified mipmap or size of all mipmaps in pixels
PIX GetMipmapOffset(INDEX iMipLevel, PIX pixWidth, PIX pixHeight)
{
  PIX pixTexSize = 0;
  PIX pixMipSize = pixWidth * pixHeight;

  INDEX iMips = GetNoOfMipmaps(pixWidth, pixHeight);
  iMips = Min(iMips, iMipLevel);

  while (iMips > 0) {
    pixTexSize += pixMipSize;
    pixMipSize >>= 2;
    iMips--;
  }

  return pixTexSize;
};

// Return offset, pointer and dimensions of a mipmap of specified size inside a texture or a shadowmap
INDEX GetMipmapOfSize(PIX pixWantedSize, ULONG *&pulFrame, PIX &pixWidth, PIX &pixHeight)
{
  INDEX iMipOffset = 0;

  while (pixWidth > 1 && pixHeight > 1) {
    const PIX pixCurrentSize = pixWidth * pixHeight;

    // Found it
    if (pixCurrentSize <= pixWantedSize) {
      break;
    }

    pulFrame += pixCurrentSize;
    pixWidth >>= 1;
    pixHeight >>= 1;

    iMipOffset++;
  }

  return iMipOffset;
};

// Add 8-bit opaque alpha channel to a 24-bit bitmap
void AddAlphaChannel(UBYTE *pubSrcBitmap, ULONG *pulDstBitmap, PIX pixSize, UBYTE *pubAlphaBitmap)
{
  UBYTE ubR, ubG, ubB, ubA;

  // Go through all pixels of the bitmap in reverse
  for (INDEX i = pixSize - 1; i >= 0; i--) {
    // Extract RGB channels
    ubR = pubSrcBitmap[i * 3 + 0];
    ubG = pubSrcBitmap[i * 3 + 1];
    ubB = pubSrcBitmap[i * 3 + 2];

    // Get alpha channel if possible, otherwise force opaqueness
    ubA = (pubAlphaBitmap != NULL) ? pubAlphaBitmap[i] : 255;

    // Write ABGR channels
    pulDstBitmap[i] = ByteSwap(RGBAToColor(ubR, ubG, ubB, ubA));
  }
};

// Remove 8-bit alpha channel from a 32-bit bitmap
void RemoveAlphaChannel(ULONG *pulSrcBitmap, UBYTE *pubDstBitmap, PIX pixSize)
{
  UBYTE ubR, ubG, ubB;

  // Go through all pixels of the bitmap
  for (INDEX i = 0; i < pixSize; i++)
  {
    // Extract RGB channels from an ABGR pixel
    ColorToRGB(ByteSwap(pulSrcBitmap[i]), ubR, ubG, ubB);

    // Write only RGB channels
    pubDstBitmap[i * 3 + 0] = ubR;
    pubDstBitmap[i * 3 + 1] = ubG;
    pubDstBitmap[i * 3 + 2] = ubB;
  }
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
