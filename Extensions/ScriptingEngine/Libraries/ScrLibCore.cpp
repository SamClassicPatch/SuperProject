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
#include <Extras/cJSON/cJSON.h>

namespace sq {

// Buffer class methods
namespace SqBuffer {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CRawDataBuffer &val) {
  // Ignore constructor arguments
  if (GetVMClass(v).GetArgumentBypass()) return 0;

  SQInteger iBytes = 0;

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
};

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

static SQInteger TextFitsInWidth(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iMaxWidth;
  sq_getinteger(v, 2, &iMaxWidth);
  const SQChar *str;
  sq_getstring(v, 3, &str);

  sq_pushinteger(v, IData::TextFitsInWidth(&val, iMaxWidth, str));
  return 1;
};

static SQInteger FormatStringForWidth(HSQUIRRELVM v, int, CDrawPort &val) {
  SQInteger iMaxWidth;
  sq_getinteger(v, 2, &iMaxWidth);
  const SQChar *str;
  sq_getstring(v, 3, &str);

  CTString strFormatted = IData::FormatStringForWidth(&val, iMaxWidth, str);
  sq_pushstring(v, strFormatted.str_String, -1);
  return 1;
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
  { "TextFitsInWidth",      &TextFitsInWidth,      3, ".ns" },
  { "FormatStringForWidth", &FormatStringForWidth, 3, ".ns" },

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

// Local storage class methods
namespace SqLocalStorage {

struct LocalStorage {
  CTString strName;
  CTString strFile;
  cJSON *pJSON;

  LocalStorage() : pJSON(NULL) {};

  ~LocalStorage() {
    if (pJSON != NULL) cJSON_Delete(pJSON);
  };
};

// Get JSON parsing error message
static const char *GetParseError(int iCode) {
  switch (iCode) {
    case cJSON_ParseCode_OK: return "OK";
    case cJSON_ParseCode_Empty: return "String buffer contents are empty";
    case cJSON_ParseCode_MemoryFail: return "Failed to allocate memory";
    case cJSON_ParseCode_ParseFail: return "Failed to parse a value";
    case cJSON_ParseCode_NotNullTerminated: return "String buffer isn't null-terminated";
  }

  return "Unknown error code";
};

// Resolve a directory-like path to access the last possible object, returning the last key instead of accessing a value under it
static cJSON *FindLastObject(cJSON *pObject, const char *strPath, BOOL bCreateNew, CTString &strLastKey) {
  // Split the path into keys
  CStringStack aKeys;
  IData::GetStrings(aKeys, strPath, '/');

  const INDEX iLast = aKeys.Count() - 1; // Go until the last key instead of the end of the array

  // Nothing
  if (iLast == -1) {
    strLastKey = "";
    return NULL;
  }

  // Make sure none of the keys are empty, otherwise it's an invalid path
  for (INDEX iCheck = 0; iCheck < iLast; iCheck++) {
    if (aKeys[iCheck] == "") return NULL;
  }

  cJSON *pValue = pObject;

  // Get the next value inside the series of nested objects
  for (INDEX i = 0; i < iLast; i++) {
    const CTString &strKey = aKeys[i];

    cJSON *pPrev = pValue;
    pValue = cJSON_GetObjectItem(pPrev, strKey);

    // It must always be an object in order to keep going
    if (!cJSON_IsObject(pValue)) {
      // Create a new one, if needed
      if (bCreateNew) {
        cJSON_DeleteItemFromObject(pPrev, strKey);
        pValue = cJSON_AddObjectToObject(pPrev, strKey);

      } else {
        strLastKey = "";
        return NULL;
      }
    }
  }

  // Return last object that should contain the last key
  strLastKey = aKeys[iLast];
  return pValue;
};

// Access a specific value inside a JSON object using a directory-like path
static cJSON *ResolvePath(cJSON *pObject, const char *strPath) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, FALSE, strLastKey);

  // No object or no key in order to return the last value
  if (pLastObject == NULL || strLastKey == "") return NULL;

  return cJSON_GetObjectItem(pLastObject, strLastKey);
};

// Create a new boolean value at the specified directory-like path
static BOOL CreateBoolAt(cJSON *pObject, const char *strPath, SQBool bValue) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return FALSE; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddBoolToObject(pLastObject, strLastKey, bValue) != NULL;
};

// Create a new null value at the specified directory-like path
static BOOL CreateNullAt(cJSON *pObject, const char *strPath) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return FALSE; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddNullToObject(pLastObject, strLastKey) != NULL;
};

// Create a new number value at the specified directory-like path
static BOOL CreateNumberAt(cJSON *pObject, const char *strPath, SQFloat fValue) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return FALSE; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddNumberToObject(pLastObject, strLastKey, fValue) != NULL;
};

// Create a new string value at the specified directory-like path
static BOOL CreateStringAt(cJSON *pObject, const char *strPath, const SQChar *strValue) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return FALSE; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddStringToObject(pLastObject, strLastKey, strValue) != NULL;
};

// Create a new array value at the specified directory-like path
static cJSON *CreateArrayAt(cJSON *pObject, const char *strPath) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return NULL; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddArrayToObject(pLastObject, strLastKey);
};

// Create a new object value at the specified directory-like path
static cJSON *CreateObjectAt(cJSON *pObject, const char *strPath) {
  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(pObject, strPath, TRUE, strLastKey);
  if (pLastObject == NULL || strLastKey == "") return NULL; // No object or no key

  // Delete last value
  if (cJSON_GetObjectItem(pLastObject, strLastKey) != NULL) cJSON_DeleteItemFromObject(pLastObject, strLastKey);

  return cJSON_AddObjectToObject(pLastObject, strLastKey);
};

