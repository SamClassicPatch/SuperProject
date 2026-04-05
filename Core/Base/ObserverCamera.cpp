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
#include "Objects/SimpleConfigs.h"

#include <Models/Player/SeriousSam/Body.h>
#include <Models/Player/SeriousSam/Player.h>

// [Cecil] TEMP: Custom poses
#define PHOTO_MODE_CUSTOM_POSES 1

// Global controls and properties for the observer camera
CObserverCamera::CameraControl CObserverCamera::cam_ctl;
CObserverCamera::CameraProps CObserverCamera::cam_props;

// Specific pose for the player in photo mode
struct SPhotoModePose {
  CTString strName;
  INDEX iLegsAnim, iBodyAnim, iLegsFrame, iBodyFrame;
  ANGLE3D aBody;

  SPhotoModePose() : iLegsAnim(-1), iBodyAnim(-1), iLegsFrame(0), iBodyFrame(0), aBody(0, 0, 0) {};
};

// Available player poses
static CStaticStackArray<SPhotoModePose> _aPhotoModePoses;
static SPhotoModePose _poseCustom;

// Specific item model with some offset for hand attachments
struct SPhotoModeItem {
  CTString strName;
  CModelObject mo;
  CPlacement3D plOffset;

  SPhotoModeItem() : plOffset(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)) {};
};

// Available items for each hand
static CStaticStackArray<SPhotoModeItem> _aPhotoModeItems;

// Convenience structure for performing actions based on pressed keys
struct SCamKey {
  bool bState;
  SCamKey() : bState(false) {};

  // Check if the button is pressed and update its state
  inline bool operator()(INDEX iKID) {
    const bool bButton = !!_pInput->GetButtonState(iKID);
    const bool bPressed = (!bState && bButton);
    bState = bButton;
    return bPressed;
  };
};

#if PHOTO_MODE_CUSTOM_POSES

static INDEX ocam_bCustomPoses = FALSE;

// [Cecil] TEMP: Lists of animations for custom poses
void PrintCustomPose(CObserverCamera &ocam, CDrawPort *pdp) {
  if (!ocam_bCustomPoses) return;
  if (_poseCustom.iLegsAnim < 0 && _poseCustom.iBodyAnim < 0) return;

  // Get legs attachment
  CAttachmentModelObject *pamo = ocam.cam_moPose.GetAttachmentModel(0);
  if (pamo == NULL) return;
  CModelObject &moLegs = pamo->amo_moModelObject;

  // Get body attachment
  pamo = moLegs.GetAttachmentModel(PLAYER_ATTACHMENT_TORSO);
  if (pamo == NULL) return;
  CModelObject &moBody = pamo->amo_moModelObject;

  INDEX i, ct;
  CAnimInfo ai;
  CTString str;

  const FLOAT fScaling = HEIGHT_SCALING(pdp);
  const PIX pixW = pdp->GetWidth();
  PIX pixY = 4 * fScaling;

  const FLOAT fAnimInfoScaling = fScaling * 0.55f;
  const PIX pixAnimH = _pfdDisplayFont->GetHeight() * fAnimInfoScaling + fAnimInfoScaling + 1;
  pdp->SetTextScaling(fAnimInfoScaling);

  const ANGLE3D &aBody = _poseCustom.aBody;
  pdp->PutTextR(CTString(0, "ocam_fPoseBodyH/P/B: [%g, %g, %g]", aBody(1), aBody(2), aBody(3)), pixW - 16 * fScaling, pixY, 0xFFFFFFFF);
  pixY += pixAnimH;

  // Available legs animations
  ct = moLegs.GetAnimsCt();

  for (i = 0; i < ct; i++) {
    const BOOL bThisAnim = (_poseCustom.iLegsAnim == i);
    moLegs.GetAnimInfo(i, ai);

    str.PrintF(bThisAnim ? "%d. %s (%d)" : "%d. %s", i, ai.ai_AnimName, _poseCustom.iLegsFrame);
    pdp->PutTextR(str, pixW - 16 * fScaling, pixY + i * pixAnimH, bThisAnim ? 0xFFFF9FFF : 0x9F3F3FBF);
  }

  // Available body animations
  ct = moBody.GetAnimsCt();

  for (i = 0; i < ct; i++) {
    const BOOL bThisAnim = (_poseCustom.iBodyAnim == i);
    moBody.GetAnimInfo(i, ai);

    str.PrintF(bThisAnim ? "%d. %s (%d)" : "%d. %s", i, ai.ai_AnimName, _poseCustom.iBodyFrame);
    pdp->PutTextR(str, pixW - 144 * fScaling, pixY + i * pixAnimH, bThisAnim ? 0xFFFF9FFF : 0x9F3F3FBF);
  }
};

