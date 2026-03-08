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

namespace sq {

// CBrushPolygon class methods
namespace SqBrushPolygon {

// Make sure the polygon in the pointer exists
#define ASSERT_POLYGON { if (val == NULL) return sq_throwerror(v, "CBrushPolygon is NULL"); }

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CBrushPolygon *&val) {
  val = NULL;

  if (ctArgs > 0) {
    GetInstanceValueVerify(CBrushPolygon *, pOther, v, 2);
    if (pOther != NULL) val = *pOther;
  }

  return 0;
};

static SQInteger Equal(HSQUIRRELVM v, int, CBrushPolygon *&val) {
  // Compare against null
  if (sq_gettype(v, 2) == OT_NULL) {
    sq_pushbool(v, val == NULL);
    return 1;
  }

  // Compare against another pointer
  GetInstanceValueVerifyN(CBrushPolygon *, pOther, v, 2, "CBrushPolygon");
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger GetPlane(HSQUIRRELVM v, int, CBrushPolygon *&val) {
  ASSERT_POLYGON;

  FLOATplane3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATplane3D", &ppl)) return SQ_ERROR;

  *ppl = val->bpo_pbplPlane->bpl_plAbsolute;
  return 1;
};

static SQInteger GetSurface(HSQUIRRELVM v, int, CBrushPolygon *&val) {
  ASSERT_POLYGON;
  sq_pushinteger(v, val->bpo_bppProperties.bpp_ubSurfaceType);
  return 1;
};

static SQInteger GetDistanceFromEdges(HSQUIRRELVM v, int, CBrushPolygon *&val) {
  ASSERT_POLYGON;
  GetInstanceValueVerify(FLOAT3D, pvPoint, v, 2);
  sq_pushfloat(v, val->GetDistanceFromEdges(*pvPoint));
  return 1;
};

static Method<CBrushPolygon *> _aMethods[] = {
  { "Equal",                &Equal,                2, ".x|o" },
  { "GetPlane",             &GetPlane,             1, "." },
  { "GetSurface",           &GetSurface,           1, "." },
  { "GetDistanceFromEdges", &GetDistanceFromEdges, 2, ".x" },
};

}; // namespace

// CCastRay class methods
namespace SqCastRay {

struct RayHolder {
  CCastRay cr;
  RayHolder() : cr(NULL, CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0))) {};
};

inline FLOAT3D CalculateRayOrigin(const CPlacement3D &plRay) {
  return plRay.pl_PositionVector;
};

inline FLOAT3D CalculateRayTarget(const CPlacement3D &plRay, FLOAT fDistance) {
  FLOAT3D vDirection;
  AnglesToDirectionVector(plRay.pl_OrientationAngle, vDirection);
  return plRay.pl_PositionVector + vDirection * fDistance;
};

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, RayHolder &val) {
  if (ctArgs < 2 || ctArgs > 3) {
    return sq_throwerror(v, "expected 2 or 3 arguments in the CCastRay constructor");
  }

  CCastRay &cr = val.cr;
  GetInstanceValueVerify(CEntityPointer, ppenOrigin, v, 2);

  // Try getting a placement first
  GetInstanceValue(CPlacement3D, pplOrigin, v, 3);

  if (pplOrigin != NULL) {
    SQFloat fMaxDist;

    // Try to setup a ray with a maximum distance
    if (ctArgs == 3 && SQ_SUCCEEDED(sq_getfloat(v, 4, &fMaxDist))) {
      cr.Init(*ppenOrigin, CalculateRayOrigin(*pplOrigin), CalculateRayTarget(*pplOrigin, fMaxDist));
      cr.cr_fHitDistance = fMaxDist;

    // Or just from some placement
    } else {
      cr.Init(*ppenOrigin, CalculateRayOrigin(*pplOrigin), CalculateRayTarget(*pplOrigin, 1.0f));
      cr.cr_fHitDistance = UpperLimit(0.0f);
    }

    return 0;
  }

  // Then try getting two positions
  GetInstanceValueVerify(FLOAT3D, pvOrigin, v, 3);
  GetInstanceValueVerify(FLOAT3D, pvTarget, v, 4);

  cr.Init(*ppenOrigin, *pvOrigin, *pvTarget);
  cr.cr_fHitDistance = (cr.cr_vTarget - cr.cr_vOrigin).Length() + 0.1f;
  return 0;
};

