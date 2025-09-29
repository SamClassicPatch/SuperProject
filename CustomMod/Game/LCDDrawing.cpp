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

// Background elements
static CTextureObject _toSamU;
static CTextureObject _toSamD;
static CTextureObject _toLeftU;
static CTextureObject _toLeftD;

// Screen area
static PIXaabbox2D _boxScreen;
static PIX _pixSizeI;
static PIX _pixSizeJ;

// [Cecil] Current resolution scale
static FLOAT _fScaleX;
static FLOAT _fScaleY;

// Current rendering parameters
static CDrawPort *_pdp = NULL;
static FLOAT _tmNow;
static ULONG _ulA;
static BOOL _bPopup;

void TiledTexture(PIXaabbox2D &boxScreen, FLOAT fStretch, MEX2D &vScreen, MEXaabbox2D &boxTexture)
{
  PIX pixW = boxScreen.Size()(1);
  PIX pixH = boxScreen.Size()(2);
  boxTexture = MEXaabbox2D(MEX2D(0, 0), MEX2D(pixW / fStretch, pixH / fStretch));
  boxTexture += vScreen;
};

void CGame::LCDInit(void) {
  try {
    _toSamU.SetData_t(CTFILENAME("TexturesMP\\General\\SamU.tex"));
    _toSamD.SetData_t(CTFILENAME("TexturesMP\\General\\SamD.tex"));
    _toLeftU.SetData_t(CTFILENAME("TexturesMP\\General\\LeftU.tex"));
    _toLeftD.SetData_t(CTFILENAME("TexturesMP\\General\\LeftD.tex"));

    // Force constant textures
    ((CTextureData *)_toSamU.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toSamD.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toLeftU.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)_toLeftD.GetData())->Force(TEX_CONSTANT);

  } catch (char *strError) {
    FatalError("%s", strError);
  }

  // [Cecil] Load theme for the first time
  _gmtTheme.Update();
};

void CGame::LCDEnd(void)
{
};

void CGame::LCDPrepare(FLOAT fFade) {
  // Get current time and alpha value
  CTimerValue tvPatch(ClassicsCore_GetInitTime()); // [Cecil]
  _tmNow = (FLOAT)(_pTimer->GetHighPrecisionTimer() - tvPatch).GetSeconds();
  _ulA = NormFloatToByte(fFade);
};

void CGame::LCDSetDrawport(CDrawPort *pdp) {
  _pdp = pdp;

  _pixSizeI = _pdp->GetWidth();
  _pixSizeJ = _pdp->GetHeight();
  _boxScreen = PIXaabbox2D(PIX2D(0, 0), PIX2D(_pixSizeI, _pixSizeJ));

  // [Cecil] Current resolution scale
  _fScaleX = _pixSizeI / 640.0f;
  _fScaleY = _pixSizeJ / 480.0f;

  if (pdp->dp_SizeIOverRasterSizeI == 1.0f) {
    _bPopup = FALSE;
  } else {
    _bPopup = TRUE;
  }
};

void CGame::LCDDrawBox(PIX pixUL, PIX pixDR, PIXaabbox2D &box, COLOR col) {
  TFEDrawBox(pixUL, pixDR, box, CECIL_COL_BORDER|255);
};

void CGame::LCDScreenBox(COLOR col) {
  LCDDrawBox(0, -1, _boxScreen, col);
};

void CGame::LCDScreenBoxOpenLeft(COLOR col) {
  TFEScreenBoxOpenLeft(CECIL_COL_BORDER|255);
};

void CGame::LCDScreenBoxOpenRight(COLOR col) {
  col = CECIL_COL_BORDER|255;

  // Top
  _pdp->DrawLine(_boxScreen.Min()(1) - 1, _boxScreen.Min()(2), 
                 _boxScreen.Max()(1) - 1, _boxScreen.Min()(2), col);
  // Bottom
  _pdp->DrawLine(_boxScreen.Min()(1) - 1, _boxScreen.Max()(2) - 1, 
                 _boxScreen.Max()(1) - 1, _boxScreen.Max()(2) - 1, col);
  // Left
  _pdp->DrawLine(_boxScreen.Min()(1), _boxScreen.Min()(2), 
                 _boxScreen.Min()(1), _boxScreen.Max()(2), col);
};