// [Cecil] TEMP: Custom pose customization
void CustomPoseKeys(void) {
  if (!ocam_bCustomPoses) return;
  static SCamKey keyLegsA1, keyLegsA2, keyBodyA1, keyBodyA2, keyLegsF1, keyLegsF2, keyBodyF1, keyBodyF2;

  if (keyLegsA1(KID_NUM8)) _poseCustom.iLegsAnim = ClampDn(_poseCustom.iLegsAnim - 1L, -1L);
  if (keyLegsA2(KID_NUM2)) _poseCustom.iLegsAnim++;

  if (keyBodyA1(KID_NUM4)) _poseCustom.iBodyAnim = ClampDn(_poseCustom.iBodyAnim - 1L, -1L);
  if (keyBodyA2(KID_NUM6)) _poseCustom.iBodyAnim++;

  if (keyLegsF1(KID_NUM1)) _poseCustom.iLegsFrame = ClampDn(_poseCustom.iLegsFrame - 1L, 0L);
  if (keyLegsF2(KID_NUM3)) _poseCustom.iLegsFrame++;

  if (keyBodyF1(KID_NUM7)) _poseCustom.iBodyFrame = ClampDn(_poseCustom.iBodyFrame - 1L, 0L);
  if (keyBodyF2(KID_NUM9)) _poseCustom.iBodyFrame++;
};

#else

__forceinline void PrintCustomPose(CObserverCamera &, CDrawPort *) {};
__forceinline void CustomPoseKeys(void) {};

#endif // PHOTO_MODE_CUSTOM_POSES

inline SPhotoModePose *GetPhotoModePose(void) {
  const INDEX ct = _aPhotoModePoses.Count();
  if (ct == 0) return NULL;

  // Index 0 is reserved for "current player pose", so return nothing on it; actual poses start with 1
  const INDEX i = CObserverCamera::cam_props.iPose - 1;
  if (i < 0) return NULL;

  return &_aPhotoModePoses[ClampUp(i, ct - 1L)];
};

inline CTString GetPhotoModePoseName(void) {
  const INDEX i = CObserverCamera::cam_props.iPose;
  if (i < 0) return CTString(0, "^cAFAFAF%s^r", TRANS("No pose"));

  SPhotoModePose *pPose = GetPhotoModePose();
  CTString strItem = (pPose != NULL) ? pPose->strName : LOCALIZE("Current player");

  return CTString(0, "^c9FDFFF%d.^r %s", i, strItem.str_String);
};

inline SPhotoModeItem *GetPhotoModeItem(BOOL bRight) {
  const INDEX ct = _aPhotoModeItems.Count();
  if (ct == 0) return NULL;

  // Index 0 is reserved for "no item", so return nothing on it; actual items start with 1
  const INDEX i = (bRight ? CObserverCamera::cam_props.iItemR : CObserverCamera::cam_props.iItemL) - 1;
  if (i < 0) return NULL;

  return &_aPhotoModeItems[ClampUp(i, ct - 1L)];
};