// Make sure local storage contains a usable JSON object
#define ASSERT_JSON { if (val.pJSON == NULL || !cJSON_IsObject(val.pJSON)) return sq_throwerror(v, "local storage instance isn't a JSON object"); }

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, LocalStorage &val) {
  const SQChar *str;

  if (ctArgs == 0 || SQ_FAILED(sq_getstring(v, 2, &str))) {
    return sq_throwerror(v, "expected storage name in argument 1");
  }

  // Convert storage name into only alphanumeric characters, underscores and spaces
  val.strName = str;
  char *pch = val.strName.str_String;

  while (*pch != '\0') {
    if (!isalnum((UBYTE)*pch) && *pch != '_' && *pch != ' ') {
      *pch = '_';
    }
    pch++;
  }

  // Compose path to the storage file
  val.strFile = "Data\\ClassicsPatch\\SquirrelLocalStorage\\" + val.strName + ".json";

  // Try to load the contents of existing storage
  if (FileExists(val.strFile)) {
    char *strBuffer = NULL;
    SLONG slSize = 0;

    // Read contents of the file into a buffer
    try {
      CTFileStream strm;
      strm.Open_t(val.strFile);

      slSize = strm.GetStreamSize();
      strBuffer = (char *)AllocMemory(slSize + 1);

      strm.Read_t(strBuffer, slSize);
      strBuffer[slSize] = '\0';

    } catch(char *strError) {
      if (strBuffer != NULL) FreeMemory(strBuffer);

      SQChar strPrintError[256];
      scsprintf(strPrintError, 256, "cannot load existing local storage '%s': %s", val.strName.str_String, strError);
      return sq_throwerror(v, strPrintError);
    }

    // Couldn't read the contents for some reason but otherwise didn't fail
    if (strBuffer == NULL) {
      SQChar strError[256];
      scsprintf(strError, 256, "cannot load existing local storage '%s': %s", val.strName.str_String, "Empty buffer contents");
      return sq_throwerror(v, strError);
    }

    // Parse buffer contents as JSON
    const char *pchParseEnd = NULL;
    int iErrorCode;

    val.pJSON = cJSON_ParseWithOpts(strBuffer, &pchParseEnd, TRUE, &iErrorCode);

    const SLONG slOffset = (pchParseEnd != NULL) ? (pchParseEnd - strBuffer) + 1 : 0;
    FreeMemory(strBuffer);

    // Failed
    if (val.pJSON == NULL) {
      SQChar strError[256];
      scsprintf(strError, 256, "cannot parse local storage '%s' at character %d: %s", val.strName.str_String, slOffset, GetParseError(iErrorCode));
      return sq_throwerror(v, strError);
    }

    // Not an object
    if (!cJSON_IsObject(val.pJSON)) {
      SQChar strError[256];
      scsprintf(strError, 256, "cannot load existing local storage '%s': %s", val.strName.str_String, "Not a JSON object");
      return sq_throwerror(v, strError);
    }

  // Create a new JSON structure
  } else {
    val.pJSON = cJSON_CreateObject();

    // Failed
    if (val.pJSON == NULL) {
      SQChar strError[256];
      scsprintf(strError, 256, "cannot create new local storage '%s'", val.strName.str_String);
      return sq_throwerror(v, strError);
    }
  }

  return 0;
};

