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

#include "RawDataBuffer.h"

namespace sq {

// Buffer class methods
namespace SqBuffer {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CRawDataBuffer &val) {
  SQInteger iBytes;

  if (ctArgs != 1 || SQ_FAILED(sq_getinteger(v, 2, &iBytes))) {
    return sq_throwerror(v, "expected amount of bytes in argument 1");
  }

  if (iBytes < 0) return sq_throwerror(v, "less than 0 bytes specified in argument 1");

  val.New(iBytes);
  return 0;
};

static SQInteger ToString(HSQUIRRELVM v, CRawDataBuffer &val) {
  const INDEX ct = val.aData.Count();
  CTString strData = "[";

  for (INDEX i = 0; i < ct; i++) {
    if (i == 0) {
      strData += CTString(0, "%02X", val.aData[i]);
    } else {
      strData += CTString(0, " %02X", val.aData[i]);
    }
  }

  strData += "]";
  sq_pushstring(v, strData.str_String, -1);
  return 1;
};

static SQInteger Equal(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  GetInstanceValueVerify(CRawDataBuffer, pOther, v, 2);
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger Size(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  sq_pushinteger(v, val.aData.Count());
  return 1;
};

static SQInteger Reset(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  val.Reset();
  return 0;
};

static SQInteger SetByte(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset, iByte;
  sq_getinteger(v, 2, &iOffset);
  sq_getinteger(v, 3, &iByte);

  if (!val.SetByte(iOffset, iByte)) return sq_throwerror(v, "offset is out of bounds");
  return 0;
};

static SQInteger SetBool(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);
  SQBool b;
  sq_getbool(v, 3, &b);

  if (!val.SetBool(iOffset, b)) return sq_throwerror(v, "offset is out of bounds");
  return 0;
};

static SQInteger SetIndex(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset, i;
  sq_getinteger(v, 2, &iOffset);
  sq_getinteger(v, 3, &i);

  if (!val.SetIndex(iOffset, i)) return sq_throwerror(v, "offset is out of bounds");
  return 0;
};

static SQInteger SetFloat(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);
  SQFloat f;
  sq_getfloat(v, 3, &f);

  if (!val.SetFloat(iOffset, f)) return sq_throwerror(v, "offset is out of bounds");
  return 0;
};

static SQInteger SetString(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);
  const SQChar *str;
  sq_getstring(v, 3, &str);

  if (!val.SetString(iOffset, str)) return sq_throwerror(v, "offset is out of bounds");
  return 0;
};

static SQInteger GetByte(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  UBYTE ub;
  if (!val.GetByte(iOffset, ub)) return sq_throwerror(v, "offset is out of bounds");

  sq_pushinteger(v, ub);
  return 1;
};

static SQInteger GetBool(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  BOOL b;
  if (!val.GetBool(iOffset, b)) return sq_throwerror(v, "offset is out of bounds");

  sq_pushbool(v, b);
  return 1;
};

