/* Copyright (c) 2024-2026 Dreamy Cecil
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

#include "ObserverCamera.h"

#include "Base/GlobalScreenshots.h"

// Global controls and properties for the observer camera
CObserverCamera::CameraControl CObserverCamera::cam_ctl;
CObserverCamera::CameraProps CObserverCamera::cam_props;

// Clamp distance from number difference
__forceinline DOUBLE ClampDistDn(DOUBLE dDiff, DOUBLE dDown) {
  DOUBLE d = Abs(dDiff);
  return (d < dDown ? dDown : d);
};

// Clamp distance from vector difference
__forceinline DOUBLE ClampDistDn(FLOAT3D vDiff, DOUBLE dDown) {
  DOUBLE d = vDiff.Length();
  return (d < dDown ? dDown : d);
};

// Calculate position within a Catmull-Rom spline using four points
// Source: https://qroph.github.io/2018/07/30/smooth-paths-using-catmull-rom-splines.html
template<class Type> __forceinline
Type CatmullRom(const Type pt0, const Type pt1, const Type pt2, const Type pt3, DOUBLE dR, DOUBLE dTension)
{
  // [Cecil] FIXME: Other values make movement less smooth between points, possibly due
  // to the fact that it's using a small segment at a time instead of the entire path.
  static const DOUBLE fAlpha = 0.0;
  DOUBLE t01 = pow(ClampDistDn(pt0 - pt1, 0.01), fAlpha);
  DOUBLE t12 = pow(ClampDistDn(pt1 - pt2, 0.01), fAlpha);
  DOUBLE t23 = pow(ClampDistDn(pt2 - pt3, 0.01), fAlpha);

  Type m1 = (pt2 - pt1 + ((pt1 - pt0) / t01 - (pt2 - pt0) / (t01 + t12)) * t12) * (1.0 - dTension);
  Type m2 = (pt2 - pt1 + ((pt3 - pt2) / t23 - (pt3 - pt1) / (t12 + t23)) * t12) * (1.0 - dTension);

  Type a = (pt1 - pt2) * +2.0 + m1 + m2;
  Type b = (pt1 - pt2) * -3.0 - m1 - m1 - m2;
  Type c = m1;
  Type d = pt1;

  return a * dR * dR * dR + b * dR * dR + c * dR + d;
};

void CObserverCamera::ReadPos(CameraPos &cp) {
  // Playback config exhausted
  if (cam_strmScript.AtEOF()) {
    // Buffer last position so it plays until that one as well
    if (!cam_acpCurve[3].bLast) {
      cam_acpCurve[3].bLast = TRUE;

    // Otherwise reset playback
    } else {
      Reset(TRUE);
    }
    return;
  }

  try {
    CTString strLine;
    cam_strmScript.GetLine_t(strLine);

    FLOAT3D &vPos = cp.Pos();
    ANGLE3D &aRot = cp.Rot();

    strLine.ScanF("%g: %g: %g %g %g:%g %g %g:%g", &cp.tmTick, &cp.fSpeed,
      &vPos(1), &vPos(2), &vPos(3), &aRot(1), &aRot(2), &aRot(3), &cp.fFOV);

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }
};

void CObserverCamera::WritePos(CameraPos &cp) {
  try {
    const FLOAT3D &vPos = cp.Pos();
    const ANGLE3D &aRot = cp.Rot();

    CTString strLine(0, "%g: %g: %g %g %g:%g %g %g:%g",
      _pTimer->GetLerpedCurrentTick() - cam_tmStartTime, 1.0f,
      vPos(1), vPos(2), vPos(3), aRot(1), aRot(2), aRot(3), cp.fFOV);

    cam_strmScript.PutLine_t(strLine);

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }
};

// Change demo playback speed
void CObserverCamera::SetSpeed(FLOAT fSpeed) {
  if (!cam_props.bPlaybackSpeedControl) return;

  static CSymbolPtr pfRealTimeFactor("dem_fRealTimeFactor");

  if (!pfRealTimeFactor.Exists()) {
    ASSERTALWAYS("'dem_fRealTimeFactor' symbol doesn't exist!");
    return;
  }

  pfRealTimeFactor.GetFloat() = fSpeed;
};

// Reset camera FOV and the banking angle
void CObserverCamera::ResetCameraAngles(void) {
  cam_ctl.fFOV = 90.0f;
  cam_cpCurrent.Rot()(3) = 0.0f;
  cam_aRotation(3) = 0.0f;
};

// Free fly controls
static INDEX ocam_kidToggle     = KID_P;
static INDEX ocam_kidToggleInfo = KID_I;

static INDEX ocam_kidBankingL   = KID_Q;
static INDEX ocam_kidBankingR   = KID_E;
static INDEX ocam_kidFocus      = KID_F;
static INDEX ocam_kidGrid       = KID_G;
static INDEX ocam_kidSnapshot   = KID_TAB;
static INDEX ocam_kidChangeRes  = KID_F10;
static INDEX ocam_kidChangeMode = KID_C;

static INDEX ocam_kidMoveF_1 = KID_W;
static INDEX ocam_kidMoveB_1 = KID_S;
static INDEX ocam_kidMoveL_1 = KID_A;
static INDEX ocam_kidMoveR_1 = KID_D;
static INDEX ocam_kidMoveU_1 = KID_LSHIFT;
static INDEX ocam_kidMoveD_1 = KID_LCONTROL;

static INDEX ocam_kidMoveF_2 = KID_ARROWUP;
static INDEX ocam_kidMoveB_2 = KID_ARROWDOWN;
static INDEX ocam_kidMoveL_2 = KID_ARROWLEFT;
static INDEX ocam_kidMoveR_2 = KID_ARROWRIGHT;
static INDEX ocam_kidMoveU_2 = KID_RSHIFT;
static INDEX ocam_kidMoveD_2 = KID_RCONTROL;

static INDEX ocam_kidZoomIn   = KID_MOUSEWHEELUP;
static INDEX ocam_kidZoomOut  = KID_MOUSEWHEELDOWN;
static INDEX ocam_kidTeleport = KID_X;
static INDEX ocam_kidReset    = KID_Z;
static INDEX ocam_kidSpeedUp  = KID_MOUSE1;
static INDEX ocam_kidRotate   = KID_MOUSE2;

// Initialize camera interface
void CObserverCamera::Init(void)
{
  // Keys for the controls
  _pShell->DeclareSymbol("persistent INDEX ocam_kidToggle;",     &ocam_kidToggle);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidToggleInfo;", &ocam_kidToggleInfo);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidBankingL;",   &ocam_kidBankingL);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidBankingR;",   &ocam_kidBankingR);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidFocus;",      &ocam_kidFocus);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidGrid;",       &ocam_kidGrid);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidSnapshot;",   &ocam_kidSnapshot);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidChangeRes;",  &ocam_kidChangeRes);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidChangeMode;", &ocam_kidChangeMode);

  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveF_1;",    &ocam_kidMoveF_1);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveB_1;",    &ocam_kidMoveB_1);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveL_1;",    &ocam_kidMoveL_1);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveR_1;",    &ocam_kidMoveR_1);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveU_1;",    &ocam_kidMoveU_1);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveD_1;",    &ocam_kidMoveD_1);

  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveF_2;",    &ocam_kidMoveF_2);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveB_2;",    &ocam_kidMoveB_2);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveL_2;",    &ocam_kidMoveL_2);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveR_2;",    &ocam_kidMoveR_2);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveU_2;",    &ocam_kidMoveU_2);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidMoveD_2;",    &ocam_kidMoveD_2);

  _pShell->DeclareSymbol("persistent INDEX ocam_kidZoomIn;",     &ocam_kidZoomIn);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidZoomOut;",    &ocam_kidZoomOut);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidTeleport;",   &ocam_kidTeleport);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidReset;",      &ocam_kidReset);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidSpeedUp;",    &ocam_kidSpeedUp);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidRotate;",     &ocam_kidRotate);

  // Camera properties
  _pShell->DeclareSymbol("           user INDEX ocam_bActive;",               &cam_props.bActive);
  _pShell->DeclareSymbol("persistent user INDEX ocam_iShowInfo;",             &cam_props.iShowInfo);
  _pShell->DeclareSymbol("persistent user INDEX ocam_bPlaybackSpeedControl;", &cam_props.bPlaybackSpeedControl);
  _pShell->DeclareSymbol("persistent user INDEX ocam_bSmoothPlayback;",       &cam_props.bSmoothPlayback);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothTension;",        &cam_props.fSmoothTension);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSpeed;",                &cam_props.fSpeed);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fTiltAngleMul;",         &cam_props.fTiltAngleMul);
  _pShell->DeclareSymbol("           user FLOAT ocam_fFOV;",                  &cam_ctl.fFOV);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fFOVChangeMul;",         &cam_props.fFOVChangeMul);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothMovement;",       &cam_props.fSmoothMovement);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothRotation;",       &cam_props.fSmoothRotation);
  _pShell->DeclareSymbol("           user INDEX ocam_iFocusPlayer;",          &cam_ctl.iFocusPlayer);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fFocusDist;",            &cam_props.fFocusDist);

  _pShell->DeclareSymbol("persistent user INDEX ocam_bGrid;",        &cam_props.bGrid);
  _pShell->DeclareSymbol("persistent user INDEX ocam_iScreenshotW;", &cam_props.iScreenshotW);
  _pShell->DeclareSymbol("persistent user INDEX ocam_iScreenshotH;", &cam_props.iScreenshotH);

  _pShell->DeclareSymbol("user INDEX ocam_bPosingMode;", &cam_props.bPosingMode);
};

// Dummy variable and function for compatibility
static INDEX _bZoomDefaultDummy = 0;

static void ResetCameraInternal(INDEX) {
  GetGameAPI()->GetCamera().ResetCameraAngles();
  _bZoomDefaultDummy = FALSE;
};

// Hook old camera commands for compatibility
void CObserverCamera::HookOldCamCommands(void) {
  _pShell->DeclareSymbol("INDEX cam_bRecord;",           &cam_props.bActive);
  _pShell->DeclareSymbol("INDEX cam_bMoveForward;",      &cam_ctl.bMoveF);
  _pShell->DeclareSymbol("INDEX cam_bMoveBackward;",     &cam_ctl.bMoveB);
  _pShell->DeclareSymbol("INDEX cam_bMoveLeft;",         &cam_ctl.bMoveL);
  _pShell->DeclareSymbol("INDEX cam_bMoveRight;",        &cam_ctl.bMoveR);
  _pShell->DeclareSymbol("INDEX cam_bMoveUp;",           &cam_ctl.bMoveU);
  _pShell->DeclareSymbol("INDEX cam_bMoveDown;",         &cam_ctl.bMoveD);
  _pShell->DeclareSymbol("INDEX cam_bTurnBankingLeft;",  &cam_ctl.bBankingL);
  _pShell->DeclareSymbol("INDEX cam_bTurnBankingRight;", &cam_ctl.bBankingR);
  _pShell->DeclareSymbol("INDEX cam_bZoomIn;",           &cam_ctl.bZoomIn);
  _pShell->DeclareSymbol("INDEX cam_bZoomOut;",          &cam_ctl.bZoomOut);

  _pShell->DeclareSymbol("void cam_ResetCameraInternal(INDEX);", &ResetCameraInternal);
  _pShell->DeclareSymbol("INDEX cam_bZoomDefault post:cam_ResetCameraInternal;", &_bZoomDefaultDummy);

  _pShell->DeclareSymbol("INDEX cam_bSnapshot;",         &cam_ctl.bSnapshot);
  _pShell->DeclareSymbol("INDEX cam_bResetToPlayer;",    &cam_ctl.bResetToPlayer);
  _pShell->DeclareSymbol("FLOAT cam_fSpeed;",            &cam_props.fSpeed);
};

// Start camera for a game (or a currently playing demo)
void CObserverCamera::Start(const CTFileName &fnmDemo) {
  // Reset variables
  Reset();

  if (fnmDemo == "") return;

  // Simple text file with a suffix
  cam_fnmDemo = fnmDemo.NoExt() + "_CAM.txt";

  // Open existing file for playback
  try {
    cam_strmScript.Open_t(cam_fnmDemo);

  } catch (char *strError) {
    (void)strError;
    return;
  }

  cam_bPlayback = TRUE;

  // Read the first positions immediately
  ReadPos(cam_acpCurve[2]); // Upcoming first point
  ReadPos(cam_acpCurve[3]); // Buffered next point
  SetSpeed(cam_acpCurve[2].fSpeed);

  // [Cecil] NOTE: Code below is needed in case the very first point in the recording doesn't begin
  // at 0 seconds. But if it does, it will just immediately skip to it. This is simply a fail-safe.

  // Wait from the very beginning before reaching the first point
  cam_acpCurve[1] = cam_acpCurve[2];
  cam_acpCurve[1].tmTick = 0.0f;

  // Skip point 0 since the curve goes from 1 to 2
  cam_acpCurve[0] = cam_acpCurve[1];
};

// Stop playback or camera altogether
void CObserverCamera::Reset(BOOL bPlayback) {
  // Full reset
  if (!bPlayback) {
    cam_ctl.Reset();

    cam_fnmDemo = CTString("");
    cam_tmStartTime = -100.0f;
    cam_tvDelta.tv_llValue = 0;
  }

  cam_bPlayback = FALSE;

  if (cam_strmScript.GetDescription() != "") {
    cam_strmScript.Close();
  }

  for (INDEX i = 0; i < 4; i++) {
    cam_acpCurve[i] = CameraPos();
  }
  cam_cpCurrent = CameraPos();

  cam_vMovement = FLOAT3D(0, 0, 0);
  cam_aRotation = ANGLE3D(0, 0, 0);
  cam_penViewer = NULL;

  SetSpeed(1.0f);
};

// Retrieve camera state field
BOOL &CObserverCamera::GetState(void) {
  return cam_props.bActive;
};

// Check if camera is on
BOOL CObserverCamera::IsActive(BOOL bAlsoDemoPlayback) {
  // Camera can only be used during observing or in demos
  const BOOL bObserver = (GetGameAPI()->GetCurrentSplitCfg() == CGame::SSC_OBSERVER);

  // Or if it needs to be used externally
  if (cam_bExternalUsage || bObserver || _pNetwork->IsPlayingDemo()) {
    return cam_props.bActive || (cam_bPlayback && bAlsoDemoPlayback);
  }

  return FALSE;
};

// Toggle photo mode during the game or demos
BOOL CObserverCamera::TogglePhotoMode(void) {
  // Toggle camera state
  cam_props.bActive = !cam_props.bActive;

  // Toggle pause in singleplayer if it's of the opposite state
  if (_gmRunningGameMode == GM_SINGLE_PLAYER) {
    if (!!IsActive(FALSE) ^ !!_pNetwork->IsPaused()) {
      _pNetwork->TogglePause();
    }

  // Toggle pause in demos if it's of the opposite state
  } else if (_gmRunningGameMode == GM_DEMO) {
    static CSymbolPtr pfSyncRate("dem_fSyncRate");

    if (pfSyncRate.Exists()) {
      bool bDemoPause = (pfSyncRate.GetFloat() == DEMOSYNC_STOP);

      if (!!IsActive(FALSE) ^ bDemoPause) {
        pfSyncRate.GetFloat() = (bDemoPause ? DEMOSYNC_REALTIME : DEMOSYNC_STOP);
      }
    }
  }

  // Camera was supposed to activate
  return cam_props.bActive;
};

// Start recording into a file
BOOL CObserverCamera::StartRecording(void) {
  // Already recording
  if (cam_strmScript.GetDescription() != "") return TRUE;

  try {
    cam_strmScript.Create_t(cam_fnmDemo);
    return TRUE;

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }

  return FALSE;
};

// Recursively freeze all model animations at their current frame
static void PauseModelAnims(CModelObject &mo) {
  mo.PauseAnim();
  mo.NextFrame();

  FOREACHINLIST(CAttachmentModelObject, amo_lnInMain, mo.mo_lhAttachments, itamo) {
    PauseModelAnims(itamo->amo_moModelObject);
  }
};

// Retrieve a photo mode model for some player entity
CModelObject *CObserverCamera::GetPoseModel(CEntity *penPlayer, CModelObject *pmoOriginalAppearance) {
  // Photo mode is inactive or this player isn't focused by the camera in photo mode
  if (!IsActive(FALSE) || cam_penViewer != penPlayer) return pmoOriginalAppearance;

  // Reset the model if not in posing mode
  if (!cam_props.bPosingMode) {
    ResetPhotoModePose(TRUE);
    return pmoOriginalAppearance;
  }

  // Setup the photo mode model by copying the player appearance onto an attachment on the first frame of the photo mode
  if (cam_penPosingPlayer != penPlayer) {
    cam_penPosingPlayer = penPlayer;

    try {
      cam_moPose.SetData_t(CTString("ModelsPatch\\PhotoModePoser.mdl"));

      CAttachmentModelObject *pamo = cam_moPose.AddAttachmentModel(0);
      CModelObject &moPlayer = pamo->amo_moModelObject;
      moPlayer.Copy(*pmoOriginalAppearance);
      moPlayer.Synchronize(*pmoOriginalAppearance);

      PauseModelAnims(moPlayer);

      // Adjust poser size for the model shadow
      FLOATaabbox3D boxFrame;
      moPlayer.GetCurrentFrameBBox(boxFrame);
      cam_moPose.StretchSingleModel(boxFrame.Size());

    } catch (char *strError) {
      // Exit posing mode if it can't be set up
      CPrintF(TRANS("Cannot setup player model for the photo mode: %s\n"), strError);
      ResetPhotoModePose(FALSE);
      return pmoOriginalAppearance;
    }
  }

  // Adjust offset and rotation
  CAttachmentModelObject *pamo = cam_moPose.GetAttachmentModel(0);
  pamo->amo_plRelative.pl_OrientationAngle(1) = cam_fPoseRotation;

  // Divide offset by poser's stretch to cancel it out
  pamo->amo_plRelative.pl_PositionVector(1) = cam_vPoseOffset(1) / ClampDn(cam_moPose.mo_Stretch(1), 0.001f);
  pamo->amo_plRelative.pl_PositionVector(2) = cam_vPoseOffset(2) / ClampDn(cam_moPose.mo_Stretch(2), 0.001f);
  pamo->amo_plRelative.pl_PositionVector(3) = cam_vPoseOffset(3) / ClampDn(cam_moPose.mo_Stretch(3), 0.001f);

  return &cam_moPose;
};

// Free fly camera control using direct button input
void CObserverCamera::UpdateControls(void) {
  // Toggle the camera itself
  const BOOL bBtnToggle = _pInput->GetButtonState(ocam_kidToggle);

  static BOOL _bToggle = FALSE;
  if (!_bToggle && bBtnToggle) TogglePhotoMode();
  _bToggle = bBtnToggle;

  // Camera is disabled
  if (!IsActive()) return;

  // [Cecil] NOTE: Needs to be here only when the camera is active, otherwise it messes with player controls
  _pInput->SetJoyPolling(FALSE);
  _pInput->GetInput(FALSE);

  // Button states for some controls
  const BOOL bBtnToggleInfo = _pInput->GetButtonState(ocam_kidToggleInfo);
  const BOOL bBtnBankingL   = _pInput->GetButtonState(ocam_kidBankingL);
  const BOOL bBtnBankingR   = _pInput->GetButtonState(ocam_kidBankingR);
  const BOOL bBtnFocus      = _pInput->GetButtonState(ocam_kidFocus);
  const BOOL bBtnGrid       = _pInput->GetButtonState(ocam_kidGrid);
  const BOOL bBtnSnap       = _pInput->GetButtonState(ocam_kidSnapshot);
  const BOOL bBtnRes        = _pInput->GetButtonState(ocam_kidChangeRes);
  const BOOL bBtnMode       = _pInput->GetButtonState(ocam_kidChangeMode);

  // Toggle camera info
  static BOOL _bToggleInfo = FALSE;
  if (!_bToggleInfo && bBtnToggleInfo) cam_props.iShowInfo = ClampDn(cam_props.iShowInfo + 1L, 1L) % 3;
  _bToggleInfo = bBtnToggleInfo;

  // Turn left
  static BOOL _bLeft = FALSE;

  if (!_bLeft && bBtnBankingL) cam_ctl.bBankingL = TRUE;
  else
  if (_bLeft && !bBtnBankingL) cam_ctl.bBankingL = FALSE;

  _bLeft = bBtnBankingL;

  // Turn right
  static BOOL _bRight = FALSE;

  if (!_bRight && bBtnBankingR) cam_ctl.bBankingR = TRUE;
  else
  if (_bRight && !bBtnBankingR) cam_ctl.bBankingR = FALSE;

  _bRight = bBtnBankingR;

  // Toggle focus
  static BOOL _bFocus = FALSE;
  if (!_bFocus && bBtnFocus) cam_ctl.iFocusPlayer = ClampDn(cam_ctl.iFocusPlayer + 1L, 1L) % 3;
  _bFocus = bBtnFocus;

  // Toggle grid
  static BOOL _bGrid = FALSE;
  if (!_bGrid && bBtnGrid) cam_props.bGrid = !cam_props.bGrid;
  _bGrid = bBtnGrid;

  // Take snapshot
  static BOOL _bSnap = FALSE;
  if (!_bSnap && bBtnSnap) cam_ctl.bSnapshot = TRUE;
  _bSnap = bBtnSnap;

  // Change screenshot resolution
  static BOOL _bRes = FALSE;
  if (!_bRes && bBtnRes) {
    switch (cam_props.iScreenshotW)
    {
      case 1920: // 1080p 21:9
        cam_props.iScreenshotW = 2560;
        cam_props.iScreenshotH = 1080;
        break;

      case 2560: // 1440p 21:9
        cam_props.iScreenshotW = 3440;
        cam_props.iScreenshotH = 1440;
        break;

      case 3440: // 4K
        cam_props.iScreenshotW = 3840;
        cam_props.iScreenshotH = 2160;
        break;

      default: // Full HD
        cam_props.iScreenshotW = 1920;
        cam_props.iScreenshotH = 1080;
        break;
    }
  }
  _bRes = bBtnRes;

  // Change posing mode
  static BOOL _bChangeMode = FALSE;
  if (!_bChangeMode && bBtnMode) cam_props.bPosingMode = !cam_props.bPosingMode;
  _bChangeMode = bBtnMode;

  // Movement and zoom
  cam_ctl.bMoveF = _pInput->GetButtonState(ocam_kidMoveF_1) || _pInput->GetButtonState(ocam_kidMoveF_2);
  cam_ctl.bMoveB = _pInput->GetButtonState(ocam_kidMoveB_1) || _pInput->GetButtonState(ocam_kidMoveB_2);
  cam_ctl.bMoveL = _pInput->GetButtonState(ocam_kidMoveL_1) || _pInput->GetButtonState(ocam_kidMoveL_2);
  cam_ctl.bMoveR = _pInput->GetButtonState(ocam_kidMoveR_1) || _pInput->GetButtonState(ocam_kidMoveR_2);
  cam_ctl.bMoveU = _pInput->GetButtonState(ocam_kidMoveU_1) || _pInput->GetButtonState(ocam_kidMoveU_2);
  cam_ctl.bMoveD = _pInput->GetButtonState(ocam_kidMoveD_1) || _pInput->GetButtonState(ocam_kidMoveD_2);
  cam_ctl.bZoomIn = _pInput->GetButtonState(ocam_kidZoomIn);
  cam_ctl.bZoomOut = _pInput->GetButtonState(ocam_kidZoomOut);
  cam_ctl.bRotate = _pInput->GetButtonState(ocam_kidRotate);
  cam_ctl.bResetToPlayer = _pInput->GetButtonState(ocam_kidTeleport);
  cam_ctl.bSpeedUp = _pInput->GetButtonState(ocam_kidSpeedUp);

  // Reset FOV and banking angle
  if (_pInput->GetButtonState(ocam_kidReset)) {
    ResetCameraAngles();
  }
};

//================================================================================================//
// Observer camera info
//================================================================================================//

const FLOAT _fInfoScale = 0.8f;

__forceinline PIX GetFontHeight(CDrawPort *pdp) {
  return pdp->dp_FontData->GetHeight() * pdp->dp_fTextScaling + pdp->dp_fTextScaling + 1;
};

__forceinline CTString CameraButton(INDEX iKID) {
  return "^c9FDFFF" + _pInput->GetButtonTransName(iKID) + "^r";
};

static void PrintCommand(CDrawPort *pdp, const CTString &strSymbol, PIX pixY, const CTString &strTextForWidth) {
  // Not displaying symbols or no symbol to display
  if (CObserverCamera::cam_props.iShowInfo <= 1 || strSymbol == "") return;
  const FLOAT fScaling = HEIGHT_SCALING(pdp);

  // Remember last font
  CFontData *pfdLast = pdp->dp_FontData;
  FLOAT fLastScaling = pdp->dp_fTextScaling;

  const PIX pixLastFontH = GetFontHeight(pdp);
  const PIX pixTextW = pdp->GetTextWidth(strTextForWidth);

  pdp->SetFont(_pfdConsoleFont);
  const PIX pixFontH = pdp->dp_FontData->GetHeight();
  pixY += (pixLastFontH - pixFontH) * 0.4f;

  pdp->Fill(30 * fScaling + pixTextW - 2, pixY - 4, pdp->GetTextWidth(strSymbol) + 6, pixFontH + 6, 0x5F);
  pdp->PutText(strSymbol, 30 * fScaling + pixTextW, pixY, 0xFFD700FF);

  // Restore last font
  pdp->SetFont(pfdLast);
  pdp->SetTextScaling(fLastScaling);
};

static void PrintLine(CDrawPort *pdp, const CTString &strText, PIX &pixY, const CTString &strSymbol, BOOL bCategory = FALSE) {
  const FLOAT fScaling = HEIGHT_SCALING(pdp);
  pdp->SetTextScaling(fScaling * _fInfoScale);
  pdp->PutText(strText, (bCategory ? 10 : 20) * fScaling, pixY, bCategory ? 0xFFD700FF : 0xFFFFFFFF);

  PrintCommand(pdp, strSymbol, pixY, strText);
  pixY += GetFontHeight(pdp);
};

__forceinline void PrintIndexField(CDrawPort *pdp, const CTString &strName, INDEX iValue, PIX &pixY, const CTString &strSymbol) {
  PrintLine(pdp, strName + CTString(0, ": %d", iValue), pixY, strSymbol);
};

__forceinline void PrintFloatField(CDrawPort *pdp, const CTString &strName, FLOAT fValue, PIX &pixY, const CTString &strSymbol) {
  PrintLine(pdp, strName + CTString(0, ": %.2f", fValue), pixY, strSymbol);
};

__forceinline void PrintStateField(CDrawPort *pdp, const CTString &strName, BOOL bValue, PIX &pixY, const CTString &strSymbol) {
  PrintLine(pdp, strName + (bValue ? ": ON" : ": OFF"), pixY, strSymbol);
};

// Print camera info and controls
void CObserverCamera::PrintCameraInfo(CDrawPort *pdp) {
  const FLOAT fScaling = HEIGHT_SCALING(pdp);

  // Black background gradient from left to right
  if (cam_props.iShowInfo > 0) {
    pdp->Fill(0, 0, 160 * fScaling, 480 * fScaling, 0x7F, 0x00, 0x7F, 0x00);
  }

  pdp->SetFont(_pfdDisplayFont);
  pdp->SetTextScaling(fScaling);

  // Info header
  PIX pixInfoY = 8 * fScaling;
  pdp->Fill(0, pixInfoY, 480 * fScaling, 48 * fScaling, 0xAF, 0x00, 0xAF, 0x00);
  pixInfoY += 10 * fScaling;

  CTString strHeader(0, TRANS("Observer camera info (press %s to toggle)"), CameraButton(ocam_kidToggleInfo));
  pdp->PutText(strHeader, 8 * fScaling, pixInfoY, 0xFFFFFFFF);
  pixInfoY += GetFontHeight(pdp);

  CTString strSwitchMode, strModeCommand;

  if (cam_bPlayback && !cam_props.bActive) {
    strSwitchMode.PrintF(TRANS("Press %s to enter free-fly mode"), CameraButton(ocam_kidToggle));
    strModeCommand = "ocam_bActive";

  } else if (cam_props.bPosingMode) {
    strSwitchMode.PrintF(TRANS("Press %s to exit posing mode"), CameraButton(ocam_kidChangeMode));
    strModeCommand = "ocam_bPosingMode";

  } else {
    strSwitchMode.PrintF(TRANS("Press %s to enter posing mode"), CameraButton(ocam_kidChangeMode));
    strModeCommand = "ocam_bPosingMode";
  }

  PrintLine(pdp, strSwitchMode, pixInfoY, strModeCommand);
  pixInfoY += GetFontHeight(pdp);

  if (cam_props.iShowInfo <= 0) return;

  // Relevant camera properties
  PrintLine(pdp, TRANS("Camera properties & controls"), pixInfoY, "", TRUE);

  // Playback properties
  if (cam_bPlayback && !cam_props.bActive) {
    PrintStateField(pdp, TRANS("Demo speed control"), cam_props.bPlaybackSpeedControl, pixInfoY, "ocam_bPlaybackSpeedControl");
    PrintStateField(pdp, TRANS("Smooth movement"),    cam_props.bSmoothPlayback,       pixInfoY, "ocam_bSmoothPlayback");
    PrintFloatField(pdp, TRANS("Movement tension"),   cam_props.fSmoothTension,        pixInfoY, "ocam_fSmoothTension");

  // Free fly properties
  } else {
    // Camera controls
    if (!cam_props.bPosingMode) {
      PrintLine(pdp, CTString(0, TRANS("Hold %s to rotate the camera"), CameraButton(ocam_kidRotate)), pixInfoY, "");
      PrintLine(pdp, CTString(0, TRANS("Hold %s to speed up flight"), CameraButton(ocam_kidSpeedUp)), pixInfoY, "");
      PrintLine(pdp, CTString(0, TRANS("Tilt left/right: %s / %s"), CameraButton(ocam_kidBankingL), CameraButton(ocam_kidBankingR))
                   + CTString(0, " (%.1f deg)", NormalizeAngle(cam_cpCurrent.Rot()(3))), pixInfoY, "");
      PrintFloatField(pdp, TRANS("Camera speed"), cam_props.fSpeed, pixInfoY, "ocam_fSpeed");
      pixInfoY += GetFontHeight(pdp);
    }

    // View controls
    PrintLine(pdp, CTString(0, TRANS("Toggle grid: %s"), CameraButton(ocam_kidGrid)) + (cam_props.bGrid ? " (ON)" : " (OFF)"), pixInfoY, "ocam_bGrid");
    PrintLine(pdp, CTString(0, TRANS("Zoom in/out: %s / %s"), CameraButton(ocam_kidZoomIn), CameraButton(ocam_kidZoomOut)), pixInfoY, "");
    PrintFloatField(pdp, TRANS("Field of view"), cam_ctl.fFOV, pixInfoY, "ocam_fFOV");
    pixInfoY += GetFontHeight(pdp);

    CTString strFocus = ": ^c9FDFFF";

    switch (cam_ctl.iFocusPlayer) {
      case 1: strFocus += TRANS("Locked on"); break;
      case 2: strFocus += TRANS("Orbiting"); break;
      default: strFocus += LOCALIZE("None");
    }

    PrintLine(pdp, CTString(0, TRANS("Focus on the current player: %s"), CameraButton(ocam_kidFocus)), pixInfoY, "");
    PrintLine(pdp, TRANS("Focus mode") + strFocus, pixInfoY, "ocam_iFocusPlayer");
    PrintFloatField(pdp, TRANS("Focus distance"), cam_props.fFocusDist, pixInfoY, "ocam_fFocusDist");
    pixInfoY += GetFontHeight(pdp);

    PrintLine(pdp, CTString(0, TRANS("Teleport to current player: %s"), CameraButton(ocam_kidTeleport)), pixInfoY, "");
    PrintLine(pdp, CTString(0, TRANS("Reset tilt and zoom: %s"), CameraButton(ocam_kidReset)), pixInfoY, "");

    if (cam_fnmDemo != "") {
      PrintLine(pdp, CTString(0, TRANS("Take position snapshot: %s"), CameraButton(ocam_kidSnapshot)), pixInfoY, "");
    }

    // Pose customization
    if (cam_props.bPosingMode) {
      pixInfoY += GetFontHeight(pdp);
      PrintLine(pdp, TRANS("Player posing"), pixInfoY, "", TRUE);

      PrintLine(pdp, TRANS("Use movement keys to offset the player model"), pixInfoY, "");
      PrintLine(pdp, TRANS("Use tilt keys to rotate the player model"), pixInfoY, "");
    }
  }

  // Screenshot controls
  pixInfoY = 424 * fScaling;
  pdp->Fill(0, pixInfoY, 480 * fScaling, 48 * fScaling, 0xAF, 0x00, 0xAF, 0x00);
  pixInfoY += 12 * fScaling;

  PrintLine(pdp, CTString(0, TRANS("Press %s to select resolution preset"), CameraButton(ocam_kidChangeRes))
               + CTString(0, ": %dx%d", cam_props.iScreenshotW, cam_props.iScreenshotH), pixInfoY, "ocam_iScreenshotW / ocam_iScreenshotH");
  PrintLine(pdp, CTString(0, TRANS("Take HQ screenshot: %s"), CameraButton(sam_kidScreenshot)), pixInfoY, "");
};

//================================================================================================//
// Observer camera usage
//================================================================================================//

// Free fly camera movement during the game
CObserverCamera::CameraPos &CObserverCamera::FreeFly(CPlayerEntity *penObserving) {
  CameraPos &cp = cam_cpCurrent;

  // "Per second" speed multiplier for this frame
  static const DOUBLE dTicks = (1.0 / _pTimer->TickQuantum);
  const DOUBLE dTimeMul = dTicks * cam_tvDelta.GetSeconds();

  // Camera rotation
  {
    cam_props.fSmoothRotation = Clamp(cam_props.fSmoothRotation, 0.0f, 1.0f);
    const BOOL bInstantRotation = (cam_props.fSmoothRotation == 1.0f);
    const BOOL bLockedOnPlayer = (cam_ctl.iFocusPlayer == 1 && penObserving != NULL);

    // Input rotation
    ANGLE3D aRotate(0, 0, 0);

    // Lock on the current player
    if (bLockedOnPlayer) {
      // Direction towards the player
      CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
      const FLOAT3D vDir = (plView.pl_PositionVector - cp.Pos()).SafeNormalize();

      // Relative angle towards the player
      DirectionVectorToAnglesNoSnap(vDir, aRotate);
      aRotate -= cp.Rot();

      aRotate(1) = NormalizeAngle(aRotate(1));
      aRotate(2) = NormalizeAngle(aRotate(2));

    // Manual mouse input during the game
    } else {
      const BOOL bInput = _pInput->IsInputEnabled() && GetGameAPI()->IsHooked()
                       && !GetGameAPI()->IsMenuOn() && GetGameAPI()->IsGameOn();

      if (bInput && cam_ctl.bRotate && !cam_props.bPosingMode) {
        // Need to do it here in case the game is paused, otherwise axis values aren't updated
        if (_pNetwork->IsPaused() || _pNetwork->GetLocalPause()) {
          _pInput->SetJoyPolling(FALSE);
          _pInput->GetInput(TRUE);
        }

        aRotate(1) = _pInput->GetAxisValue(MOUSE_X_AXIS) * -0.5f;
        aRotate(2) = _pInput->GetAxisValue(MOUSE_Y_AXIS) * +0.5f;
      }
    }

    // Rotate the player model
    if (cam_props.bPosingMode) {
      cam_fPoseRotation += FLOAT(cam_ctl.bBankingL - cam_ctl.bBankingR) * 5.0f * dTimeMul;
      cam_fPoseRotation = WrapAngle(cam_fPoseRotation);

    // Tilt the camera
    } else {
      aRotate(3) = FLOAT(cam_ctl.bBankingL - cam_ctl.bBankingR) * cam_props.fTiltAngleMul * 0.5f;
    }

    // Set immediately
    if (bInstantRotation) {
      cam_aRotation = aRotate;
      cam_aRotation(3) *= dTicks;

      if (!cam_props.bPosingMode) {
        cam_ctl.bBankingL = cam_ctl.bBankingR = 0;
      }

    // Smooth rotation
    } else {
      // Use cosine in order to make real values slightly lower than they are (e.g. 0.5 -> ~0.3)
      const FLOAT fSpeedMul = 1.0f - Cos(cam_props.fSmoothRotation * 90);
      cam_aRotation += (aRotate - cam_aRotation) * dTimeMul * fSpeedMul;

      // Override directional rotation while locked on
      if (bLockedOnPlayer) {
        cam_aRotation(1) = aRotate(1) * dTimeMul * cam_props.fSmoothRotation;
        cam_aRotation(2) = aRotate(2) * dTimeMul * cam_props.fSmoothRotation;
      }
    }

    cp.Rot() += cam_aRotation;

    // Snap banking angle on sharp movement
    if (bInstantRotation && Abs(cam_aRotation(3)) > 0.0f) {
      Snap(cp.Rot()(3), cam_props.fTiltAngleMul * 10.0f);
    }
  }

  // Orbit the player instead of simply locking onto them or following them
  if (cam_ctl.iFocusPlayer == 2 && penObserving != NULL) {
    CPlacement3D plPlayer(FLOAT3D(0, 2, 0), ANGLE3D(0, 0, 0));
    plPlayer.RelativeToAbsoluteSmooth(penObserving->GetLerpedPlacement());

    FLOAT3D vSrc = plPlayer.pl_PositionVector;

    // Directly on the player if close to zero
    if (Abs(cam_props.fFocusDist) < 0.001f) {
      cp.Pos() = vSrc;

    // Place the camera behind the player based on the current angle
    } else {
      FLOAT3D vDir;
      AnglesToDirectionVector(cp.Rot(), vDir);

      // Default to 6 meters if focus distance isn't set
      FLOAT3D vDst = vSrc - vDir * (cam_props.fFocusDist < 0.0f ? 6.0f : cam_props.fFocusDist);

      // Cast a ray to keep the camera within the map geometry
      CCastRay crRay(penObserving, vSrc, vDst);
      crRay.cr_ttHitModels = CCastRay::TT_NONE;
      crRay.cr_bHitTranslucentPortals = FALSE;
      crRay.cr_fTestR = 2.0f;
      crRay.Cast(IWorld::GetWorld());

      cp.Pos() = vSrc - vDir * (crRay.cr_fHitDistance - 1.0f);
    }

  // Camera movement
  } else if (cam_props.fSpeed != 0.0f) {
    // Movement vector
    FLOAT3D vMoveDir(0, 0, 0);

    // Follow the player and always stay close enough
    if (cam_props.fFocusDist >= 0.0f && penObserving != NULL) {
      // Above the player for better visibility when focusing on the player
      CPlacement3D plFocus(FLOAT3D(0, 4, 0), ANGLE3D(0, 0, 0));
      plFocus.RelativeToAbsoluteSmooth(penObserving->GetLerpedPlacement());
      FLOAT3D vToPlayer = (plFocus.pl_PositionVector - cp.Pos());

      if (vToPlayer.Length() > cam_props.fFocusDist) {
        vMoveDir = vToPlayer.SafeNormalize() * cam_props.fSpeed;
      }
    }

    // Add input vector to the any current movement
    FLOAT3D vInputDir(cam_ctl.bMoveR - cam_ctl.bMoveL, cam_ctl.bMoveU - cam_ctl.bMoveD, cam_ctl.bMoveB - cam_ctl.bMoveF);
    const FLOAT fInputLength = vInputDir.Length();

    if (fInputLength > 0.01f) {
      // Offset the pose model
      if (cam_props.bPosingMode) {
        cam_vPoseOffset += vInputDir * 0.05f * dTimeMul;
        cam_vPoseOffset(1) = Clamp(cam_vPoseOffset(1), -1.0f, +1.0f);
        cam_vPoseOffset(2) = Clamp(cam_vPoseOffset(2), -1.0f, +1.0f);
        cam_vPoseOffset(3) = Clamp(cam_vPoseOffset(3), -1.0f, +1.0f);

      // Move the camera around
      } else {
        FLOATmatrix3D mRot;
        MakeRotationMatrixFast(mRot, ANGLE3D(cp.Rot()(1), 0, 0)); // Only heading direction

        // Normalize vector, apply current rotation and speed
        const FLOAT fSpeedMul = cam_ctl.bSpeedUp ? 5.0f : 1.0f;
        vMoveDir += (vInputDir / fInputLength) * mRot * cam_props.fSpeed * fSpeedMul;
      }
    }

    // Set immediately
    if (cam_props.fSmoothMovement >= 1.0f) {
      cam_vMovement = vMoveDir;

    // Smooth movement
    } else {
      // Use cosine in order to make real values slightly lower than they are (e.g. 0.5 -> ~0.3)
      const FLOAT fSpeedMul = 1.0f - Cos(ClampDn(cam_props.fSmoothMovement, 0.0f) * 90);
      cam_vMovement += (vMoveDir - cam_vMovement) * dTimeMul * fSpeedMul;
    }

    cp.Pos() += cam_vMovement * dTimeMul;
  }

  const FLOAT fFOVDirSpeed = (cam_ctl.bZoomOut - cam_ctl.bZoomIn) * dTimeMul * cam_props.fFOVChangeMul;
  cam_ctl.fFOV = Clamp(FLOAT(cam_ctl.fFOV + fFOVDirSpeed), 10.0f, 170.0f);
  cp.fFOV = cam_ctl.fFOV;

  // Snap back to view of the current player
  if (cam_ctl.bResetToPlayer && penObserving != NULL) {
    cp.plPos = IWorld::GetViewpoint(penObserving, TRUE);
  }

  // Take position snapshot for a demo
  if (cam_ctl.bSnapshot && cam_fnmDemo != "") {
    cam_ctl.bSnapshot = FALSE;

    // Try starting the recording first
    if (StartRecording()) {
      CTString strTime;
      IData::PrintDetailedTime(strTime, DOUBLE(_pTimer->GetLerpedCurrentTick() - cam_tmStartTime));

      CPrintF(TRANS("OCAM: Took snapshot at %s\n"), strTime.str_String);
      WritePos(cp);
    }
  }

  return cp;
};

// Update the camera and render the world through it
BOOL CObserverCamera::Update(CEntity *pen, CDrawPort *pdp) {
  // Time variables
  static CTimerValue tvLastTick;
  const CTimerValue tvRealTick = _pTimer->GetHighPrecisionTimer();

  // Get start time when it's time to render
  if (cam_tmStartTime == -100.0f) {
    cam_tmStartTime = _pTimer->GetLerpedCurrentTick();
    cam_tvDelta = DOUBLE(0);

  // Calculate delta from last tick
  } else {
    cam_tvDelta = (tvRealTick - tvLastTick);
  }

  // Save this tick
  tvLastTick = tvRealTick;

  // Camera is currently disabled
  if (!IsActive()) {
    cam_props.bActive = FALSE; // Prevent it from suddenly switching if the conditions are met

    // Remember player view position for the next activation if it's not possible to do through the render view patch
  #if !_PATCHCONFIG_ENGINEPATCHES || !_PATCHCONFIG_FIX_RENDERING
    if (IsDerivedFromID(pen, CPlayerEntity_ClassID)) {
      // Make sure the player has initialized properly by checking entity density (CMovableEntity's default is 5000)
      CPlayerEntity *penPlayer = (CPlayerEntity *)pen;

      if (penPlayer->en_fDensity != 5000.0f) {
        cam_cpCurrent.plPos = IWorld::GetViewpoint(penPlayer, FALSE);
      }
    }
  #endif

    cam_vMovement = FLOAT3D(0, 0, 0);
    cam_aRotation = ANGLE3D(0, 0, 0);
    cam_penViewer = NULL;
    ResetPhotoModePose(FALSE);
    return FALSE;
  }

  // Remember viewer entity
  cam_penViewer = pen;

  // Determine camera position for this frame
  CameraPos &cp = cam_cpView;

  // Camera playback
  if (cam_bPlayback && !cam_props.bActive) {
    // Read next position when the destination expires
    TIME tmNow = _pTimer->GetLerpedCurrentTick() - cam_tmStartTime;

    // Skip around
    while (cam_bPlayback && tmNow > cam_acpCurve[2].tmTick) {
      // Advance positions
      INDEX i;
      for (i = 0; i < 3; i++) {
        cam_acpCurve[i] = cam_acpCurve[i + 1];
      }

      // Read the next one
      ReadPos(cam_acpCurve[i]);

      SetSpeed(cam_acpCurve[1].fSpeed);
    }

    // Playback is over
    if (!cam_bPlayback) return TRUE;

    // Interpolate between two positions
    const CameraPos *acp = cam_acpCurve;
    FLOAT fRatio = Clamp((tmNow - acp[1].tmTick) / (acp[2].tmTick - acp[1].tmTick), (TIME)0.0, (TIME)1.0);

    // Move through a curve between two points
    if (cam_props.bSmoothPlayback) {
      cam_props.fSmoothTension = Clamp(cam_props.fSmoothTension, 0.0f, 1.0f);
      cp.Pos() = CatmullRom(acp[0].Pos(), acp[1].Pos(), acp[2].Pos(), acp[3].Pos(), fRatio, cam_props.fSmoothTension);
      cp.Rot() = CatmullRom(acp[0].Rot(), acp[1].Rot(), acp[2].Rot(), acp[3].Rot(), fRatio, cam_props.fSmoothTension);
      cp.fFOV  = CatmullRom(acp[0].fFOV,  acp[1].fFOV,  acp[2].fFOV,  acp[3].fFOV,  fRatio, cam_props.fSmoothTension);

    // Linear movement from one point to another
    } else {
      cp.Pos() = Lerp(acp[1].Pos(), acp[2].Pos(), fRatio);
      cp.Rot() = Lerp(acp[1].Rot(), acp[2].Rot(), fRatio);
      cp.fFOV  = Lerp(acp[1].fFOV,  acp[2].fFOV,  fRatio);
    }

    // Update position of the free fly camera based on the playback camera for the next activation
    cam_cpCurrent.plPos = cp.plPos;
    cam_ctl.fFOV = cp.fFOV;

  // Free fly movement
  } else {
    CPlayerEntity *penObserving = NULL;

    if (IsDerivedFromID(pen, CPlayerEntity_ClassID)) {
      penObserving = (CPlayerEntity *)pen;
    }

    cp = FreeFly(penObserving);
  }

  const PIX pixW = pdp->GetWidth();
  const PIX pixH = pdp->GetHeight();

  // Prepare view projection
  CPerspectiveProjection3D prProjection;
  prProjection.FOVL() = cp.fFOV;
  prProjection.ScreenBBoxL() = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(pixW, pixH));
  prProjection.AspectRatioL() = 1.0f;
  prProjection.FrontClipDistanceL() = 0.3f;

  // Render view from the camera
  CAnyProjection3D apr;
  apr = prProjection;
  apr->ViewerPlacementL() = cp.plPos;

  CWorld &wo = _pNetwork->ga_World;
  RenderView(wo, *(CEntity *)NULL, apr, *pdp); // NULL viewer to make the player entity visible

  // Rule of thirds grid (2 pixels thick, light gray)
  if (cam_props.bGrid) {
    const PIX pixGridW = pixW / 3;
    const PIX pixGridH = pixH / 3;
    const COLOR colGrid = 0xBFBFBFFF;

    // Vertical lines
    pdp->DrawLine(pixGridW+0,   0, pixGridW+1,   pixH, colGrid);
    pdp->DrawLine(pixGridW+1,   0, pixGridW+1,   pixH, colGrid);

    pdp->DrawLine(pixGridW*2+0, 0, pixGridW*2+0, pixH, colGrid);
    pdp->DrawLine(pixGridW*2+1, 0, pixGridW*2+1, pixH, colGrid);

    // Horizontal lines
    pdp->DrawLine(0, pixGridH+0,   pixW, pixGridH+0,   colGrid);
    pdp->DrawLine(0, pixGridH+1,   pixW, pixGridH+1,   colGrid);

    pdp->DrawLine(0, pixGridH*2+0, pixW, pixGridH*2+0, colGrid);
    pdp->DrawLine(0, pixGridH*2+1, pixW, pixGridH*2+1, colGrid);
  }

  PrintCameraInfo(pdp);

  // Listen to world sounds
  cam_sliWorld.sli_vPosition = cp.Pos();
  MakeRotationMatrixFast(cam_sliWorld.sli_mRotation, cp.Rot());
  cam_sliWorld.sli_fVolume = 1.0f;
  cam_sliWorld.sli_vSpeed = cam_vMovement;
  cam_sliWorld.sli_penEntity = NULL;
  cam_sliWorld.sli_fFilter = 0.0f;

  CEnvironmentType &et = wo.wo_aetEnvironmentTypes[0];
  cam_sliWorld.sli_iEnvironmentType = et.et_iType;
  cam_sliWorld.sli_fEnvironmentSize = et.et_fSize;

  _pSound->Listen(cam_sliWorld);
  return TRUE;
};

// Take a high quality screenshot of the current view
BOOL CObserverCamera::TakeScreenshot(CImageInfo &iiScreenshot) {
  // Limit resolution
  cam_props.iScreenshotW = Clamp(cam_props.iScreenshotW, (INDEX)1, (INDEX)20000);
  cam_props.iScreenshotH = Clamp(cam_props.iScreenshotH, (INDEX)1, (INDEX)20000);

  // Create canvas for the screenshot
  CDrawPort *pdpScreenshot;
  _pGfx->CreateWorkCanvas(cam_props.iScreenshotW, cam_props.iScreenshotH, &pdpScreenshot);

  if (pdpScreenshot == NULL) return FALSE;

  BOOL bTaken = FALSE;

  if (pdpScreenshot->Lock()) {
    // Prepare view projection
    CPerspectiveProjection3D prProjection;
    prProjection.FOVL() = cam_cpView.fFOV;
    prProjection.ScreenBBoxL() = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(pdpScreenshot->GetWidth(), pdpScreenshot->GetHeight()));
    prProjection.AspectRatioL() = 1.0f;
    prProjection.FrontClipDistanceL() = 0.3f;

    // Render view from the camera
    CAnyProjection3D apr;
    apr = prProjection;
    apr->ViewerPlacementL() = cam_cpView.plPos;

    // Force lens flares to render immediately on the screenshot
    extern BOOL _bLensFlaresFullFade;
    _bLensFlaresFullFade = TRUE;

    CWorld &wo = _pNetwork->ga_World;
    RenderView(wo, *(CEntity *)NULL, apr, *pdpScreenshot); // NULL viewer to make the player entity visible

    _bLensFlaresFullFade = FALSE;

    // Take screenshot
    pdpScreenshot->GrabScreen(iiScreenshot, 0);
    pdpScreenshot->Unlock();

    bTaken = TRUE;
  }

  // Destroy screenshot canvas
  _pGfx->DestroyWorkCanvas(pdpScreenshot);

  return bTaken;
};
