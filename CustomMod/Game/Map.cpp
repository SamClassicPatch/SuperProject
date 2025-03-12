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

#include "StdAfx.h"

#include "Cecil/Map.h"

// [Cecil] Current map type
ELevelFormat _eMapType = E_LF_CURRENT;

// [Cecil] Determine map type from level name
ELevelFormat ScanLevelName(const CTString &strLevelName) {
  INDEX u, v;
  CTString &strLevel = const_cast<CTString &>(strLevelName);

  // TSE level
  if (strLevel.ScanF("%01d_%01d_", &u, &v) == 2) return E_LF_TSE;

  // TFE level
  if (strLevel.ScanF("%02d_", &u) == 1) return E_LF_TFE;

  // Unknown type
  return E_LF_CURRENT;
};

// [Cecil] Determine map type from level name and return its index
ELevelFormat ScanLevelName(INDEX &iLevel, const CTString &strLevelName) {
  INDEX u, v;
  CTString &strLevel = const_cast<CTString &>(strLevelName);

  iLevel = -1;

  // TSE level
  if (strLevel.ScanF("%01d_%01d_", &u, &v) == 2) {
    iLevel = u * 10 + v;

    extern void RemapLevelNames(INDEX &iLevel);
    RemapLevelNames(iLevel);

    return E_LF_TSE;
  }

  // TFE level
  if (strLevel.ScanF("%02d_", &u) == 1) {
    // Account for the intro level
    iLevel = u + 1;

    return E_LF_TFE;
  }

  // Unknown type
  return E_LF_CURRENT;
};

static CStaticArray<CTextureObject> _atoIcons;
static CTextureObject _toPathDot;
static CTextureObject _toMapBcgLD;
static CTextureObject _toMapBcgLU;
static CTextureObject _toMapBcgRD;
static CTextureObject _toMapBcgRU;

static void ReleaseMapData(void)
{
  for (INDEX iIcon = 0; iIcon < _atoIcons.Count(); iIcon++) {
    _atoIcons[iIcon].SetData(NULL);
  }
  _atoIcons.Clear();

  _toPathDot.SetData(NULL);
  _toMapBcgLD.SetData(NULL);
  _toMapBcgLU.SetData(NULL);
  _toMapBcgRD.SetData(NULL);
  _toMapBcgRU.SetData(NULL);
};

static BOOL ObtainMapData(void) {
  // [Cecil] Only load new textures for a new map type
  static INDEX iLastMapType = -1;

  if (iLastMapType == _eMapType) return TRUE;
  iLastMapType = _eMapType;

  // [Cecil] Reset textures
  ReleaseMapData();

  try {
    if (_eMapType == E_LF_TSE) {
      _atoIcons.New(13);
      _atoIcons[ 0].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Book.tex"));
      _atoIcons[ 1].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level00.tex"));
      _atoIcons[ 2].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level01.tex"));
      _atoIcons[ 3].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level02.tex"));
      _atoIcons[ 4].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level03.tex"));
      _atoIcons[ 5].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level04.tex"));
      _atoIcons[ 6].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level05.tex"));
      _atoIcons[ 7].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level06.tex"));
      _atoIcons[ 8].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level07.tex"));
      _atoIcons[ 9].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level08.tex"));
      _atoIcons[10].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level09.tex"));
      _atoIcons[11].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level10.tex"));
      _atoIcons[12].SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\Level11.tex"));

      _toPathDot.SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\PathDot.tex"));
      _toMapBcgLD.SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\MapBcgLD.tex"));
      _toMapBcgLU.SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\MapBcgLU.tex"));
      _toMapBcgRD.SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\MapBcgRD.tex"));
      _toMapBcgRU.SetData_t(CTFILENAME("TexturesMP\\Computer\\Map\\MapBcgRU.tex"));

    } else {
      _atoIcons.New(16);
      _atoIcons[ 0].SetData(NULL); // [Cecil] TFE has no intro picture
      _atoIcons[ 1].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level00.tex"));
      _atoIcons[ 2].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level01.tex"));
      _atoIcons[ 3].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level02.tex"));
      _atoIcons[ 4].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level03.tex"));
      _atoIcons[ 5].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level04.tex"));
      _atoIcons[ 6].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level05.tex"));
      _atoIcons[ 7].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level06.tex"));
      _atoIcons[ 8].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level07.tex"));
      _atoIcons[ 9].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level08.tex"));
      _atoIcons[10].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level09.tex"));
      _atoIcons[11].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level10.tex"));
      _atoIcons[12].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level11.tex"));
      _atoIcons[13].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level12.tex"));
      _atoIcons[14].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level13.tex"));
      _atoIcons[15].SetData_t(CTFILENAME("Textures\\Computer\\Map\\Level14.tex"));

      _toPathDot.SetData_t(CTFILENAME("Textures\\Computer\\Map\\PathDot.tex"));
      _toMapBcgLD.SetData_t(CTFILENAME("Textures\\Computer\\Map\\MapBcgLD.tex"));
      _toMapBcgLU.SetData_t(CTFILENAME("Textures\\Computer\\Map\\MapBcgLU.tex"));
      _toMapBcgRD.SetData_t(CTFILENAME("Textures\\Computer\\Map\\MapBcgRD.tex"));
      _toMapBcgRU.SetData_t(CTFILENAME("Textures\\Computer\\Map\\MapBcgRU.tex"));
    }

    // force constant textures
    for (INDEX iIcon = 0; iIcon < _atoIcons.Count(); iIcon++) {
      CTextureData *ptd = (CTextureData *)_atoIcons[iIcon].GetData();

      if (ptd != NULL) {
        ptd->Force(TEX_CONSTANT);
      }
    }

    ((CTextureData *)_toPathDot.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toMapBcgLD.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toMapBcgLU.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toMapBcgRD.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toMapBcgRU.GetData())->Force(TEX_CONSTANT);

  } catch (char *strError) {
    CPrintF("%s\n", strError);
    return FALSE;
  }

  return TRUE;
};