SQCLASS_GETSET_BOOL(GetFlagPortals,      SetFlagPortals,      RayHolder, val.cr.cr_bHitPortals, val.cr.cr_bHitPortals);
SQCLASS_GETSET_BOOL(GetFlagTransPortals, SetFlagTransPortals, RayHolder, val.cr.cr_bHitTranslucentPortals, val.cr.cr_bHitTranslucentPortals);
SQCLASS_GETSET_BOOL(GetFlagFields,       SetFlagFields,       RayHolder, val.cr.cr_bHitFields, val.cr.cr_bHitFields);
#if SE1_VER >= SE1_107
SQCLASS_GETSET_BOOL(GetFlagBrushes,      SetFlagBrushes,      RayHolder, val.cr.cr_bHitBrushes, val.cr.cr_bHitBrushes);
SQCLASS_GETSET_BOOL(GetFlagInvisTris,    SetFlagInvisTris,    RayHolder, val.cr.cr_bHitTerrainInvisibleTris, val.cr.cr_bHitTerrainInvisibleTris);
#endif
SQCLASS_GETSET_BOOL(GetFlagPhysical,     SetFlagPhysical,     RayHolder, val.cr.cr_bPhysical, val.cr.cr_bPhysical);
SQCLASS_GETSET_FLOAT(GetTestR, SetTestR, RayHolder, val.cr.cr_fTestR, val.cr.cr_fTestR);

static SQInteger GetTestType(HSQUIRRELVM v, RayHolder &val) {
  sq_pushinteger(v, val.cr.cr_ttHitModels);
  return 1;
};

static SQInteger SetTestType(HSQUIRRELVM v, RayHolder &val, SQInteger idxValue) {
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, idxValue, &i))) return sq_throwerror(v, "expected an integer value");

  val.cr.cr_ttHitModels = (CCastRay::TestType)i;
  return 1;
}

static SQInteger GetPlacement(HSQUIRRELVM v, int, RayHolder &val) {
  CPlacement3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("CPlacement3D", &ppl)) return SQ_ERROR;

  *ppl = val.cr.cl_plRay;
  return 1;
};

static SQInteger GetEntity(HSQUIRRELVM v, int, RayHolder &val) {
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val.cr.cr_penOrigin;
  return 1;
};

static SQInteger GetOrigin(HSQUIRRELVM v, int, RayHolder &val) {
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.cr.cr_vOrigin;
  return 1;
};

static SQInteger GetTarget(HSQUIRRELVM v, int, RayHolder &val) {
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.cr.cr_vTarget;
  return 1;
};

static SQInteger GetHitEntity(HSQUIRRELVM v, int, RayHolder &val) {
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val.cr.cr_penHit;
  return 1;
};

static SQInteger GetHitPoint(HSQUIRRELVM v, int, RayHolder &val) {
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.cr.cr_vHit;
  return 1;
};

static SQInteger GetHitDistance(HSQUIRRELVM v, int, RayHolder &val) {
  sq_pushfloat(v, val.cr.cr_fHitDistance);
  return 1;
};

static SQInteger GetHitPolygon(HSQUIRRELVM v, int, RayHolder &val) {
  CBrushPolygon **pbpol;
  if (!GetVMClass(v).Root().CreateInstanceOf("CBrushPolygon", &pbpol)) return SQ_ERROR;

  *pbpol = val.cr.cr_pbpoBrushPolygon;
  return 1;
};

// Safe guard against running world functions outside the game
inline BOOL IsGameOn(void) {
  return GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn();
};

static SQInteger Cast(HSQUIRRELVM v, int, RayHolder &val) {
  if (!IsGameOn()) return sq_throwerror(v, "cannot cast rays within worlds while no game is running");

  val.cr.Cast(IWorld::GetWorld());
  return 0;
};

static SQInteger ContinueCast(HSQUIRRELVM v, int, RayHolder &val) {
  if (!IsGameOn()) return sq_throwerror(v, "cannot cast rays within worlds while no game is running");

  val.cr.ContinueCast(IWorld::GetWorld());
  return 0;
};

static Method<RayHolder> _aMethods[] = {
  { "GetPlacement",   &GetPlacement,   1, "." },
  { "GetEntity",      &GetEntity,      1, "." },
  { "GetOrigin",      &GetOrigin,      1, "." },
  { "GetTarget",      &GetTarget,      1, "." },

  { "GetHitEntity",   &GetHitEntity,   1, "." },
  { "GetHitPoint",    &GetHitPoint,    1, "." },
  { "GetHitDistance", &GetHitDistance, 1, "." },
  { "GetHitPolygon",  &GetHitPolygon,  1, "." },

  { "Cast",         &Cast,         1, "." },
  { "ContinueCast", &ContinueCast, 1, "." },
};

}; // namespace

