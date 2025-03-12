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
#include "MenuStuff.h"
#include "MCustomizeAxis.h"

// [Cecil] Render special icons for each axis
static void AxisRenderCallback(CMGTrigger *pmg, CDrawPort *pdp) {
  const INDEX iValue = pmg->mg_iSelected;
  INDEX iAxis;

  if (pmg->mg_astrTexts[iValue].ScanF("%d", &iAxis) != 1) {
    iAxis = 0;
  }

  const PIXaabbox2D box = FloatBoxToPixBox(pdp, pmg->mg_boxOnScreen);
  const FLOAT fSize = box.Size()(2) * 1.25f;
  FLOAT fX = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;
  FLOAT fY = box.Min()(2) + box.Size()(2) * 0.5f - fSize / 1.75f;

  const FLOAT fUSize = 1.0f / 8.0f;
  const FLOAT fVSize = 1.0f / 4.0f;
  FLOAT fU = 0.0f;
  FLOAT fV = 0.0f;

  COLOR col = pmg->GetCurrentColor();

  pdp->InitTexture(&_pGUIM->gmCustomizeAxisMenu.gm_toAxisIcons, TRUE);

  // No axis
  if (iAxis == 0) {
    fU = 0.0f;
    fV = 0.0f;
    col = C_WHITE | CT_OPAQUE;

  // Select mouse axis
  } else if (iAxis < FIRST_JOYAXIS) {
    // First row
    fU = fUSize * (FLOAT)iAxis;
    fV = 0.0f;

  // Select controller axis
  } else {
    iAxis -= FIRST_JOYAXIS;

    // Fourth row
    fU = fUSize * FLOAT(iAxis / SDL_CONTROLLER_AXIS_MAX);
    fV = fVSize * 3.0f;

    // Add controller icon
    pdp->AddTexture(fX, fY, fX + fSize, fY + fSize, fU, fV, fU + fUSize, fV + fVSize, col);

    // Shift axis icon
    fX += fSize;

    // Second row
    fU = fUSize * FLOAT(iAxis % SDL_CONTROLLER_AXIS_MAX);
    fV = fVSize;
  }

  // Add axis icon
  pdp->AddTexture(fX, fY, fX + fSize, fY + fSize, fU, fV, fU + fUSize, fV + fVSize, col);

  pdp->FlushRenderingQueue();
};

// [Cecil] Select new tab
static void ChangeTab(void) {
  // Apply current settings
  _pGUIM->gmCustomizeAxisMenu.ApplyActionSettings();

  // Select new tab
  const INDEX iTab = ((CMGButton *)_pmgLastActivatedGadget)->mg_iIndex;
  _pGUIM->gmCustomizeAxisMenu.gm_iTab = iTab;

  _pGUIM->gmCustomizeAxisMenu.ObtainActionSettings();
};

// [Cecil] How much to shift axis actions by
const INDEX _iAxisActionShift = 3;
#define AXIS_ACTION_INDEX(_Index) ((_Index + _iAxisActionShift) % AXIS_ACTIONS_CT)

const INDEX _iAxisActionFirst = AXIS_ACTION_INDEX(0);
const INDEX _iAxisActionLast  = AXIS_ACTION_INDEX(AXIS_ACTIONS_CT - 1);

static const FLOAT _fStartX = 0.25f;
static const FLOAT _fStartY = 0.2f;
static const FLOAT _fFullBoxWidth = 1.0f - _fStartX - 0.1f;
static const FLOAT _fBoxHeight = 0.05f;

#define BOX_PART_OF_10(_Part) (_fFullBoxWidth * FLOAT(_Part) * 0.1f)

// [Cecil] Box for each gadget label
static FLOATaabbox2D BoxLabelRow(FLOAT fRow, BOOL bRight) {
  static const FLOAT aWidths[2] = {
    BOX_PART_OF_10(4.0), BOX_PART_OF_10(6.0),
  };

  FLOAT fShift = (bRight ? aWidths[0] : 0.0f);

  const FLOAT fX1 = _fStartX + fShift;
  const FLOAT fX2 = fX1 + aWidths[bRight ? 1 : 0];

  return FLOATaabbox2D(
    FLOAT2D(fX1, _fStartY + (fRow + 0) * _fBoxHeight),
    FLOAT2D(fX2, _fStartY + (fRow + 1) * _fBoxHeight));
};

