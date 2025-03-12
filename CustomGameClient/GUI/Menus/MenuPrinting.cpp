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

#include "StdH.h"

#include "MenuPrinting.h"
#include "ArrowDir.h"

// [Cecil] Classics patch
#include "Cecil/WindowModes.h"

static const FLOAT _fBigStartJ = 0.25f;
static const FLOAT _fBigSizeJ = 0.066f;
static const FLOAT _fMediumSizeJ = 0.04f;

static const FLOAT _fNoStartI = 0.25f;
static const FLOAT _fNoSizeI = 0.04f;
static const FLOAT _fNoSpaceI = 0.01f;
static const FLOAT _fNoUpStartJ = 0.24f;
static const FLOAT _fNoDownStartJ = 0.44f;
static const FLOAT _fNoSizeJ = 0.04f;

// [Cecil] Side size ratios
extern FLOAT _fGadgetSideRatioL;
extern FLOAT _fGadgetSideRatioR;

FLOATaabbox2D BoxTitle(void) {
  return FLOATaabbox2D(
    FLOAT2D(0, _fBigSizeJ),
    FLOAT2D(1, _fBigSizeJ));
}

FLOATaabbox2D BoxNoUp(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fNoStartI + fRow * (_fNoSizeI + _fNoSpaceI), _fNoUpStartJ),
    FLOAT2D(_fNoStartI + fRow * (_fNoSizeI + _fNoSpaceI) + _fNoSizeI, _fNoUpStartJ + _fNoSizeJ));
}

FLOATaabbox2D BoxNoDown(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fNoStartI + fRow * (_fNoSizeI + _fNoSpaceI), _fNoDownStartJ),
    FLOAT2D(_fNoStartI + fRow * (_fNoSizeI + _fNoSpaceI) + _fNoSizeI, _fNoDownStartJ + _fNoSizeJ));
}

FLOATaabbox2D BoxBigRow(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.1f, _fBigStartJ + fRow * _fBigSizeJ),
    FLOAT2D(0.9f, _fBigStartJ + (fRow + 1) * _fBigSizeJ));
}
FLOATaabbox2D BoxBigLeft(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.1f, _fBigStartJ + fRow * _fBigSizeJ),
    FLOAT2D(_fGadgetSideRatioL, _fBigStartJ + (fRow + 1) * _fBigSizeJ));
}

FLOATaabbox2D BoxBigRight(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fGadgetSideRatioR, _fBigStartJ + fRow * _fBigSizeJ),
    FLOAT2D(0.9f, _fBigStartJ + (fRow + 1) * _fBigSizeJ));
}

