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
#include "MControls.h"

extern CTFileName _fnmControlsToCustomize;

static void StartCustomizeKeyboardMenu(void) {
  // [Cecil] Set extra controls for editing player controls
  _pGUIM->gmCustomizeKeyboardMenu.SetControls(GetGameAPI()->GetControls());
  ChangeToMenu(&_pGUIM->gmCustomizeKeyboardMenu);
}

// [Cecil] Start customization of common controls
static void StartCustomizeCommonControlsMenu(void) {
  _pGUIM->gmCustomizeKeyboardMenu.SetControls(GetGameAPI()->pctrlCommon);
  ChangeToMenu(&_pGUIM->gmCustomizeKeyboardMenu);
};

static BOOL LSLoadControls(const CTFileName &fnm) {
  try {
    ControlsMenuOn();
    GetGameAPI()->GetControls()->Load_t(fnm);
    ControlsMenuOff();
  } catch (char *strError) {
    CPrintF("%s", strError);
  }

  MenuGoToParent();
  return TRUE;
};

static void StartControlsLoadMenu(void) {
  CLoadSaveMenu &gmCurrent = _pGUIM->gmLoadSaveMenu;

  gmCurrent.gm_mgTitle.SetName(LOCALIZE("LOAD CONTROLS"));
  gmCurrent.gm_bAllowThumbnails = FALSE;
  gmCurrent.gm_iSortType = LSSORT_FILEUP;
  gmCurrent.gm_bSave = FALSE;
  gmCurrent.gm_bManage = FALSE;
  gmCurrent.gm_fnmDirectory = CTString("Controls\\");
  gmCurrent.gm_strSelected = CTString("");
  gmCurrent.gm_fnmExt = CTString(".ctl");
  gmCurrent.gm_ulListFlags = FLF_SEARCHMOD; // [Cecil]
  gmCurrent.gm_pAfterFileChosen = &LSLoadControls;
  gmCurrent.gm_mgNotes.SetText("");

  ChangeToMenu(&gmCurrent);
};

void CControlsMenu::Initialize_t(void) {
  gm_strName = "Controls";
  gm_pmgSelectedByDefault = &gm_mgButtons;

  // intialize player and controls menu
  gm_mgTitle.mg_boxOnScreen = BoxTitle();
  gm_mgTitle.SetName(LOCALIZE("CONTROLS"));
  AddChild(&gm_mgTitle);

  gm_mgNameLabel.SetText("");
  gm_mgNameLabel.mg_boxOnScreen = BoxMediumRow(0.0);
  gm_mgNameLabel.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgNameLabel.mg_iCenterI = -1;
  gm_mgNameLabel.mg_bEnabled = FALSE;
  gm_mgNameLabel.mg_bLabel = TRUE;
  AddChild(&gm_mgNameLabel);

  gm_mgButtons.SetText(LOCALIZE("CUSTOMIZE BUTTONS"));
  gm_mgButtons.mg_boxOnScreen = BoxMediumRow(2.0);
  gm_mgButtons.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgButtons.mg_iCenterI = 0;
  AddChild(&gm_mgButtons);
  gm_mgButtons.mg_pmgUp = &gm_mgCommon;
  gm_mgButtons.mg_pmgDown = &gm_mgAdvanced;
  gm_mgButtons.mg_pActivatedFunction = &StartCustomizeKeyboardMenu;
  gm_mgButtons.mg_strTip = LOCALIZE("customize buttons in current controls");

  gm_mgAdvanced.SetText(LOCALIZE("ADVANCED JOYSTICK SETUP"));
  gm_mgAdvanced.mg_iCenterI = 0;
  gm_mgAdvanced.mg_boxOnScreen = BoxMediumRow(3);
  gm_mgAdvanced.mg_bfsFontSize = BFS_MEDIUM;
  AddChild(&gm_mgAdvanced);
  gm_mgAdvanced.mg_pmgUp = &gm_mgButtons;
  gm_mgAdvanced.mg_pmgDown = &gm_mgSensitivity;
  gm_mgAdvanced.mg_pActivatedFunction = &CCustomizeAxisMenu::ChangeTo;
  gm_mgAdvanced.mg_strTip = LOCALIZE("adjust advanced settings for joystick axis");

  gm_mgSensitivity.mg_boxOnScreen = BoxMediumRow(4.5);
  gm_mgSensitivity.SetText(LOCALIZE("SENSITIVITY"));
  gm_mgSensitivity.mg_pmgUp = &gm_mgAdvanced;
  gm_mgSensitivity.mg_pmgDown = &gm_mgInvertTrigger;
  gm_mgSensitivity.mg_strTip = LOCALIZE("sensitivity for all axis in this control set");
  AddChild(&gm_mgSensitivity);

  TRIGGER_MG(gm_mgInvertTrigger, 5.5, gm_mgSensitivity, gm_mgSmoothTrigger, LOCALIZE("INVERT LOOK"), astrNoYes);
  gm_mgInvertTrigger.mg_strTip = LOCALIZE("invert up/down looking");
  TRIGGER_MG(gm_mgSmoothTrigger, 6.5, gm_mgInvertTrigger, gm_mgAccelTrigger, LOCALIZE("SMOOTH AXIS"), astrNoYes);
  gm_mgSmoothTrigger.mg_strTip = LOCALIZE("smooth mouse/joystick movements");
  TRIGGER_MG(gm_mgAccelTrigger, 7.5, gm_mgSmoothTrigger, gm_mgIFeelTrigger, LOCALIZE("MOUSE ACCELERATION"), astrNoYes);
  gm_mgAccelTrigger.mg_strTip = LOCALIZE("allow mouse acceleration");
  TRIGGER_MG(gm_mgIFeelTrigger, 8.5, gm_mgAccelTrigger, gm_mgPredefined, LOCALIZE("ENABLE IFEEL"), astrNoYes);
  gm_mgIFeelTrigger.mg_strTip = LOCALIZE("enable support for iFeel tactile feedback mouse");

  gm_mgPredefined.SetText(LOCALIZE("LOAD PREDEFINED SETTINGS"));
  gm_mgPredefined.mg_iCenterI = 0;
  gm_mgPredefined.mg_boxOnScreen = BoxMediumRow(10);
  gm_mgPredefined.mg_bfsFontSize = BFS_MEDIUM;
  AddChild(&gm_mgPredefined);
  gm_mgPredefined.mg_pmgUp = &gm_mgIFeelTrigger;
  gm_mgPredefined.mg_pmgDown = &gm_mgCommon;
  gm_mgPredefined.mg_pActivatedFunction = &StartControlsLoadMenu;
  gm_mgPredefined.mg_strTip = LOCALIZE("load one of several predefined control settings");

  // [Cecil] Common controls
  gm_mgCommon.SetText(TRANS("CUSTOMIZE COMMON CONTROLS"));
  gm_mgCommon.mg_strTip = TRANS("customize buttons in common game controls");
  gm_mgCommon.mg_boxOnScreen = BoxMediumRow(11);
  gm_mgCommon.mg_bfsFontSize = BFS_MEDIUM;
  gm_mgCommon.mg_iCenterI = 0;
  gm_mgCommon.mg_pmgUp = &gm_mgPredefined;
  gm_mgCommon.mg_pmgDown = &gm_mgButtons;
  gm_mgCommon.mg_pActivatedFunction = &StartCustomizeCommonControlsMenu;

  // Disable the button if common controls haven't been hooked
  gm_mgCommon.mg_bEnabled = (GetGameAPI()->pctrlCommon != NULL);

  AddChild(&gm_mgCommon);
}