static SQInteger GetIndex(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  INDEX i;
  if (!val.GetIndex(iOffset, i)) return sq_throwerror(v, "offset is out of bounds");

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger GetFloat(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  FLOAT f;
  if (!val.GetFloat(iOffset, f)) return sq_throwerror(v, "offset is out of bounds");

  sq_pushfloat(v, f);
  return 1;
};

static SQInteger GetString(HSQUIRRELVM v, int, CRawDataBuffer &val) {
  SQInteger iOffset, iMaxChars;
  sq_getinteger(v, 2, &iOffset);
  sq_getinteger(v, 3, &iMaxChars);

  if (iMaxChars < 0) return sq_throwerror(v, "invalid maximum amount of characters (non-positive integer)");

  CTString str;
  if (!val.GetString(iOffset, str, iMaxChars)) return sq_throwerror(v, "offset is out of bounds");

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static Method<CRawDataBuffer> _aMethods[] = {
  { "Equal", &Equal, 2, ".x" },

  { "Size",  &Size,  1, "." },
  { "Reset", &Reset, 1, "." },

  { "SetByte",   &SetByte,   3, ".nn" },
  { "SetBool",   &SetBool,   3, ".nb" },
  { "SetIndex",  &SetIndex,  3, ".nn" },
  { "SetFloat",  &SetFloat,  3, ".nn" },
  { "SetString", &SetString, 3, ".ns" },

  { "GetByte",   &GetByte,   2, ".n" },
  { "GetBool",   &GetBool,   2, ".n" },
  { "GetIndex",  &GetIndex,  2, ".n" },
  { "GetFloat",  &GetFloat,  2, ".n" },
  { "GetString", &GetString, 3, ".nn" },
};

}; // namespace

// CBrushPolygon class methods
namespace SqBrushPolygon {

// Make sure the polygon in the pointer exists
#define ASSERT_POLYGON { if (val == NULL) return sq_throwerror(v, "CBrushPolygon is NULL"); }

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CBrushPolygon *&val) {
  val = NULL;

  if (ctArgs > 0) {
    GetInstanceValueVerifyN(CBrushPolygon *, pOther, v, 2, "CBrushPolygon");
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
  PushNewInstance(FLOATplane3D, ppl, GetVMClass(v).Root(), "FLOATplane3D");
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
  SQ_RESTRICT(v);

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
  SQ_RESTRICT(v);
  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = val.cr.cl_plRay;
  return 1;
};

static SQInteger GetEntity(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val.cr.cr_penOrigin;
  return 1;
};

static SQInteger GetOrigin(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  *pv = val.cr.cr_vOrigin;
  return 1;
};

static SQInteger GetTarget(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  *pv = val.cr.cr_vTarget;
  return 1;
};

static SQInteger GetHitEntity(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = val.cr.cr_penHit;
  return 1;
};

static SQInteger GetHitPoint(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  *pv = val.cr.cr_vHit;
  return 1;
};

static SQInteger GetHitDistance(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  sq_pushfloat(v, val.cr.cr_fHitDistance);
  return 1;
};

static SQInteger GetHitPolygon(HSQUIRRELVM v, int, RayHolder &val) {
  SQ_RESTRICT(v);
  PushNewInstance(CBrushPolygon *, pbpol, GetVMClass(v).Root(), "CBrushPolygon");
  *pbpol = val.cr.cr_pbpoBrushPolygon;
  return 1;
};

// Safe guard against running world functions outside the game
inline BOOL IsGameOn(void) {
  return GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn();
};

static SQInteger Cast(HSQUIRRELVM v, int, RayHolder &val) {
  if (!IsGameOn()) return sq_throwerror(v, "cannot cast rays within worlds while no game is running");
  SQ_RESTRICT(v);

  val.cr.Cast(IWorld::GetWorld());
  return 0;
};

static SQInteger ContinueCast(HSQUIRRELVM v, int, RayHolder &val) {
  if (!IsGameOn()) return sq_throwerror(v, "cannot cast rays within worlds while no game is running");
  SQ_RESTRICT(v);

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

// CDrawPort class methods
namespace SqDrawPort {

static SQInteger Constructor(HSQUIRRELVM v, int, CDrawPort &val) {
  return sq_throwerror(v, "CDrawPort cannot be created within scripts");
};

static SQInteger Equal(HSQUIRRELVM v, int, CDrawPort &val) {
  // Compare against another pointer
  GetInstanceValueVerify(CDrawPort, pOther, v, 2);
  sq_pushbool(v, &val == pOther);
  return 1;
};

static SQInteger IsDualHead(HSQUIRRELVM v, int, CDrawPort &val) {
  sq_pushbool(v, val.IsDualHead());
  return 1;
};

static SQInteger IsWideScreen(HSQUIRRELVM v, int, CDrawPort &val) {
  sq_pushbool(v, val.IsWideScreen());
  return 1;
};

static SQInteger GetWidth(HSQUIRRELVM v, int, CDrawPort &val) {
  sq_pushinteger(v, val.GetWidth());
  return 1;
};

static SQInteger GetHeight(HSQUIRRELVM v, int, CDrawPort &val) {
  sq_pushinteger(v, val.GetHeight());
  return 1;
};

static SQInteger SetFont(HSQUIRRELVM v, int, CDrawPort &val) {
  GetInstanceValueVerify(CFontData, pfd, v, 2);
  val.SetFont(pfd);
  return 0;
};

static SQInteger SetTextCharSpacing(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iSpacing;
  sq_getinteger(v, 2, &iSpacing);
  val.SetTextCharSpacing(iSpacing);
  return 0;
};

static SQInteger SetTextLineSpacing(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iSpacing;
  sq_getinteger(v, 2, &iSpacing);
  val.SetTextLineSpacing(iSpacing);
  return 0;
};

static SQInteger SetTextScaling(HSQUIRRELVM v, int, CDrawPort &val) {
  SQFloat fScalingFactor;
  sq_getfloat(v, 2, &fScalingFactor);
  val.SetTextScaling(fScalingFactor);
  return 0;
};

static SQInteger SetTextAspect(HSQUIRRELVM v, int, CDrawPort &val) {
  SQFloat fAspectRatio;
  sq_getfloat(v, 2, &fAspectRatio);
  val.SetTextAspect(fAspectRatio);
  return 0;
};

static SQInteger SetTextMode(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iMode;
  sq_getinteger(v, 2, &iMode);
  val.SetTextMode(iMode);
  return 0;
};

static SQInteger GetTextWidth(HSQUIRRELVM v, int, CDrawPort &val) {
  const SQChar *strText;
  sq_getstring(v, 2, &strText);
  sq_pushinteger(v, val.GetTextWidth(strText));
  return 1;
};

static SQInteger PutText(HSQUIRRELVM v, int, CDrawPort &val) {
  const SQChar *strText;
  sq_getstring(v, 2, &strText);

  SQInteger iX, iY, colBlend;
  sq_getinteger(v, 3, &iX);
  sq_getinteger(v, 4, &iY);
  sq_getinteger(v, 5, &colBlend);

  val.PutText(strText, iX, iY, colBlend);
  return 0;
};

static SQInteger PutTextC(HSQUIRRELVM v, int, CDrawPort &val) {
  const SQChar *strText;
  sq_getstring(v, 2, &strText);

  SQInteger iX, iY, colBlend;
  sq_getinteger(v, 3, &iX);
  sq_getinteger(v, 4, &iY);
  sq_getinteger(v, 5, &colBlend);

  val.PutTextC(strText, iX, iY, colBlend);
  return 0;
};

static SQInteger PutTextCXY(HSQUIRRELVM v, int, CDrawPort &val) {
  const SQChar *strText;
  sq_getstring(v, 2, &strText);

  SQInteger iX, iY, colBlend;
  sq_getinteger(v, 3, &iX);
  sq_getinteger(v, 4, &iY);
  sq_getinteger(v, 5, &colBlend);

  val.PutTextCXY(strText, iX, iY, colBlend);
  return 0;
};

static SQInteger PutTextR(HSQUIRRELVM v, int, CDrawPort &val) {
  const SQChar *strText;
  sq_getstring(v, 2, &strText);

  SQInteger iX, iY, colBlend;
  sq_getinteger(v, 3, &iX);
  sq_getinteger(v, 4, &iY);
  sq_getinteger(v, 5, &colBlend);

  val.PutTextR(strText, iX, iY, colBlend);
  return 0;
};

static SQInteger PutTexture(HSQUIRRELVM v, int, CDrawPort &val) {
  VM &vm = GetVMClass(v);

  GetInstanceValueVerify(CTextureObject, pto, v, 2);

  SQInteger iX0, iY0, iX1, iY1;
  if (!vm.ArrayPopFirst(3, &iX0)) return sq_throwerror(v, "cannot retrieve starting X position from the array in argument 2");
  if (!vm.ArrayPopFirst(3, &iY0)) return sq_throwerror(v, "cannot retrieve starting Y position from the array in argument 2");
  if (!vm.ArrayPopFirst(4, &iX1)) return sq_throwerror(v, "cannot retrieve ending X position from the array in argument 3");
  if (!vm.ArrayPopFirst(4, &iY1)) return sq_throwerror(v, "cannot retrieve ending Y position from the array in argument 3");

  const PIXaabbox2D boxTexture(PIX2D(iX0, iY0), PIX2D(iX1, iY1));
  SQInteger col0, col1, col2, col3;

  // One color
  if (SQ_SUCCEEDED(sq_getinteger(v, 5, &col0))) {
    val.PutTexture(pto, boxTexture, col0);

  // Color per corner
  } else {
    if (!vm.ArrayPopFirst(5, &col0)) return sq_throwerror(v, "cannot retrieve upper left color from the array in argument 4");
    if (!vm.ArrayPopFirst(5, &col1)) return sq_throwerror(v, "cannot retrieve upper right color from the array in argument 4");
    if (!vm.ArrayPopFirst(5, &col2)) return sq_throwerror(v, "cannot retrieve lower left color from the array in argument 4");
    if (!vm.ArrayPopFirst(5, &col3)) return sq_throwerror(v, "cannot retrieve lower right color from the array in argument 4");

    val.PutTexture(pto, boxTexture, col0, col1, col2, col3);
  }
  return 0;
};

static SQInteger PutTextureUV(HSQUIRRELVM v, int, CDrawPort &val) {
  VM &vm = GetVMClass(v);

  GetInstanceValueVerify(CTextureObject, pto, v, 2);

  SQInteger iX0, iY0, iX1, iY1;
  if (!vm.ArrayPopFirst(3, &iX0)) return sq_throwerror(v, "cannot retrieve starting X position from the array in argument 2");
  if (!vm.ArrayPopFirst(3, &iY0)) return sq_throwerror(v, "cannot retrieve starting Y position from the array in argument 2");
  if (!vm.ArrayPopFirst(4, &iX1)) return sq_throwerror(v, "cannot retrieve ending X position from the array in argument 3");
  if (!vm.ArrayPopFirst(4, &iY1)) return sq_throwerror(v, "cannot retrieve ending Y position from the array in argument 3");

  SQFloat fU0, fV0, fU1, fV1;
  if (!vm.ArrayPopFirst(5, &fU0)) return sq_throwerror(v, "cannot retrieve starting X position from the array in argument 4");
  if (!vm.ArrayPopFirst(5, &fV0)) return sq_throwerror(v, "cannot retrieve starting Y position from the array in argument 4");
  if (!vm.ArrayPopFirst(6, &fU1)) return sq_throwerror(v, "cannot retrieve ending X position from the array in argument 5");
  if (!vm.ArrayPopFirst(6, &fV1)) return sq_throwerror(v, "cannot retrieve ending Y position from the array in argument 5");

  const PIXaabbox2D boxTexture(PIX2D(iX0, iY0), PIX2D(iX1, iY1));
  const MEXaabbox2D boxUV(MEX2D(fU0, fV0), MEX2D(fU1, fV1));
  SQInteger col0, col1, col2, col3;

  // One color
  if (SQ_SUCCEEDED(sq_getinteger(v, 7, &col0))) {
    val.PutTexture(pto, boxTexture, boxUV, col0);

  // Color per corner
  } else {
    if (!vm.ArrayPopFirst(7, &col0)) return sq_throwerror(v, "cannot retrieve upper left color from the array in argument 6");
    if (!vm.ArrayPopFirst(7, &col1)) return sq_throwerror(v, "cannot retrieve upper right color from the array in argument 6");
    if (!vm.ArrayPopFirst(7, &col2)) return sq_throwerror(v, "cannot retrieve lower left color from the array in argument 6");
    if (!vm.ArrayPopFirst(7, &col3)) return sq_throwerror(v, "cannot retrieve lower right color from the array in argument 6");

    val.PutTexture(pto, boxTexture, boxUV, col0, col1, col2, col3);
  }
  return 0;
};

static SQInteger InitTexture(HSQUIRRELVM v, int, CDrawPort &val) {
  GetInstanceValueVerify(CTextureObject, pto, v, 2);

  SQBool bClamp;
  sq_getbool(v, 3, &bClamp);

  val.InitTexture(pto, bClamp);
  return 0;
};

static SQInteger AddTexture(HSQUIRRELVM v, int, CDrawPort &val) {
  VM &vm = GetVMClass(v);

  SQFloat fX0, fY0, fX1, fY1;
  if (!vm.ArrayPopFirst(2, &fX0)) return sq_throwerror(v, "cannot retrieve starting X position from the array in argument 1");
  if (!vm.ArrayPopFirst(2, &fY0)) return sq_throwerror(v, "cannot retrieve starting Y position from the array in argument 1");
  if (!vm.ArrayPopFirst(3, &fX1)) return sq_throwerror(v, "cannot retrieve ending X position from the array in argument 2");
  if (!vm.ArrayPopFirst(3, &fY1)) return sq_throwerror(v, "cannot retrieve ending Y position from the array in argument 2");

  SQInteger col;
  sq_getinteger(v, 4, &col);

  val.AddTexture(fX0, fY0, fX1, fY1, col);
  return 0;
};

static SQInteger AddTextureUV(HSQUIRRELVM v, int, CDrawPort &val) {
  VM &vm = GetVMClass(v);

  SQFloat fX0, fY0, fX1, fY1, fU0, fV0, fU1, fV1;
  if (!vm.ArrayPopFirst(2, &fX0)) return sq_throwerror(v, "cannot retrieve starting X position from the array in argument 1");
  if (!vm.ArrayPopFirst(2, &fY0)) return sq_throwerror(v, "cannot retrieve starting Y position from the array in argument 1");
  if (!vm.ArrayPopFirst(3, &fX1)) return sq_throwerror(v, "cannot retrieve ending X position from the array in argument 2");
  if (!vm.ArrayPopFirst(3, &fY1)) return sq_throwerror(v, "cannot retrieve ending Y position from the array in argument 2");

  if (!vm.ArrayPopFirst(4, &fU0)) return sq_throwerror(v, "cannot retrieve starting U coordinate from the array in argument 3");
  if (!vm.ArrayPopFirst(4, &fV0)) return sq_throwerror(v, "cannot retrieve starting V coordinate from the array in argument 3");
  if (!vm.ArrayPopFirst(5, &fU1)) return sq_throwerror(v, "cannot retrieve ending U coordinate from the array in argument 4");
  if (!vm.ArrayPopFirst(5, &fV1)) return sq_throwerror(v, "cannot retrieve ending V coordinate from the array in argument 4");

  SQInteger col;
  sq_getinteger(v, 6, &col);

  val.AddTexture(fX0, fY0, fX1, fY1, fU0, fV0, fU1, fV1, col);
  return 0;
};

static SQInteger AddTexture_4points(HSQUIRRELVM v, int, CDrawPort &val) {
  VM &vm = GetVMClass(v);

  SQFloat fX0, fY0, fU0, fV0, fX1, fY1, fU1, fV1, fX2, fY2, fU2, fV2, fX3, fY3, fU3, fV3;
  if (!vm.ArrayPopFirst(2, &fX0)) return sq_throwerror(v, "cannot retrieve X0 position from the array in argument 1");
  if (!vm.ArrayPopFirst(2, &fY0)) return sq_throwerror(v, "cannot retrieve Y0 position from the array in argument 1");
  if (!vm.ArrayPopFirst(3, &fU0)) return sq_throwerror(v, "cannot retrieve U0 coordinate from the array in argument 2");
  if (!vm.ArrayPopFirst(3, &fV0)) return sq_throwerror(v, "cannot retrieve V0 coordinate from the array in argument 2");

  if (!vm.ArrayPopFirst(4, &fX1)) return sq_throwerror(v, "cannot retrieve X1 position from the array in argument 3");
  if (!vm.ArrayPopFirst(4, &fY1)) return sq_throwerror(v, "cannot retrieve Y1 position from the array in argument 3");
  if (!vm.ArrayPopFirst(5, &fU1)) return sq_throwerror(v, "cannot retrieve U1 coordinate from the array in argument 4");
  if (!vm.ArrayPopFirst(5, &fV1)) return sq_throwerror(v, "cannot retrieve V1 coordinate from the array in argument 4");

  if (!vm.ArrayPopFirst(6, &fX2)) return sq_throwerror(v, "cannot retrieve X2 position from the array in argument 5");
  if (!vm.ArrayPopFirst(6, &fY2)) return sq_throwerror(v, "cannot retrieve Y2 position from the array in argument 5");
  if (!vm.ArrayPopFirst(7, &fU2)) return sq_throwerror(v, "cannot retrieve U2 coordinate from the array in argument 6");
  if (!vm.ArrayPopFirst(7, &fV2)) return sq_throwerror(v, "cannot retrieve V2 coordinate from the array in argument 6");

  if (!vm.ArrayPopFirst(8, &fX3)) return sq_throwerror(v, "cannot retrieve X3 position from the array in argument 7");
  if (!vm.ArrayPopFirst(8, &fY3)) return sq_throwerror(v, "cannot retrieve Y3 position from the array in argument 7");
  if (!vm.ArrayPopFirst(9, &fU3)) return sq_throwerror(v, "cannot retrieve U3 coordinate from the array in argument 8");
  if (!vm.ArrayPopFirst(9, &fV3)) return sq_throwerror(v, "cannot retrieve V3 coordinate from the array in argument 8");

  SQInteger col0, col1, col2, col3;
  sq_getinteger(v, 10, &col0);
  sq_getinteger(v, 11, &col1);
  sq_getinteger(v, 12, &col2);
  sq_getinteger(v, 13, &col3);

  val.AddTexture(fX0, fY0, fU0, fV0, fX1, fY1, fU1, fV1, fX2, fY2, fU2, fV2, fX3, fY3, fU3, fV3, col0, col1, col2, col3);
  return 0;
};

static SQInteger FlushRenderingQueue(HSQUIRRELVM v, int, CDrawPort &val) {
  val.FlushRenderingQueue();
  return 0;
};

#if SE1_VER > SE1_105

static SQInteger DrawPoint(HSQUIRRELVM v, int ctArgs, CDrawPort &val) {
  SQInteger iX, iY, col;
  sq_getinteger(v, 2, &iX);
  sq_getinteger(v, 3, &iY);
  sq_getinteger(v, 4, &col);

  SQInteger iRadius = 1;
  if (ctArgs > 3) sq_getinteger(v, 5, &iRadius);

  val.DrawPoint(iX, iY, col, iRadius);
  return 0;
};

#endif

static SQInteger DrawLine(HSQUIRRELVM v, int ctArgs, CDrawPort &val) {
  SQInteger iX1, iY1, iX2, iY2, col;
  sq_getinteger(v, 2, &iX1);
  sq_getinteger(v, 3, &iY1);
  sq_getinteger(v, 4, &iX2);
  sq_getinteger(v, 5, &iY2);
  sq_getinteger(v, 6, &col);

  SQInteger iLine = _FULL_;
  if (ctArgs > 5) sq_getinteger(v, 7, &iLine);

  val.DrawLine(iX1, iY1, iX2, iY2, col, iLine);
  return 0;
};

static SQInteger DrawBorder(HSQUIRRELVM v, int ctArgs, CDrawPort &val) {
  SQInteger iX, iY, iW, iH, col;
  sq_getinteger(v, 2, &iX);
  sq_getinteger(v, 3, &iY);
  sq_getinteger(v, 4, &iW);
  sq_getinteger(v, 5, &iH);
  sq_getinteger(v, 6, &col);

  SQInteger iLine = _FULL_;
  if (ctArgs > 5) sq_getinteger(v, 7, &iLine);

  val.DrawBorder(iX, iY, iW, iH, col, iLine);
  return 0;
};

static SQInteger Fill(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iX, iY, iW, iH, col;
  sq_getinteger(v, 2, &iX);
  sq_getinteger(v, 3, &iY);
  sq_getinteger(v, 4, &iW);
  sq_getinteger(v, 5, &iH);
  sq_getinteger(v, 6, &col);
  val.Fill(iX, iY, iW, iH, col);
  return 0;
};

static SQInteger FillGradient(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iX, iY, iW, iH, colUL, colUR, colDL, colDR;
  sq_getinteger(v, 2, &iX);
  sq_getinteger(v, 3, &iY);
  sq_getinteger(v, 4, &iW);
  sq_getinteger(v, 5, &iH);
  sq_getinteger(v, 6, &colUL);
  sq_getinteger(v, 7, &colUR);
  sq_getinteger(v, 8, &colDL);
  sq_getinteger(v, 9, &colDR);
  val.Fill(iX, iY, iW, iH, colUL, colUR, colDL, colDR);
  return 0;
};

static SQInteger FillScreen(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger col;
  sq_getinteger(v, 2, &col);
  val.Fill(col);
  return 0;
};

static Method<CDrawPort> _aMethods[] = {
  { "Equal", &Equal, 2, ".x" },

  // Resolution info
  { "IsDualHead",   &IsDualHead,   1, "." },
  { "IsWideScreen", &IsWideScreen, 1, "." },
  { "GetWidth",     &GetWidth,     1, "." },
  { "GetHeight",    &GetHeight,    1, "." },

  // Font setup
  { "SetFont",            &SetFont,            2, ".x" },
  { "SetTextCharSpacing", &SetTextCharSpacing, 2, ".n" },
  { "SetTextLineSpacing", &SetTextLineSpacing, 2, ".n" },
  { "SetTextScaling",     &SetTextScaling,     2, ".n" },
  { "SetTextAspect",      &SetTextAspect,      2, ".n" },
  { "SetTextMode",        &SetTextMode,        2, ".n" },

  // Text
  { "GetTextWidth", &GetTextWidth, 2, ".s" },
  { "PutText",      &PutText,      5, ".snnn" },
  { "PutTextC",     &PutTextC,     5, ".snnn" },
  { "PutTextCXY",   &PutTextCXY,   5, ".snnn" },
  { "PutTextR",     &PutTextR,     5, ".snnn" },

  // Textures
  { "PutTexture",          &PutTexture,          5, ".xaan|a" },
  { "PutTextureUV",        &PutTextureUV,        7, ".xaaaan|a" },
  { "InitTexture",         &InitTexture,         3, ".xb" },
  { "AddTexture",          &AddTexture,          4, ".aan" },
  { "AddTextureUV",        &AddTextureUV,        6, ".aaaan" },
  { "AddTexture_4points",  &AddTexture_4points, 13, ".aaaaaaaannnn" },
  { "FlushRenderingQueue", &FlushRenderingQueue, 1, "." },

  // Shapes
#if SE1_VER > SE1_105
  { "DrawPoint",    &DrawPoint,   -4, ".nnnn" },
#endif
  { "DrawLine",     &DrawLine,    -6, ".nnnnnn" },
  { "DrawBorder",   &DrawBorder,  -6, ".nnnnnn" },
  { "Fill",         &Fill,         6, ".nnnnn" },
  { "FillGradient", &FillGradient, 9, ".nnnnnnnn" },
  { "FillScreen",   &FillScreen,   2, ".n" },
};

}; // namespace

namespace Core {

static SQInteger GetVersion(HSQUIRRELVM v) {
  UBYTE ubRelease, ubUpdate, ubPatch;
  GetVersionNumbers(ClassicsCore_GetVersion(), ubRelease, ubUpdate, ubPatch);

  sq_newarray(v, 0);
  sq_pushinteger(v, ubRelease);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, ubUpdate);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, ubPatch);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger GetVersionName(HSQUIRRELVM v) {
  sq_pushstring(v, ClassicsCore_GetVersionName(), -1);
  return 1;
};

static SQInteger GetInitTime(HSQUIRRELVM v) {
  Table sqtTimer(GetVMClass(v).Root().GetValue("Timer"));
  PushNewInstance(CTimerValue, ptv, sqtTimer, "Value");
  *ptv = CTimerValue(ClassicsCore_GetInitTime());
  return 1;
};

static SQInteger IsGame(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsGameApp());
  return 1;
};

static SQInteger IsServer(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsServerApp());
  return 1;
};

static SQInteger IsEditor(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsEditorApp());
  return 1;
};