FLOATaabbox2D BoxSaveLoad(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.20f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.95f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxVersion(void) {
  return FLOATaabbox2D(
    FLOAT2D(0.05f, _fBigStartJ + -5.5f * _fMediumSizeJ),
    FLOAT2D(0.97f, _fBigStartJ + (-5.5f + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxMediumRow(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.05f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.95f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxKeyRow(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.15f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.85f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxMediumLeft(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.05f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(_fGadgetSideRatioL, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxPlayerSwitch(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.05f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.65f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxMediumMiddle(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fNoStartI, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.95f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxPlayerEdit(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fNoStartI, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.65f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

FLOATaabbox2D BoxMediumRight(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(_fGadgetSideRatioR, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.95f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

// [Cecil] Horizontal popup positions regardless of the aspect ratio
static const FLOAT _fPopupW = 0.35f;

#define POPUP_L(Add) (0.5f - (_fPopupW - Add) / _fAspectRatio * (4.0f / 3.0f))
#define POPUP_R(Add) (0.5f + (_fPopupW - Add) / _fAspectRatio * (4.0f / 3.0f))

// [Cecil] Height adjustment for popup boxes
FLOATaabbox2D BoxPopup(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_L(0.0f), 0.5f - fHeight),
    FLOAT2D(POPUP_R(0.0f), 0.5f + fHeight));
}

FLOATaabbox2D BoxPopupLabel(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_L(0.02f), 0.53f - fHeight),
    FLOAT2D(POPUP_R(0.02f), 0.59f - fHeight));
}

FLOATaabbox2D BoxPopupYesLarge(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_L(0.1f),  0.41f + fHeight),
    FLOAT2D(POPUP_L(0.31f), 0.47f + fHeight));
}

FLOATaabbox2D BoxPopupNoLarge(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_R(0.31f), 0.41f + fHeight),
    FLOAT2D(POPUP_R(0.1f),  0.47f + fHeight));
}

FLOATaabbox2D BoxPopupYesSmall(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_L(0.1f),  0.44f + fHeight),
    FLOAT2D(POPUP_L(0.34f), 0.49f + fHeight));
}

FLOATaabbox2D BoxPopupNoSmall(FLOAT fHeight) {
  fHeight *= 0.5f;
  return FLOATaabbox2D(
    FLOAT2D(POPUP_R(0.34f), 0.44f + fHeight),
    FLOAT2D(POPUP_R(0.1f),  0.49f + fHeight));
}

FLOATaabbox2D BoxChangePlayer(INDEX iTable, INDEX iButton) {
  return FLOATaabbox2D(
    FLOAT2D(0.5f + 0.15f * (iButton - 1), _fBigStartJ + _fMediumSizeJ * 2.0f * iTable),
    FLOAT2D(0.5f + 0.15f * (iButton + 0), _fBigStartJ + _fMediumSizeJ * 2.0f * (iTable + 1)));
}

FLOATaabbox2D BoxInfoTable(INDEX iTable) {
  switch (iTable) {
    case 0:
    case 1:
    case 2:
    case 3:
      return FLOATaabbox2D(
        FLOAT2D(0.1f, _fBigStartJ + _fMediumSizeJ * 2.0f * iTable),
        FLOAT2D(0.5f, _fBigStartJ + _fMediumSizeJ * 2.0f * (iTable + 1)));

    default:
      ASSERT(FALSE);
    case -1: // single player table
      return FLOATaabbox2D(
        FLOAT2D(0.1f, 1 - 0.2f - _fMediumSizeJ * 2.0f),
        FLOAT2D(0.5f, 1 - 0.2f));
  }
}

FLOATaabbox2D BoxArrow(enum ArrowDir ad) {
  switch (ad) {
    default: ASSERT(FALSE);
    case AD_UP:
      return FLOATaabbox2D(
        FLOAT2D(0.02f, _fBigStartJ - 3 * _fMediumSizeJ),
        FLOAT2D(0.15f, _fBigStartJ - 2 * _fMediumSizeJ));
    case AD_DOWN:
      return FLOATaabbox2D(
        FLOAT2D(0.02f, _fBigStartJ + 15 * _fMediumSizeJ),
        FLOAT2D(0.15f, _fBigStartJ + 16 * _fMediumSizeJ));
  }
}

// [Cecil] Horizontal arrow positions regardless of the aspect ratio
#define ARROW_L (1.0f - 0.045f / _fAspectRatio * (4.0f / 3.0f))
#define ARROW_R (1.0f - 0.020f / _fAspectRatio * (4.0f / 3.0f))

// [Cecil] Arrows around the scrollbar
FLOATaabbox2D BoxScrollbarArrow(ArrowDir ad) {
  switch (ad) {
    default: ASSERT(FALSE);
    case AD_UP:
      return FLOATaabbox2D(
        FLOAT2D(ARROW_L, _fBigStartJ - 3.0f * _fMediumSizeJ),
        FLOAT2D(ARROW_R, _fBigStartJ - 2.1f * _fMediumSizeJ));
    case AD_DOWN:
      return FLOATaabbox2D(
        FLOAT2D(ARROW_L, _fBigStartJ + 15.1f * _fMediumSizeJ),
        FLOAT2D(ARROW_R, _fBigStartJ + 16.0f * _fMediumSizeJ));
  }
};

// [Cecil] Scrollbar between the arrows
FLOATaabbox2D BoxScrollbar(void) {
  return FLOATaabbox2D(
    FLOAT2D(ARROW_L, _fBigStartJ -  1.95f * _fMediumSizeJ),
    FLOAT2D(ARROW_R, _fBigStartJ + 14.95f * _fMediumSizeJ));
};

FLOATaabbox2D BoxBack(void) {
  return FLOATaabbox2D(
    FLOAT2D(0.02f, 0.95f),
    FLOAT2D(0.15f, 1.0f));
}

FLOATaabbox2D BoxNext(void) {
  return FLOATaabbox2D(
    FLOAT2D(0.85f, 0.95f),
    FLOAT2D(0.98f, 1.0f));
}

FLOATaabbox2D BoxLeftColumn(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.02f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.15f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
}

// [Cecil]
FLOATaabbox2D BoxRightColumn(FLOAT fRow) {
  return FLOATaabbox2D(
    FLOAT2D(0.85f, _fBigStartJ + fRow * _fMediumSizeJ),
    FLOAT2D(0.98f, _fBigStartJ + (fRow + 1) * _fMediumSizeJ));
};

FLOATaabbox2D BoxPlayerModel(void) {
  return FLOATaabbox2D(FLOAT2D(0.68f, 0.235f), FLOAT2D(0.965f, 0.78f));
}

FLOATaabbox2D BoxPlayerModelName(void) {
  return FLOATaabbox2D(FLOAT2D(0.68f, 0.78f), FLOAT2D(0.965f, 0.82f));
}

PIXaabbox2D FloatBoxToPixBox(CDrawPort *pdp, const FLOATaabbox2D &boxF) {
  // [Cecil] NOTE: Half the width for dualhead drawports. They are never dualhead
  // during rendering, so this is purely for logic that uses _pdpMenu directly instead.
  PIX pixW = pdp->GetWidth() * (pdp->IsDualHead() ? 0.5f : 1.0f);
  PIX pixH = pdp->GetHeight();
  return PIXaabbox2D(
    PIX2D(boxF.Min()(1) * pixW, boxF.Min()(2) * pixH),
    PIX2D(boxF.Max()(1) * pixW, boxF.Max()(2) * pixH));
}

FLOATaabbox2D PixBoxToFloatBox(CDrawPort *pdp, const PIXaabbox2D &boxP) {
  FLOAT fpixW = pdp->GetWidth();
  FLOAT fpixH = pdp->GetHeight();
  return FLOATaabbox2D(
    FLOAT2D(boxP.Min()(1) / fpixW, boxP.Min()(2) / fpixH),
    FLOAT2D(boxP.Max()(1) / fpixW, boxP.Max()(2) / fpixH));
}

// [Cecil] Determine base text scaling
static __forceinline FLOAT BaseScaling(CDrawPort *pdp) {
  // Custom text scale multiplied by vanilla scaling
  return IConfig::mod[k_EModDataProps_MenuTextScale].GetFloat() * ((FLOAT)pdp->GetWidth() / 640.0f) * pdp->dp_fWideAdjustment;
};

extern CFontData _fdTitle;
void SetFontTitle(CDrawPort *pdp) {
  pdp->SetFont(&_fdTitle);

  // [Cecil] Disallow fonts bigger than normal by scaling them relative to vanilla
  const FLOAT fRelScale = ClampUp(32.0f / (FLOAT)_fdTitle.GetHeight(), 1.0f);

  // [Cecil] Use height instead of width for text scaling
  pdp->SetTextScaling(fRelScale * 1.25f * BaseScaling(pdp));
  pdp->SetTextAspect(1.0f);
}

extern CFontData _fdBig;
extern CFontData _fdMedium;

// [Cecil] Added text scale
void SetFontBig(CDrawPort *pdp, FLOAT fScale) {
  pdp->SetFont(&_fdBig);

  // [Cecil] Disallow fonts bigger than normal by scaling them relative to vanilla
  const FLOAT fRelScale = ClampUp(32.0f / (FLOAT)_fdBig.GetHeight(), 1.0f);

  // [Cecil] Use height instead of width for text scaling
  pdp->SetTextScaling(fRelScale * fScale * BaseScaling(pdp));
  pdp->SetTextAspect(1.0f);
}

// [Cecil] Added text scale
void SetFontMedium(CDrawPort *pdp, FLOAT fScale) {
  pdp->SetFont(&_fdMedium);

  // [Cecil] Disallow fonts bigger than normal by scaling them relative to vanilla
  const FLOAT fRelScale = ClampUp(16.0f / (FLOAT)_fdMedium.GetHeight(), 1.0f);

  // [Cecil] Use height instead of width for text scaling
  pdp->SetTextScaling(fRelScale * fScale * BaseScaling(pdp));
  pdp->SetTextAspect(0.75f);
}

// [Cecil] Added text scale
void SetFontSmall(CDrawPort *pdp, FLOAT fScale) {
  pdp->SetFont(_pfdConsoleFont);
  pdp->SetTextScaling(fScale);
  pdp->SetTextAspect(1.0f);
}