// [Cecil] Box for sensitivity gadgets
static FLOATaabbox2D BoxSenseRow(INDEX iColumn, FLOAT fRow) {
  static const FLOAT aWidths[4] = {
    BOX_PART_OF_10(3.0), BOX_PART_OF_10(1.0), BOX_PART_OF_10(3.0), BOX_PART_OF_10(3.0),
  };

  FLOAT fShift = 0.0f;

  for (INDEX i = 0; i < iColumn; i++) {
    fShift += aWidths[i];
  }

  const FLOAT fX1 = _fStartX + fShift;
  const FLOAT fX2 = fX1 + aWidths[iColumn];

  return FLOATaabbox2D(
    FLOAT2D(fX1, _fStartY + (fRow + 0) * _fBoxHeight),
    FLOAT2D(fX2, _fStartY + (fRow + 1) * _fBoxHeight));
};

// [Cecil] Box for flag gadgets
static FLOATaabbox2D BoxFlagRow(INDEX iColumn, FLOAT fRow) {
  static const FLOAT aWidths[5] = {
    BOX_PART_OF_10(3.0), BOX_PART_OF_10(1.0), BOX_PART_OF_10(2.0), BOX_PART_OF_10(2.0), BOX_PART_OF_10(2.0),
  };

  FLOAT fShift = 0.0f;

  for (INDEX i = 0; i < iColumn; i++) {
    fShift += aWidths[i];
  }

  const FLOAT fX1 = _fStartX + fShift;
  const FLOAT fX2 = fX1 + aWidths[iColumn];

  return FLOATaabbox2D(
    FLOAT2D(fX1, _fStartY + (fRow + 0) * _fBoxHeight),
    FLOAT2D(fX2, _fStartY + (fRow + 1) * _fBoxHeight));
};