static SQInteger IsModeler(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsModelerApp());
  return 1;
};

static SQInteger GetSeason(HSQUIRRELVM v) {
  sq_pushinteger(v, ClassicsCore_GetSeason());
  return 1;
};

static SQInteger IsCustomModActive(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsCustomModActive());
  return 1;
};

static SQInteger IsEntitiesModded(HSQUIRRELVM v) {
  sq_pushbool(v, ClassicsCore_IsEntitiesModded());
  return 1;
};

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
  if (!vm.CompileFromFile(strFile)) return SQ_ERROR;

  // Return compiled closure
  return 1;
};

}; // namespace

// "Core" namespace functions
static SQRegFunction _aCoreFuncs[] = {
  { "GetVersion",        &Core::GetVersion,        1, "." },
  { "GetVersionName",    &Core::GetVersionName,    1, "." },
  { "GetInitTime",       &Core::GetInitTime,       1, "." },
  { "IsGame",            &Core::IsGame,            1, "." },
  { "IsServer",          &Core::IsServer,          1, "." },
  { "IsEditor",          &Core::IsEditor,          1, "." },
  { "IsModeler",         &Core::IsModeler,         1, "." },
  { "GetSeason",         &Core::GetSeason,         1, "." },
  { "IsCustomModActive", &Core::IsCustomModActive, 1, "." },
  { "IsEntitiesModded",  &Core::IsEntitiesModded,  1, "." },
};

