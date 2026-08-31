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

403
%{
#include "StdH.h"
#include "Players/Player.h"
#include "Players/PlayerWeapons.h"
%}


enum ViewType {
  0 VT_PLAYERDEATH    "",   // player death
  1 VT_PLAYERREBIRTH  "",   // player rebirth (player is spawned)
  2 VT_CAMERA         "",   // camera view
  3 VT_3RDPERSONVIEW  "",   // 3rd person view
};

// input parameter for viewer
event EViewInit {
  CEntityPointer penOwner,        // who owns it
  CEntityPointer penCamera,       // first camera for camera view
  enum ViewType vtView,           // view type
  BOOL bDeathFixed,
};

%{

void CPlayerView_Precache(void) 
{
  CDLLEntityClass *pdec = &CPlayerView_DLLClass;
  pdec->PrecacheModel(MODEL_MARKER);
  pdec->PrecacheTexture(TEXTURE_MARKER);
}

%}

class export CPlayerView : CMovableEntity {
name      "Player View";
thumbnail "";
features "CanBePredictable";

properties:
  1 CEntityPointer m_penOwner,            // class which owns it
  2 INDEX m_iViewType=0,                  // view type
  3 FLOAT m_fDistance = 1.0f,             // current distance
  4 FLOAT3D m_vZLast = FLOAT3D(0,0,0), 
  5 FLOAT3D m_vTargetLast = FLOAT3D(0,0,0), 
  6 BOOL m_bFixed = FALSE,  // fixed view (player falling in abyss)

components:
  1 model   MODEL_MARKER     "Models\\Editor\\Axis.mdl",
  2 texture TEXTURE_MARKER   "Models\\Editor\\Vector.tex"

functions:
  // add to prediction any entities that this entity depends on
  void AddDependentsToPrediction(void)
  {
    m_penOwner->AddToPrediction();
  }
  void PreMoving() {};
  void DoMoving() {
    en_plLastPlacement = GetPlacement();  // remember old placement for lerping
  };
  void PostMoving() 
  {
    SetCameraPosition();
  }
  CPlacement3D GetLerpedPlacement(void) const
  {
    FLOAT fLerpFactor;

    // [Cecil] Don't use second lerp factor for local prediction
    if (IsPredictor() || DisableLocalPrediction(m_penOwner)) {
      fLerpFactor = _pTimer->GetLerpFactor();
    } else {
      fLerpFactor = _pTimer->GetLerpFactor2();
    }
    return LerpPlacementsPrecise(en_plLastPlacement, en_plPlacement, fLerpFactor);
    //return CMovableEntity::GetLerpedPlacement();
  }

  // render particles
  void RenderParticles(void)
  {
    if (Particle_GetViewer()==this) {
      Particles_ViewerLocal(this);
    }
  }

  // [Cecil] Separate function for setting camera position for this entity (akin to vanilla SetCameraPosition)
  void SetCameraPosition(void) {
    // Prepare data for the calculation of the physical position
    CPlayer *penPlayer = (CPlayer *)(CEntity *)m_penOwner;
    CPlacement3D plPlayer = penPlayer->GetPlacement();
    CPlacement3D plView = penPlayer->en_plViewpoint; // Base viewpoint that will be transformed into new camera position
    FLOATmatrix3D m; // Precalculated rotation from plView.pl_OrientationAngle

    // Retreive the calculated view placement and the rotation matrix (as well as setting camera distance)
    m_fDistance = CalcCameraPosition(penPlayer, plPlayer, plView, m);

    SetPlacement_internal(plView, m, TRUE); // TRUE = Try to optimize for small movements
  };