void CCustomizeAxisMenu::Initialize_t(void) {
  gm_strName = "CustomizeAxis";
  gm_pmgSelectedByDefault = &gm_amgDetect[_iAxisActionShift];
  gm_iTab = 0; // [Cecil]

  // intialize axis menu
  gm_mgTitle.SetName(LOCALIZE("CUSTOMIZE AXIS"));
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  AddChild(&gm_mgTitle);

  // [Cecil] Setup labels
  gm_amgLabels[0].SetText(LOCALIZE("MOUNTED TO"));
  gm_amgLabels[0].mg_boxOnScreen = BoxLabelRow(0, FALSE);
  gm_amgLabels[0].mg_iCenterI = 0;

  gm_amgLabels[1].SetText(LOCALIZE("SENSITIVITY") + CTString(" / ") + LOCALIZE("DEAD ZONE"));
  gm_amgLabels[1].mg_boxOnScreen = BoxLabelRow(0, TRUE);
  gm_amgLabels[1].mg_iCenterI = 0;

  gm_amgLabels[2].SetText(LOCALIZE("INVERTED") + CTString(" / ") + LOCALIZE("RELATIVE") + CTString(" / ") + LOCALIZE("SMOOTH"));
  gm_amgLabels[2].mg_boxOnScreen = BoxLabelRow(0, TRUE);
  gm_amgLabels[2].mg_iCenterI = 0;

  for (INDEX iLabel = 0; iLabel < 3; iLabel++) {
    CMGButton &mg = gm_amgLabels[iLabel];
    mg.mg_bfsFontSize = BFS_MEDIUM;
    mg.mg_bEnabled = FALSE;
    mg.mg_bLabel = TRUE;
    AddChild(&mg);
  }

  // [Cecil] Setup tab buttons
  gm_mgSenseTab.SetText(LOCALIZE("SENSITIVITY"));
  gm_mgSenseTab.mg_iIndex = 0;
  gm_mgSenseTab.mg_boxOnScreen = BoxSenseRow(2, 13);
  gm_mgSenseTab.mg_iCenterI = 0;
  gm_mgSenseTab.mg_pmgLeft  = &gm_mgFlagsTab;
  gm_mgSenseTab.mg_pmgRight = &gm_mgFlagsTab;
  gm_mgSenseTab.mg_pActivatedFunction = &ChangeTab;
  AddChild(&gm_mgSenseTab);

  gm_mgFlagsTab.SetText(TRANS("PROPERTIES"));
  gm_mgFlagsTab.mg_iIndex = 1;
  gm_mgFlagsTab.mg_boxOnScreen = BoxSenseRow(3, 13);
  gm_mgFlagsTab.mg_iCenterI = 0;
  gm_mgFlagsTab.mg_pmgLeft  = &gm_mgSenseTab;
  gm_mgFlagsTab.mg_pmgRight = &gm_mgSenseTab;
  gm_mgFlagsTab.mg_pActivatedFunction = &ChangeTab;
  AddChild(&gm_mgFlagsTab);

  // [Cecil] Load axis icons
  try {
    gm_toAxisIcons.SetData_t(CTFILENAME("TexturesPatch\\General\\AxisIcons.tex"));
  } catch (char *strError) {
    CPrintF("%s\n", strError);
  }

  // [Cecil] Setup gadgets for each axis
  for (INDEX i = 0; i < AXIS_ACTIONS_CT; i++) {
    const INDEX iPrev = (i + AXIS_ACTIONS_CT - 1) % AXIS_ACTIONS_CT;
    const INDEX iNext = (i                   + 1) % AXIS_ACTIONS_CT;

    const FLOAT fRow = FLOAT((i + AXIS_ACTIONS_CT - _iAxisActionShift) % AXIS_ACTIONS_CT) * 1.25f + 1.25f;

    TRIGGER_MG(gm_amgMounted[i], 0, gm_amgMounted[iPrev], gm_amgMounted[iNext], TRANSV(GetGameAPI()->GetAxisName(i)), astrNoYes);
    gm_amgMounted[i].mg_strTip = LOCALIZE("choose controller axis that will perform the action");
    gm_amgMounted[i].mg_boxOnScreen = BoxSenseRow(0, fRow);
    gm_amgMounted[i].mg_iCenterJ = 0;

    INDEX ctAxis = _pInput->GetAvailableAxisCount();
    gm_amgMounted[i].mg_astrTexts = new CTString[ctAxis];
    gm_amgMounted[i].mg_ctTexts = ctAxis;

    // [Cecil] If axis icons have been loaded
    if (gm_toAxisIcons.GetData() != NULL) {
      // Set custom rendering method
      gm_amgMounted[i].mg_bVisual = TRUE;
      gm_amgMounted[i].mg_pRenderCallback = &AxisRenderCallback;

      // And use axis index for each value
      for (INDEX iAxis = 0; iAxis < ctAxis; iAxis++) {
        gm_amgMounted[i].mg_astrTexts[iAxis].PrintF("%d", iAxis);
      }

    // Otherwise use axis display names
    } else {
      for (INDEX iAxis = 0; iAxis < ctAxis; iAxis++) {
        gm_amgMounted[i].mg_astrTexts[iAxis] = _pInput->GetAxisTransName(iAxis);
      }
    }

    gm_amgDetect[i].SetText("DETECT");
    gm_amgDetect[i].mg_strTip = TRANS("select axis automatically from any movement");
    gm_amgDetect[i].mg_iIndex = i;
    gm_amgDetect[i].mg_boxOnScreen = BoxSenseRow(1, fRow);
    gm_amgDetect[i].mg_iCenterI = 0;
    gm_amgDetect[i].mg_iCenterJ = 0;
    gm_amgDetect[i].mg_pmgUp   = &gm_amgDetect[iPrev];
    gm_amgDetect[i].mg_pmgDown = &gm_amgDetect[iNext];
    gm_amgDetect[i].mg_pmgLeft = &gm_amgMounted[i];
    gm_amgDetect[i].mg_pmgRight = &gm_amgSensitivity[i];
    AddChild(&gm_amgDetect[i]);

    gm_amgSensitivity[i].SetText("");
    gm_amgSensitivity[i].mg_strTip = LOCALIZE("set sensitivity for this axis");
    gm_amgSensitivity[i].mg_boxOnScreen = BoxSenseRow(2, fRow);
    gm_amgSensitivity[i].mg_iCenterJ = 0;
    gm_amgSensitivity[i].mg_pmgUp   = &gm_amgSensitivity[iPrev];
    gm_amgSensitivity[i].mg_pmgDown = &gm_amgSensitivity[iNext];
    AddChild(&gm_amgSensitivity[i]);

    gm_amgDeadzone[i].SetText("");
    gm_amgDeadzone[i].mg_strTip = LOCALIZE("set dead zone for this axis");
    gm_amgDeadzone[i].mg_boxOnScreen = BoxSenseRow(3, fRow);
    gm_amgDeadzone[i].mg_iCenterJ = 0;
    gm_amgDeadzone[i].mg_pmgUp   = &gm_amgDeadzone[iPrev];
    gm_amgDeadzone[i].mg_pmgDown = &gm_amgDeadzone[iNext];
    AddChild(&gm_amgDeadzone[i]);

    TRIGGER_MG(gm_amgInvert[i], 9, gm_amgInvert[iPrev], gm_amgInvert[iNext], "", astrNoYes);
    gm_amgInvert[i].mg_strTip = LOCALIZE("choose whether to invert this axis or not");
    gm_amgInvert[i].mg_boxOnScreen = BoxFlagRow(2, fRow);
    gm_amgInvert[i].mg_iCenterI = +1;
    gm_amgInvert[i].mg_iCenterJ = 0;

    TRIGGER_MG(gm_amgRelative[i], 10, gm_amgRelative[iPrev], gm_amgRelative[iNext], "", astrNoYes);
    gm_amgRelative[i].mg_strTip = LOCALIZE("select relative or absolute axis reading");
    gm_amgRelative[i].mg_boxOnScreen = BoxFlagRow(3, fRow);
    gm_amgRelative[i].mg_iCenterI = 0;
    gm_amgRelative[i].mg_iCenterJ = 0;

    TRIGGER_MG(gm_amgSmooth[i], 11, gm_amgSmooth[iPrev], gm_amgSmooth[iNext], "", astrNoYes);
    gm_amgSmooth[i].mg_strTip = LOCALIZE("turn this on to filter readings on this axis");
    gm_amgSmooth[i].mg_boxOnScreen = BoxFlagRow(4, fRow);
    gm_amgSmooth[i].mg_iCenterI = -1;
    gm_amgSmooth[i].mg_iCenterJ = 0;
  }

  // Connect first and last axis gadgets to tabs
  gm_amgMounted    [_iAxisActionFirst].mg_pmgUp = gm_amgMounted    [_iAxisActionLast].mg_pmgDown = &gm_mgSenseTab;
  gm_amgDetect     [_iAxisActionFirst].mg_pmgUp = gm_amgDetect     [_iAxisActionLast].mg_pmgDown = &gm_mgSenseTab;
  gm_amgSensitivity[_iAxisActionFirst].mg_pmgUp = gm_amgSensitivity[_iAxisActionLast].mg_pmgDown = &gm_mgSenseTab;
  gm_amgDeadzone   [_iAxisActionFirst].mg_pmgUp = gm_amgDeadzone   [_iAxisActionLast].mg_pmgDown = &gm_mgFlagsTab;
  gm_amgInvert     [_iAxisActionFirst].mg_pmgUp = gm_amgInvert     [_iAxisActionLast].mg_pmgDown = &gm_mgSenseTab;
  gm_amgRelative   [_iAxisActionFirst].mg_pmgUp = gm_amgRelative   [_iAxisActionLast].mg_pmgDown = &gm_mgSenseTab;
  gm_amgSmooth     [_iAxisActionFirst].mg_pmgUp = gm_amgSmooth     [_iAxisActionLast].mg_pmgDown = &gm_mgFlagsTab;
};