inline CTString GetPhotoModeItemName(BOOL bRight) {
  const INDEX i = (bRight ? CObserverCamera::cam_props.iItemR : CObserverCamera::cam_props.iItemL);
  if (i < 0) return CTString(0, "^cAFAFAF%s^r", TRANS("Current item"));

  SPhotoModeItem *pItem = GetPhotoModeItem(bRight);
  CTString strItem = (pItem != NULL) ? pItem->strName : LOCALIZE("None");

  return CTString(0, "^c9FDFFF%d.^r %s", i, strItem.str_String);
};

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

// Reload photo mode poses
static void ReloadPhotoModePoses(void) {
  CFileList aConfigs;
  ListGameFiles(aConfigs, "Scripts\\ClassicsPatch\\PhotoMode\\Poses\\", "*.ini", 0);

  const INDEX ct = aConfigs.Count();
  _aPhotoModePoses.PopAll();

  INDEX ctLoaded = 0;

  for (INDEX i = 0; i < ct; i++) {
    CIniConfig ini;
    ini.Load_t(aConfigs[i], TRUE);

    const IniSections &map = ini.GetMap();
    IniSections::const_iterator it;

    for (it = map.begin(); it != map.end(); it++) {
      SPhotoModePose &pose = _aPhotoModePoses.Push();
      const char *strPose = it->first.c_str();

      pose.strName = strPose;
      pose.iLegsAnim  = ini.GetIntValue(strPose, "LegsAnim",  -1);
      pose.iLegsFrame = ini.GetIntValue(strPose, "LegsFrame", -1);
      pose.iBodyAnim  = ini.GetIntValue(strPose, "BodyAnim",  -1);
      pose.iBodyFrame = ini.GetIntValue(strPose, "BodyFrame", -1);
      pose.aBody(1) = ini.GetDoubleValue(strPose, "BodyH", 0.0);
      pose.aBody(2) = ini.GetDoubleValue(strPose, "BodyP", 0.0);
      pose.aBody(3) = ini.GetDoubleValue(strPose, "BodyB", 0.0);
      ctLoaded++;
    }
  }

  CPrintF(TRANS("Loaded %d photo mode poses\n"), ctLoaded);

  // Reload pose model
  GetGameAPI()->GetCamera().ResetPhotoModePose(TRUE);
};

static SPhotoModeItem *_pLoadingItem = NULL;

static void SetOffset(CSimpleConfig &cfg, CTString &strValue) {
  CPlacement3D &pl = _pLoadingItem->plOffset;
  strValue.ScanF("%g;%g;%g", &pl.pl_PositionVector(1), &pl.pl_PositionVector(2), &pl.pl_PositionVector(3));
};

static void SetRotation(CSimpleConfig &cfg, CTString &strValue) {
  CPlacement3D &pl = _pLoadingItem->plOffset;
  strValue.ScanF("%g;%g;%g", &pl.pl_OrientationAngle(1), &pl.pl_OrientationAngle(2), &pl.pl_OrientationAngle(3));
};