// Global functions
static SQRegFunction _aGlobalFuncs[] = {
  { "IncludeScript", &Core::IncludeScript, 2, ".s" },
  { "CompileScript", &Core::CompileScript, 2, ".s" },
};

void VM::RegisterCore(void) {
  Table sqtCore = Root().RegisterTable("Core");
  INDEX i;

  // Register classes
  {
    Class<CRawDataBuffer> sqcBuffer(GetVM(), "CRawDataBuffer", &SqBuffer::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqBuffer::_aMethods); i++) {
      sqcBuffer.RegisterMethod(SqBuffer::_aMethods[i]);
    }

    // Metamethods
    sqcBuffer.RegisterMetamethod(E_MM_TOSTRING, &SqBuffer::ToString);

    Root().AddClass(sqcBuffer);
  }
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
    sqcCastRay.RegisterVar("bHitPortals",              &SqCastRay::GetFlagPortals,      &SqCastRay::SetFlagPortals);
    sqcCastRay.RegisterVar("bHitTranslucentPortals",   &SqCastRay::GetFlagTransPortals, &SqCastRay::SetFlagTransPortals);
    sqcCastRay.RegisterVar("ttHitModels",              &SqCastRay::GetTestType,         &SqCastRay::SetTestType);
    sqcCastRay.RegisterVar("bHitFields",               &SqCastRay::GetFlagFields,       &SqCastRay::SetFlagFields);
  #if SE1_VER >= SE1_107
    sqcCastRay.RegisterVar("bHitBrushes",              &SqCastRay::GetFlagBrushes,      &SqCastRay::SetFlagBrushes);
    sqcCastRay.RegisterVar("bHitTerrainInvisibleTris", &SqCastRay::GetFlagInvisTris,    &SqCastRay::SetFlagInvisTris);
  #endif
    sqcCastRay.RegisterVar("bPhysical",                &SqCastRay::GetFlagPhysical,     &SqCastRay::SetFlagPhysical);
    sqcCastRay.RegisterVar("fTestR",                   &SqCastRay::GetTestR,            &SqCastRay::SetTestR);

    Root().AddClass(sqcCastRay);
  }
  {
    Class<CDrawPort> sqcDrawPort(GetVM(), "CDrawPort", &SqDrawPort::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqDrawPort::_aMethods); i++) {
      sqcDrawPort.RegisterMethod(SqDrawPort::_aMethods[i]);
    }

    Root().AddClass(sqcDrawPort);
  }

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aCoreFuncs); i++) {
    sqtCore.RegisterFunc(_aCoreFuncs[i]);
  }

  for (i = 0; i < ARRAYCOUNT(_aGlobalFuncs); i++) {
    Root().RegisterFunc(_aGlobalFuncs[i]);
  }

  // Classics Patch seasons
  Enumeration enSeasons(GetVM());