void RenderMap( CDrawPort *pdp, ULONG ulLevelMask, CProgressHookInfo *pphi)
{
  if (!ObtainMapData()) {
    ReleaseMapData();
    return;
  }

  PIX pixdpw = pdp->GetWidth();
  PIX pixdph = pdp->GetHeight();
  PIX imgw = 512;
  PIX imgh = 480;
  FLOAT fStretch = 0.25f;
  
  // determine max available picture stretch
  if( pixdpw>=imgw*2 && pixdph>=imgh*2) {
    fStretch = 2.0f;
  } else if(pixdpw>=imgw && pixdph>=imgh) {
    fStretch = 1.0f;
  } else if(pixdpw>=imgw/2 && pixdph>=imgh/2) {
    fStretch = 0.5f;
  } 

  // calculate LU offset so picture would be centerd in dp
  PIX pixSX = (pixdpw-imgw*fStretch)/2;
  PIX pixSY = Max( PIX((pixdph-imgh*fStretch)/2), PIX(0));
  
  PIX pixC1S = pixSX;                  // column 1 start pixel
  PIX pixR1S = pixSY;                  // raw 1 start pixel
  PIX pixC1E = pixSX+256*fStretch;     // column 1 end pixel
  PIX pixR1E = pixSY+256*fStretch;     // raw 1 end pixel
  PIX pixC2S = pixC1E-fStretch;        // column 2 start pixel
  PIX pixR2S = pixR1E-fStretch;        // raw 2 start pixel
  PIX pixC2E = pixC2S+256*fStretch;    // column 2 end pixel
  PIX pixR2E = pixR2S+256*fStretch;    // raw 2 end pixel

  // [Cecil] Select arrays for the map type
  const IntPair_t *aIconCoords = (_eMapType == E_LF_TSE ? _aIconCoordsTSE : _aIconCoordsTFE);
  const PathDots_t *aPathDots = (_eMapType == E_LF_TSE ? _aPathDotsTSE : _aPathDotsTFE);
  const IntPair_t *aPathPrevNextLevels = (_eMapType == E_LF_TSE ? _aPathBetweenLevelsTSE : _aPathBetweenLevelsTFE);

  // Render intro background
  if (ulLevelMask == 0x1 && _atoIcons[0].GetData() != NULL) {
    PIX pixX = aIconCoords[0][0] * fStretch + pixC1S;
    PIX pixY = aIconCoords[0][1] * fStretch + pixR1S;

    pdp->PutTexture(&_atoIcons[0], PIXaabbox2D(PIX2D(pixC1S, pixR1S), PIX2D(pixC2E, pixR2E)), C_WHITE|255);

  } else {
    // Render map background
    pdp->PutTexture(&_toMapBcgLU, PIXaabbox2D(PIX2D(pixC1S, pixR1S), PIX2D(pixC1E, pixR1E)), C_WHITE|255);
    pdp->PutTexture(&_toMapBcgRU, PIXaabbox2D(PIX2D(pixC2S, pixR1S), PIX2D(pixC2E, pixR1E)), C_WHITE|255);
    pdp->PutTexture(&_toMapBcgLD, PIXaabbox2D(PIX2D(pixC1S, pixR2S), PIX2D(pixC1E, pixR2E)), C_WHITE|255);
    pdp->PutTexture(&_toMapBcgRD, PIXaabbox2D(PIX2D(pixC2S, pixR2S), PIX2D(pixC2E, pixR2E)), C_WHITE|255);

    // Render map icons
    for (INDEX iIcon = 1; iIcon < _atoIcons.Count(); iIcon++)
    {
      // if level's icon should be rendered
      if( ulLevelMask & (1UL<<iIcon))
      {
        PIX pixX = aIconCoords[iIcon][0]*fStretch+pixC1S;
        PIX pixY = aIconCoords[iIcon][1]*fStretch+pixR1S;

        CTextureObject *pto = &_atoIcons[iIcon];
        PIX pixImgW = ((CTextureData *)pto->GetData())->GetPixWidth() * fStretch;
        PIX pixImgH = ((CTextureData *)pto->GetData())->GetPixHeight() * fStretch;

        pdp->PutTexture(pto, PIXaabbox2D(PIX2D(pixX, pixY), PIX2D(pixX + pixImgW, pixY + pixImgH)), C_WHITE|255);
      }
    }
  }
  
  // render paths
  for (INDEX iPath = 0; ; iPath++)
  {
    INDEX iPrevLevelBit = aPathPrevNextLevels[iPath][0];
    INDEX iNextLevelBit = aPathPrevNextLevels[iPath][1];

    // [Cecil] Reached the end
    if (iPrevLevelBit == -1 || iNextLevelBit == -1) break;
    
    // if path dots should be rendered:
    // if path src and dst levels were discovered and secret level isn't inbetween or hasn't been discovered
    if( ulLevelMask&(1UL<<iPrevLevelBit) &&
        ulLevelMask&(1UL<<iNextLevelBit) &&
        ((iNextLevelBit-iPrevLevelBit)==1 || !(ulLevelMask&(1UL<<(iNextLevelBit-1)))))
    {
      for( INDEX iDot=0; iDot<10; iDot++)
      {
        PIX pixDotX=pixC1S+aPathDots[iPath][iDot][0]*fStretch;
        PIX pixDotY=pixR1S+aPathDots[iPath][iDot][1]*fStretch;
        if(aPathDots[iPath][iDot][0]==-1) break;

        COLOR colPathDot = (_eMapType == E_LF_TSE ? C_BLACK : C_WHITE) | 255;
        pdp->PutTexture(&_toPathDot, PIXaabbox2D(PIX2D(pixDotX, pixDotY), PIX2D(pixDotX + 8 * fStretch, pixDotY + 8 * fStretch)), colPathDot);
      }
    }
  }

  if( pphi != NULL)
  {
    // set font
    pdp->SetFont( _pfdDisplayFont);
    pdp->SetTextScaling( fStretch);
    pdp->SetTextAspect( 1.0f);
    
    INDEX iPosX, iPosY;
    COLOR colText;

    // set coordinates and dot colors
    if (_eMapType == E_LF_TSE) {
      if (ulLevelMask == 0x1) {
        iPosX = 200;
        iPosY = 330;
        colText = 0x5c6a9aff;
      } else {
        iPosX = 395; 
        iPosY = 403;
        colText = 0xc87832ff;
      }

    } else {
      iPosX = 116;
      iPosY = 220;
      colText = RGBToColor(200, 128, 56) | CT_OPAQUE;
    }

    PIX pixhtcx = pixC1S+iPosX*fStretch;
    PIX pixhtcy = pixR1S+iPosY*fStretch;

    pdp->PutTextC( pphi->phi_strDescription, pixhtcx, pixhtcy, colText);
    for( INDEX iProgresDot=0; iProgresDot<16; iProgresDot+=1)
    {
      if (_eMapType == E_LF_TSE) {
        PIX pixDotX = pixC1S + ((iPosX - 68) + iProgresDot * 8) * fStretch;
        PIX pixDotY = pixR1S + (iPosY + 19) * fStretch;

        COLOR colDot = colText|255;

        if (iProgresDot > pphi->phi_fCompleted * 16) {
          colDot = C_BLACK|64;
        }

        pdp->PutTexture(&_toPathDot, PIXaabbox2D(PIX2D(pixDotX, pixDotY),
          PIX2D(pixDotX + 2 + 8 * fStretch, pixDotY + 2 + 8 * fStretch)), C_BLACK|255);

        pdp->PutTexture(&_toPathDot, PIXaabbox2D(PIX2D(pixDotX, pixDotY),
          PIX2D(pixDotX + 8 * fStretch, pixDotY + 8 * fStretch)), colDot);

      } else {
        PIX pixDotX = pixC1S + (48 + iProgresDot * 8) * fStretch;
        PIX pixDotY = pixR1S + 249 * fStretch;

        COLOR colDot = C_WHITE|255;

        if (iProgresDot > pphi->phi_fCompleted * 16) {
          colDot = C_WHITE|64;
        }

        pdp->PutTexture(&_toPathDot, PIXaabbox2D(PIX2D(pixDotX, pixDotY),
          PIX2D(pixDotX + 8 * fStretch, pixDotY + 8 * fStretch)), colDot);
      }
    }
  }
}