// Reload photo mode items
static void ReloadPhotoModeItems(void) {
  CFileList aConfigs;
  ListGameFiles(aConfigs, "Scripts\\ClassicsPatch\\PhotoMode\\Items\\", "*.amc", 0);

  const INDEX ct = aConfigs.Count();
  _aPhotoModeItems.PopAll();

  CModelConfig cfg;
  cfg.AddProcessor("Offset", &SetOffset);
  cfg.AddProcessor("Rotate", &SetRotation);

  INDEX ctLoaded = 0;

  for (INDEX i = 0; i < ct; i++) {
    _pLoadingItem = &_aPhotoModeItems.Push();
    cfg._pmo = &_pLoadingItem->mo;

    if (cfg.SetModel(aConfigs[i], _pLoadingItem->strName)) {
      ctLoaded++;
    } else {
      _aPhotoModeItems.Pop();
    }
  }

  CPrintF(TRANS("Loaded %d photo mode items\n"), ctLoaded);

  // Reload pose model
  GetGameAPI()->GetCamera().ResetPhotoModePose(TRUE);
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

static INDEX ocam_kidPosePrev      = KID_MOUSE2;
static INDEX ocam_kidPoseNext      = KID_MOUSE1;
static INDEX ocam_kidItemLeftPrev  = KID_MINUS;
static INDEX ocam_kidItemLeftNext  = KID_EQUALS;
static INDEX ocam_kidItemRightPrev = KID_LBRACKET;
static INDEX ocam_kidItemRightNext = KID_RBRACKET;

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

  _pShell->DeclareSymbol("persistent INDEX ocam_kidPosePrev;",      &ocam_kidPosePrev);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidPoseNext;",      &ocam_kidPoseNext);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidItemLeftPrev;",  &ocam_kidItemLeftPrev);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidItemLeftNext;",  &ocam_kidItemLeftNext);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidItemRightPrev;", &ocam_kidItemRightPrev);
  _pShell->DeclareSymbol("persistent INDEX ocam_kidItemRightNext;", &ocam_kidItemRightNext);

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
  _pShell->DeclareSymbol("user INDEX ocam_iPose;",       &cam_props.iPose);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseItemL;",  &cam_props.iItemL);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseItemR;",  &cam_props.iItemR);

#if PHOTO_MODE_CUSTOM_POSES
  _pShell->DeclareSymbol("user INDEX ocam_bCustomPoses;",   &ocam_bCustomPoses);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseLegsAnim;",  &_poseCustom.iLegsAnim);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseLegsFrame;", &_poseCustom.iLegsFrame);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseBodyAnim;",  &_poseCustom.iBodyAnim);
  _pShell->DeclareSymbol("user INDEX ocam_iPoseBodyFrame;", &_poseCustom.iBodyFrame);
#endif
  _pShell->DeclareSymbol("user FLOAT ocam_fPoseBodyH;",     &_poseCustom.aBody(1));
  _pShell->DeclareSymbol("user FLOAT ocam_fPoseBodyP;",     &_poseCustom.aBody(2));
  _pShell->DeclareSymbol("user FLOAT ocam_fPoseBodyB;",     &_poseCustom.aBody(3));

  _pShell->DeclareSymbol("user void ocam_ReloadPoses(void);", &ReloadPhotoModePoses);
  _pShell->DeclareSymbol("user void ocam_ReloadItems(void);", &ReloadPhotoModeItems);

  // Load poses and items for the first time
  ReloadPhotoModePoses();
  ReloadPhotoModeItems();
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

const CTString _strPoserModel = "ModelsPatch\\PhotoModePoser.mdl";

// Setup pose model in photo mode for the original player appearance
BOOL CObserverCamera::SetupPoseModel(CModelObject *pmoOriginal) {
  // Reset temporary variables
  cam_aOriginalBody = ANGLE3D(0, 0, 0);
  cam_iLastItemL = cam_iLastItemR = -1;

  try {
    // Load the poser base for offsetting the player model
    cam_moPose.SetData_t(_strPoserModel);
    cam_moOriginalBody.SetData(NULL);

    // Copy the original appearance onto a poser attachment
    CAttachmentModelObject *pamo = cam_moPose.AddAttachmentModel(0);
    CModelObject &moLegs = pamo->amo_moModelObject;
    moLegs.Copy(*pmoOriginal);
    moLegs.Synchronize(*pmoOriginal);

    // Cache the original body attachment
    pamo = moLegs.GetAttachmentModel(PLAYER_ATTACHMENT_TORSO);

    if (pamo != NULL) {
      cam_moOriginalBody.Copy(pamo->amo_moModelObject);
      cam_moOriginalBody.Synchronize(pamo->amo_moModelObject);
      cam_aOriginalBody = pamo->amo_plRelative.pl_OrientationAngle;
    }

    // Adjust poser size for the model shadow
    FLOATaabbox3D boxFrame;
    moLegs.GetCurrentFrameBBox(boxFrame);
    cam_moPose.StretchSingleModel(boxFrame.Size());
    return TRUE;

  } catch (char *strError) {
    CPrintF(TRANS("Cannot setup player model for the photo mode: %s\n"), strError);
  }

  return FALSE;
};