void CGame::LCDRenderClouds1(void) {
  // [Cecil] Render backdrop if it exists
  if (_gmtTheme.toBackdrop.GetData() != NULL) {
    _pdp->PutTexture(&_gmtTheme.toBackdrop, _boxScreen, C_WHITE|255);
  }

  if (!_bPopup) {
    // [Cecil] Draw elements using proper aspect ratio
    PIXaabbox2D box;
    INDEX iSize, iYU, iYM, iYB, iXL, iXR;

    // Right character - Sam
    if (_gmtTheme.bSam) {
      iSize = 170;
      iYU = 120;
      iYM = iYU + iSize;
      iYB = iYM + iSize;
      iXL = 420 * _fScaleX / _fScaleY;
      iXR = iXL + iSize;

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYU * _fScaleY), PIX2D(iXR * _fScaleY, iYM * _fScaleY));
      _pdp->PutTexture(&_toSamU, box, SE_COL_BLUE_NEUTRAL|255);

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYM * _fScaleY), PIX2D(iXR * _fScaleY, iYB * _fScaleY));
      _pdp->PutTexture(&_toSamD, box, SE_COL_BLUE_NEUTRAL|255);
    }

    // Left column
    if (_gmtTheme.bColumn) {
      iSize = 120;
      iYU = 0;
      iYM = iYU + iSize;
      iYB = iYM + iSize;
      iXL = -20;
      iXR = iXL + iSize;

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYU * _fScaleY), PIX2D(iXR * _fScaleY, iYM * _fScaleY));
      _pdp->PutTexture(&_toLeftU, box, SE_COL_BLUE_NEUTRAL|200);

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYM * _fScaleY), PIX2D(iXR * _fScaleY, iYB * _fScaleY));
      _pdp->PutTexture(&_toLeftD, box, SE_COL_BLUE_NEUTRAL|200);

      iYU = iYB;
      iYM = iYU + iSize;
      iYB = iYM + iSize;
      iXL = -20;
      iXR = iXL + iSize;

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYU * _fScaleY), PIX2D(iXR * _fScaleY, iYM * _fScaleY));
      _pdp->PutTexture(&_toLeftU, box, SE_COL_BLUE_NEUTRAL|200);

      box = PIXaabbox2D(PIX2D(iXL * _fScaleY, iYM * _fScaleY), PIX2D(iXR * _fScaleY, iYB * _fScaleY));
      _pdp->PutTexture(&_toLeftD, box, SE_COL_BLUE_NEUTRAL|200);
    }
  }

  if (_gmtTheme.bTFEClouds1) {
    TFERenderClouds1();

  // [Cecil] Clouds texture exists
  } else if (_gmtTheme.toClouds.GetData() != NULL) {
    MEXaabbox2D boxBcgClouds1;

    TiledTexture(_boxScreen, 1.2f * _fScaleX, MEX2D(sin(_tmNow * 0.5) * 35.0, sin(_tmNow * 0.7 + 1.0) * 21.0), boxBcgClouds1);
    _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, C_BLACK | (_ulA >> 2));

    TiledTexture(_boxScreen, 0.7f * _fScaleX, MEX2D(sin(_tmNow * 0.6 + 1.0) * 32.0, sin(_tmNow * 0.8f) * 25.0), boxBcgClouds1);
    _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, C_BLACK | (_ulA >> 2));
  }
};

void CGame::LCDRenderCloudsForComp(void)
{
  if (_gmtTheme.bTFEClouds1) {
    TFERenderClouds1();
    
  // [Cecil] Clouds texture exists
  } else if (_gmtTheme.toClouds.GetData() != NULL) {
    MEXaabbox2D boxBcgClouds1;

    TiledTexture(_boxScreen, 1.856f * _fScaleX, MEX2D(sin(_tmNow * 0.5) * 35.0, sin(_tmNow * 0.7) * 21.0), boxBcgClouds1);
    _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, CECIL_COL_COMPCLOUDS | (_ulA >> 2));

    TiledTexture(_boxScreen, 1.323f * _fScaleX, MEX2D(sin(_tmNow * 0.6) * 31.0, sin(_tmNow * 0.8) * 25.0), boxBcgClouds1);
    _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, CECIL_COL_COMPCLOUDS | (_ulA >> 2));
  }
};