#define ADD_SEASON(_SeasonType) enSeasons.RegisterValue(#_SeasonType, (SQInteger)k_EClassicsPatchSeason_##_SeasonType)
  ADD_SEASON(None);
  ADD_SEASON(Valentine);
  ADD_SEASON(Birthday);
  ADD_SEASON(Halloween);
  ADD_SEASON(Christmas);
  ADD_SEASON(Anniversary);
  ADD_SEASON(Max);
#undef ADD_SEASON

  Const().AddEnum("Season", enSeasons);

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

  // Pre-defined line types
  Enumeration enLineTypes(GetVM());

  enLineTypes.RegisterValue("Full",         (SQInteger)_FULL_);
  enLineTypes.RegisterValue("Symmetric_16", (SQInteger)_SYMMET16_);
  enLineTypes.RegisterValue("Symmetric_32", (SQInteger)_SYMMET32_);
  enLineTypes.RegisterValue("Point",        (SQInteger)_POINT_);

  enLineTypes.RegisterValue("Dot_2",  (SQInteger)_DOT2_);
  enLineTypes.RegisterValue("Dot_4",  (SQInteger)_DOT4_);
  enLineTypes.RegisterValue("Dot_8",  (SQInteger)_DOT8_);
  enLineTypes.RegisterValue("Dot_16", (SQInteger)_DOT16_);

  enLineTypes.RegisterValue("Pattern_3_1",  (SQInteger)_TY31_);
  enLineTypes.RegisterValue("Pattern_6_2",  (SQInteger)_TY62_);
  enLineTypes.RegisterValue("Pattern_12_4", (SQInteger)_TY124_);
  enLineTypes.RegisterValue("Pattern_1_3",  (SQInteger)_TY13_);
  enLineTypes.RegisterValue("Pattern_2_6",  (SQInteger)_TY26_);
  enLineTypes.RegisterValue("Pattern_4_12", (SQInteger)_TY412_);

  Const().AddEnum("LineMask", enLineTypes);
};

}; // namespace