// Set new item attachment for a specific hand of the photo mode player body
void CObserverCamera::SetItemForHand(CModelObject &moBody, BOOL bRight) {
  INDEX &iLastItem = (bRight ? cam_iLastItemR : cam_iLastItemL);
  const INDEX iItem = (bRight ? cam_props.iItemR : cam_props.iItemL);

  // Item is already set
  if (iLastItem == iItem) return;
  iLastItem = iItem;

  // Find item attachment
  const INDEX iAttach = (bRight ? BODY_ATTACHMENT_COLT_RIGHT : BODY_ATTACHMENT_COLT_LEFT);
  CAttachmentModelObject *pamo = moBody.GetAttachmentModel(iAttach);

  // No attachment
  if (pamo == NULL) return;

  SPhotoModeItem *pItem = GetPhotoModeItem(bRight);

  // Simply hide whatever item there is right now if no item selected
  if (pItem == NULL) {
    pamo->amo_moModelObject.StretchModel(FLOAT3D(0, 0, 0));
    return;
  }

  // Copy item model and offset it
  pamo->amo_moModelObject.SetData(NULL);
  pamo->amo_moModelObject.Copy(pItem->mo);

  CPlacement3D &pl = pamo->amo_plRelative;
  pl = pItem->plOffset;

  // Mirroring and extra offsetting for the left hand (difference between left and right Colts)
  if (!bRight) {
    pl.pl_PositionVector(1)   = -pl.pl_PositionVector(1);
    pl.pl_OrientationAngle(1) = -pl.pl_OrientationAngle(1);
    pl.pl_OrientationAngle(3) = -pl.pl_OrientationAngle(3);

    pl.pl_PositionVector += FLOAT3D(0.03f, 0.01f, 0.0f);
  }
};