void CGame::LCDRenderClouds2(void) {
  if (_gmtTheme.bTFEClouds2) {
    TFERenderClouds2();
  }
};

// [Cecil] Render TFE grid
static void RenderTFEGrid(void) {
  MEXaabbox2D boxBcgGrid;
  TiledTexture(_boxScreen, 1.0f, MEX2D(0, 0), boxBcgGrid);
  _pdp->PutTexture(&_gmtTheme.toGrid, _boxScreen, boxBcgGrid, SE_COL_BLUE_DARK | _ulA);
};

void CGame::LCDRenderGrid(void) {
  // [Cecil] No grid texture
  if (_gmtTheme.toGrid.GetData() == NULL) return;

  if (_gmtTheme.bTFEGrid) {
    RenderTFEGrid();
  }
};

void CGame::LCDRenderCompGrid(void) {
  // [Cecil] No grid texture
  if (_gmtTheme.toGrid.GetData() == NULL) return;

  if (_gmtTheme.bTFEGrid) {
    RenderTFEGrid();

  } else {
    MEXaabbox2D boxBcgGrid;
    TiledTexture(_boxScreen, 0.5f * _pixSizeI / (_pdp->dp_SizeIOverRasterSizeI * 640.0f), MEX2D(0, 0), boxBcgGrid);
    _pdp->PutTexture(&_gmtTheme.toGrid, _boxScreen, boxBcgGrid, SE_COL_BLUE_NEUTRAL | (_ulA >> 1));
  }
};

void CGame::LCDDrawPointer(PIX pixI, PIX pixJ) {
  CDisplayMode dmCurrent;
  _pGfx->GetCurrentDisplayMode(dmCurrent);

  // [Cecil] Draw custom pointer in borderless window mode
  BOOL bShowPointer = dmCurrent.IsFullScreen();
  extern INDEX gam_bShowPointerInBorderless;

  if (gam_bShowPointerInBorderless) {
    static CSymbolPtr iWindowMode("sam_iWindowMode");

    if (iWindowMode.Exists() && iWindowMode.GetIndex() != 0) {
      bShowPointer = TRUE;
    }
  }

  if (bShowPointer) {
    while (ShowCursor(FALSE) >= 0);

  } else {
    if (!_pInput->IsInputEnabled()) {
      while (ShowCursor(TRUE) < 0);
    }
    return;
  }

  PIX pixSizeI = _gmtTheme.toPointer.GetWidth();
  PIX pixSizeJ = _gmtTheme.toPointer.GetHeight();
  pixI -= 1;
  pixJ -= 1;

  _pdp->PutTexture(&_gmtTheme.toPointer, PIXaabbox2D(PIX2D(pixI, pixJ), PIX2D(pixI + pixSizeI, pixJ + pixSizeJ)), LCDFadedColor(C_WHITE|255));
};

COLOR CGame::LCDGetColor(COLOR colDefault, const char *strName)
{
  if (!strcmp(strName, "thumbnail border")) {
    colDefault = SE_COL_BLUE_NEUTRAL|255;
  } else if (!strcmp(strName, "no thumbnail")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "popup box")) {
    colDefault = SE_COL_BLUE_NEUTRAL|255;
  } else if (!strcmp(strName, "tool tip")) {
    colDefault = CECIL_COL_LIGHT|255; // [Cecil] New separate color
  } else if (!strcmp(strName, "unselected")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "selected")) {
    colDefault = CECIL_COL_LIGHT|255; // [Cecil] New separate color
  } else if (!strcmp(strName, "disabled selected")) {
    colDefault = SE_COL_ORANGE_DARK_LT|255;
  } else if (!strcmp(strName, "disabled unselected")) {
    colDefault = SE_COL_ORANGE_DARK|255;
  } else if (!strcmp(strName, "label")) {
    colDefault = C_WHITE|255;
  } else if (!strcmp(strName, "title")) {
    colDefault = C_WHITE|255;
  } else if (!strcmp(strName, "editing")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "hilited")) {
    colDefault = CECIL_COL_LIGHT|255; // [Cecil] New separate color
  } else if (!strcmp(strName, "hilited rectangle")) {
    colDefault = CECIL_COL_NEUTRAL|255; // [Cecil] New separate color
  } else if (!strcmp(strName, "edit fill")) {
    colDefault = SE_COL_BLUE_DARK_LT|75;
  } else if (!strcmp(strName, "editing cursor")) {
    colDefault = CECIL_COL_NEUTRAL|255; // [Cecil] New separate color
  } else if (!strcmp(strName, "model box")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "hiscore header")) {
    colDefault = SE_COL_ORANGE_LIGHT|255;
  } else if (!strcmp(strName, "hiscore data")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "hiscore last set")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "slider box")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "file info")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "display mode")) {
    colDefault = SE_COL_ORANGE_NEUTRAL|255;
  } else if (!strcmp(strName, "bcg fill")) {
    colDefault = SE_COL_BLUE_DARK|255;
  }

  return colDefault;
};