static SQInteger ToString(HSQUIRRELVM v, LocalStorage &val) {
  ASSERT_JSON;
  char *strPrint = cJSON_Print(val.pJSON);

  if (strPrint == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot convert local storage '%s' to string", val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, strPrint, -1);
  cJSON_free(strPrint); // Free printed JSON
  return 1;
};

static SQInteger Save(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;
  char *strPrint = cJSON_PrintUnformatted(val.pJSON);

  if (strPrint == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot convert local storage '%s' to string", val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  // Create directories to the local storage file
  IDir::CreateDir(val.strFile);

  // Write local storage to disk
  try {
    CTFileStream strm;
    strm.Create_t(val.strFile);
    strm.PutString_t(strPrint);

  } catch (char *strError) {
    cJSON_free(strPrint); // Free printed JSON

    SQChar strPrintError[256];
    scsprintf(strPrintError, 256, "cannot save local storage '%s': %s", val.strName.str_String, strError);
    return sq_throwerror(v, strPrintError);
  }

  cJSON_free(strPrint); // Free printed JSON
  return 0;
};

static SQInteger Clear(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  // Recreate the entire object
  cJSON_Delete(val.pJSON);
  val.pJSON = cJSON_CreateObject();

  // Failed
  if (val.pJSON == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot create new local storage '%s'", val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger KeyExists(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, (pValue != NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger DeleteKey(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  CTString strLastKey;
  cJSON *pLastObject = FindLastObject(val.pJSON, strPath, FALSE, strLastKey);

  cJSON_DeleteItemFromObject(pLastObject, strLastKey);
  return 0;
};

#define LOCALSTORAGE_CHECK_TYPE(_FuncName) \
  static SQInteger _FuncName(HSQUIRRELVM v, int, LocalStorage &val) { \
    ASSERT_JSON; \
    const SQChar *strPath; \
    sq_getstring(v, 2, &strPath); \
    cJSON *pValue = ResolvePath(val.pJSON, strPath); \
    sq_pushbool(v, cJSON_##_FuncName(pValue) ? SQTrue : SQFalse); \
    return 1; \
  }

// All of these will simply return false if the key cannot be found at the specified path
LOCALSTORAGE_CHECK_TYPE(IsFalse);
LOCALSTORAGE_CHECK_TYPE(IsTrue);
LOCALSTORAGE_CHECK_TYPE(IsBool);
LOCALSTORAGE_CHECK_TYPE(IsNull);
LOCALSTORAGE_CHECK_TYPE(IsNumber);
LOCALSTORAGE_CHECK_TYPE(IsString);
LOCALSTORAGE_CHECK_TYPE(IsArray);
LOCALSTORAGE_CHECK_TYPE(IsObject);

#undef LOCALSTORAGE_CHECK_TYPE

static BOOL VerifyVector(cJSON *pArray, FLOAT3D *pvValue) {
  if (!cJSON_IsArray(pArray)) return FALSE;

  // Should have exactly 3 values
  if (cJSON_GetArraySize(pArray) != 3) return FALSE;

  // All values must be numbers
  cJSON *p0 = cJSON_GetArrayItem(pArray, 0);
  if (!cJSON_IsNumber(p0)) return FALSE;

  cJSON *p1 = cJSON_GetArrayItem(pArray, 1);
  if (!cJSON_IsNumber(p1)) return FALSE;

  cJSON *p2 = cJSON_GetArrayItem(pArray, 2);
  if (!cJSON_IsNumber(p2)) return FALSE;

  if (pvValue != NULL) {
    (*pvValue)(1) = cJSON_GetNumberValue(p0);
    (*pvValue)(2) = cJSON_GetNumberValue(p1);
    (*pvValue)(3) = cJSON_GetNumberValue(p2);
  }
  return TRUE;
};

static BOOL VerifyPlane(cJSON *pArray, FLOATplane3D *pplValue) {
  if (!cJSON_IsArray(pArray)) return FALSE;

  // Should have exactly 2 values
  if (cJSON_GetArraySize(pArray) != 2) return FALSE;

  // Values must be a vector followed by a number
  FLOAT3D *pvVector = (pplValue != NULL ? (FLOAT3D *)pplValue : NULL);
  if (!VerifyVector(cJSON_GetArrayItem(pArray, 0), pvVector)) return FALSE;

  cJSON *pDist = cJSON_GetArrayItem(pArray, 1);
  if (!cJSON_IsNumber(pDist)) return FALSE;
  if (pplValue != NULL) pplValue->Distance() = cJSON_GetNumberValue(pDist);

  return TRUE;
};

static BOOL VerifyMatrix(cJSON *pArray, FLOATmatrix3D *pmValue) {
  if (!cJSON_IsArray(pArray)) return FALSE;

  // Should have exactly 9 values
  if (cJSON_GetArraySize(pArray) != 9) return FALSE;

  // All values must be numbers
  cJSON *p11 = cJSON_GetArrayItem(pArray, 0);
  if (!cJSON_IsNumber(p11)) return FALSE;
  cJSON *p12 = cJSON_GetArrayItem(pArray, 1);
  if (!cJSON_IsNumber(p12)) return FALSE;
  cJSON *p13 = cJSON_GetArrayItem(pArray, 2);
  if (!cJSON_IsNumber(p13)) return FALSE;

  cJSON *p21 = cJSON_GetArrayItem(pArray, 3);
  if (!cJSON_IsNumber(p21)) return FALSE;
  cJSON *p22 = cJSON_GetArrayItem(pArray, 4);
  if (!cJSON_IsNumber(p22)) return FALSE;
  cJSON *p23 = cJSON_GetArrayItem(pArray, 5);
  if (!cJSON_IsNumber(p23)) return FALSE;

  cJSON *p31 = cJSON_GetArrayItem(pArray, 6);
  if (!cJSON_IsNumber(p31)) return FALSE;
  cJSON *p32 = cJSON_GetArrayItem(pArray, 7);
  if (!cJSON_IsNumber(p32)) return FALSE;
  cJSON *p33 = cJSON_GetArrayItem(pArray, 8);
  if (!cJSON_IsNumber(p33)) return FALSE;

  if (pmValue != NULL) {
    (*pmValue)(1, 1) = cJSON_GetNumberValue(p11);
    (*pmValue)(1, 2) = cJSON_GetNumberValue(p12);
    (*pmValue)(1, 3) = cJSON_GetNumberValue(p13);

    (*pmValue)(2, 1) = cJSON_GetNumberValue(p21);
    (*pmValue)(2, 2) = cJSON_GetNumberValue(p22);
    (*pmValue)(2, 3) = cJSON_GetNumberValue(p23);

    (*pmValue)(3, 1) = cJSON_GetNumberValue(p31);
    (*pmValue)(3, 2) = cJSON_GetNumberValue(p32);
    (*pmValue)(3, 3) = cJSON_GetNumberValue(p33);
  }
  return TRUE;
};

static BOOL VerifyAABB(cJSON *pArray, FLOATaabbox3D *pboxValue) {
  if (!cJSON_IsArray(pArray)) return FALSE;

  // Should have exactly 6 values
  if (cJSON_GetArraySize(pArray) != 6) return FALSE;

  // All values must be numbers
  cJSON *pMin1 = cJSON_GetArrayItem(pArray, 0);
  if (!cJSON_IsNumber(pMin1)) return FALSE;
  cJSON *pMin2 = cJSON_GetArrayItem(pArray, 1);
  if (!cJSON_IsNumber(pMin2)) return FALSE;
  cJSON *pMin3 = cJSON_GetArrayItem(pArray, 2);
  if (!cJSON_IsNumber(pMin3)) return FALSE;

  cJSON *pMax1 = cJSON_GetArrayItem(pArray, 3);
  if (!cJSON_IsNumber(pMax1)) return FALSE;
  cJSON *pMax2 = cJSON_GetArrayItem(pArray, 4);
  if (!cJSON_IsNumber(pMax2)) return FALSE;
  cJSON *pMax3 = cJSON_GetArrayItem(pArray, 5);
  if (!cJSON_IsNumber(pMax3)) return FALSE;

  if (pboxValue != NULL) {
    pboxValue->minvect(1) = cJSON_GetNumberValue(pMin1);
    pboxValue->minvect(2) = cJSON_GetNumberValue(pMin2);
    pboxValue->minvect(3) = cJSON_GetNumberValue(pMin3);

    pboxValue->maxvect(1) = cJSON_GetNumberValue(pMax1);
    pboxValue->maxvect(2) = cJSON_GetNumberValue(pMax2);
    pboxValue->maxvect(3) = cJSON_GetNumberValue(pMax3);
  }
  return TRUE;
};

static BOOL VerifyPlacement(cJSON *pArray, CPlacement3D *pplValue) {
  if (!cJSON_IsArray(pArray)) return FALSE;

  // Should have exactly 2 values
  if (cJSON_GetArraySize(pArray) != 2) return FALSE;

  // All values must be vectors
  FLOAT3D *pvPos = (pplValue != NULL ? &pplValue->pl_PositionVector : NULL);
  if (!VerifyVector(cJSON_GetArrayItem(pArray, 0), pvPos)) return FALSE;

  FLOAT3D *pvRot = (pplValue != NULL ? &pplValue->pl_OrientationAngle : NULL);
  if (!VerifyVector(cJSON_GetArrayItem(pArray, 1), pvRot)) return FALSE;

  return TRUE;
};

static SQInteger IsVector(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, VerifyVector(pValue, NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger IsPlane(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, VerifyPlane(pValue, NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger IsMatrix(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, VerifyMatrix(pValue, NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger IsAABB(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, VerifyAABB(pValue, NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger IsPlacement(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  sq_pushbool(v, VerifyPlacement(pValue, NULL) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger SetBool(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  SQBool bValue;
  sq_getbool(v, 3, &bValue);

  if (!CreateBoolAt(val.pJSON, strPath, bValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create a boolean value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger SetNull(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  if (!CreateNullAt(val.pJSON, strPath)) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create a null value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger SetNumber(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  SQFloat fValue;
  sq_getfloat(v, 3, &fValue);

  if (!CreateNumberAt(val.pJSON, strPath, fValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create a number value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger SetString(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  const SQChar *strValue;
  sq_getstring(v, 3, &strValue);

  if (!CreateStringAt(val.pJSON, strPath, strValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create a string value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger PopulateJSONArray(HSQUIRRELVM v, SQInteger idxArray, cJSON *pArray);
static SQInteger PopulateJSONObject(HSQUIRRELVM v, SQInteger idxObject, cJSON *pObject);
static BOOL ConvertVectorToJSONArray(const FLOAT3D &v, cJSON *pArray);
static BOOL ConvertPlaneToJSONArray(const FLOATplane3D &pl, cJSON *pArray);
static BOOL ConvertMatrixToJSONArray(const FLOATmatrix3D &m, cJSON *pArray);
static BOOL ConvertAABBToJSONArray(const FLOATaabbox3D &box, cJSON *pArray);
static BOOL ConvertPlacementToJSONArray(const CPlacement3D &pl, cJSON *pArray);

SQInteger PopulateJSONArray(HSQUIRRELVM v, SQInteger idxArray, cJSON *pArray) {
  if (pArray == NULL) return sq_throwerror(v, "no JSON array to populate");

  // Iterate through the object
  sq_pushnull(v); // Null iterator

  // Shift the top index because of the null iterator
  if (idxArray < 0) idxArray--;

  // Temporary pointer for the last created value
  cJSON *pNewValue = NULL;

  while (SQ_SUCCEEDED(sq_next(v, idxArray))) {
    switch (sq_gettype(v, -1))
    {
      case OT_NULL: {
        pNewValue = cJSON_CreateNull();
        if (!cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
      } break;

      case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(v, -1, &i);
        pNewValue = cJSON_CreateNumber(i);
        if (!cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
      } break;

      case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(v, -1, &f);
        pNewValue = cJSON_CreateNumber(f);
        if (!cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
      } break;

      case OT_BOOL: {
        SQBool b;
        sq_getbool(v, -1, &b);
        pNewValue = cJSON_CreateBool(b);
        if (!cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
      } break;

      case OT_STRING: {
        const SQChar *str;
        sq_getstring(v, -1, &str);
        pNewValue = cJSON_CreateString(str);
        if (!cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
      } break;

      case OT_ARRAY: {
        cJSON *pNewArray = cJSON_CreateArray();
        if (SQ_FAILED(PopulateJSONArray(v, -1, pNewArray))) goto failure;
        if (!cJSON_AddItemToArray(pArray, pNewArray)) goto failure;
      } break;

      case OT_TABLE: {
        cJSON *pNewObject = cJSON_CreateObject();
        if (SQ_FAILED(PopulateJSONObject(v, -1, pNewObject))) goto failure;
        if (!cJSON_AddItemToArray(pArray, pNewObject)) goto failure;
      } break;

      case OT_INSTANCE: {
        pNewValue = cJSON_CreateArray();

        // Try plane (before vector because it's derived from FLOAT3D)
        GetInstanceValue(FLOATplane3D, ppl, v, -1);

        if (ppl != NULL) {
          if (!ConvertPlaneToJSONArray(*ppl, pNewValue)
           || !cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
          break;
        }

        // Try vector
        GetInstanceValue(FLOAT3D, pv, v, -1);

        if (pv != NULL) {
          if (!ConvertVectorToJSONArray(*pv, pNewValue)
           || !cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
          break;
        }

        // Try matrix
        GetInstanceValue(FLOATmatrix3D, pm, v, -1);

        if (pm != NULL) {
          if (!ConvertMatrixToJSONArray(*pm, pNewValue)
           || !cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
          break;
        }

        // Try bounding box
        GetInstanceValue(FLOATaabbox3D, pbox, v, -1);

        if (pbox != NULL) {
          if (!ConvertAABBToJSONArray(*pbox, pNewValue)
           || !cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
          break;
        }

        // Try placement
        GetInstanceValue(CPlacement3D, ppl3d, v, -1);

        if (ppl3d != NULL) {
          if (!ConvertPlacementToJSONArray(*ppl3d, pNewValue)
           || !cJSON_AddItemToArray(pArray, pNewValue)) goto failure;
          break;
        }
      }

      default: {
      failure:
        if (pNewValue != NULL) cJSON_Delete(pNewValue);
        sq_pop(v, 3); // Pop key, value, null iterator
        return sq_throwerror(v, "unsupported Squirrel type for JSON");
      }
    }

    // Last value was successfully added and quit the switch block
    pNewValue = NULL;

    sq_pop(v, 2); // Pop key and value before the next iteration
  }

  sq_poptop(v); // Null iterator
  return 0;
};

SQInteger PopulateJSONObject(HSQUIRRELVM v, SQInteger idxObject, cJSON *pObject) {
  if (pObject == NULL) return sq_throwerror(v, "no JSON object to populate");

  // Iterate through the object
  sq_pushnull(v); // Null iterator

  // Shift the top index because of the null iterator
  if (idxObject < 0) idxObject--;

  while (SQ_SUCCEEDED(sq_next(v, idxObject))) {
    CTString strKey;
    {
      sq_tostring(v, -2); // Push key as a string

      const SQChar *strKeyName;
      sq_getstring(v, -1, &strKeyName);
      strKey = strKeyName;

      sq_poptop(v); // Pop key as a string
    }

    switch (sq_gettype(v, -1))
    {
      case OT_NULL: {
        if (cJSON_AddNullToObject(pObject, strKey) == NULL) goto failure;
      } break;

      case OT_INTEGER: {
        SQInteger i;
        sq_getinteger(v, -1, &i);
        if (cJSON_AddNumberToObject(pObject, strKey, i) == NULL) goto failure;
      } break;

      case OT_FLOAT: {
        SQFloat f;
        sq_getfloat(v, -1, &f);
        if (cJSON_AddNumberToObject(pObject, strKey, f) == NULL) goto failure;
      } break;

      case OT_BOOL: {
        SQBool b;
        sq_getbool(v, -1, &b);
        if (cJSON_AddBoolToObject(pObject, strKey, b) == NULL) goto failure;
      } break;

      case OT_STRING: {
        const SQChar *str;
        sq_getstring(v, -1, &str);
        if (cJSON_AddStringToObject(pObject, strKey, str) == NULL) goto failure;
      } break;

      case OT_ARRAY: {
        cJSON *pNewArray = cJSON_AddArrayToObject(pObject, strKey);
        if (SQ_FAILED(PopulateJSONArray(v, -1, pNewArray))) goto failure;
      } break;

      case OT_TABLE: {
        cJSON *pNewObject = cJSON_AddObjectToObject(pObject, strKey);
        if (SQ_FAILED(PopulateJSONObject(v, -1, pNewObject))) goto failure;
      } break;

      case OT_INSTANCE: {
        cJSON *pNewArray = cJSON_AddArrayToObject(pObject, strKey);
        if (pNewArray == NULL) goto failure;

        // Try plane (before vector because it's derived from FLOAT3D)
        GetInstanceValue(FLOATplane3D, ppl, v, -1);

        if (ppl != NULL) {
          if (!ConvertPlaneToJSONArray(*ppl, pNewArray)) goto failure;
          break;
        }

        // Try vector
        GetInstanceValue(FLOAT3D, pv, v, -1);

        if (pv != NULL) {
          if (!ConvertVectorToJSONArray(*pv, pNewArray)) goto failure;
          break;
        }

        // Try matrix
        GetInstanceValue(FLOATmatrix3D, pm, v, -1);

        if (pm != NULL) {
          if (!ConvertMatrixToJSONArray(*pm, pNewArray)) goto failure;
          break;
        }

        // Try bounding box
        GetInstanceValue(FLOATaabbox3D, pbox, v, -1);

        if (pbox != NULL) {
          if (!ConvertAABBToJSONArray(*pbox, pNewArray)) goto failure;
          break;
        }

        // Try placement
        GetInstanceValue(CPlacement3D, ppl3d, v, -1);

        if (ppl3d != NULL) {
          if (!ConvertPlacementToJSONArray(*ppl3d, pNewArray)) goto failure;
          break;
        }
      }

      default: {
      failure:
        sq_pop(v, 3); // Pop key, value, null iterator
        return sq_throwerror(v, "unsupported Squirrel type for JSON");
      }
    }

    sq_pop(v, 2); // Pop key and value before the next iteration
  }

  sq_poptop(v); // Null iterator
  return 0;
};

BOOL ConvertVectorToJSONArray(const FLOAT3D &v, cJSON *pArray) {
  if (pArray == NULL) return FALSE;

  cJSON *pNew = NULL;

  pNew = cJSON_CreateNumber(v(1));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(v(2));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(v(3));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;

  return TRUE;

failure:
  if (pNew != NULL) cJSON_Delete(pNew);
  return FALSE;
};

BOOL ConvertPlaneToJSONArray(const FLOATplane3D &pl, cJSON *pArray) {
  if (pArray == NULL) return FALSE;

  cJSON *pNew = NULL;

  pNew = cJSON_CreateArray();
  if (!ConvertVectorToJSONArray(pl, pNew)
   || !cJSON_AddItemToArray(pArray, pNew)) goto failure;

  pNew = cJSON_CreateNumber(pl.Distance());
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;

  return TRUE;

failure:
  if (pNew != NULL) cJSON_Delete(pNew);
  return FALSE;
};

BOOL ConvertMatrixToJSONArray(const FLOATmatrix3D &m, cJSON *pArray) {
  if (pArray == NULL) return FALSE;

  cJSON *pNew = NULL;

  pNew = cJSON_CreateNumber(m(1, 1));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(1, 2));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(1, 3));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;

  pNew = cJSON_CreateNumber(m(2, 1));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(2, 2));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(2, 3));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;

  pNew = cJSON_CreateNumber(m(3, 1));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(3, 2));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;
  pNew = cJSON_CreateNumber(m(3, 3));
  if (!cJSON_AddItemToArray(pArray, pNew)) goto failure;

  return TRUE;

failure:
  if (pNew != NULL) cJSON_Delete(pNew);
  return FALSE;
};

BOOL ConvertAABBToJSONArray(const FLOATaabbox3D &box, cJSON *pArray) {
  if (pArray == NULL) return FALSE;

  // Append values of both vectors to the same array
  if (!ConvertVectorToJSONArray(box.Min(), pArray)) return FALSE;
  if (!ConvertVectorToJSONArray(box.Max(), pArray)) return FALSE;

  return TRUE;
};

BOOL ConvertPlacementToJSONArray(const CPlacement3D &pl, cJSON *pArray) {
  if (pArray == NULL) return FALSE;

  cJSON *pNew = NULL;

  pNew = cJSON_CreateArray();
  if (!ConvertVectorToJSONArray(pl.pl_PositionVector, pNew)
   || !cJSON_AddItemToArray(pArray, pNew)) goto failure;

  pNew = cJSON_CreateArray();
  if (!ConvertVectorToJSONArray(pl.pl_OrientationAngle, pNew)
   || !cJSON_AddItemToArray(pArray, pNew)) goto failure;

  return TRUE;

failure:
  if (pNew != NULL) cJSON_Delete(pNew);
  return FALSE;
};

static SQInteger SetArray(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return PopulateJSONArray(v, 3, pArray);
};

static SQInteger SetObject(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pObject = CreateObjectAt(val.pJSON, strPath);

  if (pObject == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an object value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return PopulateJSONObject(v, 3, pObject);
};

static SQInteger SetVector(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetInstanceValueVerify(FLOAT3D, pv, v, 3);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  if (!ConvertVectorToJSONArray(*pv, pArray)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot set vector to '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetPlane(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetInstanceValueVerify(FLOATplane3D, ppl, v, 3);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  if (!ConvertPlaneToJSONArray(*ppl, pArray)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot set plane to '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetMatrix(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetInstanceValueVerify(FLOATmatrix3D, pm, v, 3);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  if (!ConvertMatrixToJSONArray(*pm, pArray)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot set matrix to '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetAABB(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetInstanceValueVerify(FLOATaabbox3D, pbox, v, 3);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  if (!ConvertAABBToJSONArray(*pbox, pArray)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot set AABB to '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetPlacement(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetInstanceValueVerify(CPlacement3D, ppl, v, 3);

  cJSON *pArray = CreateArrayAt(val.pJSON, strPath);

  if (pArray == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "failed to create an array value at '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  if (!ConvertPlacementToJSONArray(*ppl, pArray)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot set placement to '%s' in local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetValue(HSQUIRRELVM v, int ctArgs, LocalStorage &val) {
  switch (sq_gettype(v, 3)) {
    case OT_NULL:   return SetNull(v, ctArgs, val);
    case OT_INTEGER:
    case OT_FLOAT:  return SetNumber(v, ctArgs, val);
    case OT_BOOL:   return SetBool(v, ctArgs, val);
    case OT_STRING: return SetString(v, ctArgs, val);
    case OT_ARRAY:  return SetArray(v, ctArgs, val);
    case OT_TABLE:  return SetObject(v, ctArgs, val);

    case OT_INSTANCE: {
      // Try plane (before vector because it's derived from FLOAT3D)
      GetInstanceValue(FLOATplane3D, ppl, v, 3);
      if (ppl != NULL) return SetPlane(v, ctArgs, val);

      // Try vector
      GetInstanceValue(FLOAT3D, pv, v, 3);
      if (pv != NULL) return SetVector(v, ctArgs, val);

      // Try matrix
      GetInstanceValue(FLOATmatrix3D, pm, v, 3);
      if (pm != NULL) return SetMatrix(v, ctArgs, val);

      // Try bounding box
      GetInstanceValue(FLOATaabbox3D, pbox, v, 3);
      if (pbox != NULL) return SetAABB(v, ctArgs, val);

      // Try placement
      GetInstanceValue(CPlacement3D, ppl3d, v, 3);
      if (ppl3d != NULL) return SetPlacement(v, ctArgs, val);
    }
  }

  return sq_throwerror(v, "unsupported Squirrel type for JSON");
};

static SQInteger GetBool(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsBool(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve boolean value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  sq_pushbool(v, cJSON_IsTrue(pValue));
  return 1;
};

static SQInteger GetNull(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsNull(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve null value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  sq_pushnull(v);
  return 1;
};

static SQInteger GetNumber(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsNumber(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve number value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  sq_pushfloat(v, pValue->valuedouble);
  return 1;
};

static SQInteger GetString(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsString(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve string value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, pValue->valuestring, -1);
  return 1;
};

static SQInteger PushJSONArray(HSQUIRRELVM v, cJSON *pArray);
static SQInteger PushJSONObject(HSQUIRRELVM v, cJSON *pObject);

static SQInteger PushJSONValue(HSQUIRRELVM v, cJSON *pValue) {
  if (cJSON_IsNull(pValue)) {
    sq_pushnull(v);
  } else if (cJSON_IsBool(pValue)) {
    sq_pushbool(v, cJSON_IsTrue(pValue));
  } else if (cJSON_IsNumber(pValue)) {
    sq_pushfloat(v, pValue->valuedouble);
  } else if (cJSON_IsString(pValue)) {
    sq_pushstring(v, pValue->valuestring, -1);
  } else if (cJSON_IsArray(pValue)) {
    return PushJSONArray(v, pValue);
  } else if (cJSON_IsObject(pValue)) {
    return PushJSONObject(v, pValue);
  } else {
    return sq_throwerror(v, "unsupported JSON value type");
  }

  return 1; // Ends up here after pushing simple types, e.g. a string
};

SQInteger PushJSONArray(HSQUIRRELVM v, cJSON *pArray) {
  cJSON *pElement;
  INDEX i = 0;
  sq_newarray(v, 0);

  cJSON_ArrayForEach(pElement, pArray) {
    // Push value
    if (SQ_FAILED(PushJSONValue(v, pElement))) {
      SQChar strError[256];
      scsprintf(strError, 256, "unsupported JSON value type in array element %d", i);
      return sq_throwerror(v, strError);
    }

    // Add value to the array
    sq_arrayappend(v, -2);
    i++;
  }

  return 1;
};

SQInteger PushJSONObject(HSQUIRRELVM v, cJSON *pObject) {
  cJSON *pElement;
  sq_newtable(v);

  cJSON_ArrayForEach(pElement, pObject) {
    // Push key
    sq_pushstring(v, pElement->string, -1);

    // Push value
    if (SQ_FAILED(PushJSONValue(v, pElement))) {
      SQChar strError[256];
      scsprintf(strError, 256, "unsupported JSON value type in object pair '%s'", pElement->string);
      return sq_throwerror(v, strError);
    }

    // Add pair to the table
    sq_rawset(v, -3);
  }

  return 1;
};

static SQInteger GetArray(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsArray(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve array value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return PushJSONArray(v, pValue);
};

static SQInteger GetObject(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (!cJSON_IsObject(pValue)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve object value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return PushJSONObject(v, pValue);
};

static SQInteger GetVector(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOAT3D vResult;

  if (!VerifyVector(pValue, &vResult)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve vector value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
  *pv = vResult;
  return 1;
};

static SQInteger GetPlane(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATplane3D plResult;

  if (!VerifyPlane(pValue, &plResult)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve plane value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(FLOATplane3D, ppl, GetVMClass(v).Root(), "FLOATplane3D");
  *ppl = plResult;
  return 1;
};

static SQInteger GetMatrix(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATmatrix3D mResult;

  if (!VerifyMatrix(pValue, &mResult)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve matrix value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(FLOATmatrix3D, pm, GetVMClass(v).Root(), "FLOATmatrix3D");
  *pm = mResult;
  return 1;
};

static SQInteger GetAABB(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATaabbox3D boxResult;

  if (!VerifyAABB(pValue, &boxResult)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve AABB value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
  *pbox = boxResult;
  return 1;
};

static SQInteger GetPlacement(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  CPlacement3D plResult;

  if (!VerifyPlacement(pValue, &plResult)) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve placement value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
  *ppl = plResult;
  return 1;
};

static SQInteger GetValue(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (pValue == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "cannot retrieve value '%s' from local storage '%s'", strPath, val.strName.str_String);
    return sq_throwerror(v, strError);
  }

  return PushJSONValue(v, pValue);
};

static SQInteger GetBoolDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (cJSON_IsBool(pValue)) {
    sq_pushbool(v, cJSON_IsTrue(pValue));
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetNumberDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (cJSON_IsNumber(pValue)) {
    sq_pushfloat(v, pValue->valuedouble);
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetStringDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);

  if (cJSON_IsString(pValue)) {
    sq_pushstring(v, pValue->valuestring, -1);
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetArrayDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  if (cJSON_IsArray(pValue)) return PushJSONArray(v, pValue);

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetObjectDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  if (cJSON_IsObject(pValue)) return PushJSONObject(v, pValue);

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetVectorDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOAT3D vResult;

  if (VerifyVector(pValue, &vResult)) {
    PushNewInstance(FLOAT3D, pv, GetVMClass(v).Root(), "FLOAT3D");
    *pv = vResult;
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetPlaneDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATplane3D plResult;

  if (VerifyPlane(pValue, &plResult)) {
    PushNewInstance(FLOATplane3D, ppl, GetVMClass(v).Root(), "FLOATplane3D");
    *ppl = plResult;
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetMatrixDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATmatrix3D mResult;

  if (VerifyMatrix(pValue, &mResult)) {
    PushNewInstance(FLOATmatrix3D, pm, GetVMClass(v).Root(), "FLOATmatrix3D");
    *pm = mResult;
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetAABBDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  FLOATaabbox3D boxResult;

  if (VerifyAABB(pValue, &boxResult)) {
    PushNewInstance(FLOATaabbox3D, pbox, GetVMClass(v).Root(), "FLOATaabbox3D");
    *pbox = boxResult;
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetPlacementDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  CPlacement3D plResult;

  if (VerifyPlacement(pValue, &plResult)) {
    PushNewInstance(CPlacement3D, ppl, GetVMClass(v).Root(), "CPlacement3D");
    *ppl = plResult;
    return 1;
  }

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static SQInteger GetValueDefault(HSQUIRRELVM v, int, LocalStorage &val) {
  ASSERT_JSON;

  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);

  cJSON *pValue = ResolvePath(val.pJSON, strPath);
  if (pValue != NULL) return PushJSONValue(v, pValue);

  // Push provided default value
  sq_push(v, 3);
  return 1;
};

static Method<LocalStorage> _aMethods[] = {
  { "Save",  &Save,  1, "." },
  { "Clear", &Clear, 1, "." },

  { "KeyExists", &KeyExists, 2, ".s" },
  { "DeleteKey", &DeleteKey, 2, ".s" },

  { "IsFalse",  &IsFalse,  2, ".s" },
  { "IsTrue",   &IsTrue,   2, ".s" },
  { "IsBool",   &IsBool,   2, ".s" },
  { "IsNull",   &IsNull,   2, ".s" },
  { "IsNumber", &IsNumber, 2, ".s" },
  { "IsString", &IsString, 2, ".s" },
  { "IsArray",  &IsArray,  2, ".s" },
  { "IsObject", &IsObject, 2, ".s" },
  { "IsVector",    &IsVector,    2, ".s" },
  { "IsPlane",     &IsPlane,     2, ".s" },
  { "IsMatrix",    &IsMatrix,    2, ".s" },
  { "IsAABB",      &IsAABB,      2, ".s" },
  { "IsPlacement", &IsPlacement, 2, ".s" },

  { "SetBool",   &SetBool,   3, ".sb" },
  { "SetNull",   &SetNull,   2, ".s" },
  { "SetNumber", &SetNumber, 3, ".sn" },
  { "SetString", &SetString, 3, ".ss" },
  { "SetArray",  &SetArray,  3, ".sa" },
  { "SetObject", &SetObject, 3, ".st" },
  { "SetVector",    &SetVector,    3, ".sx" },
  { "SetPlane",     &SetPlane,     3, ".sx" },
  { "SetMatrix",    &SetMatrix,    3, ".sx" },
  { "SetAABB",      &SetAABB,      3, ".sx" },
  { "SetPlacement", &SetPlacement, 3, ".sx" },
  { "SetValue",  &SetValue,  3, ".sb|o|n|s|a|t|x" },

  { "GetBool",   &GetBool,   2, ".s" },
  { "GetNull",   &GetNull,   2, ".s" },
  { "GetNumber", &GetNumber, 2, ".s" },
  { "GetString", &GetString, 2, ".s" },
  { "GetArray",  &GetArray,  2, ".s" },
  { "GetObject", &GetObject, 2, ".s" },
  { "GetVector",    &GetVector,    2, ".s" },
  { "GetPlane",     &GetPlane,     2, ".s" },
  { "GetMatrix",    &GetMatrix,    2, ".s" },
  { "GetAABB",      &GetAABB,      2, ".s" },
  { "GetPlacement", &GetPlacement, 2, ".s" },
  { "GetValue",  &GetValue,  2, ".s" },

  { "GetBoolDefault",   &GetBoolDefault,   3, ".sb" },
  { "GetNumberDefault", &GetNumberDefault, 3, ".sn" },
  { "GetStringDefault", &GetStringDefault, 3, ".ss" },
  { "GetArrayDefault",  &GetArrayDefault,  3, ".sa" },
  { "GetObjectDefault", &GetObjectDefault, 3, ".st" },
  { "GetVectorDefault",    &GetVectorDefault,    3, ".sx" },
  { "GetPlaneDefault",     &GetPlaneDefault,     3, ".sx" },
  { "GetMatrixDefault",    &GetMatrixDefault,    3, ".sx" },
  { "GetAABBDefault",      &GetAABBDefault,      3, ".sx" },
  { "GetPlacementDefault", &GetPlacementDefault, 3, ".sx" },
  { "GetValueDefault",  &GetValueDefault,  3, ".s." },
};

}; // namespace

namespace Core {

static SQInteger IsSE105(HSQUIRRELVM v) {
#if SE1_VER < SE1_107
  sq_pushbool(v, SQTrue);
#else
  sq_pushbool(v, SQFalse);
#endif
  return 1;
};

static SQInteger IsSE107(HSQUIRRELVM v) {
#if SE1_VER >= SE1_107
  sq_pushbool(v, SQTrue);
#else
  sq_pushbool(v, SQFalse);
#endif
  return 1;
};

static SQInteger IsTFE(HSQUIRRELVM v) {
  sq_pushbool(v, CHOOSE_FOR_GAME(SQTrue, SQFalse, SQFalse));
  return 1;
};

static SQInteger IsTSE(HSQUIRRELVM v) {
  sq_pushbool(v, CHOOSE_FOR_GAME(SQFalse, SQTrue, SQTrue));
  return 1;
};

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
  const SQChar *strFile;
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
  const SQChar *strFile;
  sq_getstring(v, 2, &strFile);

  VM &vm = GetVMClass(v);
  if (!vm.CompileFromFile(strFile)) return SQ_ERROR;

  // Return compiled closure
  return 1;
};

static SQInteger StringLength(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  sq_pushinteger(v, str.Length());
  return 1;
};

static SQInteger StringLengthNaked(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  sq_pushinteger(v, str.LengthNaked());
  return 1;
};

static SQInteger StringUndecorated(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  sq_pushstring(v, str.Undecorated().str_String, -1);
  return 1;
};

static SQInteger StringFindSubstr(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strSub;
  sq_getstring(v, 3, &strSub);

  CTString str(strSQ);
  sq_pushinteger(v, str.FindSubstr(strSub));
  return 1;
};

static SQInteger StringReplaceSubstr(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strSub;
  sq_getstring(v, 3, &strSub);
  const SQChar *strNewSub;
  sq_getstring(v, 4, &strNewSub);

  // Separate the string by the old substring
  CStringStack astr;
  IData::GetStrings(astr, strSQ, strSub);

  const INDEX ct = astr.Count();

  // Nothing to replace
  if (ct == 0) {
    sq_pushstring(v, strSQ, -1);
    return 1;
  }

  // Reassemble it with the new substring starting with the first part
  CTString str = astr[0];

  for (INDEX i = 1; i < ct; i++) {
    str += strNewSub + astr[i];
  }

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringHasPrefix(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strPrefix;
  sq_getstring(v, 3, &strPrefix);

  CTString str(strSQ);
  sq_pushbool(v, str.HasPrefix(strPrefix));
  return 1;
};

static SQInteger StringRemovePrefix(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strPrefix;
  sq_getstring(v, 3, &strPrefix);

  CTString str(strSQ);
  str.RemovePrefix(strPrefix); // Unused return value
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringTrimLeft(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iCharacters;
  sq_getinteger(v, 3, &iCharacters);

  CTString str(strSQ);
  INDEX iCharsRemoved = str.TrimLeft(iCharacters);

  sq_newarray(v, 0);
  sq_pushstring(v, str.str_String, -1);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, iCharsRemoved);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger StringTrimRight(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iCharacters;
  sq_getinteger(v, 3, &iCharacters);

  CTString str(strSQ);
  INDEX iCharsRemoved = str.TrimRight(iCharacters);

  sq_newarray(v, 0);
  sq_pushstring(v, str.str_String, -1);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, iCharsRemoved);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger StringTrimSpacesLeft(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  INDEX iCharsRemoved = str.TrimSpacesLeft();

  sq_newarray(v, 0);
  sq_pushstring(v, str.str_String, -1);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, iCharsRemoved);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger StringTrimSpacesRight(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  INDEX iCharsRemoved = str.TrimSpacesRight();

  sq_newarray(v, 0);
  sq_pushstring(v, str.str_String, -1);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, iCharsRemoved);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger StringGetHash(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  sq_pushinteger(v, str.GetHash());
  return 1;
};

static SQInteger StringOnlyFirstLine(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTString str(strSQ);
  str.OnlyFirstLine();
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringMatches(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strOther;
  sq_getstring(v, 3, &strOther);

  CTString str(strSQ);
  sq_pushbool(v, IData::MatchWildcards(str, strOther));
  return 1;
};

static SQInteger StringSplit(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iPos;
  sq_getinteger(v, 3, &iPos);

  CTString str(strSQ);
  CTString str1, str2;
  str.Split(iPos, str1, str2);

  sq_newarray(v, 0);
  sq_pushstring(v, str1.str_String, -1);
  sq_arrayappend(v, -2);
  sq_pushstring(v, str2.str_String, -1);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger StringInsertChar(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iPos, iChar;
  sq_getinteger(v, 3, &iPos);
  sq_getinteger(v, 4, &iChar);

  CTString str(strSQ);
  str.InsertChar(iPos, iChar);
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringDeleteChar(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iPos;
  sq_getinteger(v, 3, &iPos);

  CTString str(strSQ);
  str.DeleteChar(iPos);
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringLoad(HSQUIRRELVM v) {
  const SQChar *strFile;
  sq_getstring(v, 2, &strFile);

  CTString str;

  try {
    str.Load_t(CTString(strFile));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringSave(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  const SQChar *strFile;
  sq_getstring(v, 3, &strFile);

  CTString str(strSQ);

  try {
    str.Save_t(CTString(strFile));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger StringReplaceChar(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iOld, iNew;
  sq_getinteger(v, 3, &iOld);
  sq_getinteger(v, 4, &iNew);

  CTString str(strSQ);
  IData::ReplaceChar(str.str_String, iOld, iNew);
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger StringFindChar(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);
  SQInteger iChar, iFrom;
  sq_getinteger(v, 3, &iChar);
  sq_getinteger(v, 4, &iFrom);

  ULONG ulFrom = (ULONG)(INDEX)iFrom; // Trim to 32 bits and make it unsigned

  CTString str(strSQ);
  INDEX iResult = IData::FindChar(str.str_String, iChar, ulFrom); // Make result signed
  sq_pushinteger(v, iResult);
  return 1;
};

static SQInteger StringCountChar(HSQUIRRELVM v) {
  const SQChar *str;
  sq_getstring(v, 2, &str);
  SQInteger iChar, iFrom;
  sq_getinteger(v, 3, &iChar);
  sq_getinteger(v, 4, &iFrom);

  ULONG ulFrom = (ULONG)(INDEX)iFrom; // Trim to 32 bits and make it unsigned
  sq_pushinteger(v, IData::CountChar(str, iChar, ulFrom));
  return 1;
};

static SQInteger StringExtractSubstr(HSQUIRRELVM v) {
  const SQChar *str;
  sq_getstring(v, 2, &str);
  SQInteger iFrom, iChars;
  sq_getinteger(v, 3, &iFrom);
  sq_getinteger(v, 4, &iChars);

  ULONG ulFrom = (ULONG)(INDEX)iFrom; // Trim to 32 bits and make it unsigned
  ULONG ulChars = (ULONG)(INDEX)iChars; // Trim to 32 bits and make it unsigned
  CTString strSub = IData::ExtractSubstr(str, ulFrom, ulChars);
  sq_pushstring(v, strSub.str_String, -1);
  return 1;
};

static SQInteger StringSeparateByChar(HSQUIRRELVM v) {
  const SQChar *strIn;
  sq_getstring(v, 2, &strIn);
  SQInteger iDelimiter;
  sq_getinteger(v, 3, &iDelimiter);

  CStringStack astr;
  IData::GetStrings(astr, strIn, (char)iDelimiter);

  sq_newarray(v, 0);

  // Fill the array with strings separated by a character
  const INDEX ct = astr.Count();

  for (INDEX i = 0; i < ct; i++) {
    sq_pushstring(v, astr[i], -1);
    sq_arrayappend(v, -2);
  }
  return 1;
};

static SQInteger StringSeparateByString(HSQUIRRELVM v) {
  const SQChar *strIn;
  sq_getstring(v, 2, &strIn);
  const SQChar *strDelimiter;
  sq_getstring(v, 3, &strDelimiter);

  CStringStack astr;
  IData::GetStrings(astr, strIn, strDelimiter);

  sq_newarray(v, 0);

  // Fill the array with strings separated by a string
  const INDEX ct = astr.Count();

  for (INDEX i = 0; i < ct; i++) {
    sq_pushstring(v, astr[i], -1);
    sq_arrayappend(v, -2);
  }
  return 1;
};

static SQInteger StringGetDecoratedChar(HSQUIRRELVM v) {
  const SQChar *str;
  sq_getstring(v, 2, &str);
  SQInteger iChar;
  sq_getinteger(v, 3, &iChar);

  sq_pushinteger(v, IData::GetDecoratedChar(str, iChar));
  return 1;
};

static SQInteger StringMatchesMask(HSQUIRRELVM v) {
  const SQChar *str;
  sq_getstring(v, 2, &str);
  const SQChar *strMask;
  sq_getstring(v, 3, &strMask);

  sq_pushbool(v, IData::MatchesMask(str, strMask));
  return 1;
};

static SQInteger StringFileDir(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTFileName fnm = CTString(strSQ);
  sq_pushstring(v, fnm.FileDir().str_String, -1);
  return 1;
};

static SQInteger StringFileName(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTFileName fnm = CTString(strSQ);
  sq_pushstring(v, fnm.FileName().str_String, -1);
  return 1;
};

static SQInteger StringFileExt(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTFileName fnm = CTString(strSQ);
  sq_pushstring(v, fnm.FileExt().str_String, -1);
  return 1;
};

static SQInteger StringNoExt(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTFileName fnm = CTString(strSQ);
  sq_pushstring(v, fnm.NoExt().str_String, -1);
  return 1;
};

static SQInteger StringRemoveAppPath(HSQUIRRELVM v) {
  const SQChar *strSQ;
  sq_getstring(v, 2, &strSQ);

  CTFileName fnm = CTString(strSQ);

  try {
    fnm.RemoveApplicationPath_t();
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, fnm.str_String, -1);
  return 1;
};

}; // namespace

// "Core" namespace functions
static SQRegFunction _aCoreFuncs[] = {
  { "IsSE105",           &Core::IsSE105,           1, "." },
  { "IsSE107",           &Core::IsSE107,           1, "." },
  { "IsTFE",             &Core::IsTFE,             1, "." },
  { "IsTSE",             &Core::IsTSE,             1, "." },
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

  // Engine string methods
  { "StringLength",          &Core::StringLength,          2, ".s" },
  { "StringLengthNaked",     &Core::StringLengthNaked,     2, ".s" },
  { "StringUndecorated",     &Core::StringUndecorated,     2, ".s" },
  { "StringFindSubstr",      &Core::StringFindSubstr,      3, ".ss" },
  { "StringReplaceSubstr",   &Core::StringReplaceSubstr,   4, ".sss" },
  { "StringHasPrefix",       &Core::StringHasPrefix,       3, ".ss" },
  { "StringRemovePrefix",    &Core::StringRemovePrefix,    3, ".ss" },
  { "StringTrimLeft",        &Core::StringTrimLeft,        3, ".sn" },
  { "StringTrimRight",       &Core::StringTrimRight,       3, ".sn" },
  { "StringTrimSpacesLeft",  &Core::StringTrimSpacesLeft,  2, ".s" },
  { "StringTrimSpacesRight", &Core::StringTrimSpacesRight, 2, ".s" },
  { "StringGetHash",         &Core::StringGetHash,         2, ".s" },
  { "StringOnlyFirstLine",   &Core::StringOnlyFirstLine,   2, ".s" },
  { "StringMatches",         &Core::StringMatches,         3, ".ss" },
  { "StringSplit",           &Core::StringSplit,           3, ".sn" },
  { "StringInsertChar",      &Core::StringInsertChar,      4, ".snn" },
  { "StringDeleteChar",      &Core::StringDeleteChar,      3, ".sn" },
  { "StringLoad",            &Core::StringLoad,            2, ".s" },
  { "StringSave",            &Core::StringSave,            3, ".ss" },

  // IData interface methods
  { "StringReplaceChar",      &Core::StringReplaceChar,      4, ".snn" },
  { "StringFindChar",         &Core::StringFindChar,         4, ".snn" },
  { "StringCountChar",        &Core::StringCountChar,        4, ".snn" },
  { "StringExtractSubstr",    &Core::StringExtractSubstr,    4, ".snn" },
  { "StringSeparateByChar",   &Core::StringSeparateByChar,   3, ".sn" },
  { "StringSeparateByString", &Core::StringSeparateByString, 3, ".ss" },
  { "StringGetDecoratedChar", &Core::StringGetDecoratedChar, 3, ".sn" },
  { "StringMatchesMask",      &Core::StringMatchesMask,      3, ".ss" },

  // Engine filename methods
  { "StringFileDir",       &Core::StringFileDir,       2, ".s" },
  { "StringFileName",      &Core::StringFileName,      2, ".s" },
  { "StringFileExt",       &Core::StringFileExt,       2, ".s" },
  { "StringNoExt",         &Core::StringNoExt,         2, ".s" },
  { "StringRemoveAppPath", &Core::StringRemoveAppPath, 2, ".s" },
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
  {
    Class<SqLocalStorage::LocalStorage> sqcLocalStorage(GetVM(), "LocalStorage", &SqLocalStorage::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqLocalStorage::_aMethods); i++) {
      sqcLocalStorage.RegisterMethod(SqLocalStorage::_aMethods[i]);
    }

    // Metamethods
    sqcLocalStorage.RegisterMetamethod(E_MM_TOSTRING, &SqLocalStorage::ToString);

    sqtCore.AddClass(sqcLocalStorage);
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