  // [Cecil] Reworked SetCameraPosition() function that can calculate camera position for any purpose
  // Takes player entity with its placement and viewpoint as input and outputs camera placement into the same viewpoint variable
  FLOAT CalcCameraPosition(CPlayer *penPlayer, const CPlacement3D &plPlayer, CPlacement3D &plView, FLOATmatrix3D &m)
  {
    // [Cecil] Origin view placement from the player eyes
    CPlacement3D plEyesView = plView;
    plEyesView.RelativeToAbsoluteSmooth(plPlayer);

    // 3rd person view
    FLOAT fDistance = 1.0f;
    CPlacement3D &pl = plView; // [Cecil] Base view placement on the passed placement
    BOOL bFollowCrossHair;

    if (m_iViewType == VT_3RDPERSONVIEW) {
      // little above player eyes so it can be seen where he is firing
      pl.pl_OrientationAngle(2) -= 12.0f; //10.0f;
      pl.pl_PositionVector(2) += 1.0f;
      fDistance = 4.2f;//5.75f;
      bFollowCrossHair = TRUE;
    // death
    } else if (m_iViewType == VT_PLAYERDEATH) {
      fDistance = 3.5f;
      bFollowCrossHair = FALSE;
    }

    pl.pl_OrientationAngle(3) = 0.0f;

    // transform rotation angle
    pl.RelativeToAbsoluteSmooth(plPlayer);
    // make base placement to back out from
    FLOAT3D vBase;
    EntityInfo *pei= (EntityInfo *)penPlayer->GetEntityInfo();
    GetEntityInfoPosition(plPlayer, pei->vSourceCenter, vBase); // [Cecil] Based on the placement

    // create a set of rays to test
    MakeRotationMatrixFast(m, pl.pl_OrientationAngle);
    FLOAT3D vRight = m.GetColumn(1);
    FLOAT3D vUp    = m.GetColumn(2);
    FLOAT3D vFront = m.GetColumn(3);

    FLOAT3D vDest[5];
    vDest[0] = vBase+vFront*fDistance+vUp*1.0f;
    vDest[1] = vBase+vFront*fDistance-vUp*1.0f;
    vDest[2] = vBase+vFront*fDistance+vRight*1.0f;
    vDest[3] = vBase+vFront*fDistance-vRight*1.0f;
    vDest[4] = vBase+vFront*fDistance;

    FLOAT fBack = 0;
    // for each ray
    for (INDEX i=0; i<5; i++) {
      // cast a ray to find if any brush is hit
      CCastRay crRay(penPlayer, vBase, vDest[i]);
      crRay.cr_bHitTranslucentPortals = FALSE;
      crRay.cr_ttHitModels = CCastRay::TT_COLLISIONBOX;
      GetWorld()->CastRay(crRay);

      // if hit something
      if (crRay.cr_penHit!=NULL) {
        // clamp distance
        fDistance = Min(fDistance, crRay.cr_fHitDistance-0.5f);
        // if hit polygon
        if (crRay.cr_pbpoBrushPolygon!=NULL) {
          // back off
          FLOAT3D vDir = (vDest[i]-vBase).Normalize();
          FLOAT fD = Abs(FLOAT3D(crRay.cr_pbpoBrushPolygon->bpo_pbplPlane->bpl_plAbsolute)%vDir)*0.25f;
          fBack = Max(fBack, fD);
        }
      }
    }

    fDistance = ClampDn(fDistance - fBack, 0.0f);
    vBase += vFront * fDistance;

    // [Cecil] Don't follow the crosshair and slightly tilt the camera upwards instead
    if (plr_bStaticThirdPersonAngle) {
      bFollowCrossHair = FALSE;

      FLOATmatrix3D mRot, mAdd;
      MakeRotationMatrixFast(mRot, pl.pl_OrientationAngle);
      MakeRotationMatrixFast(mAdd, ANGLE3D(0, 10, 0));
      DecomposeRotationMatrixNoSnap(pl.pl_OrientationAngle, mRot * mAdd);
    }

    CPlayerWeapons *ppw = penPlayer->GetPlayerWeapons();

    if (bFollowCrossHair) {
      // [Cecil] Retrieve real ray hit position instead of using precalculated physics-based m_vRayHit
      FLOAT3D vTarget = vBase - ppw->CalcTargetPosition(plEyesView);
      FLOAT fLen = vTarget.Length();
      if (fLen>0.01) {
        vTarget/=fLen;
      } else {
        vTarget = FLOAT3D(0,1,0);
      }

      FLOAT3D vX;
      FLOAT3D vY = m.GetColumn(2);
      FLOAT3D vZ = vTarget;
      vZ.Normalize();

      if (Abs(vY%vZ)>0.9f) {
        vY = -m.GetColumn(3);
      }

      vX = vY*vZ;
      vX.Normalize();
      vY = vZ*vX;
      vY.Normalize();
      m_vZLast = vZ;

      m(1,1) = vX(1); m(1,2) = vY(1); m(1,3) = vZ(1);
      m(2,1) = vX(2); m(2,2) = vY(2); m(2,3) = vZ(2);
      m(3,1) = vX(3); m(3,2) = vY(3); m(3,3) = vZ(3);
      DecomposeRotationMatrixNoSnap(pl.pl_OrientationAngle, m);
    }

    if (m_bFixed) {
      pl.pl_PositionVector = GetPlacement().pl_PositionVector;
      pl.pl_OrientationAngle = ANGLE3D(0,-90, 0);
      fDistance = (pl.pl_PositionVector - plPlayer.pl_PositionVector).Length();
      MakeRotationMatrixFast(m, pl.pl_OrientationAngle);
    } else {
      pl.pl_PositionVector = vBase;
    }

    // [Cecil] Return calculated distance
    return fDistance;
  };

procedures:

  Main(EViewInit eInit) {
    // remember the initial parameters
    ASSERT(eInit.penOwner!=NULL);
    m_penOwner = eInit.penOwner;
    m_iViewType = eInit.vtView;
    m_bFixed = eInit.bDeathFixed;
    ASSERT(IsOfClass(m_penOwner, "Player"));

    // init as model
    InitAsEditorModel();
    SetFlags(GetFlags()|ENF_CROSSESLEVELS);
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL|EPF_MOVABLE);
    SetCollisionFlags(ECF_IMMATERIAL);
    // set appearance
    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);

    // add to movers list if needed
    if (m_iViewType == VT_PLAYERDEATH) {
      AddToMovers();
    }

    SendEvent(EStart());
    wait() {
      on (EBegin) : { resume; }
      on (EStart) : {  
        SetCameraPosition();
        en_plLastPlacement = GetPlacement();  // remember old placement for lerping
        m_vTargetLast = ((CPlayer&) *m_penOwner).GetPlayerWeapons()->m_vRayHit;
        resume;
      };
      on (EEnd) : { stop; }
      otherwise() : { resume; }
    }
    // cease to exist
    Destroy();

    return;
  };
};

