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

    strLine.ScanF("%g: %g: %g %g %g:%g %g %g:%g",
      &cp.tmTick, &cp.fSpeed, &cp.vPos(1), &cp.vPos(2), &cp.vPos(3),
      &cp.aRot(1), &cp.aRot(2), &cp.aRot(3), &cp.fFOV);

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }
};

void CObserverCamera::WritePos(CameraPos &cp) {
  try {
    CTString strLine(0, "%g: %g: %g %g %g:%g %g %g:%g",
      _pTimer->GetLerpedCurrentTick() - cam_tmStartTime, 1.0f,
      cp.vPos(1), cp.vPos(2), cp.vPos(3), cp.aRot(1), cp.aRot(2), cp.aRot(3), cp.fFOV);

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
  cam_cpCurrent.aRot(3) = 0.0f;
  cam_aRotation(3) = 0.0f;
};

// Default control buttons
static INDEX ocam_kidToggle = KID_P;

static INDEX ocam_kidBankingL   = KID_Q;
static INDEX ocam_kidBankingR   = KID_E;
static INDEX ocam_kidFollow     = KID_F;
static INDEX ocam_kidSnapshot   = KID_TAB;
static INDEX ocam_kidChangeRes  = KID_F10;

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

// Initialize camera interface
void CObserverCamera::Init(void)
{
  // Bindable camera controls
  _pShell->DeclareSymbol("user INDEX ocam_bMoveForward;",      &cam_ctl.bMoveF);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveBackward;",     &cam_ctl.bMoveB);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveLeft;",         &cam_ctl.bMoveL);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveRight;",        &cam_ctl.bMoveR);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveUp;",           &cam_ctl.bMoveU);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveDown;",         &cam_ctl.bMoveD);
  _pShell->DeclareSymbol("user INDEX ocam_bTurnBankingLeft;",  &cam_ctl.bBankingL);
  _pShell->DeclareSymbol("user INDEX ocam_bTurnBankingRight;", &cam_ctl.bBankingR);
  _pShell->DeclareSymbol("user INDEX ocam_bZoomIn;",           &cam_ctl.bZoomIn);
  _pShell->DeclareSymbol("user INDEX ocam_bZoomOut;",          &cam_ctl.bZoomOut);
  _pShell->DeclareSymbol("user FLOAT ocam_fFOV;",              &cam_ctl.fFOV);
  _pShell->DeclareSymbol("user INDEX ocam_bResetToPlayer;",    &cam_ctl.bResetToPlayer);
  _pShell->DeclareSymbol("user INDEX ocam_bFollowPlayer;",     &cam_ctl.bFollowPlayer);
  _pShell->DeclareSymbol("user INDEX ocam_bSnapshot;",         &cam_ctl.bSnapshot);

  // Keys for the default controls
  _pShell->DeclareSymbol("persistent INDEX ocam_kidToggle;",     &ocam_kidToggle);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidBankingL;",   &ocam_kidBankingL);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidBankingR;",   &ocam_kidBankingR);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidFollow;",     &ocam_kidFollow);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidSnapshot;",   &ocam_kidSnapshot);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidChangeRes;",  &ocam_kidChangeRes);

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

  // Camera properties
  _pShell->DeclareSymbol("           user INDEX ocam_bActive;",               &cam_props.bActive);
  _pShell->DeclareSymbol("persistent user INDEX ocam_iShowInfo;",             &cam_props.iShowInfo);
  _pShell->DeclareSymbol("persistent user INDEX ocam_bDefaultControls;",      &cam_props.bDefaultControls);
  _pShell->DeclareSymbol("persistent user INDEX ocam_bPlaybackSpeedControl;", &cam_props.bPlaybackSpeedControl);
  _pShell->DeclareSymbol("persistent user INDEX ocam_bSmoothPlayback;",       &cam_props.bSmoothPlayback);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothTension;",        &cam_props.fSmoothTension);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSpeed;",                &cam_props.fSpeed);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fTiltAngleMul;",         &cam_props.fTiltAngleMul);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fFOVChangeMul;",         &cam_props.fFOVChangeMul);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothMovement;",       &cam_props.fSmoothMovement);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothRotation;",       &cam_props.fSmoothRotation);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fFollowDist;",           &cam_props.fFollowDist);

  _pShell->DeclareSymbol("persistent user INDEX ocam_iScreenshotW;", &cam_props.iScreenshotW);
  _pShell->DeclareSymbol("persistent user INDEX ocam_iScreenshotH;", &cam_props.iScreenshotH);
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

  SetSpeed(1.0f);
};