CCustomizeAxisMenu::~CCustomizeAxisMenu(void) {
  for (INDEX i = 0; i < AXIS_ACTIONS_CT; i++) {
    delete[] gm_amgMounted[i].mg_astrTexts;
  }

  gm_toAxisIcons.SetData(NULL); // [Cecil]
}

void CCustomizeAxisMenu::ObtainActionSettings(void) {
  ControlsMenuOn();
  CControls &ctrls = *GetGameAPI()->GetControls();

  // [Cecil] Select current tab
  const BOOL bSenseTab = (gm_mgSenseTab.mg_iIndex == gm_iTab);
  gm_mgSenseTab.mg_bEnabled = !bSenseTab;
  gm_mgFlagsTab.mg_bEnabled =  bSenseTab;

  if (bSenseTab) {
    gm_amgLabels[1].Appear();
    gm_amgLabels[2].Disappear();
  } else {
    gm_amgLabels[1].Disappear();
    gm_amgLabels[2].Appear();
  }

  for (INDEX i = 0; i < AXIS_ACTIONS_CT; i++) {
    gm_amgMounted[i].mg_iSelected = ctrls.ctrl_aaAxisActions[i].aa_iAxisAction;

    gm_amgSensitivity[i].mg_iMinPos = 0;
    gm_amgSensitivity[i].mg_iMaxPos = 50;
    gm_amgSensitivity[i].mg_iCurPos = ctrls.ctrl_aaAxisActions[i].aa_fSensitivity / 2;
    gm_amgSensitivity[i].ApplyCurrentPosition();

    gm_amgDeadzone[i].mg_iMinPos = 0;
    gm_amgDeadzone[i].mg_iMaxPos = 50;
    gm_amgDeadzone[i].mg_iCurPos = ctrls.ctrl_aaAxisActions[i].aa_fDeadZone / 2;
    gm_amgDeadzone[i].ApplyCurrentPosition();

    gm_amgInvert[i].mg_iSelected = ctrls.ctrl_aaAxisActions[i].aa_bInvert ? 1 : 0;
    gm_amgRelative[i].mg_iSelected = ctrls.ctrl_aaAxisActions[i].aa_bRelativeControler ? 1 : 0;
    gm_amgSmooth[i].mg_iSelected = ctrls.ctrl_aaAxisActions[i].aa_bSmooth ? 1 : 0;

    gm_amgMounted[i].ApplyCurrentSelection();
    gm_amgInvert[i].ApplyCurrentSelection();
    gm_amgRelative[i].ApplyCurrentSelection();
    gm_amgSmooth[i].ApplyCurrentSelection();

    // [Cecil] Toggle settings for tabs
    if (bSenseTab) {
      // Swap gadgets to the right of detection
      gm_amgDetect[i].mg_pmgRight = &gm_amgSensitivity[i];

      gm_amgSensitivity[i].Appear();
      gm_amgDeadzone[i].Appear();

      gm_amgInvert[i].Disappear();
      gm_amgRelative[i].Disappear();
      gm_amgSmooth[i].Disappear();

    } else {
      // Swap gadgets to the right of detection
      gm_amgDetect[i].mg_pmgRight = &gm_amgInvert[i];

      gm_amgSensitivity[i].Disappear();
      gm_amgDeadzone[i].Disappear();

      gm_amgInvert[i].Appear();
      gm_amgRelative[i].Appear();
      gm_amgSmooth[i].Appear();
    }
  }

  if (bSenseTab) {
    gm_mgSenseTab.mg_pmgUp   = &gm_amgSensitivity[_iAxisActionLast];
    gm_mgSenseTab.mg_pmgDown = &gm_amgSensitivity[_iAxisActionFirst];
    gm_mgFlagsTab.mg_pmgUp   = &gm_amgDeadzone[_iAxisActionLast];
    gm_mgFlagsTab.mg_pmgDown = &gm_amgDeadzone[_iAxisActionFirst];

  } else {
    gm_mgSenseTab.mg_pmgUp   = &gm_amgInvert[_iAxisActionLast];
    gm_mgSenseTab.mg_pmgDown = &gm_amgInvert[_iAxisActionFirst];
    gm_mgFlagsTab.mg_pmgUp   = &gm_amgSmooth[_iAxisActionLast];
    gm_mgFlagsTab.mg_pmgDown = &gm_amgSmooth[_iAxisActionFirst];
  }
};