// Update player pose in photo mode
void CObserverCamera::UpdatePose(CModelObject &moLegs, CModelObject &moOriginal) {
  CAnimInfo ai;

  // Currently selected pose
  const SPhotoModePose *pPose = GetPhotoModePose();
  BOOL bCustomAnims = FALSE;

#if PHOTO_MODE_CUSTOM_POSES
  if (ocam_bCustomPoses) {
    bCustomAnims = (_poseCustom.iLegsAnim >= 0 || _poseCustom.iBodyAnim >= 0);
    if (bCustomAnims) pPose = &_poseCustom;
  }
#endif

  // Play specified animation for the legs
  if (pPose != NULL && pPose->iLegsAnim >= 0) {
    // Pause the animation on some frame only if the frame is specified
    ULONG ulFlags = (pPose->iLegsFrame >= 0 ? AOF_PAUSED : AOF_LOOPING);
    moLegs.PlayAnim(pPose->iLegsAnim, ulFlags | AOF_NORESTART);

    if (pPose->iLegsFrame >= 0) {
      moLegs.GetAnimInfo(pPose->iLegsAnim, ai);
      moLegs.SelectFrameInTime(ai.ai_SecsPerFrame * pPose->iLegsFrame);
    }

  // Synchronize animation with the original legs
  } else {
    moLegs.CAnimObject::Synchronize(moOriginal);
  }

  // Get body attachment
  CAttachmentModelObject *pamoBody = moLegs.GetAttachmentModel(PLAYER_ATTACHMENT_TORSO);
  if (pamoBody == NULL) return;

  CModelObject &moBody = pamoBody->amo_moModelObject;
  CAttachmentModelObject *pamoOriginalBody = moOriginal.GetAttachmentModel(PLAYER_ATTACHMENT_TORSO);

  // Set body rotation from the pose
  if (pPose != NULL) {
    pamoBody->amo_plRelative.pl_OrientationAngle = pPose->aBody;

  // Synchronize rotation with the original body
  } else {
    ANGLE3D aBody = (pamoOriginalBody != NULL ? pamoOriginalBody->amo_plRelative.pl_OrientationAngle : cam_aOriginalBody);
    pamoBody->amo_plRelative.pl_OrientationAngle = aBody;
  }

  // Add custom rotation
  if (!bCustomAnims) {
    pamoBody->amo_plRelative.pl_OrientationAngle += _poseCustom.aBody;
  }

  // Restore the original attachments if no items have been specified
  if (cam_props.iItemL < 0 && cam_props.iItemR < 0) {
    // Only if requiring a new change
    if (cam_iLastItemL != -1 || cam_iLastItemR != -1) {
      cam_iLastItemL = cam_iLastItemR = -1;

      if (cam_moOriginalBody.GetData() != NULL) {
        moBody.SetData(NULL);
        moBody.Copy(cam_moOriginalBody);
        moBody.Synchronize(cam_moOriginalBody);
      }
    }

  // Set items for each hand (or remove them)
  } else {
    // If there used to be no items selected
    if (cam_iLastItemL == -1 && cam_iLastItemR == -1) {
      CAttachmentModelObject *pamoItem;

      // Remove all item attachments from the body
      FORDELETELIST(CAttachmentModelObject, amo_lnInMain, moBody.mo_lhAttachments, itamo) {
        pamoItem = itamo;
        const INDEX i = pamoItem->amo_iAttachedPosition;

        if (i > BODY_ATTACHMENT_HEAD && i <= BODY_ATTACHMENT_ITEM) {
          pamoItem->amo_lnInMain.Remove();
          delete pamoItem;
        }
      }

      // Add dummy item attachments to each hand, if possible
      pamoItem = moBody.AddAttachmentModel(BODY_ATTACHMENT_COLT_LEFT);

      if (pamoItem != NULL) {
        pamoItem->amo_moModelObject.SetData_t(_strPoserModel);
        pamoItem->amo_moModelObject.StretchModel(FLOAT3D(0, 0, 0));
      }

      pamoItem = moBody.AddAttachmentModel(BODY_ATTACHMENT_COLT_RIGHT);

      if (pamoItem != NULL) {
        pamoItem->amo_moModelObject.SetData_t(_strPoserModel);
        pamoItem->amo_moModelObject.StretchModel(FLOAT3D(0, 0, 0));
      }
    }

    SetItemForHand(moBody, FALSE);
    SetItemForHand(moBody, TRUE);
  }

  // Play specified animation for the body
  if (pPose != NULL && pPose->iBodyAnim >= 0) {
    // Pause the animation on some frame only if the frame is specified
    ULONG ulFlags = (pPose->iBodyFrame >= 0 ? AOF_PAUSED : AOF_LOOPING);
    moBody.PlayAnim(pPose->iBodyAnim, ulFlags | AOF_NORESTART);

    if (pPose->iBodyFrame >= 0) {
      moBody.GetAnimInfo(pPose->iBodyAnim, ai);
      moBody.SelectFrameInTime(ai.ai_SecsPerFrame * pPose->iBodyFrame);
    }

  // Synchronize animation with the original body
  } else if (pamoOriginalBody != NULL) {
    moBody.CAnimObject::Synchronize(pamoOriginalBody->amo_moModelObject);
  }
};

