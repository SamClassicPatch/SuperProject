/* Copyright (c) 2023-2025 Dreamy Cecil
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

#include "Cecil/SimpleConfigs.h"

enum CompModelParticles {
  E_PRT_NONE,
  E_PRT_AIR,
  E_PRT_FIRE,
};

static CModelObject _moModel;
static CModelObject _moFloor;
static CPlacement3D _plModel;
static ANGLE3D _aRotation;
static FLOAT _fFloorY = 0;
static FLOAT _fFOV = 90;
static FLOAT3D _vLightDir = FLOAT3D(-0.2f, -0.2f, -0.2f);
static COLOR _colLight = C_GRAY;
static COLOR _colAmbient = C_vdGRAY;
static CompModelParticles _eParticles = E_PRT_NONE;

static void SetPlacement(CSimpleConfig &cfg, CTString &strValue) {
  FLOAT3D &vPos = _plModel.pl_PositionVector;
  ANGLE3D &aRot = _plModel.pl_OrientationAngle;
  strValue.ScanF("%g;%g;%g;%g;%g;%g", &vPos(1), &vPos(2), &vPos(3), &aRot(1), &aRot(2), &aRot(3));
};

static void SetRotation(CSimpleConfig &cfg, CTString &strValue) {
  strValue.ScanF("%g;%g;%g", &_aRotation(1), &_aRotation(2), &_aRotation(3));
};

static void SetFloor(CSimpleConfig &cfg, CTString &strValue) {
  strValue.ScanF("%g", &_fFloorY);
};

static void SetFOV(CSimpleConfig &cfg, CTString &strValue) {
  strValue.ScanF("%g", &_fFOV);
};

static void SetLightDir(CSimpleConfig &cfg, CTString &strValue) {
  strValue.ScanF("%g;%g;%g", &_vLightDir(1), &_vLightDir(2), &_vLightDir(3));
};

static void SetColors(CSimpleConfig &cfg, CTString &strValue) {
  strValue.ScanF("%i; %i", &_colLight, &_colAmbient);
};

static void SetParticles(CSimpleConfig &cfg, CTString &strValue) {
  strValue.TrimSpacesRight();

  if (strValue == "air") {
    _eParticles = E_PRT_AIR;
  } else if (strValue == "fire") {
    _eParticles = E_PRT_FIRE;
  }
};

static void SetupCompModel_t(const CTString &strName)
{
  CModelObject *pmo = &_moModel;
  pmo->mo_colBlendColor = 0xFFFFFFFF;
  pmo->StretchModel(FLOAT3D(1, 1, 1));
  pmo->SetData(NULL);

  _aRotation = ANGLE3D(0, 0, 0);
  _fFloorY = 0;
  _fFOV = 90;
  _vLightDir = FLOAT3D(-0.2f, -0.2f, -0.2f);
  _colLight = C_GRAY;
  _colAmbient = C_vdGRAY;
  _eParticles = E_PRT_NONE;

  // White computer floor that can be colorized
  _moFloor.SetData_t(CTFILENAME("ModelsPatch\\Computer\\FloorWhite.mdl"));
  _moFloor.mo_toTexture.SetData_t(CTFILENAME("ModelsPatch\\Computer\\FloorWhite.tex"));

  static CModelConfig cfg;
  cfg._pmo = pmo;

  // Initial setup
  static BOOL bSetupConfigReader = TRUE;

  if (bSetupConfigReader) {
    bSetupConfigReader = FALSE;

    cfg.AddProcessor("CompPlace", &SetPlacement);
    cfg.AddProcessor("CompRotate", &SetRotation);
    cfg.AddProcessor("CompFloor", &SetFloor);
    cfg.AddProcessor("CompFOV", &SetFOV);
    cfg.AddProcessor("CompLight", &SetLightDir);
    cfg.AddProcessor("CompColors", &SetColors);
    cfg.AddProcessor("CompParticles", &SetParticles);
  }

  static CTString strDummy;
  CModelConfig::SetModel(cfg, "Models\\CompConfigs\\" + strName + ".amc", strDummy);
};

void RenderMessageModel(CDrawPort *pdp, const CTString &strModel)
{
  static CTString strLastModel = "";
  static BOOL bModelLoaded = FALSE;

  if (strLastModel != strModel) {
    strLastModel = strModel;
    bModelLoaded = FALSE;

    try {
      SetupCompModel_t(strModel);
      bModelLoaded = TRUE;

    } catch (char *strError) {
      CPrintF(TRANS("Cannot setup model '%s':\n%s\n"), strModel.str_String, strError);
      return;
    }
  }

  if (!bModelLoaded || _moModel.GetData() == NULL) return;

  // Colorize the floor (for some reason '_moFloor.mo_colBlendColor' also colorizes the computer model)
  _moFloor.GetData()->md_colDiffuse = _gmtTheme.colCompFloor | 255;

#if SE1_VER >= SE1_107
  for (INDEX iEye = STEREO_LEFT; iEye <= (Stereo_IsEnabled() ? STEREO_RIGHT : STEREO_LEFT); iEye++)
#endif
  {
    CPerspectiveProjection3D pr;
    pr.FOVL() = _fFOV;
    pr.ScreenBBoxL() = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(pdp->GetWidth(), pdp->GetHeight()));
    pr.AspectRatioL() = 1;
    pr.FrontClipDistanceL() = 0.3f;
    pr.ViewerPlacementL() = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

  #if SE1_VER >= SE1_107
    Stereo_SetBuffer(iEye);
    Stereo_AdjustProjection(pr, iEye, 0.16f);

    pdp->FillZBuffer(ZBUF_BACK);
  #endif

    // Initialize rendering
    CAnyProjection3D apr;
    apr = pr;
    BeginModelRenderingView(apr, pdp);

    extern FLOAT _fMsgAppearFade;
    const FLOAT fDistance = 1 + 10 * (1.0f / (_fMsgAppearFade + 0.01f) - 1.0f / 1.01f);

    CRenderModel rm;
    rm.rm_vLightDirection = _vLightDir;

    // Floor position
    CPlacement3D pl = _plModel;
    pl.pl_OrientationAngle = ANGLE3D(0, 0, 0);
    pl.pl_PositionVector = _plModel.pl_PositionVector;
    pl.pl_PositionVector(2) += _fFloorY;
    pl.pl_PositionVector(3) *= fDistance;
    rm.SetObjectPlacement(pl);

    // Render the floor
    rm.rm_colLight   = C_WHITE;
    rm.rm_colAmbient = C_WHITE;
    rm.rm_fDistanceFactor = -999;

    _moFloor.SetupModelRendering(rm);
    _moFloor.RenderModel(rm);

    // Model position
    pl.pl_OrientationAngle = _plModel.pl_OrientationAngle + _aRotation*_pTimer->GetLerpedCurrentTick();
    pl.pl_PositionVector = _plModel.pl_PositionVector;
    pl.pl_PositionVector(3) *= fDistance / pdp->dp_fWideAdjustment;
    rm.SetObjectPlacement(pl);

    // Render the model
    rm.rm_colLight = _colLight;
    rm.rm_colAmbient = _colAmbient;
    rm.rm_fDistanceFactor = -999;
    _moModel.SetupModelRendering(rm);

    FLOATplane3D plFloorPlane = FLOATplane3D(FLOAT3D(0, 1, 0), _plModel.pl_PositionVector(2) + _fFloorY);
    CPlacement3D plLightPlacement = CPlacement3D(_plModel.pl_PositionVector
      + rm.rm_vLightDirection * _plModel.pl_PositionVector(3) * 5, ANGLE3D(0, 0, 0));

    _moModel.RenderShadow(rm, plLightPlacement, 200, 200, 1, plFloorPlane);
    _moModel.RenderModel(rm);

  #if SE1_GAME != SS_TFE
    if (_eParticles != E_PRT_NONE) {
      Particle_PrepareSystem(pdp, apr);
      Particle_PrepareEntity(1, FALSE, FALSE, NULL);

      switch (_eParticles) {
        case E_PRT_AIR: Particles_AirElemental_Comp(&_moModel, 1, 1, pl); break;
        case E_PRT_FIRE: Particles_Burning_Comp(&_moModel, 0.25f, pl); break;
      }

      Particle_EndSystem();
    }
  #endif

    EndModelRenderingView();
  }

#if SE1_VER >= SE1_107
  Stereo_SetBuffer(STEREO_BOTH);
#endif
};