COLOR CGame::LCDFadedColor(COLOR col) {
  return MulColors(C_WHITE | _ulA, col);
};

COLOR CGame::LCDBlinkingColor(COLOR col0, COLOR col1) {
  return LerpColor(col0, col1, sin(_tmNow * 10.0f) * 0.5f + 0.5f);
};

// [Cecil] Old LCD functions from TFE that are still used on their own in computer
void CGame::TFEDrawBox(PIX pixUL, PIX pixDR, PIXaabbox2D &box, COLOR col) {
  // Top
  _pdp->DrawLine(box.Min()(1) - pixUL, box.Min()(2) - pixUL,
                 box.Max()(1) + pixDR, box.Min()(2) - pixUL, col);
  // Bottom
  _pdp->DrawLine(box.Min()(1) - pixUL, box.Max()(2) + pixDR,
                 box.Max()(1) + pixDR, box.Max()(2) + pixDR, col);
  // Left
  _pdp->DrawLine(box.Min()(1) - pixUL, box.Min()(2) - pixUL,
                 box.Min()(1) - pixUL, box.Max()(2) + pixDR, col);
  // Right
  _pdp->DrawLine(box.Max()(1) + pixDR, box.Min()(2) - pixUL,
                 box.Max()(1) + pixDR, box.Max()(2) + pixDR + 1, col);
};

void CGame::TFEScreenBox(COLOR col) {
  TFEDrawBox(0, -1, _boxScreen, col);
};

void CGame::TFEScreenBoxOpenLeft(COLOR col) {
  // Top
  _pdp->DrawLine(_boxScreen.Min()(1) - 1, _boxScreen.Min()(2),
                 _boxScreen.Max()(1) - 1, _boxScreen.Min()(2), col);
  // Bottom
  _pdp->DrawLine(_boxScreen.Min()(1) - 1, _boxScreen.Max()(2) - 1,
                 _boxScreen.Max()(1) - 1, _boxScreen.Max()(2) - 1, col);
  // Right
  _pdp->DrawLine(_boxScreen.Max()(1) - 1, _boxScreen.Min()(2),
                 _boxScreen.Max()(1) - 1, _boxScreen.Max()(2), col);
};

void CGame::TFERenderClouds1(void) {
  // [Cecil] No clouds texture
  if (_gmtTheme.toClouds.GetData() == NULL) return;

  MEXaabbox2D boxBcgClouds1;

  TiledTexture(_boxScreen, 1.3f * _fScaleX, MEX2D(sin(_tmNow * 0.75) * 50.0, sin(_tmNow * 0.9) * 40.0), boxBcgClouds1);
  _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, SE_COL_BLUE_DARK | (_ulA >> 1));

  TiledTexture(_boxScreen, 0.8f * _fScaleX, MEX2D(sin(_tmNow * 0.95) * 50.0, sin(_tmNow * 0.8) * 40.0), boxBcgClouds1);
  _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds1, SE_COL_BLUE_DARK | (_ulA >> 1));
};

void CGame::TFERenderClouds2(void) {
  // [Cecil] No clouds texture
  if (_gmtTheme.toClouds.GetData() == NULL) return;

  MEXaabbox2D boxBcgClouds2;
  TiledTexture(_boxScreen, 0.5f * _fScaleX, MEX2D(2, 10), boxBcgClouds2);
  _pdp->PutTexture(&_gmtTheme.toClouds, _boxScreen, boxBcgClouds2, C_BLACK | (_ulA >> 1));
};
