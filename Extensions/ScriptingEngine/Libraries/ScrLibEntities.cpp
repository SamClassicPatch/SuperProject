/* Copyright (c) 2025-2026 Dreamy Cecil
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

#include <Core/Networking/ExtPackets.h>
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

// Make sure the client is currently running a server
#define ASSERT_SERVER { if (!_pNetwork->IsServer()) return sq_throwerror(v, "cannot send entity packets while not hosting a game"); }

namespace sq {

// CEntity class methods
namespace SqEntity {

// Make sure the entity in the pointer exists
#define ASSERT_ENTITY { if (val == NULL) return sq_throwerror(v, "CEntityPointer is NULL"); }

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CEntityPointer &val) {
  if (ctArgs > 0) {
    GetInstanceValueVerify(CEntityPointer, pOther, v, 2);
    if (pOther != NULL) val = *pOther;
  }

  return 0;
};

static SQInteger ToString(HSQUIRRELVM v, CEntityPointer &val) {
  if (val == NULL) {
    sq_pushstring(v, "(none)", -1);
    return 1;
  }

  CTString strEntity(0, "(%s : %u)", val->GetClass()->ec_pdecDLLClass->dec_strName, val->en_ulID);
  sq_pushstring(v, strEntity.str_String, -1);
  return 1;
};

static SQInteger Equal(HSQUIRRELVM v, int, CEntityPointer &val) {
  // Compare against null
  if (sq_gettype(v, 2) == OT_NULL) {
    sq_pushbool(v, val == NULL);
    return 1;
  }

  // Compare against another pointer
  GetInstanceValueVerify(CEntityPointer, pOther, v, 2);
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger GetID(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->en_ulID);
  return 1;
};

static SQInteger GetRenderType(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetRenderType());
  return 1;
};

static SQInteger GetFlags(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetFlags());
  return 1;
};

static SQInteger GetSpawnFlags(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetSpawnFlags());
  return 1;
};

static SQInteger GetPhysicsFlags(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetPhysicsFlags());
  return 1;
};

static SQInteger GetCollisionFlags(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetCollisionFlags());
  return 1;
};

static SQInteger IsPredictor(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredictor());
  return 1;
};

static SQInteger IsPredicted(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredicted());
  return 1;
};

static SQInteger IsPredictable(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredictable());
  return 1;
};

static SQInteger GetPredictor(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val->GetPredictor();
  return 1;
};

static SQInteger GetPredicted(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val->GetPredicted();
  return 1;
};

static SQInteger GetSpatialClassificationRadius(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushfloat(v, val->en_fSpatialClassificationRadius);
  return 1;
};

static SQInteger GetSpatialClassificationBox(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
  *pbox = val->en_boxSpatialClassification;
  return 1;
};

static SQInteger GetBoundingBox(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
  val->GetBoundingBox(*pbox);
  return 1;
};

static SQInteger GetSize(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
  val->GetSize(*pbox);
  return 1;
};

static SQInteger GetEntityPointRatio(HSQUIRRELVM v, int ctArgs, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get point ratio
  GetInstanceValueVerify(FLOAT3D, pvRatio, v, 2);

  // Get optional lerped flag
  SQBool bLerped = false;
  if (ctArgs > 1) sq_getbool(v, 3, &bLerped);

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  val->GetEntityPointRatio(*pvRatio, *pv, bLerped);
  return 1;
};

static SQInteger GetEntityPointFixed(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get fixed point
  GetInstanceValueVerify(FLOAT3D, pvFixed, v, 2);

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  val->GetEntityPointFixed(*pvFixed, *pv);
  return 1;
};

static SQInteger GetPlacement(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = val->GetPlacement();
  return 1;
};

static SQInteger GetLerpedPlacement(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = val->GetLerpedPlacement();
  return 1;
};

static SQInteger GetRotationMatrix(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(FLOATmatrix3D, pm, GetVMClass(v).Root(), "FLOATmatrix3D");
  *pm = val->GetRotationMatrix();
  return 1;
};

static SQInteger GetViewpoint(HSQUIRRELVM v, int ctArgs, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Make sure it's a player entity
  if (!IsDerivedFromID(val, CPlayerEntity_ClassID)) return sq_throwerror(v, "cannot retrieve a viewpoint from a non-player entity");
  CPlayerEntity *penPlayer = (CPlayerEntity *)(CEntity *)val;

  // Get optional lerped flag
  SQBool bLerped = false;
  if (ctArgs > 0) sq_getbool(v, 2, &bLerped);

  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = IWorld::GetViewpoint(penPlayer, bLerped);
  return 1;
};

static SQInteger GetPlacementRelativeToParent(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = val->en_plRelativeToParent;
  return 1;
};

static SQInteger GetParent(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val->GetParent();
  return 1;
};

static SQInteger GetChildOfClass(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  const SQChar *str;
  sq_getstring(v, 2, &str);

  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val->GetChildOfClass(str);
  return 1;
};

static SQInteger GetName(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetName().str_String, -1);
  return 1;
};

static SQInteger GetDescription(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetDescription().str_String, -1);
  return 1;
};

static SQInteger GetTarget(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val->GetTarget();
  return 1;
};

static SQInteger GetClassificationBoxStretch(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  *pv = val->GetClassificationBoxStretch();
  return 1;
};

static SQInteger GetHealth(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  if (!::IsLiveEntity(val)) return sq_throwerror(v, "CEntityPointer does not contain a live entity");

  CLiveEntity *penLive = (CLiveEntity *)(CEntity *)val;
  sq_pushfloat(v, penLive->GetHealth());
  return 1;
};

// Make sure the entity in the pointer is derived from CMovableEntity
#define ASSERT_MOVABLE { if (!::IsDerivedFromID(val, CMovableEntity_ClassID)) return sq_throwerror(v, "CEntityPointer does not contain a movable entity"); }

static SQInteger GetDesiredTranslationRelative(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_vDesiredTranslationRelative;
  return 1;
};

static SQInteger GetDesiredRotationRelative(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_aDesiredRotationRelative;
  return 1;
};

static SQInteger GetCurrentTranslationAbsolute(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_vCurrentTranslationAbsolute;
  return 1;
};

static SQInteger GetCurrentRotationAbsolute(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_aCurrentRotationAbsolute;
  return 1;
};

static SQInteger GetReference(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *ppen = penMovable->en_penReference;
  return 1;
};

static SQInteger GetReferencePlane(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_vReferencePlane;
  return 1;
};

static SQInteger GetReferenceSurface(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushinteger(v, penMovable->en_iReferenceSurface);
  return 1;
};

static SQInteger GetLastValidReference(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *ppen = penMovable->en_penLastValidReference;
  return 1;
};

static SQInteger GetLastSignificantVerticalMovement(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmLastSignificantVerticalMovement);
  return 1;
};

static SQInteger GetLastBreathed(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmLastBreathed);
  return 1;
};

static SQInteger GetMaxHoldBreath(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmMaxHoldBreath);
  return 1;
};

static SQInteger GetDensity(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fDensity);
  return 1;
};

static SQInteger GetLastSwimDamage(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmLastSwimDamage);
  return 1;
};

static SQInteger GetUpContent(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushinteger(v, penMovable->en_iUpContent);
  return 1;
};

static SQInteger GetDnContent(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushinteger(v, penMovable->en_iDnContent);
  return 1;
};

static SQInteger GetImmersionFactor(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fImmersionFactor);
  return 1;
};

static SQInteger GetGravityDir(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_vGravityDir;
  return 1;
};

static SQInteger GetGravityA(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fGravityA);
  return 1;
};

static SQInteger GetGravityV(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fGravityV);
  return 1;
};

static SQInteger GetForceDir(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pv = penMovable->en_vForceDir;
  return 1;
};

static SQInteger GetForceA(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fForceA);
  return 1;
};

static SQInteger GetForceV(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fForceV);
  return 1;
};

static SQInteger GetJumped(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmJumped);
  return 1;
};

static SQInteger GetMaxJumpControl(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_tmMaxJumpControl);
  return 1;
};

static SQInteger GetJumpControlMultiplier(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fJumpControlMultiplier);
  return 1;
};

static SQInteger GetAcceleration(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fAcceleration);
  return 1;
};

static SQInteger GetDeceleration(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fDeceleration);
  return 1;
};

static SQInteger GetStepUpHeight(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fStepUpHeight);
  return 1;
};

static SQInteger GetStepDnHeight(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fStepDnHeight);
  return 1;
};

static SQInteger GetBounceDampParallel(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fBounceDampParallel);
  return 1;
};

static SQInteger GetBounceDampNormal(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fBounceDampNormal);
  return 1;
};

static SQInteger GetCollisionSpeedLimit(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fCollisionSpeedLimit);
  return 1;
};

static SQInteger GetCollisionDamageFactor(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->en_fCollisionDamageFactor);
  return 1;
};

static SQInteger GetLastPlacement(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *ppl = penMovable->en_plLastPlacement;
  return 1;
};

static SQInteger GetPolygonUnderneath(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(CBrushPolygon *, pbpol, GetVMClass(v).Root(), "CBrushPolygon");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *pbpol = penMovable->en_pbpoStandOn;
  return 1;
};

static SQInteger GetRelativeHeading(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->GetRelativeHeading(*pvDir));
  return 1;
};

static SQInteger GetRelativePitch(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushfloat(v, penMovable->GetRelativePitch(*pvDir));
  return 1;
};

static SQInteger GetReferenceHeadingDirection(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;

  GetInstanceValueVerify(FLOAT3D, pvRef, v, 2);

  SQFloat fH;
  sq_getfloat(v, 3, &fH);

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  penMovable->GetReferenceHeadingDirection(*pvRef, fH, *pv);
  return 1;
};

static SQInteger GetHeadingDirection(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;

  SQFloat fH;
  sq_getfloat(v, 2, &fH);

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  penMovable->GetHeadingDirection(fH, *pv);
  return 1;
};

static SQInteger GetPitchDirection(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;

  SQFloat fH;
  sq_getfloat(v, 2, &fH);

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  penMovable->GetPitchDirection(fH, *pv);
  return 1;
};

static SQInteger MiscDamageInflictor(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  *ppen = penMovable->MiscDamageInflictor();
  return 1;
};

static SQInteger IsStandingOnPolygon(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;

  GetInstanceValueVerifyN(CBrushPolygon *, pbpol, v, 2, "CBrushPolygon");

  // No polygon
  if (*pbpol == NULL) {
    sq_pushbool(v, false);
    return 1;
  }

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushbool(v, penMovable->IsStandingOnPolygon(*pbpol));
  return 1;
};

static SQInteger IsPolygonBelowPoint(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;

  GetInstanceValueVerifyN(CBrushPolygon *, pbpol, v, 2, "CBrushPolygon");
  GetInstanceValueVerify(FLOAT3D, pvPoint, v, 3);

  SQFloat fMaxDist;
  sq_getfloat(v, 4, &fMaxDist);

  // No polygon
  if (*pbpol == NULL) {
    sq_pushbool(v, false);
    return 1;
  }

  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushbool(v, penMovable->IsPolygonBelowPoint(*pbpol, *pvPoint, fMaxDist));
  return 1;
};

static SQInteger WouldFallInNextPosition(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY; ASSERT_MOVABLE;
  CMovableEntity *penMovable = (CMovableEntity *)(CEntity *)val;
  sq_pushbool(v, penMovable->WouldFallInNextPosition());
  return 1;
};

static SQInteger GetClassFile(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetClass()->GetName().str_String, -1);
  return 1;
};

static SQInteger GetClassName(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetClass()->ec_pdecDLLClass->dec_strName, -1);
  return 1;
};

static SQInteger GetClassID(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetClass()->ec_pdecDLLClass->dec_iID);
  return 1;
};

static SQInteger IsOfClass(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  const SQChar *strClass;
  sq_getstring(v, 2, &strClass);
  sq_pushbool(v, ::IsOfClass(val, strClass));
  return 1;
};

static SQInteger IsDerivedFromClass(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  const SQChar *strClass;
  sq_getstring(v, 2, &strClass);
  sq_pushbool(v, ::IsDerivedFromClass(val, strClass));
  return 1;
};

static SQInteger IsOfClassID(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  SQInteger iClassID;
  sq_getinteger(v, 2, &iClassID);
  sq_pushbool(v, ::IsOfClassID(val, iClassID));
  return 1;
};

static SQInteger IsDerivedFromID(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  SQInteger iClassID;
  sq_getinteger(v, 2, &iClassID);
  sq_pushbool(v, ::IsDerivedFromID(val, iClassID));
  return 1;
};

static SQInteger IsLiveEntity(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, ::IsLiveEntity(val));
  return 1;
};

static SQInteger IsRationalEntity(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, ::IsRationalEntity(val));
  return 1;
};

static SQInteger GetPropertyForId(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property ID
  SQInteger iID;
  sq_getinteger(v, 2, &iID);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForId(val, iID);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForHash(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property hash
  SQInteger iHash;
  sq_getinteger(v, 2, &iHash);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForHash(val, iHash);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForIdOrOffset(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type, ID and offset
  SQInteger iType, iID, iOffset;
  sq_getinteger(v, 2, &iType);
  sq_getinteger(v, 3, &iID);
  sq_getinteger(v, 4, &iOffset);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForIdOrOffset(val, iType, iID, iOffset);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForName(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type and name
  SQInteger iType;
  sq_getinteger(v, 2, &iType);
  const SQChar *strName;
  sq_getstring(v, 3, &strName);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForName(val, iType, strName);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForNameOrId(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type, name and ID
  SQInteger iType, iID;
  sq_getinteger(v, 2, &iType);
  const SQChar *strName;
  sq_getstring(v, 3, &strName);
  sq_getinteger(v, 4, &iID);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForNameOrId(val, iType, strName, iID);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForVariable(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get class and variable names
  const SQChar *strClass;
  const SQChar *strVariable;
  sq_getstring(v, 2, &strClass);
  sq_getstring(v, 3, &strVariable);

  // Make sure the property exists
  CEntityProperty *pepCheck = NULL;

#if SE1_GAME != SS_REV
  // Find property data and try to find it by name or ID
  const CEntityProperty *pepData = FindPropertyByVariable(strClass, strVariable);

  if (pepData != NULL) {
    pepCheck = IWorld::PropertyForNameOrId(val, pepData->ep_eptType, pepData->ep_strName, pepData->ep_ulID);
  }
#else
  pepCheck = val->PropertyForVariable(strVariable);
#endif

  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  PushNewInstance(CEntityProperty *, ppep, sqtEntities, "Property");
  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropValue(HSQUIRRELVM v, int, CEntityPointer &val) {
  ASSERT_ENTITY;
  GetInstanceValueVerifyN(CEntityProperty *, ppep, v, 2, "Entities.Property");

  const CEntityProperty &ep = **ppep;
  CEntity *pen = val;

  switch (ep.ep_eptType) {
    case CEntityProperty::EPT_ENUM:
    case CEntityProperty::EPT_COLOR:
    case CEntityProperty::EPT_INDEX:
    case CEntityProperty::EPT_ANIMATION:
    case CEntityProperty::EPT_ILLUMINATIONTYPE:
    case CEntityProperty::EPT_FLAGS: {
      sq_pushinteger(v, ENTITYPROPERTY(pen, ep.ep_slOffset, INDEX));
    } break;

    case CEntityProperty::EPT_BOOL: {
      sq_pushbool(v, ENTITYPROPERTY(pen, ep.ep_slOffset, BOOL));
    } break;

    case CEntityProperty::EPT_FLOAT:
    case CEntityProperty::EPT_RANGE:
    case CEntityProperty::EPT_ANGLE: {
      sq_pushfloat(v, ENTITYPROPERTY(pen, ep.ep_slOffset, FLOAT));
    } break;

    case CEntityProperty::EPT_STRING:
    case CEntityProperty::EPT_FILENAMENODEP:
    case CEntityProperty::EPT_STRINGTRANS: {
      sq_pushstring(v, ENTITYPROPERTY(pen, ep.ep_slOffset, CTString).str_String, -1);
    } break;

    case CEntityProperty::EPT_FILENAME: {
      sq_pushstring(v, ENTITYPROPERTY(pen, ep.ep_slOffset, CTFileName).str_String, -1);
    } break;

    case CEntityProperty::EPT_ENTITYPTR: {
      PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
      *ppen = ENTITYPROPERTY(pen, ep.ep_slOffset, CEntityPointer);
    } break;

    case CEntityProperty::EPT_FLOATAABBOX3D: {
      PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
      *pbox = ENTITYPROPERTY(pen, ep.ep_slOffset, FLOATaabbox3D);
    } break;

    case CEntityProperty::EPT_FLOAT3D:
    case CEntityProperty::EPT_ANGLE3D: {
      PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
      *pv = ENTITYPROPERTY(pen, ep.ep_slOffset, FLOAT3D);
    } break;

    case CEntityProperty::EPT_FLOATplane3D: {
      PushNewInstance(FLOATplane3D, ppl, GetVMClass(v).Root(), "FLOATplane3D");
      *ppl = ENTITYPROPERTY(pen, ep.ep_slOffset, FLOATplane3D);
    } break;

    case CEntityProperty::EPT_PLACEMENT3D: {
      PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
      *ppl = ENTITYPROPERTY(pen, ep.ep_slOffset, CPlacement3D);
    } break;

    case CEntityProperty::EPT_FLOATMATRIX3D: {
      PushNewInstance(FLOATmatrix3D, pm, GetVMClass(v).Root(), "FLOATmatrix3D");
      *pm = ENTITYPROPERTY(pen, ep.ep_slOffset, FLOATmatrix3D);
    } break;

    // Unsupported types
    case CEntityProperty::EPT_MODELOBJECT:
    case CEntityProperty::EPT_ANIMOBJECT:
    case CEntityProperty::EPT_SOUNDOBJECT:
    case CEntityProperty::EPT_FLOATQUAT3D:
  #if SE1_VER >= SE1_107
    case CEntityProperty::EPT_MODELINSTANCE:
  #endif
    default: {
      SQChar strError[256];
      scsprintf(strError, 256, "cannot retrieve value from the unknown/unsupported property type %d", ep.ep_eptType);
      return sq_throwerror(v, strError);
    }
  }

  return 1;
};

static Method<CEntityPointer> _aMethods[] = {
  { "Equal", &Equal, 2, ".x|o" },

  // Internal
  { "GetID", &GetID, 1, "." },
  { "GetRenderType", &GetRenderType, 1, "." },

  { "GetFlags",          &GetFlags,          1, "." },
  { "GetSpawnFlags",     &GetSpawnFlags,     1, "." },
  { "GetPhysicsFlags",   &GetPhysicsFlags,   1, "." },
  { "GetCollisionFlags", &GetCollisionFlags, 1, "." },

  { "IsPredictor",   &IsPredictor,   1, "." },
  { "IsPredicted",   &IsPredicted,   1, "." },
  { "IsPredictable", &IsPredictable, 1, "." },
  { "GetPredictor",  &GetPredictor,  1, "." },
  { "GetPredicted",  &GetPredicted,  1, "." },

  // Placement
  { "GetSpatialClassificationRadius", &GetSpatialClassificationRadius, 1, "." },
  { "GetSpatialClassificationBox",    &GetSpatialClassificationBox,    1, "." },
  { "GetBoundingBox",      &GetBoundingBox,       1, "." },
  { "GetSize",             &GetSize,              1, "." },
  { "GetEntityPointRatio", &GetEntityPointRatio, -2, ".xb" },
  { "GetEntityPointFixed", &GetEntityPointFixed,  2, ".x" },

  { "GetPlacement",        &GetPlacement,       1, "." },
  { "GetLerpedPlacement",  &GetLerpedPlacement, 1, "." },
  { "GetRotationMatrix",   &GetRotationMatrix,  1, "." },
  { "GetViewpoint",        &GetViewpoint,      -1, ".b" },

  // Parents and children
  { "GetPlacementRelativeToParent", &GetPlacementRelativeToParent, 1, "." },
  { "GetParent",       &GetParent,       1, "." },
  { "GetChildOfClass", &GetChildOfClass, 2, ".s" },

  // Class-specific overloads
  { "GetName",        &GetName,        1, "." },
  { "GetDescription", &GetDescription, 1, "." },
  { "GetTarget",      &GetTarget,      1, "." },
  { "GetClassificationBoxStretch", &GetClassificationBoxStretch, 1, "." },

  // Base class methods
  { "GetHealth",            &GetHealth,            1, "." },
  { "GetLastPlacement",     &GetLastPlacement,     1, "." },
  { "GetPolygonUnderneath", &GetPolygonUnderneath, 1, "." },

  { "GetRelativeHeading",           &GetRelativeHeading,           2, ".x" },
  { "GetRelativePitch",             &GetRelativePitch,             2, ".x" },
  { "GetReferenceHeadingDirection", &GetReferenceHeadingDirection, 3, ".xn" },
  { "GetHeadingDirection",          &GetHeadingDirection,          2, ".n" },
  { "GetPitchDirection",            &GetPitchDirection,            2, ".n" },
  { "MiscDamageInflictor",          &MiscDamageInflictor,          1, "." },
  { "IsStandingOnPolygon",          &IsStandingOnPolygon,          2, ".x" },
  { "IsPolygonBelowPoint",          &IsPolygonBelowPoint,          4, ".xxn" },
  { "WouldFallInNextPosition",      &WouldFallInNextPosition,      1, "." },

  // Entity class data
  { "GetClassFile", &GetClassFile, 1, "." },
  { "GetClassName", &GetClassName, 1, "." },
  { "GetClassID",   &GetClassID,   1, "." },

  { "IsOfClass",          &IsOfClass,          2, ".s" },
  { "IsDerivedFromClass", &IsDerivedFromClass, 2, ".s" },
  { "IsOfClassID",        &IsOfClassID,        2, ".n" },
  { "IsDerivedFromID",    &IsDerivedFromID,    2, ".n" },
  { "IsLiveEntity",       &IsLiveEntity,       1, "." },
  { "IsRationalEntity",   &IsRationalEntity,   1, "." },

  // Property lookup
  { "GetPropertyForId",         &GetPropertyForId,         2, ".n" },
  { "GetPropertyForHash",       &GetPropertyForHash,       2, ".n" },
  { "GetPropertyForIdOrOffset", &GetPropertyForIdOrOffset, 4, ".nnn" },
  { "GetPropertyForName",       &GetPropertyForName,       3, ".ns" },
  { "GetPropertyForNameOrId",   &GetPropertyForNameOrId,   4, ".nsn" },
  { "GetPropertyForVariable",   &GetPropertyForVariable,   3, ".ss" },

  { "GetPropValue", &GetPropValue, 2, ".x" },
};

}; // namespace

// CEntityProperty class methods
namespace SqProp {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CEntityProperty *&val) {
  val = NULL;

  if (ctArgs > 0) {
    GetInstanceValueVerify(CEntityProperty *, pOther, v, 2);
    if (pOther != NULL) val = *pOther;
  }

  return 0;
};

static SQInteger GetPropType(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_eptType);
  return 1;
};

static SQInteger GetPropID(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_ulID);
  return 1;
};

static SQInteger GetPropOffset(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_slOffset);
  return 1;
};

static SQInteger GetPropName(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushstring(v, val->ep_strName, -1);
  return 1;
};

static SQInteger GetPropFlags(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_ulFlags);
  return 1;
};

static SQInteger GetPropShortcut(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_chShortcut);
  return 1;
};

static SQInteger GetPropColor(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_colColor);
  return 1;
};

static SQInteger Equal(HSQUIRRELVM v, int, CEntityProperty *&val) {
  // Compare against null
  if (sq_gettype(v, 2) == OT_NULL) {
    sq_pushbool(v, val == NULL);
    return 1;
  }

  // Compare against another pointer
  GetInstanceValueVerifyN(CEntityProperty *, pOther, v, 2, "Entities.Property");
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger GetEnumValueCount(HSQUIRRELVM v, int, CEntityProperty *&val) {
  if (val->ep_pepetEnumType == NULL) {
    return sq_throwerror(v, "entity property does not point to any enum type");
  }

  sq_pushinteger(v, val->ep_pepetEnumType->epet_ctValues);
  return 1;
};

static SQInteger GetEnumValues(HSQUIRRELVM v, int, CEntityProperty *&val) {
  if (val->ep_pepetEnumType == NULL) {
    return sq_throwerror(v, "entity property does not point to any enum type");
  }

  // Create an array that will contain all enum values
  sq_newarray(v, 0);

  for (INDEX i = 0; i < val->ep_pepetEnumType->epet_ctValues; i++) {
    CEntityPropertyEnumValue &epev = val->ep_pepetEnumType->epet_aepevValues[i];

    // Create an array for each enum value that contains its index and name
    sq_newarray(v, 0);

    sq_pushinteger(v, epev.epev_iValue);
    sq_arrayappend(v, -2);
    sq_pushstring(v, epev.epev_strName, -1);
    sq_arrayappend(v, -2);

    // Append this array to the main one
    sq_arrayappend(v, -2);
  }

  return 1;
};

static Method<CEntityProperty *> _aMethods[] = {
  { "Equal",             &Equal,             2, ".x|o" },
  { "GetEnumValueCount", &GetEnumValueCount, 1, "." },
  { "GetEnumValues",     &GetEnumValues,     1, "." },
};

}; // namespace

// Event class methods
namespace SqEvent {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, EExtEntityEvent &val) {
  if (ctArgs <= 0) return 0;

  // Try copying from another event
  GetInstanceValue(EExtEntityEvent, pOther, v, 2);

  if (pOther != NULL) {
    val.Copy(*pOther);
    return 0;
  }

  // Otherwise try setting up the event with an event code
  SQInteger iEventCode;

  if (SQ_SUCCEEDED(sq_getinteger(v, 2, &iEventCode))) {
    val.ee_slEvent = iEventCode;
    return 0;
  }

  return sq_throwerror(v, "expected CEntityEvent or an event code");
};

static SQInteger SetArgs(HSQUIRRELVM v, int ctArgs, EExtEntityEvent &val) {
  if (ctArgs >= EXT_ENTITY_EVENT_FIELDS) return sq_throwerror(v, "too many arguments for the event");

  // Separate counter for used fields inside the event
  ULONG iField = 0;
  val.Reset(FALSE);

  for (INDEX i = 0; i < ctArgs; i++) {
    // Make sure the vector doesn't exceed the field count
    if (iField >= EXT_ENTITY_EVENT_FIELDS) {
      SQChar strError[256];
      scsprintf(strError, 256, "event argument %d exceeds the event size", i + 1);
      return sq_throwerror(v, strError);
    }

    switch (sq_gettype(v, 2 + i)) {
      case OT_NULL: {
        iField = val.SetInt(iField, 0);
      } break;

      case OT_INTEGER: {
        SQInteger iArg;
        sq_getinteger(v, 2 + i, &iArg);
        iField = val.SetInt(iField, iArg);
      } break;

      case OT_FLOAT: {
        SQFloat fArg;
        sq_getfloat(v, 2 + i, &fArg);
        iField = val.SetFloat(iField, fArg);
      } break;

      case OT_BOOL: {
        SQBool bArg;
        sq_getbool(v, 2 + i, &bArg);
        iField = val.SetInt(iField, bArg);
      } break;

      case OT_STRING: {
        const SQChar *strArg;
        sq_getstring(v, 2 + i, &strArg);
        iField = val.SetString(iField, strArg);
      } break;

      case OT_INSTANCE: {
        // Try vector
        GetInstanceValue(FLOAT3D, pvArg, v, 2 + i);

        if (pvArg != NULL) {
          // Make sure the vector doesn't exceed the field count
          if (iField + 2 >= EXT_ENTITY_EVENT_FIELDS) {
            SQChar strError[256];
            scsprintf(strError, 256, "event argument %d exceeds the event size", i + 1);
            return sq_throwerror(v, strError);
          }

          iField = val.SetVector(iField, *pvArg);
          break;
        }

        // Try entity
        GetInstanceValue(CEntityPointer, ppenArg, v, 2 + i);

        if (ppenArg != NULL) {
          if (*ppenArg != NULL) {
            iField = val.SetEntity(iField, *ppenArg);
          } else {
            iField = val.SetEntity(iField, NULL);
          }
          break;
        }
      }

      default: {
        SQChar strError[256];
        scsprintf(strError, 256, "unsupported argument type for the event argument %d", i + 1);
        return sq_throwerror(v, strError);
      }
    }
  }

  return 0;
};

static Method<EExtEntityEvent> _aMethods[] = {
  { "SetArgs", &SetArgs, -1, "." },
};

}; // namespace

namespace Entities {

#if _PATCHCONFIG_EXT_PACKETS

static SQInteger Create(HSQUIRRELVM v) {
  ASSERT_SERVER;

  const SQChar *strClassFile;
  sq_getstring(v, 2, &strClassFile);

  // Get placement
  GetInstanceValueVerify(CPlacement3D, pplPos, v, 3);

  CExtEntityCreate pck;
  pck("fnmClass", strClassFile);
  pck("plPos", *pplPos);
  pck.SendToClients();
  return 0;
};

static SQInteger Delete(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  SQBool bSameClass;
  sq_getbool(v, 3, &bSameClass);

  CExtEntityDelete pck;
  pck("ulEntity", (int)iEntity);
  pck("bSameClass", !!bSameClass);
  pck.SendToClients();
  return 0;
};

static SQInteger Copy(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iCopies;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iCopies);

  if (iCopies < 0 || iCopies > 31) {
    return sq_throwerror(v, "amount of copies is outside the 0-31 range");
  }

  CExtEntityCopy pck;
  pck("ulEntity", (int)iEntity);
  pck("iCopies", (int)iCopies);
  pck.SendToClients();
  return 0;
};

static SQInteger SendEvent(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  // Get event
  GetInstanceValueVerifyN(EExtEntityEvent, pEvent, v, 3, "CEntityEvent");

  CExtEntityEvent pck;
  pck("ulEntity", (int)iEntity);
  pck.Copy(*pEvent);
  pck.SendToClients();
  return 0;
};

static SQInteger ReceiveItem(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  // Get event
  GetInstanceValueVerifyN(EExtEntityEvent, pEvent, v, 3, "CEntityEvent");

  CExtEntityItem pck;
  pck("ulEntity", (int)iEntity);
  pck.Copy(*pEvent);
  pck.SendToClients();
  return 0;
};

static SQInteger Initialize(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  // Get optional event
  if (sq_gettop(v) > 2) {
    GetInstanceValue(EExtEntityEvent, pEvent, v, 3);

    CExtEntityInit pck;
    pck("ulEntity", (int)iEntity);
    pck.Copy(*pEvent);
    pck.SendToClients();

  } else {
    CExtEntityInit pck;
    pck("ulEntity", (int)iEntity);
    pck.SetEvent(EVoid());
    pck.SendToClients();
  }
  return 0;
};

static SQInteger SetPos(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(FLOAT3D, pvSet, v, 3);

  SQBool bRelative;
  sq_getbool(v, 4, &bRelative);

  CExtEntityPosition pck;
  pck("ulEntity", (int)iEntity);
  pck("vSet", *pvSet);
  pck("bRotation", false);
  pck("bRelative", !!bRelative);
  pck.SendToClients();
  return 0;
};

static SQInteger SetRot(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(FLOAT3D, pvSet, v, 3);

  SQBool bRelative;
  sq_getbool(v, 4, &bRelative);

  CExtEntityPosition pck;
  pck("ulEntity", (int)iEntity);
  pck("vSet", *pvSet);
  pck("bRotation", true);
  pck("bRelative", !!bRelative);
  pck.SendToClients();
  return 0;
};

static SQInteger Teleport(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(CPlacement3D, pplSet, v, 3);

  SQBool bRelative;
  sq_getbool(v, 4, &bRelative);

  CExtEntityTeleport pck;
  pck("ulEntity", (int)iEntity);
  pck("plSet", *pplSet);
  pck("bRelative", !!bRelative);
  pck.SendToClients();
  return 0;
};

static SQInteger Parent(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iParent;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iParent);

  CExtEntityParent pck;
  pck("ulEntity", (int)iEntity);
  pck("ulParent", (int)iParent);
  pck.SendToClients();
  return 0;
};

static SQInteger SetProperty(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  const SQChar *strProp = NULL;
  SQInteger iPropID = -1;

  if (sq_gettype(v, 3) == OT_STRING) {
    sq_getstring(v, 3, &strProp);

  } else if (SQ_FAILED(sq_getinteger(v, 3, &iPropID))) {
    return sq_throwerror(v, "expected property name or property ID in argument 2");
  }

  const SQChar *strValue = NULL;
  SQFloat fValue = 0.0f;

  if (sq_gettype(v, 4) == OT_STRING) {
    sq_getstring(v, 4, &strValue);

  } else if (sq_gettype(v, 4) == OT_BOOL) {
    SQBool bValue;
    sq_getbool(v, 4, &bValue);
    fValue = (bValue ? 1.0f : 0.0f);

  } else if (SQ_FAILED(sq_getfloat(v, 4, &fValue))) {
    return sq_throwerror(v, "expected a string or a number for the property value in argument 3");
  }

  CExtEntityProp pck;
  pck("ulEntity", (int)iEntity);

  // Set property name or ID
  if (strProp != NULL) {
    pck.SetProperty(CTString(strProp));
  } else {
    pck.SetProperty((ULONG)iPropID);
  }

  // Set string or float value
  if (strValue != NULL) {
    pck.SetValue(CTString(strValue));
  } else {
    pck.SetValue((DOUBLE)fValue);
  }

  pck.SendToClients();
  return 0;
};

static SQInteger SetHealth(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  SQFloat fHealth;
  sq_getfloat(v, 3, &fHealth);

  CExtEntityHealth pck;
  pck("ulEntity", (int)iEntity);
  pck("fHealth", (FLOAT)fHealth);
  pck.SendToClients();
  return 0;
};

static SQInteger SetFlags(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iFlags;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iFlags);

  SQBool bRemove;
  sq_getbool(v, 4, &bRemove);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.EntityFlags(iFlags, bRemove);
  pck.SendToClients();
  return 0;
};

static SQInteger SetPhysicsFlags(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iFlags;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iFlags);

  SQBool bRemove;
  sq_getbool(v, 4, &bRemove);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.PhysicalFlags(iFlags, bRemove);
  pck.SendToClients();
  return 0;
};

static SQInteger SetCollisionFlags(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iFlags;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iFlags);

  SQBool bRemove;
  sq_getbool(v, 4, &bRemove);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.CollisionFlags(iFlags, bRemove);
  pck.SendToClients();
  return 0;
};

static SQInteger Move(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(FLOAT3D, pvSpeed, v, 3);

  CExtEntityMove pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", *pvSpeed);
  pck.SendToClients();
  return 0;
};

static SQInteger Rotate(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(FLOAT3D, pvSpeed, v, 3);

  CExtEntityRotate pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", *pvSpeed);
  pck.SendToClients();
  return 0;
};

static SQInteger GiveImpulse(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity;
  sq_getinteger(v, 2, &iEntity);

  GetInstanceValueVerify(FLOAT3D, pvSpeed, v, 3);

  CExtEntityImpulse pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", *pvSpeed);
  pck.SendToClients();
  return 0;
};

static SQInteger InflictDirectDamage(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iTarget, iDamageType;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iTarget);
  sq_getinteger(v, 4, &iDamageType);

  SQFloat fDamage;
  sq_getfloat(v, 5, &fDamage);

  GetInstanceValueVerify(FLOAT3D, pvHit, v, 6);
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 7);

  CExtEntityDirectDamage pck;
  pck("ulEntity", (int)iEntity);
  pck("ulTarget", (int)iTarget);
  pck("eDamageType", (int)iDamageType);
  pck("fDamage", (FLOAT)fDamage);
  pck("vHitPoint", *pvHit);
  pck("vDirection", *pvDir);
  pck.SendToClients();
  return 0;
};

static SQInteger InflictRangeDamage(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iDamageType;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iDamageType);

  SQFloat fDamage, fFallOff, fHotSpot;
  sq_getfloat(v, 4, &fDamage);

  GetInstanceValueVerify(FLOAT3D, pvCenter, v, 5);

  sq_getfloat(v, 6, &fFallOff);
  sq_getfloat(v, 7, &fHotSpot);

  CExtEntityRangeDamage pck;
  pck("ulEntity", (int)iEntity);
  pck("eDamageType", (int)iDamageType);
  pck("fDamage", (FLOAT)fDamage);
  pck("vCenter", *pvCenter);
  pck("fFallOff", (FLOAT)fFallOff);
  pck("fHotSpot", (FLOAT)fHotSpot);
  pck.SendToClients();
  return 0;
};

static SQInteger InflictBoxDamage(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iEntity, iDamageType;
  sq_getinteger(v, 2, &iEntity);
  sq_getinteger(v, 3, &iDamageType);

  SQFloat fDamage;
  sq_getfloat(v, 4, &fDamage);

  GetInstanceValueVerify(FLOATaabbox3D, pboxArea, v, 5);

  CExtEntityBoxDamage pck;
  pck("ulEntity", (int)iEntity);
  pck("eDamageType", (int)iDamageType);
  pck("fDamage", (FLOAT)fDamage);
  pck("boxArea", *pboxArea);
  pck.SendToClients();
  return 0;
};

#else

#define DISABLE_EXT_PACKET_FUNC(_FuncName) \
  static SQInteger _FuncName(HSQUIRRELVM v) { \
    return sq_throwerror(v, "functions that utilize extension packets have been disabled in this build"); \
  };

DISABLE_EXT_PACKET_FUNC(Create);
DISABLE_EXT_PACKET_FUNC(Delete);
DISABLE_EXT_PACKET_FUNC(Copy);
DISABLE_EXT_PACKET_FUNC(SendEvent);
DISABLE_EXT_PACKET_FUNC(ReceiveItem);
DISABLE_EXT_PACKET_FUNC(Initialize);
DISABLE_EXT_PACKET_FUNC(SetPos);
DISABLE_EXT_PACKET_FUNC(SetRot);
DISABLE_EXT_PACKET_FUNC(Teleport);
DISABLE_EXT_PACKET_FUNC(Parent);
DISABLE_EXT_PACKET_FUNC(SetProperty);
DISABLE_EXT_PACKET_FUNC(SetHealth);
DISABLE_EXT_PACKET_FUNC(SetFlags);
DISABLE_EXT_PACKET_FUNC(SetPhysicsFlags);
DISABLE_EXT_PACKET_FUNC(SetCollisionFlags);
DISABLE_EXT_PACKET_FUNC(Move);
DISABLE_EXT_PACKET_FUNC(Rotate);
DISABLE_EXT_PACKET_FUNC(GiveImpulse);
DISABLE_EXT_PACKET_FUNC(InflictDirectDamage);
DISABLE_EXT_PACKET_FUNC(InflictRangeDamage);
DISABLE_EXT_PACKET_FUNC(InflictBoxDamage);

#endif // _PATCHCONFIG_EXT_PACKETS

}; // namespace

// "Entities" namespace functions
static SQRegFunction _aEntitiesFuncs[] = {
  { "Create",              &Entities::Create,              3, ".sx" },
  { "Delete",              &Entities::Delete,              3, ".nn" },
  { "Copy",                &Entities::Copy,                3, ".nn" },
  { "SendEvent",           &Entities::SendEvent,           3, ".nx" },
  { "ReceiveItem",         &Entities::ReceiveItem,         3, ".nx" },
  { "Initialize",          &Entities::Initialize,         -2, ".nx" },
  { "SetPos",              &Entities::SetPos,              4, ".nxb" },
  { "SetRot",              &Entities::SetRot,              4, ".nxb" },
  { "Teleport",            &Entities::Teleport,            4, ".nxb" },
  { "Parent",              &Entities::Parent,              3, ".nn" },
  { "SetProperty",         &Entities::SetProperty,         4, ".ns|ns|n|b" },
  { "SetHealth",           &Entities::SetHealth,           3, ".nn" },
  { "SetFlags",            &Entities::SetFlags,            4, ".nnb" },
  { "SetPhysicsFlags",     &Entities::SetPhysicsFlags,     4, ".nnb" },
  { "SetCollisionFlags",   &Entities::SetCollisionFlags,   4, ".nnb" },
  { "Move",                &Entities::Move,                3, ".nx" },
  { "Rotate",              &Entities::Rotate,              3, ".nx" },
  { "GiveImpulse",         &Entities::GiveImpulse,         3, ".nx" },
  { "InflictDirectDamage", &Entities::InflictDirectDamage, 7, ".nnnnxx" },
  { "InflictRangeDamage",  &Entities::InflictRangeDamage,  7, ".nnnxnn" },
  { "InflictBoxDamage",    &Entities::InflictBoxDamage,    5, ".nnnx" },
};

void VM::RegisterEntities(void) {
  // [Cecil] NOTE: Everything entity-related should be read-only!!!
  // Creation and modification should be handled exclusively through extension packets for proper synchronization!
  Table sqtEntities = Root().RegisterTable("Entities");
  INDEX i;

  // Register classes
  {
    Class<CEntityPointer> sqcEntity(GetVM(), "CEntityPointer", &SqEntity::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqEntity::_aMethods); i++) {
      sqcEntity.RegisterMethod(SqEntity::_aMethods[i]);
    }

    // Metamethods
    sqcEntity.RegisterMetamethod(E_MM_TOSTRING, &SqEntity::ToString);

    // CMovableEntity properties
    sqcEntity.RegisterVar("en_vDesiredTranslationRelative", &SqEntity::GetDesiredTranslationRelative, NULL);
    sqcEntity.RegisterVar("en_aDesiredRotationRelative",    &SqEntity::GetDesiredRotationRelative, NULL);
    sqcEntity.RegisterVar("en_vCurrentTranslationAbsolute", &SqEntity::GetCurrentTranslationAbsolute, NULL);
    sqcEntity.RegisterVar("en_aCurrentRotationAbsolute",    &SqEntity::GetCurrentRotationAbsolute, NULL);

    sqcEntity.RegisterVar("en_penReference",      &SqEntity::GetReference, NULL);
    sqcEntity.RegisterVar("en_vReferencePlane",   &SqEntity::GetReferencePlane, NULL);
    sqcEntity.RegisterVar("en_iReferenceSurface", &SqEntity::GetReferenceSurface, NULL);

    sqcEntity.RegisterVar("en_penLastValidReference",             &SqEntity::GetLastValidReference, NULL);
    sqcEntity.RegisterVar("en_tmLastSignificantVerticalMovement", &SqEntity::GetLastSignificantVerticalMovement, NULL);

    sqcEntity.RegisterVar("en_tmLastBreathed",   &SqEntity::GetLastBreathed, NULL);
    sqcEntity.RegisterVar("en_tmMaxHoldBreath",  &SqEntity::GetMaxHoldBreath, NULL);
    sqcEntity.RegisterVar("en_fDensity",         &SqEntity::GetDensity, NULL);
    sqcEntity.RegisterVar("en_tmLastSwimDamage", &SqEntity::GetLastSwimDamage, NULL);
    sqcEntity.RegisterVar("en_iUpContent",       &SqEntity::GetUpContent, NULL);
    sqcEntity.RegisterVar("en_iDnContent",       &SqEntity::GetDnContent, NULL);
    sqcEntity.RegisterVar("en_fImmersionFactor", &SqEntity::GetImmersionFactor, NULL);

    sqcEntity.RegisterVar("en_vGravityDir", &SqEntity::GetGravityDir, NULL);
    sqcEntity.RegisterVar("en_fGravityA",   &SqEntity::GetGravityA, NULL);
    sqcEntity.RegisterVar("en_fGravityV",   &SqEntity::GetGravityV, NULL);
    sqcEntity.RegisterVar("en_vForceDir",   &SqEntity::GetForceDir, NULL);
    sqcEntity.RegisterVar("en_fForceA",     &SqEntity::GetForceA, NULL);
    sqcEntity.RegisterVar("en_fForceV",     &SqEntity::GetForceV, NULL);

    sqcEntity.RegisterVar("en_tmJumped",               &SqEntity::GetJumped, NULL);
    sqcEntity.RegisterVar("en_tmMaxJumpControl",       &SqEntity::GetMaxJumpControl, NULL);
    sqcEntity.RegisterVar("en_fJumpControlMultiplier", &SqEntity::GetJumpControlMultiplier, NULL);

    sqcEntity.RegisterVar("en_fAcceleration",          &SqEntity::GetAcceleration, NULL);
    sqcEntity.RegisterVar("en_fDeceleration",          &SqEntity::GetDeceleration, NULL);
    sqcEntity.RegisterVar("en_fStepUpHeight",          &SqEntity::GetStepUpHeight, NULL);
    sqcEntity.RegisterVar("en_fStepDnHeight",          &SqEntity::GetStepDnHeight, NULL);
    sqcEntity.RegisterVar("en_fBounceDampParallel",    &SqEntity::GetBounceDampParallel, NULL);
    sqcEntity.RegisterVar("en_fBounceDampNormal",      &SqEntity::GetBounceDampNormal, NULL);
    sqcEntity.RegisterVar("en_fCollisionSpeedLimit",   &SqEntity::GetCollisionSpeedLimit, NULL);
    sqcEntity.RegisterVar("en_fCollisionDamageFactor", &SqEntity::GetCollisionDamageFactor, NULL);

    Root().AddClass(sqcEntity);
  }
  {
    Class<CEntityProperty *> sqcProp(GetVM(), "Property", &SqProp::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqProp::_aMethods); i++) {
      sqcProp.RegisterMethod(SqProp::_aMethods[i]);
    }

    sqcProp.RegisterVar("type",     &SqProp::GetPropType,     NULL);
    sqcProp.RegisterVar("id",       &SqProp::GetPropID,       NULL);
    sqcProp.RegisterVar("offset",   &SqProp::GetPropOffset,   NULL);
    sqcProp.RegisterVar("name",     &SqProp::GetPropName,     NULL);
    sqcProp.RegisterVar("flags",    &SqProp::GetPropFlags,    NULL);
    sqcProp.RegisterVar("shortcut", &SqProp::GetPropShortcut, NULL);
    sqcProp.RegisterVar("color",    &SqProp::GetPropColor,    NULL);

    sqtEntities.AddClass(sqcProp);
  }
  {
    Class<EExtEntityEvent> sqcEvent(GetVM(), "CEntityEvent", &SqEvent::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqEvent::_aMethods); i++) {
      sqcEvent.RegisterMethod(SqEvent::_aMethods[i]);
    }

    Root().AddClass(sqcEvent);
  }

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aEntitiesFuncs); i++) {
    sqtEntities.RegisterFunc(_aEntitiesFuncs[i]);
  }

  // Render types
  Enumeration enRenderTypes(GetVM());

#define ADD_RT(_RenderTypeName) enRenderTypes.RegisterValue(#_RenderTypeName, (SQInteger)CEntity::RT_##_RenderTypeName)
  ADD_RT(ILLEGAL);
  ADD_RT(NONE);
  ADD_RT(MODEL);
  ADD_RT(BRUSH);
  ADD_RT(EDITORMODEL);
  ADD_RT(VOID);
  ADD_RT(FIELDBRUSH);
  ADD_RT(SKAMODEL);
  ADD_RT(SKAEDITORMODEL);
  ADD_RT(TERRAIN);
#undef ADD_RT

  Const().AddEnum("RT", enRenderTypes);

#define ADD_FLAG(_FlagType, _FlagName) enFlags.RegisterValue(#_FlagName, (SQInteger)_FlagType##_##_FlagName)
  {
    // Spawn flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(SPF, EASY);
    ADD_FLAG(SPF, NORMAL);
    ADD_FLAG(SPF, HARD);
    ADD_FLAG(SPF, EXTREME);
    ADD_FLAG(SPF, TOURIST);
    ADD_FLAG(SPF, MASK_DIFFICULTY);
    ADD_FLAG(SPF, SINGLEPLAYER);
    ADD_FLAG(SPF, DEATHMATCH);
    ADD_FLAG(SPF, COOPERATIVE);
    ADD_FLAG(SPF, FLYOVER);
    ADD_FLAG(SPF, MASK_GAMEMODE);
    Const().AddEnum("SPF", enFlags);
  }
  {
    // Entity flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(ENF, SELECTED);
    ADD_FLAG(ENF, ZONING);
    ADD_FLAG(ENF, DELETED);
    ADD_FLAG(ENF, ALIVE);
    ADD_FLAG(ENF, INRENDERING);
    ADD_FLAG(ENF, VALIDSHADINGINFO);
    ADD_FLAG(ENF, SEETHROUGH);
    ADD_FLAG(ENF, FOUNDINGRIDSEARCH);
    ADD_FLAG(ENF, CLUSTERSHADOWS);
    ADD_FLAG(ENF, BACKGROUND);
    ADD_FLAG(ENF, ANCHORED);
    ADD_FLAG(ENF, HASPARTICLES);
    ADD_FLAG(ENF, INVISIBLE);
    ADD_FLAG(ENF, DYNAMICSHADOWS);
    ADD_FLAG(ENF, NOTIFYLEVELCHANGE);
    ADD_FLAG(ENF, CROSSESLEVELS);
    ADD_FLAG(ENF, PREDICTABLE);
    ADD_FLAG(ENF, PREDICTOR);
    ADD_FLAG(ENF, PREDICTED);
    ADD_FLAG(ENF, WILLBEPREDICTED);
    ADD_FLAG(ENF, TEMPPREDICTOR);
    ADD_FLAG(ENF, HIDDEN);
    ADD_FLAG(ENF, NOSHADINGINFO);
    Const().AddEnum("ENF", enFlags);
  }
  {
    // Physics flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(EPF, ORIENTEDBYGRAVITY);
    ADD_FLAG(EPF, TRANSLATEDBYGRAVITY);
    ADD_FLAG(EPF, PUSHABLE);
    ADD_FLAG(EPF, STICKYFEET);
    ADD_FLAG(EPF, RT_SYNCHRONIZED);
    ADD_FLAG(EPF, ABSOLUTETRANSLATE);
    ADD_FLAG(EPF, NOACCELERATION);
    ADD_FLAG(EPF, HASLUNGS);
    ADD_FLAG(EPF, HASGILLS);
    ADD_FLAG(EPF, MOVABLE);
    ADD_FLAG(EPF, NOIMPACT);
    ADD_FLAG(EPF, NOIMPACTTHISTICK);
    ADD_FLAG(EPF, CANFADESPINNING);
    ADD_FLAG(EPF, ONSTEEPSLOPE);
    ADD_FLAG(EPF, ORIENTINGTOGRAVITY);
    ADD_FLAG(EPF, FLOATING);
    ADD_FLAG(EPF, FORCEADDED);

    ADD_FLAG(EPF, ONBLOCK_MASK);
    ADD_FLAG(EPF, ONBLOCK_STOP);
    ADD_FLAG(EPF, ONBLOCK_SLIDE);
    ADD_FLAG(EPF, ONBLOCK_CLIMBORSLIDE);
    ADD_FLAG(EPF, ONBLOCK_BOUNCE);
    ADD_FLAG(EPF, ONBLOCK_PUSH);
    ADD_FLAG(EPF, ONBLOCK_STOPEXACT);
    Const().AddEnum("EPF", enFlags);
  }
  {
    // Collision bits
    Enumeration enFlags(GetVM());
    ADD_FLAG(ECB, COUNT);
    ADD_FLAG(ECB, IS);
    ADD_FLAG(ECB, TEST);
    ADD_FLAG(ECB, PASS);
    Const().AddEnum("ECB", enFlags);
  }
  {
    // Collision flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(ECF, MASK);
    ADD_FLAG(ECF, ISMASK);
    ADD_FLAG(ECF, TESTMASK);
    ADD_FLAG(ECF, PASSMASK);
    ADD_FLAG(ECF, IGNOREBRUSHES);
    ADD_FLAG(ECF, IGNOREMODELS);
    Const().AddEnum("ECF", enFlags);
  }
#undef ADD_FLAG

  // Property types
  Enumeration enPropTypes(GetVM());

#define ADD_EPT(_PropTypeName) enPropTypes.RegisterValue(#_PropTypeName, (SQInteger)CEntityProperty::EPT_##_PropTypeName)
  ADD_EPT(ENUM);
  ADD_EPT(BOOL);
  ADD_EPT(FLOAT);
  ADD_EPT(COLOR);
  ADD_EPT(STRING);
  ADD_EPT(RANGE);
  ADD_EPT(ENTITYPTR);
  ADD_EPT(FILENAME);
  ADD_EPT(INDEX);
  ADD_EPT(ANIMATION);
  ADD_EPT(ILLUMINATIONTYPE);
  ADD_EPT(FLOATAABBOX3D);
  ADD_EPT(ANGLE);
  ADD_EPT(FLOAT3D);
  ADD_EPT(ANGLE3D);
  ADD_EPT(FLOATplane3D);
  ADD_EPT(MODELOBJECT);
  ADD_EPT(PLACEMENT3D);
  ADD_EPT(ANIMOBJECT);
  ADD_EPT(FILENAMENODEP);
  ADD_EPT(SOUNDOBJECT);
  ADD_EPT(STRINGTRANS);
  ADD_EPT(FLOATQUAT3D);
  ADD_EPT(FLOATMATRIX3D);
  ADD_EPT(FLAGS);
#if SE1_VER >= SE1_107
  ADD_EPT(MODELINSTANCE);
#endif
#undef ADD_EPT

  Const().AddEnum("EPT", enPropTypes);

  // Entity events
  Enumeration enEventCodes(GetVM());

#define ADD_EVENT(_EventName) enEventCodes.RegisterValue(#_EventName, (SQInteger)EVENTCODE_VNL_##_EventName)
  ADD_EVENT(EStop);
  ADD_EVENT(EStart);
  ADD_EVENT(EActivate);
  ADD_EVENT(EDeactivate);
  ADD_EVENT(EEnvironmentStart);
  ADD_EVENT(EEnvironmentStop);
  ADD_EVENT(EEnd);
  ADD_EVENT(ETrigger);
  ADD_EVENT(ETeleportMovingBrush);
  ADD_EVENT(EReminder);
  ADD_EVENT(EStartAttack);
  ADD_EVENT(EStopAttack);
  ADD_EVENT(EStopBlindness);
  ADD_EVENT(EStopDeafness);
  ADD_EVENT(EReceiveScore);
  ADD_EVENT(EKilledEnemy);
  ADD_EVENT(ESecretFound);
  ADD_EVENT(ESound);

  ADD_EVENT(EScroll);
  ADD_EVENT(ETextFX);
  ADD_EVENT(EHudPicFX);
  ADD_EVENT(ECredits);
  ADD_EVENT(ECenterMessage);
  ADD_EVENT(EComputerMessage);
  ADD_EVENT(EVoiceMessage);
  ADD_EVENT(EHitBySpaceShipBeam);

  ADD_EVENT(EAmmoItem);
  ADD_EVENT(EAmmoPackItem);
  ADD_EVENT(EArmor);
  ADD_EVENT(EHealth);
  ADD_EVENT(EKey);
  ADD_EVENT(EMessageItem);
  ADD_EVENT(EPowerUp);
  ADD_EVENT(EWeaponItem);

  ADD_EVENT(ERestartAttack);
  ADD_EVENT(EReconsiderBehavior);
  ADD_EVENT(EForceWound);
  ADD_EVENT(ESelectWeapon);
  ADD_EVENT(EBoringWeapon);
  ADD_EVENT(EFireWeapon);
  ADD_EVENT(EReleaseWeapon);
  ADD_EVENT(EReloadWeapon);
  ADD_EVENT(EWeaponChanged);

  ADD_EVENT(EAirShockwave);
  ADD_EVENT(EAirWave);
  ADD_EVENT(ESpawnEffect);
  ADD_EVENT(ESpawnSpray);
  ADD_EVENT(EBulletInit);
  ADD_EVENT(ELaunchCannonBall);
  ADD_EVENT(ECyborgBike);
  ADD_EVENT(ESpawnDebris);
  ADD_EVENT(EDevilProjectile);
  ADD_EVENT(ESpawnEffector);
  ADD_EVENT(EFlame);
  ADD_EVENT(ELaunchLarvaOffspring);
  ADD_EVENT(EAnimatorInit);
  ADD_EVENT(EViewInit);
  ADD_EVENT(EWeaponsInit);
  ADD_EVENT(EWeaponEffectInit);
  ADD_EVENT(ELaunchProjectile);
  ADD_EVENT(EReminderInit);
  ADD_EVENT(ESeriousBomb);
  ADD_EVENT(ESpawnerProjectile);
  ADD_EVENT(ESpinnerInit);
  ADD_EVENT(ETwister);
  ADD_EVENT(EWatcherInit);
  ADD_EVENT(EWater);
#undef ADD_EVENT

  Const().AddEnum("EVENTCODE", enEventCodes);

  // Damage types
  Enumeration enDamageTypes(GetVM());

#define ADD_DMT(_DamageTypeName) enDamageTypes.RegisterValue(#_DamageTypeName, (SQInteger)DMT_##_DamageTypeName)
  ADD_DMT(EXPLOSION);
  ADD_DMT(PROJECTILE);
  ADD_DMT(CLOSERANGE);
  ADD_DMT(BULLET);
  ADD_DMT(DROWNING);
  ADD_DMT(IMPACT);
  ADD_DMT(BRUSH);
  ADD_DMT(BURNING);
  ADD_DMT(ACID);
  ADD_DMT(TELEPORT);
  ADD_DMT(FREEZING);
  ADD_DMT(CANNONBALL);
  ADD_DMT(CANNONBALL_EXPLOSION);
  ADD_DMT(SPIKESTAB);
  ADD_DMT(ABYSS);
  ADD_DMT(HEAT);
  ADD_DMT(DAMAGER);
  ADD_DMT(CHAINSAW);
  ADD_DMT(NONE);
#undef ADD_DMT

  Const().AddEnum("DMT", enDamageTypes);
};

}; // namespace