// Retrieve a photo mode model for some player entity
CModelObject *CObserverCamera::GetPoseModel(CEntity *penPlayer, CModelObject *pmoOriginalAppearance) {
  // Photo mode is inactive or this player isn't focused by the camera in photo mode
  if (!IsActive(FALSE) || cam_penViewer != penPlayer) return pmoOriginalAppearance;

  BOOL bPose = (cam_props.iPose >= 0);

#if PHOTO_MODE_CUSTOM_POSES
  if (ocam_bCustomPoses) {
    bPose |= (_poseCustom.iLegsAnim >= 0 || _poseCustom.iBodyAnim >= 0);
  }
#endif

  // Reset the model if no pose is selected to reload it again next time
  if (!bPose) {
    ResetPhotoModePose(TRUE);
    return pmoOriginalAppearance;
  }

  // Setup the photo mode model for this player entity
  if (cam_penPosingPlayer != penPlayer) {
    cam_penPosingPlayer = penPlayer;

    if (!SetupPoseModel(pmoOriginalAppearance)) {
      // Exit posing mode if the pose model could not be set up
      ResetPhotoModePose(FALSE);
      return pmoOriginalAppearance;
    }
  }

  // Adjust offset and rotation
  CAttachmentModelObject *pamo = cam_moPose.GetAttachmentModel(0);
  pamo->amo_plRelative.pl_OrientationAngle(1) = cam_fPoseRotation;

  // Snap offset to the 0.05m grid and divide it by poser's stretch to cancel it out
  FLOAT3D vOffset = cam_vPoseOffset;
  Snap(vOffset(1), 0.05f);
  Snap(vOffset(2), 0.05f);
  Snap(vOffset(3), 0.05f);
  pamo->amo_plRelative.pl_PositionVector(1) = vOffset(1) / ClampDn(cam_moPose.mo_Stretch(1), 0.001f);
  pamo->amo_plRelative.pl_PositionVector(2) = vOffset(2) / ClampDn(cam_moPose.mo_Stretch(2), 0.001f);
  pamo->amo_plRelative.pl_PositionVector(3) = vOffset(3) / ClampDn(cam_moPose.mo_Stretch(3), 0.001f);

  CModelObject &moLegs = pamo->amo_moModelObject;
  UpdatePose(moLegs, *pmoOriginalAppearance);

  // Adjust poser size for the model shadow
  FLOATaabbox3D boxFrame;
  moLegs.GetCurrentFrameBBox(boxFrame);
  cam_moPose.StretchSingleModel(boxFrame.Size());

  // Return set up photo mode pose
  return &cam_moPose;
};