void CCustomizeAxisMenu::ApplyActionSettings(void) {
  CControls &ctrls = *GetGameAPI()->GetControls();

  for (INDEX i = 0; i < AXIS_ACTIONS_CT; i++) {
    INDEX iMountedAxis = gm_amgMounted[i].mg_iSelected;
    FLOAT fSensitivity =
      FLOAT(gm_amgSensitivity[i].mg_iCurPos - gm_amgSensitivity[i].mg_iMinPos) /
      FLOAT(gm_amgSensitivity[i].mg_iMaxPos - gm_amgSensitivity[i].mg_iMinPos) * 100.0f;
    FLOAT fDeadZone =
      FLOAT(gm_amgDeadzone[i].mg_iCurPos - gm_amgDeadzone[i].mg_iMinPos) /
      FLOAT(gm_amgDeadzone[i].mg_iMaxPos - gm_amgDeadzone[i].mg_iMinPos) * 100.0f;

    BOOL bInvert = gm_amgInvert[i].mg_iSelected != 0;
    BOOL bRelative = gm_amgRelative[i].mg_iSelected != 0;
    BOOL bSmooth = gm_amgSmooth[i].mg_iSelected != 0;

    ctrls.ctrl_aaAxisActions[i].aa_iAxisAction = iMountedAxis;
    if (INDEX(ctrls.ctrl_aaAxisActions[i].aa_fSensitivity) != INDEX(fSensitivity)) {
      ctrls.ctrl_aaAxisActions[i].aa_fSensitivity = fSensitivity;
    }
    if (INDEX(ctrls.ctrl_aaAxisActions[i].aa_fDeadZone) != INDEX(fDeadZone)) {
      ctrls.ctrl_aaAxisActions[i].aa_fDeadZone = fDeadZone;
    }
    ctrls.ctrl_aaAxisActions[i].aa_bInvert = bInvert;
    ctrls.ctrl_aaAxisActions[i].aa_bRelativeControler = bRelative;
    ctrls.ctrl_aaAxisActions[i].aa_bSmooth = bSmooth;
    ctrls.CalculateInfluencesForAllAxis();
  }

  ControlsMenuOff();
};