void CControlsMenu::StartMenu(void) {
  gm_pmgSelectedByDefault = &gm_mgButtons;
  INDEX iPlayer = GetGameAPI()->GetProfileForSP();
  if (_iLocalPlayer >= 0 && _iLocalPlayer < MAX_GAME_LOCAL_PLAYERS) {
    iPlayer = GetGameAPI()->GetProfileForMenu(_iLocalPlayer);
  }
  _fnmControlsToCustomize.PrintF("Controls\\Controls%d.ctl", iPlayer);

  ControlsMenuOn();

  CTString strControls;
  strControls.PrintF(LOCALIZE("CONTROLS FOR: %s"), GetGameAPI()->GetPlayerCharacter(iPlayer)->GetNameForPrinting());

  gm_mgNameLabel.SetText(strControls);

  ObtainActionSettings();
  CGameMenu::StartMenu();
}

void CControlsMenu::EndMenu(void) {
  ApplyActionSettings();

  ControlsMenuOff();

  CGameMenu::EndMenu();
}

void CControlsMenu::ObtainActionSettings(void) {
  CControls &ctrls = *GetGameAPI()->GetControls();

  gm_mgSensitivity.mg_iMinPos = 0;
  gm_mgSensitivity.mg_iMaxPos = 50;
  gm_mgSensitivity.mg_iCurPos = ctrls.ctrl_fSensitivity / 2;
  gm_mgSensitivity.ApplyCurrentPosition();

  gm_mgInvertTrigger.mg_iSelected = ctrls.ctrl_bInvertLook ? 1 : 0;
  gm_mgSmoothTrigger.mg_iSelected = ctrls.ctrl_bSmoothAxes ? 1 : 0;
  gm_mgAccelTrigger.mg_iSelected = _pShell->GetINDEX("inp_bAllowMouseAcceleration") ? 1 : 0;
  gm_mgIFeelTrigger.mg_bEnabled = _pShell->GetINDEX("sys_bIFeelEnabled") ? 1 : 0;
  gm_mgIFeelTrigger.mg_iSelected = _pShell->GetFLOAT("inp_fIFeelGain") > 0 ? 1 : 0;

  gm_mgInvertTrigger.ApplyCurrentSelection();
  gm_mgSmoothTrigger.ApplyCurrentSelection();
  gm_mgAccelTrigger.ApplyCurrentSelection();
  gm_mgIFeelTrigger.ApplyCurrentSelection();
}

void CControlsMenu::ApplyActionSettings(void) {
  CControls &ctrls = *GetGameAPI()->GetControls();

  FLOAT fSensitivity =
    FLOAT(gm_mgSensitivity.mg_iCurPos - gm_mgSensitivity.mg_iMinPos) /
    FLOAT(gm_mgSensitivity.mg_iMaxPos - gm_mgSensitivity.mg_iMinPos) * 100.0f;

  BOOL bInvert = gm_mgInvertTrigger.mg_iSelected != 0;
  BOOL bSmooth = gm_mgSmoothTrigger.mg_iSelected != 0;
  BOOL bAccel = gm_mgAccelTrigger.mg_iSelected != 0;
  BOOL bIFeel = gm_mgIFeelTrigger.mg_iSelected != 0;

  if (INDEX(ctrls.ctrl_fSensitivity) != INDEX(fSensitivity)) {
    ctrls.ctrl_fSensitivity = fSensitivity;
  }
  ctrls.ctrl_bInvertLook = bInvert;
  ctrls.ctrl_bSmoothAxes = bSmooth;
  _pShell->SetINDEX("inp_bAllowMouseAcceleration", bAccel);
  _pShell->SetFLOAT("inp_fIFeelGain", bIFeel ? 1.0f : 0.0f);
  ctrls.CalculateInfluencesForAllAxis();
}

// [Cecil] Change to the menu
void CControlsMenu::ChangeTo(void) {
  ChangeToMenu(&_pGUIM->gmControls);
};