namespace Utils {

static bool IncludeReturnCallback(sq::VM &) {
  return false; // Leave return value in the stack
};

// Execute a script from a file in place
static SQInteger IncludeScript(HSQUIRRELVM v) {
  const SQChar *strFile = "";
  sq_getstring(v, 2, &strFile);

  VM &vm = GetVMClass(v);

  if (!vm.ExecuteFile(strFile, &IncludeReturnCallback)) {
    // Pass execution error
    return sq_throwerror(v, vm.GetError());
  }

  // Reuse return value from the script
  return 1;
};

// Compile a script from a file and return it as a closure
static SQInteger CompileScript(HSQUIRRELVM v) {
  const SQChar *strFile = "";
  sq_getstring(v, 2, &strFile);

  VM &vm = GetVMClass(v);
  vm.CompileFromFile(strFile);

  if (!vm.CanBeExecuted()) {
    // Pass compilation error
    return sq_throwerror(v, vm.GetError());
  }

  // Return compiled closure
  return 1;
};

}; // namespace

// Global functions
static SQRegFunction _aGlobalFuncs[] = {
  { "IncludeScript", &Utils::IncludeScript, 2, ".s" },
  { "CompileScript", &Utils::CompileScript, 2, ".s" },
};

void VM::RegisterUtils(void) {
  Table sqtUtils = Root().RegisterTable("Utils");
  INDEX i;

  // Register classes
  {
    Class<CBrushPolygon *> sqcPolygon(GetVM(), "CBrushPolygon", &SqBrushPolygon::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqBrushPolygon::_aMethods); i++) {
      sqcPolygon.RegisterMethod(SqBrushPolygon::_aMethods[i]);
    }

    Root().AddClass(sqcPolygon);
  }
  {
    Class<SqCastRay::RayHolder> sqcCastRay(GetVM(), "CCastRay", &SqCastRay::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqCastRay::_aMethods); i++) {
      sqcCastRay.RegisterMethod(SqCastRay::_aMethods[i]);
    }

    // Setup variables
    sqcCastRay.RegisterVar("cr_bHitPortals",              &SqCastRay::GetFlagPortals,      &SqCastRay::SetFlagPortals);
    sqcCastRay.RegisterVar("cr_bHitTranslucentPortals",   &SqCastRay::GetFlagTransPortals, &SqCastRay::SetFlagTransPortals);
    sqcCastRay.RegisterVar("cr_ttHitModels",              &SqCastRay::GetTestType,         &SqCastRay::SetTestType);
    sqcCastRay.RegisterVar("cr_bHitFields",               &SqCastRay::GetFlagFields,       &SqCastRay::SetFlagFields);
  #if SE1_VER >= SE1_107
    sqcCastRay.RegisterVar("cr_bHitBrushes",              &SqCastRay::GetFlagBrushes,      &SqCastRay::SetFlagBrushes);
    sqcCastRay.RegisterVar("cr_bHitTerrainInvisibleTris", &SqCastRay::GetFlagInvisTris,    &SqCastRay::SetFlagInvisTris);
  #endif
    sqcCastRay.RegisterVar("cr_bPhysical",                &SqCastRay::GetFlagPhysical,     &SqCastRay::SetFlagPhysical);
    sqcCastRay.RegisterVar("cr_fTestR",                   &SqCastRay::GetTestR,            &SqCastRay::SetTestR);

    Root().AddClass(sqcCastRay);
  }

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aGlobalFuncs); i++) {
    Root().RegisterFunc(_aGlobalFuncs[i]);
  }

  // Ray test types
  Enumeration enRayTestTypes(GetVM());

#define ADD_TESTTYPE(_TestType) enRayTestTypes.RegisterValue(#_TestType, (SQInteger)CCastRay::TT_##_TestType)
  ADD_TESTTYPE(NONE);
  ADD_TESTTYPE(SIMPLE);
  ADD_TESTTYPE(COLLISIONBOX);
  ADD_TESTTYPE(FULL);
  ADD_TESTTYPE(FULLSEETHROUGH);
#undef ADD_TESTTYPE

  Const().AddEnum("TestType", enRayTestTypes);
};

}; // namespace