// Retrieve camera state field
BOOL &CObserverCamera::GetState(void) {
  return cam_props.bActive;
};

// Check if camera is on
BOOL CObserverCamera::IsActive(void) {
  // Camera can only be used during observing or in demos
  const BOOL bObserver = (GetGameAPI()->GetCurrentSplitCfg() == CGame::SSC_OBSERVER);

  // Or if it needs to be used externally
  if (cam_bExternalUsage || bObserver || _pNetwork->IsPlayingDemo()) {
    return cam_props.bActive || cam_bPlayback;
  }

  return FALSE;
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

// Direct button input using default controls
void CObserverCamera::UpdateControls(void) {
  // Toggle the camera itself
  const BOOL bBtnToggle = _pInput->GetButtonState(ocam_kidToggle);

  static BOOL _bToggle = FALSE;
  if (!_bToggle && bBtnToggle) cam_props.bActive = !cam_props.bActive;
  _bToggle = bBtnToggle;

  // Camera or default controls are disabled
  if (!cam_props.bDefaultControls || !IsActive()) return;

  _pInput->SetJoyPolling(FALSE);
  _pInput->GetInput(FALSE);

  // Button states for some controls
  const BOOL bBtnBankingL = _pInput->GetButtonState(ocam_kidBankingL);
  const BOOL bBtnBankingR = _pInput->GetButtonState(ocam_kidBankingR);
  const BOOL bBtnFollow   = _pInput->GetButtonState(ocam_kidFollow);
  const BOOL bBtnSnap     = _pInput->GetButtonState(ocam_kidSnapshot);
  const BOOL bBtnRes      = _pInput->GetButtonState(ocam_kidChangeRes);

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

  // Toggle following
  static BOOL _bFollow = FALSE;
  if (!_bFollow && bBtnFollow) cam_ctl.bFollowPlayer = !cam_ctl.bFollowPlayer;
  _bFollow = bBtnFollow;

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

  // Movement and zoom
  cam_ctl.bMoveF = _pInput->GetButtonState(ocam_kidMoveF_1) || _pInput->GetButtonState(ocam_kidMoveF_2);
  cam_ctl.bMoveB = _pInput->GetButtonState(ocam_kidMoveB_1) || _pInput->GetButtonState(ocam_kidMoveB_2);
  cam_ctl.bMoveL = _pInput->GetButtonState(ocam_kidMoveL_1) || _pInput->GetButtonState(ocam_kidMoveL_2);
  cam_ctl.bMoveR = _pInput->GetButtonState(ocam_kidMoveR_1) || _pInput->GetButtonState(ocam_kidMoveR_2);
  cam_ctl.bMoveU = _pInput->GetButtonState(ocam_kidMoveU_1) || _pInput->GetButtonState(ocam_kidMoveU_2);
  cam_ctl.bMoveD = _pInput->GetButtonState(ocam_kidMoveD_1) || _pInput->GetButtonState(ocam_kidMoveD_2);
  // [Cecil] FIXME: Scroll wheel isn't being updated when the game is paused
  cam_ctl.bZoomIn = _pInput->GetButtonState(ocam_kidZoomIn);
  cam_ctl.bZoomOut = _pInput->GetButtonState(ocam_kidZoomOut);
  cam_ctl.bResetToPlayer = _pInput->GetButtonState(ocam_kidTeleport);

  // Reset FOV and banking angle
  if (_pInput->GetButtonState(ocam_kidReset)) {
    ResetCameraAngles();
  }
};

// Print info and default controls for the camera
void CObserverCamera::PrintCameraInfo(CDrawPort *pdp) {
  if (cam_props.iShowInfo <= 0) return;

  const FLOAT fScaling = HEIGHT_SCALING(pdp);
  const FLOAT fTextScaling = fScaling * 0.8f;
  const PIX pixLineHeight = _pfdDisplayFont->GetHeight() * fTextScaling + fTextScaling + 1;

  pdp->SetFont(_pfdDisplayFont);
  pdp->SetTextScaling(fScaling);

  // Info header
  PIX pixInfoY = 16 * fScaling;
  pdp->PutText(TRANS("Observer camera info (ocam_iShowInfo)"), 8 * fScaling, pixInfoY, 0xFFFFFF9F);

  // Relevant camera properties
  pdp->SetTextScaling(fTextScaling);

  pixInfoY += pixLineHeight * 2;
  pdp->PutText(TRANS("Camera properties"), 8 * fScaling, pixInfoY, 0xFFD700FF);

  CTString strProps = "";

  if (cam_bPlayback && !cam_props.bActive) {
    strProps += CTString(0, "ocam_bPlaybackSpeedControl = %d\n", cam_props.bPlaybackSpeedControl);
    strProps += CTString(0, "ocam_bSmoothPlayback = %d\n", cam_props.bSmoothPlayback);
    strProps += CTString(0, "ocam_fSmoothTension = %g\n", cam_props.fSmoothTension);

  } else {
    strProps += CTString(0, "ocam_bDefaultControls = %d\n", cam_props.bDefaultControls);
    strProps += CTString(0, "ocam_fSpeed = %g\n", cam_props.fSpeed);
    strProps += CTString(0, "ocam_fTiltAngleMul = %g\n", cam_props.fTiltAngleMul);
    strProps += CTString(0, "ocam_fFOVChangeMul = %g\n", cam_props.fFOVChangeMul);
    strProps += CTString(0, "ocam_fSmoothMovement = %g\n", cam_props.fSmoothMovement);
    strProps += CTString(0, "ocam_fSmoothRotation = %g\n", cam_props.fSmoothRotation);
    strProps += CTString(0, "ocam_fFOV = %g\n", cam_ctl.fFOV);
    strProps += CTString(0, "ocam_bFollowPlayer = %d\n", cam_ctl.bFollowPlayer);
    strProps += CTString(0, "ocam_fFollowDist = %g\n", cam_props.fFollowDist);
  }

  strProps += CTString(0, "ocam_iScreenshotW/H = %dx%d\n", cam_props.iScreenshotW, cam_props.iScreenshotH);

  pixInfoY += pixLineHeight;
  pdp->PutText(strProps, 16 * fScaling, pixInfoY, 0xFFFFFFFF);

  // Default controls for free fly camera
  if (cam_props.bActive && cam_props.iShowInfo > 1) {
    pixInfoY += pixLineHeight * 10;
    pdp->PutText(TRANS("Default camera controls"), 8 * fScaling, pixInfoY, 0xFFD700FF);

    CTString strControls = TRANS("Disabled");

    if (cam_props.bDefaultControls) {
      strControls = CTString(0, TRANS("Zoom in/out: %s/%s\n"), _pInput->GetButtonTransName(ocam_kidZoomIn), _pInput->GetButtonTransName(ocam_kidZoomOut));
      strControls += CTString(0, TRANS("Follow current player: %s\n"), _pInput->GetButtonTransName(ocam_kidFollow));
      strControls += CTString(0, TRANS("Teleport to current player: %s\n"), _pInput->GetButtonTransName(ocam_kidTeleport));
      strControls += CTString(0, TRANS("Reset tilt and zoom: %s\n"), _pInput->GetButtonTransName(ocam_kidReset));

      if (cam_fnmDemo != "") {
        strControls += CTString(0, TRANS("Take position snapshot: %s\n"), _pInput->GetButtonTransName(ocam_kidSnapshot));
      }

      // Screenshot controls
      strControls += "\n";
      strControls += CTString(0, TRANS("Select resolution preset: %s\n"), _pInput->GetButtonTransName(ocam_kidChangeRes));
      strControls += CTString(0, TRANS("Take HQ screenshot: %s\n"), _pInput->GetButtonTransName(sam_kidScreenshot));
    }

    pixInfoY += pixLineHeight;
    pdp->PutText(strControls, 16 * fScaling, pixInfoY, 0xFFFFFFFF);
  }
};

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
    const BOOL bFollowing = (cam_ctl.bFollowPlayer && penObserving != NULL);

    // Input rotation
    ANGLE3D aRotate(0, 0, 0);

    // Focus on the current player
    if (bFollowing) {
      // Direction towards the player
      CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
      const FLOAT3D vDir = (plView.pl_PositionVector - cp.vPos).SafeNormalize();

      // Relative angle towards the player
      DirectionVectorToAnglesNoSnap(vDir, aRotate);
      aRotate -= cp.aRot;

      aRotate(1) = NormalizeAngle(aRotate(1));
      aRotate(2) = NormalizeAngle(aRotate(2));

    // Manual mouse input during the game
    } else {
      BOOL bInput = _pInput->IsInputEnabled() && GetGameAPI()->IsHooked()
                 && !GetGameAPI()->IsMenuOn() && GetGameAPI()->IsGameOn();

      if (bInput) {
        // Need to do it here in case the game is paused, otherwise axis values aren't updated
        if (_pNetwork->IsPaused() || _pNetwork->GetLocalPause()) {
          _pInput->SetJoyPolling(FALSE);
          _pInput->GetInput(FALSE);
        }

        aRotate(1) = _pInput->GetAxisValue(MOUSE_X_AXIS) * -0.5f;
        aRotate(2) = _pInput->GetAxisValue(MOUSE_Y_AXIS) * +0.5f;
      }
    }

    aRotate(3) = FLOAT(cam_ctl.bBankingL - cam_ctl.bBankingR) * cam_props.fTiltAngleMul * 0.5f;

    // Set immediately
    if (bInstantRotation) {
      cam_aRotation = aRotate;
      cam_aRotation(3) *= dTicks;
      cam_ctl.bBankingL = cam_ctl.bBankingR = 0;

    // Smooth rotation
    } else {
      // Use cosine in order to make real values slightly lower than they are (e.g. 0.5 -> ~0.3)
      const FLOAT fSpeedMul = 1.0f - Cos(cam_props.fSmoothRotation * 90);
      cam_aRotation += (aRotate - cam_aRotation) * dTimeMul * fSpeedMul;

      // Override directional rotation while following
      if (bFollowing) {
        cam_aRotation(1) = aRotate(1) * dTimeMul * cam_props.fSmoothRotation;
        cam_aRotation(2) = aRotate(2) * dTimeMul * cam_props.fSmoothRotation;
      }
    }

    cp.aRot += cam_aRotation;

    // Snap banking angle on sharp movement
    if (bInstantRotation && Abs(cam_aRotation(3)) > 0.0f) {
      Snap(cp.aRot(3), cam_props.fTiltAngleMul * 10.0f);
    }
  }

  // Camera movement
  if (cam_props.fSpeed != 0.0f) {
    // Movement vector
    FLOAT3D vMoveDir(0, 0, 0);

    // Follow the player and always stay close enough
    if (cam_props.fFollowDist >= 0.0f && penObserving != NULL) {
      CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
      FLOAT3D vToPlayer = (plView.pl_PositionVector - cp.vPos);

      if (vToPlayer.Length() > cam_props.fFollowDist) {
        vMoveDir = vToPlayer.SafeNormalize() * cam_props.fSpeed;
      }
    }

    // Add input vector to the any current movement
    FLOAT3D vInputDir(cam_ctl.bMoveR - cam_ctl.bMoveL, cam_ctl.bMoveU - cam_ctl.bMoveD, cam_ctl.bMoveB - cam_ctl.bMoveF);
    const FLOAT fInputLength = vInputDir.Length();

    if (fInputLength > 0.01f) {
      FLOATmatrix3D mRot;
      MakeRotationMatrixFast(mRot, ANGLE3D(cp.aRot(1), 0, 0)); // Only heading direction

      // Normalize vector, apply current rotation and speed
      vMoveDir += (vInputDir / fInputLength) * mRot * cam_props.fSpeed;
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

    cp.vPos += cam_vMovement * dTimeMul;
  }

  const FLOAT fFOVDirSpeed = (cam_ctl.bZoomOut - cam_ctl.bZoomIn) * dTimeMul * cam_props.fFOVChangeMul;
  cam_ctl.fFOV = Clamp(FLOAT(cam_ctl.fFOV + fFOVDirSpeed), 10.0f, 170.0f);
  cp.fFOV = cam_ctl.fFOV;

  // Snap back to view of the current player
  if (cam_ctl.bResetToPlayer && penObserving != NULL)
  {
    CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
    cp.vPos = plView.pl_PositionVector;
    cp.aRot = plView.pl_OrientationAngle;
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

    // Remember player view position for the next activation
    if (IsDerivedFromID(pen, CPlayerEntity_ClassID)) {
      // Make sure the player has initialized properly by checking entity density (CMovableEntity's default is 5000)
      CPlayerEntity *penPlayer = (CPlayerEntity *)pen;

      if (penPlayer->en_fDensity != 5000.0f) {
        CPlacement3D plView = IWorld::GetViewpoint(penPlayer, FALSE);

        cam_cpCurrent.vPos = plView.pl_PositionVector;
        cam_cpCurrent.aRot = plView.pl_OrientationAngle;
      }
    }

    cam_vMovement = FLOAT3D(0, 0, 0);
    cam_aRotation = ANGLE3D(0, 0, 0);
    return FALSE;
  }

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
      cp.vPos = CatmullRom(acp[0].vPos, acp[1].vPos, acp[2].vPos, acp[3].vPos, fRatio, cam_props.fSmoothTension);
      cp.aRot = CatmullRom(acp[0].aRot, acp[1].aRot, acp[2].aRot, acp[3].aRot, fRatio, cam_props.fSmoothTension);
      cp.fFOV = CatmullRom(acp[0].fFOV, acp[1].fFOV, acp[2].fFOV, acp[3].fFOV, fRatio, cam_props.fSmoothTension);

    // Linear movement from one point to another
    } else {
      cp.vPos = Lerp(acp[1].vPos, acp[2].vPos, fRatio);
      cp.aRot = Lerp(acp[1].aRot, acp[2].aRot, fRatio);
      cp.fFOV = Lerp(acp[1].fFOV, acp[2].fFOV, fRatio);
    }

  // Free fly movement
  } else {
    CPlayerEntity *penObserving = NULL;

    if (IsDerivedFromID(pen, CPlayerEntity_ClassID)) {
      penObserving = (CPlayerEntity *)pen;
    }

    cp = FreeFly(penObserving);
  }

  // Prepare view projection
  CPerspectiveProjection3D prProjection;
  prProjection.FOVL() = cp.fFOV;
  prProjection.ScreenBBoxL() = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(pdp->GetWidth(), pdp->GetHeight()));
  prProjection.AspectRatioL() = 1.0f;
  prProjection.FrontClipDistanceL() = 0.3f;

  // Render view from the camera
  CAnyProjection3D apr;
  apr = prProjection;
  apr->ViewerPlacementL() = cp.GetPlacement();

  CWorld &wo = _pNetwork->ga_World;
  RenderView(wo, *(CEntity *)NULL, apr, *pdp);

  PrintCameraInfo(pdp);

  // Listen to world sounds
  cam_sliWorld.sli_vPosition = cp.vPos;
  MakeRotationMatrixFast(cam_sliWorld.sli_mRotation, cp.aRot);
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
    apr->ViewerPlacementL() = cam_cpView.GetPlacement();

    CWorld &wo = _pNetwork->ga_World;
    RenderView(wo, *(CEntity *)NULL, apr, *pdpScreenshot);

    // Take screenshot
    pdpScreenshot->GrabScreen(iiScreenshot, 0);
    pdpScreenshot->Unlock();

    bTaken = TRUE;
  }

  // Destroy screenshot canvas
  _pGfx->DestroyWorkCanvas(pdpScreenshot);

  return bTaken;
};