// Free fly camera control using direct button input
void CObserverCamera::UpdateControls(void) {
  // Toggle the camera itself
  static SCamKey keyToggle;
  if (keyToggle(ocam_kidToggle)) TogglePhotoMode();

  // Camera is disabled
  if (!IsActive()) return;

  // [Cecil] NOTE: Needs to be here only when the camera is active, otherwise it messes with player controls
  _pInput->SetJoyPolling(FALSE);
  _pInput->GetInput(FALSE);

  // Toggle camera info
  static SCamKey keyToggleInfo;
  if (keyToggleInfo(ocam_kidToggleInfo)) cam_props.iShowInfo = ClampDn(cam_props.iShowInfo + 1L, 1L) % 3;

  // Turn left
  const BOOL bBtnBankingL = _pInput->GetButtonState(ocam_kidBankingL);
  static BOOL _bLeft = FALSE;

  if (!_bLeft && bBtnBankingL) cam_ctl.bBankingL = TRUE;
  else
  if (_bLeft && !bBtnBankingL) cam_ctl.bBankingL = FALSE;

  _bLeft = bBtnBankingL;

  // Turn right
  const BOOL bBtnBankingR = _pInput->GetButtonState(ocam_kidBankingR);
  static BOOL _bRight = FALSE;

  if (!_bRight && bBtnBankingR) cam_ctl.bBankingR = TRUE;
  else
  if (_bRight && !bBtnBankingR) cam_ctl.bBankingR = FALSE;

  _bRight = bBtnBankingR;

  // Toggle focus
  static SCamKey keyFocus;
  if (keyFocus(ocam_kidFocus)) cam_ctl.iFocusPlayer = ClampDn(cam_ctl.iFocusPlayer + 1L, 1L) % 3;

  // Toggle grid
  static SCamKey keyGrid;
  if (keyGrid(ocam_kidGrid)) cam_props.bGrid = !cam_props.bGrid;

  // Take snapshot
  static SCamKey keySnap;
  if (keySnap(ocam_kidSnapshot)) cam_ctl.bSnapshot = TRUE;

  // Change screenshot resolution
  static SCamKey keyRes;

  if (keyRes(ocam_kidChangeRes)) {
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

  // Change posing mode
  static SCamKey keyChangeMode;
  if (keyChangeMode(ocam_kidChangeMode)) cam_props.bPosingMode = !cam_props.bPosingMode;

  CustomPoseKeys();

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

  // Player pose customization
  if (!cam_props.bPosingMode) return;

  static SCamKey keyPose1, keyPose2, keyItemL1, keyItemL2, keyItemR1, keyItemR2;

  // Change pose
  if (keyPose1(ocam_kidPosePrev)) cam_props.iPose = Clamp(cam_props.iPose - 1L, -1L, _aPhotoModePoses.Count());
  if (keyPose2(ocam_kidPoseNext)) cam_props.iPose = Clamp(cam_props.iPose + 1L, -1L, _aPhotoModePoses.Count());

  // Change left item
  if (keyItemL1(ocam_kidItemLeftPrev)) cam_props.iItemL = Clamp(cam_props.iItemL - 1L, -1L, _aPhotoModeItems.Count());
  if (keyItemL2(ocam_kidItemLeftNext)) cam_props.iItemL = Clamp(cam_props.iItemL + 1L, -1L, _aPhotoModeItems.Count());

  // Change right item
  if (keyItemR1(ocam_kidItemRightPrev)) cam_props.iItemR = Clamp(cam_props.iItemR - 1L, -1L, _aPhotoModeItems.Count());
  if (keyItemR2(ocam_kidItemRightNext)) cam_props.iItemR = Clamp(cam_props.iItemR + 1L, -1L, _aPhotoModeItems.Count());
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

  PrintCustomPose(*this, pdp);

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

      PrintLine(pdp, CTString(0, TRANS("Select player pose: %s / %s"), CameraButton(ocam_kidPosePrev), CameraButton(ocam_kidPoseNext)), pixInfoY, "");
      PrintLine(pdp, CTString(0, TRANS("Select left hand item: %s / %s"), CameraButton(ocam_kidItemLeftPrev), CameraButton(ocam_kidItemLeftNext)), pixInfoY, "");
      PrintLine(pdp, CTString(0, TRANS("Select right hand item: %s / %s"), CameraButton(ocam_kidItemRightPrev), CameraButton(ocam_kidItemRightNext)), pixInfoY, "");
      pixInfoY += GetFontHeight(pdp);

      PrintLine(pdp, TRANS("Selected pose") + CTString(": ") + GetPhotoModePoseName(), pixInfoY, "ocam_iPose");
      PrintLine(pdp, TRANS("Left hand")  + CTString(": ") + GetPhotoModeItemName(FALSE), pixInfoY, "ocam_iPoseItemL");
      PrintLine(pdp, TRANS("Right hand") + CTString(": ") + GetPhotoModeItemName(TRUE),  pixInfoY, "ocam_iPoseItemR");

      if (_poseCustom.aBody != ANGLE3D(0, 0, 0)) {
        const ANGLE3D &aBody = _poseCustom.aBody;
        PrintLine(pdp, TRANS("Custom body rotation") + CTString(0, ": ^c9FDFFF%.1f^C; ^c9FDFFF%.1f^C; ^c9FDFFF%.1f", aBody(1), aBody(2), aBody(3)), pixInfoY, "ocam_fPoseBodyH/P/B");
      }

      if (cam_props.iPose >= 0) {
        pixInfoY += GetFontHeight(pdp);
        PrintLine(pdp, TRANS("Use movement keys to offset the player model"), pixInfoY, "");
        PrintLine(pdp, TRANS("Use tilt keys to rotate the player model"), pixInfoY, "");
      }
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

  if (cam_props.iShowInfo >= 0) {
    PrintCameraInfo(pdp);
  }

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
