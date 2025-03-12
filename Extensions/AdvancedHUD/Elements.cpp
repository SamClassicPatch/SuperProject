/* Copyright (c) 2023-2024 Dreamy Cecil
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

#include "HUD.h"

#include "Sorting.inl"

// Prepare color transitions
void CHud::PrepareColorTransitions(COLOR colFine, COLOR colHigh, COLOR colMedium, COLOR colLow,
  FLOAT fMediumHigh, FLOAT fLowMedium, BOOL bSmooth)
{
  _cttHUD.ctt_colFine     = colFine;
  _cttHUD.ctt_colHigh     = colHigh;   
  _cttHUD.ctt_colMedium   = colMedium;
  _cttHUD.ctt_colLow      = colLow;
  _cttHUD.ctt_fMediumHigh = fMediumHigh;
  _cttHUD.ctt_fLowMedium  = fLowMedium;
  _cttHUD.ctt_bSmooth     = bSmooth;
};

// Calculate shake amount and color value depending on value change
COLOR CHud::AddShaker(const PIX pixAmount, const INDEX iCurrentValue, INDEX &iLastValue,
                      FLOAT &tmChanged, FLOAT &fMoverX, FLOAT &fMoverY)
{
  // Update shaking if needed
  fMoverX = 0.0f;
  fMoverY = 0.0f;

  const TIME tmNow = _pTimer->GetLerpedCurrentTick();
  #define SHAKE_TIME (2.0f)

  if (iCurrentValue != iLastValue) {
    iLastValue = iCurrentValue;
    tmChanged  = tmNow;

  } else {
    // In case of loading (timer got reset)
    tmChanged = ClampUp((FLOAT)tmChanged, (FLOAT)tmNow);
  }

  const TIME tmDelta = tmNow - tmChanged;

  // No shake
  if (tmDelta > SHAKE_TIME) return NONE;

  ASSERT(tmDelta >= 0);

  // Add shake
  const FLOAT fAmount = _vScaling(1) * _fCustomScaling * pixAmount;
  const FLOAT fMultiplier = (SHAKE_TIME - tmDelta) / SHAKE_TIME * fAmount;
  const INDEX iRandomizer = INDEX(tmNow * 511.0f) * fAmount * iCurrentValue;
  const FLOAT fNormRnd1 = FLOAT((iRandomizer ^ (iRandomizer >> 9)) & 1023) * 0.0009775f; // 1/1023 - normalized
  const FLOAT fNormRnd2 = FLOAT((iRandomizer ^ (iRandomizer >> 7)) & 1023) * 0.0009775f; // 1/1023 - normalized

  // Set and clamp to adjusted amounts
  if (_psIconShake.GetIndex()) {
    fMoverX = Clamp((fNormRnd1 - 0.5f) * fMultiplier, -fAmount, fAmount);
    fMoverY = Clamp((fNormRnd2 - 0.5f) * fMultiplier, -fAmount, fAmount);
  }

  if (tmDelta < SHAKE_TIME / 3.0f) {
    return C_WHITE;
  }

  return NONE;
};

// Get current color from local color transitions table
COLOR CHud::GetCurrentColor(FLOAT fNormValue)
{
  // Plain low color
  if (fNormValue < _cttHUD.ctt_fLowMedium) return (_cttHUD.ctt_colLow & 0xFFFFFF00);

  // Plain high color
  if (fNormValue > 1.0f) return (_cttHUD.ctt_colFine & 0xFFFFFF00);

  // If colors should be blended
  if (_cttHUD.ctt_bSmooth) {
    // Determine two colors for interpolation
    FLOAT f1, f2;
    COLOR col1, col2;

    if (fNormValue > _cttHUD.ctt_fMediumHigh) {
      f1 = 1.0f;
      f2 = _cttHUD.ctt_fMediumHigh;
      col1 = _cttHUD.ctt_colHigh;
      col2 = _cttHUD.ctt_colMedium;

    } else {
      f1 = _cttHUD.ctt_fMediumHigh;
      f2 = _cttHUD.ctt_fLowMedium;
      col1 = _cttHUD.ctt_colMedium;
      col2 = _cttHUD.ctt_colLow;
    }

    // Determine interpolation strength
    FLOAT fDelta = (fNormValue - f2) / (f1 - f2);

    // Convert colors to HSV
    UBYTE ubH1, ubS1, ubV1, ubH2, ubS2, ubV2;
    ColorToHSV(col1, ubH1, ubS1, ubV1);
    ColorToHSV(col2, ubH2, ubS2, ubV2);

    // Interpolate HSV components
    ubH1 = UBYTE(ubH1 * fDelta + ubH2 * (1.0f - fDelta));
    ubS1 = UBYTE(ubS1 * fDelta + ubS2 * (1.0f - fDelta));
    ubV1 = UBYTE(ubV1 * fDelta + ubV2 * (1.0f - fDelta));

    // Convert back to the color value
    return HSVToColor(ubH1, ubS1, ubV1);
  }

  // Simple color picker
  if (fNormValue > _cttHUD.ctt_fMediumHigh) {
    return (_cttHUD.ctt_colHigh & 0xFFFFFF00);
  }

  return (_cttHUD.ctt_colMedium & 0xFFFFFF00);
};

// Gather all players in the array
void CHud::GatherPlayers(void) {
  _cenPlayers.Clear();

  // [Cecil] NOTE: Not using CEntity::GetPlayerEntity() in case there are
  // other CPlayer entities that aren't real players (e.g. modded bots).
  FOREACHINDYNAMICCONTAINER(_penPlayer->GetWorld()->wo_cenEntities, CEntity, iten) {
    CEntity *pen = iten;
    if (!IsDerivedFromID(pen, CPlayer_ClassID)) continue;

    // Skip invalid players and predictors
    if (pen->GetFlags() & ENF_DELETED || pen->IsPredictor()) continue;

    // Count this player
    _cenPlayers.Add((CPlayer *)pen);
  }
};

// Fill array with players sorted by a specific statistic
void CHud::SetAllPlayersStats(CDynamicContainer<CPlayer> &cen, INDEX iSortKey) {
  // Copy all players
  cen.CopyArray(_cenPlayers);

  // Pick sorting function
  typedef int (*CSortingFunc)(const void *, const void *);

  static CSortingFunc apFunctions[] = {
    &qsort_CompareNames,
    &qsort_CompareHealth,
    &qsort_CompareScores,
    &qsort_CompareManas,
    &qsort_CompareFrags,
    &qsort_CompareDeaths,
  };

  // Sort the array
  if (iSortKey >= 0 && iSortKey < 6) {
    qsort(cen.sa_Array, cen.Count(), sizeof(CPlayer *), apFunctions[iSortKey]);
  }
};

// Draw border using a tile texture
void CHud::DrawBorder(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, COLOR colTiles)
{
  // Determine location
  const FLOAT fCenterI = fX * _vScaling(1);
  const FLOAT fCenterJ = fY * _vScaling(2);
  const FLOAT fSizeI = fW * _vScaling(1);
  const FLOAT fSizeJ = fH * _vScaling(1);
  const FLOAT fTileSize = 8 * _vScaling(1) * _fCustomScaling;

  // Determine exact positions
  const FLOAT fLeft  = fCenterI - fSizeI / 2 - 1; 
  const FLOAT fRight = fCenterI + fSizeI / 2 + 1; 
  const FLOAT fUp    = fCenterJ - fSizeJ / 2 - 1; 
  const FLOAT fDown  = fCenterJ + fSizeJ / 2 + 1;
  const FLOAT fLeftEnd  = fLeft  + fTileSize;
  const FLOAT fRightBeg = fRight - fTileSize;
  const FLOAT fUpEnd    = fUp    + fTileSize;
  const FLOAT fDownBeg  = fDown  - fTileSize;

  colTiles |= _ulAlphaHUD;

  // Put corners
  _pdp->InitTexture(&tex.toTile, TRUE);
  _pdp->AddTexture(fLeft,  fUp,   fLeftEnd,  fUpEnd,   colTiles);
  _pdp->AddTexture(fRight, fUp,   fRightBeg, fUpEnd,   colTiles);
  _pdp->AddTexture(fRight, fDown, fRightBeg, fDownBeg, colTiles);
  _pdp->AddTexture(fLeft,  fDown, fLeftEnd,  fDownBeg, colTiles);

  // Put edges
  _pdp->AddTexture(fLeftEnd, fUp,    fRightBeg, fUpEnd,   0.4f, 0.0f, 0.6f, 1.0f, colTiles);
  _pdp->AddTexture(fLeftEnd, fDown,  fRightBeg, fDownBeg, 0.4f, 0.0f, 0.6f, 1.0f, colTiles);
  _pdp->AddTexture(fLeft,    fUpEnd, fLeftEnd,  fDownBeg, 0.0f, 0.4f, 1.0f, 0.6f, colTiles);
  _pdp->AddTexture(fRight,   fUpEnd, fRightBeg, fDownBeg, 0.0f, 0.4f, 1.0f, 0.6f, colTiles);

  // Fill center
  _pdp->AddTexture(fLeftEnd, fUpEnd, fRightBeg, fDownBeg, 0.4f, 0.4f, 0.6f, 0.6f, colTiles);
  _pdp->FlushRenderingQueue();
};

// Draw icon texture
void CHud::DrawIcon(FLOAT fX, FLOAT fY, CTextureObject &toIcon, COLOR colDefault, FLOAT fNormValue, BOOL bBlink)
{
  // Blink only when the value is lower than half of the medium threshold
  if (bBlink && fNormValue <= _cttHUD.ctt_fLowMedium * 0.5f) {
    if (INDEX(_tmNow * 4.0f) & 1) {
      colDefault = C_vdGRAY;
    }
  }

  // Determine location and dimensions
  const FLOAT fCenterI = fX * _vScaling(1);
  const FLOAT fCenterJ = fY * _vScaling(2);

  CTextureData *ptd = (CTextureData *)toIcon.GetData();
  const FLOAT fSize = 16 * _vScaling(1) * _fCustomScaling;

  _pdp->InitTexture(&toIcon);
  _pdp->AddTexture(fCenterI - fSize, fCenterJ - fSize, fCenterI + fSize, fCenterJ + fSize, colDefault | _ulAlphaHUD);
  _pdp->FlushRenderingQueue();
};

// Draw text
void CHud::DrawString(FLOAT fX, FLOAT fY, const CTString &strText, COLOR colDefault, FLOAT fNormValue)
{
  // Determine location
  const FLOAT fFontScaling = (FLOAT)_pfdCurrentNumbers->GetHeight() * 0.03125f; // (1 / 32)

  _pdp->SetTextScaling(_vScaling(1) * _fCustomScaling / fFontScaling);
  _pdp->PutTextCXY(strText, fX * _vScaling(1), fY * _vScaling(2), colDefault | _ulAlphaHUD);
};

// Draw percentage bar
void CHud::DrawBar(FLOAT fX, FLOAT fY, PIX pixW, PIX pixH, EBarDir eBarDir, COLOR colDefault, FLOAT fNormValue)
{
  // Determine location and size
  PIX pixSizeI = pixW * _vScaling(1);
  PIX pixSizeJ = pixH * _vScaling(1);

  // Fill bar background area
  PIX pixLeft  = PIX(fX * _vScaling(1)) - (pixSizeI >> 1);
  PIX pixUpper = PIX(fY * _vScaling(2)) - (pixSizeJ >> 1);

  // Determine bar position and inner size
  switch (eBarDir) {
    case E_BD_UP:
      pixSizeJ *= fNormValue;
      break;

    case E_BD_DOWN:
      pixUpper = pixUpper + (PIX)ceil(pixSizeJ * (1.0f - fNormValue));
      pixSizeJ *= fNormValue;
      break;

    case E_BD_LEFT:
      pixSizeI *= fNormValue;
      break;

    case E_BD_RIGHT:
      pixLeft = pixLeft + (PIX)ceil(pixSizeI * (1.0f - fNormValue));
      pixSizeI *= fNormValue;
      break;
  }

  _pdp->Fill(pixLeft, pixUpper, pixSizeI, pixSizeJ, colDefault | _ulAlphaHUD);
};

// Draw texture rotated at a certain angle
void CHud::DrawRotatedQuad(CTextureObject *pto, FLOAT fX, FLOAT fY, FLOAT fSize, ANGLE aAngle, COLOR col) {
  FLOAT fSinA = Sin(aAngle);
  FLOAT fCosA = Cos(aAngle);
  FLOAT fSinPCos = fCosA * fSize + fSinA * fSize;
  FLOAT fSinMCos = fSinA * fSize - fCosA * fSize;

  FLOAT fI0 = fX - fSinPCos;
  FLOAT fJ0 = fY - fSinMCos;

  FLOAT fI1 = fX + fSinMCos;
  FLOAT fJ1 = fY - fSinPCos;

  FLOAT fI2 = fX + fSinPCos;
  FLOAT fJ2 = fY + fSinMCos;

  FLOAT fI3 = fX - fSinMCos;
  FLOAT fJ3 = fY + fSinPCos;

  _pdp->InitTexture(pto);
  _pdp->AddTexture(fI0, fJ0, 0, 0, col, fI1, fJ1, 0, 1, col, fI2, fJ2, 1, 1, col, fI3, fJ3, 1, 0, col);
  _pdp->FlushRenderingQueue();  
};

// Draw texture with a correct aspect ratio
void CHud::DrawCorrectTexture(CTextureObject *pto, FLOAT fX, FLOAT fY, FLOAT fWidth, COLOR col) {
  CTextureData *ptd = (CTextureData *)pto->GetData();
  FLOAT fTexSizeJ = ptd->GetPixHeight();
  FLOAT fHeight = fWidth * fTexSizeJ / fTexSizeJ;

  _pdp->InitTexture(pto);
  _pdp->AddTexture(fX - fWidth * 0.5f, fY - fHeight * 0.5f, fX + fWidth * 0.5f, fY + fHeight * 0.5f, 0, 0, 1, 1, col);
  _pdp->FlushRenderingQueue();
};

#if SE1_GAME != SS_TFE

// Draw sniper mask
void CHud::DrawSniperMask(void) {
  // Determine location
  const FLOAT fW = _vpixScreen(1);
  const FLOAT fH = _vpixScreen(2);
  const FLOAT fX = _vpixScreen(1) * 0.5f;
  const FLOAT fY = _vpixScreen(2) * 0.5f;
  const FLOAT fBorder = (_vpixScreen(1) - _vpixScreen(2)) * 0.5f;

  const UBYTE ubScopeAlpha = NormFloatToByte(_psScopeAlpha.GetFloat());
  COLOR colMask = 0xFFFFFF00 | ubScopeAlpha;

  // Sniper mask
  _pdp->InitTexture(&tex.toSniperMask);
  _pdp->AddTexture(fBorder, 0,  fX,           fY, 0.98f, 0.02f,  0.0f,  1.0f, colMask);
  _pdp->AddTexture(fX,      0,  fW - fBorder, fY,  0.0f, 0.02f, 0.98f,  1.0f, colMask);
  _pdp->AddTexture(fBorder, fY, fX,           fH, 0.98f,  1.0f,  0.0f, 0.02f, colMask);
  _pdp->AddTexture(fX,      fY, fW - fBorder, fH,  0.0f,  1.0f, 0.98f, 0.02f, colMask);
  _pdp->FlushRenderingQueue();

  // Side borders
  _pdp->Fill(0,            0, fBorder, fH, C_BLACK | ubScopeAlpha);
  _pdp->Fill(fW - fBorder, 0, fBorder, fH, C_BLACK | ubScopeAlpha);

  // Center dot with inverted alpha
  const PIX pixDotSize = 2 * _vScaling(1);
  _pdp->Fill(fX - (pixDotSize >> 1), fY - (pixDotSize >> 1), pixDotSize, pixDotSize, C_BLACK | UBYTE(~ubScopeAlpha));

  colMask = LerpColor(COL_ScopeMask(), C_WHITE, 0.25f);

  FLOAT fScalingY = fH / 480.0f;

  FLOAT fDistance = _penWeapons->m_fRayHitDistance;
  FLOAT aFOV = Lerp(_penWeapons->m_fSniperFOVlast, _penWeapons->m_fSniperFOV, _pTimer->GetLerpFactor());

  // Zoom wheel
  FLOAT fZoom = 1.0f / tan(RadAngle(aFOV) * 0.5f); // 2.0 - 8.0

  FLOAT fAFact = (Clamp(aFOV, 14.2f, 53.1f) - 14.2f) / (53.1f - 14.2f); // For zooms 2 to 4 times
  ANGLE aAngle = 314.0f + fAFact * 292.0f;

  COLOR colSniperWheel = colMask | 0x44;
  const FLOAT fEnemyHealth = _penWeapons->m_fEnemyHealth;

  if (_psScopeColoring.GetIndex()) {
    if (fEnemyHealth > 0.0f) {
      if (fEnemyHealth < 0.25f) {
        colSniperWheel = C_RED;

      } else if (fEnemyHealth < 0.6f) {
        colSniperWheel = C_YELLOW;

      } else {
        colSniperWheel = C_GREEN;
      }

    } else {
      colSniperWheel = C_lGRAY;
    }

    colSniperWheel |= 0x5F;
  }

  DrawRotatedQuad(&tex.toSniperWheel, fX, fY, 40.0f * fScalingY, aAngle, colSniperWheel);

  COLOR colLED;

  // Blinking
  if (_penWeapons->m_tmLastSniperFire + 1.25f < _pTimer->GetLerpedCurrentTick()) {
    colLED = COL_ScopeLedIdle();
  } else {
    colLED = COL_ScopeLedFire();
  }

  // Reload indicator
  DrawCorrectTexture(&tex.toSniperLed, fX - 37.0f * fScalingY,
    fY + 36.0f * fScalingY, 15.0f * fScalingY, colLED);

  if (_vScaling(1) >= 1.0f) {
    FLOAT fIconSize;
    FLOAT fLeftX,  fLeftYU,  fLeftYD;
    FLOAT fRightX, fRightYU, fRightYD;

    if (_vScaling(1) <= 1.3f) {
      _pdp->SetFont(_pfdConsoleFont);
      _pdp->SetTextAspect(1.0f);
      _pdp->SetTextScaling(1.0f);
      fIconSize = 22.8f;
      fLeftX = 159.0f;
      fLeftYU = 8.0f;
      fLeftYD = 6.0f;
      fRightX = 159.0f;
      fRightYU = 11.0f;
      fRightYD = 6.0f;

    } else {
      _pdp->SetFont(_pfdCurrentText);
      _pdp->SetTextAspect(1.0f);
      _pdp->SetTextScaling(0.7f * fScalingY * _fTextFontScale);
      fIconSize = 19.0f;
      fLeftX = 162.0f;
      fLeftYU = 8.0f;
      fLeftYD = 6.0f;
      fRightX = 162.0f;
      fRightYU = 11.0f;
      fRightYD = 6.0f;
    }

    const COLOR colDetails = COL_ScopeDetails() | 0x99;

    // Arrow + distance
    DrawCorrectTexture(&tex.toSniperArrow, fX - fLeftX * fScalingY,
      fY - fLeftYU * fScalingY, fIconSize * fScalingY, colDetails);

    CTString strTmp = "---.-";

    if (fDistance <= 9999.9f) {
      strTmp.PrintF("%.1f", fDistance);
    }

    _pdp->PutTextC(strTmp, fX - fLeftX * fScalingY, fY + fLeftYD * fScalingY, colMask | 0xAA);

    // Eye + zoom level
    DrawCorrectTexture(&tex.toSniperEye, fX + fRightX * fScalingY,
      fY - fRightYU * fScalingY, fIconSize * fScalingY, colDetails);

    strTmp.PrintF("%.1fx", fZoom);

    _pdp->PutTextC(strTmp, fX + fRightX * fScalingY, fY + fRightYD * fScalingY, colMask | 0xAA);
  }
};

#endif