void CCustomizeAxisMenu::StartMenu(void) {
  CGameMenu::StartMenu();

  // [Cecil] Obtain settings *after* entering the menu in order to select a proper tab
  ObtainActionSettings();
};

void CCustomizeAxisMenu::EndMenu(void) {
  ApplyActionSettings();
  CGameMenu::EndMenu();
};

// [Cecil] Extra rendering
void CCustomizeAxisMenu::PostRender(CDrawPort *pdp) {
  static FLOAT2D vLastRes(-1, -1);
  const FLOAT2D vRes(pdp->GetWidth(), pdp->GetHeight());

  // Resize labels to fit nicely every resolution update
  if (vLastRes != vRes) {
    vLastRes = vRes;

    const FLOAT fRatio = vRes(1) / vRes(2);
    const FLOAT fMaxSize = 200;

    SetFontMedium(pdp, 1.0f);

    // Sensitivity
    PIX pixLabel = IRender::GetTextWidth(pdp, gm_amgLabels[1].GetText());
    pixLabel /= HEIGHT_SCALING(pdp) * fRatio;

    if (pixLabel > fMaxSize) {
      gm_amgLabels[1].mg_fTextScale = fMaxSize / (FLOAT)pixLabel;
    } else {
      gm_amgLabels[1].mg_fTextScale = 1.0f;
    }

    // Flags
    pixLabel = IRender::GetTextWidth(pdp, gm_amgLabels[2].GetText());
    pixLabel /= HEIGHT_SCALING(pdp) * fRatio;

    if (pixLabel > fMaxSize) {
      gm_amgLabels[2].mg_fTextScale = fMaxSize / (FLOAT)pixLabel;
    } else {
      gm_amgLabels[2].mg_fTextScale = 1.0f;
    }
  }

  if (!_bDefiningKey) return;

  // Render popup box
  CGameMenu::RenderPopup(pdp, 0.2f);

  SetFontMedium(pdp, 1.0f);

  extern CFontData _fdMedium;
  const PIX pixTextHeight = _fdMedium.GetHeight() * pdp->dp_fTextScaling * 1.25f;
  const COLOR col = _pGame->LCDGetColor(C_GREEN, "unselected");

  pdp->PutTextCXY(TRANS("Move your mouse or controller joystick in any direction"),
    vRes(1) / 2, vRes(2) / 2 - pixTextHeight / 2, col);
  pdp->PutTextCXY(TRANS("Press Escape, RMB or B/Back on a controller to cancel"),
    vRes(1) / 2, vRes(2) / 2 + pixTextHeight / 2, col);
};

// [Cecil] Change to the menu
void CCustomizeAxisMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmCustomizeAxisMenu);
};
