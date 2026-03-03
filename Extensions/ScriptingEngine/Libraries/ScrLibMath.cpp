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

// FLOAT3D class methods
namespace SqVector {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, FLOAT3D &val) {
  val = FLOAT3D(0, 0, 0);

  SQFloat f;
  if (ctArgs > 0 && SQ_SUCCEEDED(sq_getfloat(v, 2, &f))) val(1) = f;
  if (ctArgs > 1 && SQ_SUCCEEDED(sq_getfloat(v, 3, &f))) val(2) = f;
  if (ctArgs > 2 && SQ_SUCCEEDED(sq_getfloat(v, 4, &f))) val(3) = f;

  return 0;
};

static SQInteger Clone(HSQUIRRELVM v, FLOAT3D &val, FLOAT3D &valOther) {
  val = valOther;
  return 0;
};

static SQInteger Add(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  *pv = val + *pOther;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  *pv = val - *pOther;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a matrix value
  GetInstanceValue(FLOATmatrix3D, pOtherMat, v, idxOther);

  if (pOtherMat != NULL) {
    *pv = val * *pOtherMat;
    return 1;
  }

  // Or a vector value
  GetInstanceValue(FLOAT3D, pOtherVec, v, idxOther);

  if (pOtherVec != NULL) {
    *pv = val * *pOtherVec;
    return 1;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected FLOATmatrix3D or FLOAT3D or float value");

  *pv = val * f;
  return 1;
};

static SQInteger Div(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected float value");

  *pv = val / f;
  return 1;
};

static SQInteger Mod(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);
  sq_pushfloat(v, val % *pOther);
  return 1;
};

static SQInteger UnaryMinus(HSQUIRRELVM v, FLOAT3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = -val;
  return 1;
};

static SQInteger ToString(HSQUIRRELVM v, FLOAT3D &val) {
  CTString str(0, "[%g, %g, %g]", val(1), val(2), val(3));
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

SQCLASS_GETSET_FLOAT(GetX, SetX, FLOAT3D, val(1), val(1));
SQCLASS_GETSET_FLOAT(GetY, SetY, FLOAT3D, val(2), val(2));
SQCLASS_GETSET_FLOAT(GetZ, SetZ, FLOAT3D, val(3), val(3));

static SQInteger Equal(HSQUIRRELVM v, int, FLOAT3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger Length(HSQUIRRELVM v, int, FLOAT3D &val) {
  sq_pushfloat(v, val.Length());
  return 1;
};

static SQInteger ManhattanNorm(HSQUIRRELVM v, int, FLOAT3D &val) {
  sq_pushfloat(v, val.ManhattanNorm());
  return 1;
};

static SQInteger MaxNorm(HSQUIRRELVM v, int, FLOAT3D &val) {
  sq_pushfloat(v, val.MaxNorm());
  return 1;
};

static SQInteger Normalize(HSQUIRRELVM v, int, FLOAT3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.Normalize();
  return 1;
};

static SQInteger SafeNormalize(HSQUIRRELVM v, int, FLOAT3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.SafeNormalize();
  return 1;
};

static SQInteger Flip(HSQUIRRELVM v, int, FLOAT3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.Flip();
  return 1;
};

static Method<FLOAT3D> _aMethods[] = {
  { "Equal",         &Equal,         2, ".x" },
  { "Length",        &Length,        1, "." },
  { "ManhattanNorm", &ManhattanNorm, 1, "." },
  { "MaxNorm",       &MaxNorm,       1, "." },
  { "Normalize",     &Normalize,     1, "." },
  { "SafeNormalize", &SafeNormalize, 1, "." },
  { "Flip",          &Flip,          1, "." },
};

}; // namespace

// FLOATplane3D class methods
namespace SqPlane {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, FLOATplane3D &val) {
  if (ctArgs <= 0) {
    val = FLOATplane3D(FLOAT3D(0, 1, 0), 0.0f);
    return 0;
  }

  // Create a plane from three points
  if (ctArgs == 3) {
    GetInstanceValueVerify(FLOAT3D, pv0, v, 2);
    GetInstanceValueVerify(FLOAT3D, pv1, v, 3);
    GetInstanceValueVerify(FLOAT3D, pv2, v, 4);

    val = FLOATplane3D(*pv0, *pv1, *pv2);
    return 0;
  }

  // Try to get the normal as the first argument
  GetInstanceValueVerify(FLOAT3D, pvNormal, v, 2);

  GetInstanceValue(FLOAT3D, pvPoint, v, 3);
  SQFloat fDist;

  // Try creating a plane with a distance towards some point
  if (pvPoint != NULL) {
    val = FLOATplane3D(*pvNormal, *pvPoint);
    return 0;

  // Or a plane with a specific distance
  } else if (SQ_SUCCEEDED(sq_getfloat(v, 3, &fDist))) {
    val = FLOATplane3D(*pvNormal, fDist);
    return 0;
  }

  return sq_throwerror(v, "expected FLOAT3D or number value in argument 2");
};

static SQInteger Clone(HSQUIRRELVM v, FLOATplane3D &val, FLOATplane3D &valOther) {
  val = valOther;
  return 0;
};

static SQInteger Add(HSQUIRRELVM v, FLOATplane3D &val, SQInteger idxOther) {
  // Create a plane instance
  FLOATplane3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATplane3D", &ppl)) return SQ_ERROR;

  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  *ppl = val + *pOther;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOATplane3D &val, SQInteger idxOther) {
  // Create a plane instance
  FLOATplane3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATplane3D", &ppl)) return SQ_ERROR;

  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  *ppl = val - *pOther;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOATplane3D &val, SQInteger idxOther) {
  // Create a plane instance
  FLOATplane3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATplane3D", &ppl)) return SQ_ERROR;

  // Get a matrix value
  GetInstanceValueVerify(FLOATmatrix3D, pOther, v, idxOther);

  *ppl = val * *pOther;
  return 1;
};

static SQInteger UnaryMinus(HSQUIRRELVM v, FLOATplane3D &val) {
  // Create a plane instance
  FLOATplane3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATplane3D", &ppl)) return SQ_ERROR;

  *ppl = -val;
  return 1;
};

static SQInteger ToString(HSQUIRRELVM v, FLOATplane3D &val) {
  CTString str(0, "pl[%g, %g, %g; %g]", val(1), val(2), val(3), val.Distance());
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

SQCLASS_GET_INSTANCE(GetVector, FLOATplane3D, FLOAT3D, (FLOAT3D &)val);
SQCLASS_SET_INSTANCE(SetVector, FLOATplane3D, FLOAT3D, (FLOAT3D &)val);

SQCLASS_GETSET_FLOAT(GetDist, SetDist, FLOATplane3D, val.Distance(), val.Distance());

static SQInteger Offset(HSQUIRRELVM v, int, FLOATplane3D &val) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  val.Offset(f);
  return 0;
};

static SQInteger GetMaxNormal(HSQUIRRELVM v, int, FLOATplane3D &val) {
  sq_pushinteger(v, val.GetMaxNormal());
  return 1;
};

static SQInteger ReferencePoint(HSQUIRRELVM v, int ctArgs, FLOATplane3D &val) {
  FLOAT3D *pvOrigin = NULL;

  if (ctArgs > 0) {
    GetInstanceValueVerify(FLOAT3D, pvOriginArg, v, 2);
    pvOrigin = pvOriginArg;
  }

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  if (pvOrigin != NULL) {
    *pv = val.ReferencePoint(*pvOrigin);
  } else {
    *pv = val.ReferencePoint();
  }
  return 1;
};

static SQInteger PointDistance(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOAT3D, pvPoint, v, 2);
  sq_pushfloat(v, val.PointDistance(*pvPoint));
  return 1;
};

static SQInteger PlaneDistance(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOATplane3D, pplOther, v, 2);
  sq_pushfloat(v, val.PlaneDistance(*pplOther));
  return 1;
};

static SQInteger GetCoordinate(HSQUIRRELVM v, int, FLOATplane3D &val) {
  SQInteger iIndex;
  sq_getinteger(v, 2, &iIndex);

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  val.GetCoordinate(iIndex, *pv);
  return 1;
};

static SQInteger ProjectPoint(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOAT3D, pvPoint, v, 2);

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.ProjectPoint(*pvPoint);
  return 1;
};

static SQInteger ProjectDirection(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.ProjectDirection(*pvDir);
  return 1;
};

static SQInteger DeprojectPoint(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOATplane3D, pplOther, v, 2);
  GetInstanceValueVerify(FLOAT3D, pvPoint, v, 3);

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.DeprojectPoint(*pplOther, *pvPoint);
  return 1;
};

static SQInteger DeprojectDirection(HSQUIRRELVM v, int, FLOATplane3D &val) {
  GetInstanceValueVerify(FLOATplane3D, pplOther, v, 2);
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 3);

  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.DeprojectDirection(*pplOther, *pvDir);
  return 1;
};

static Method<FLOATplane3D> _aMethods[] = {
  { "Offset",         &Offset,          2, ".n" },
  { "GetMaxNormal",   &GetMaxNormal,    1, "." },
  { "ReferencePoint", &ReferencePoint, -1, ".x" },

  { "PointDistance",  &PointDistance,   2, ".x" },
  { "PlaneDistance",  &PlaneDistance,   2, ".x" },
  { "GetCoordinate",  &GetCoordinate,   2, ".n" },

  { "ProjectPoint",       &ProjectPoint,       2, ".x" },
  { "ProjectDirection",   &ProjectDirection,   2, ".x" },
  { "DeprojectPoint",     &DeprojectPoint,     3, ".xx" },
  { "DeprojectDirection", &DeprojectDirection, 3, ".xx" },
};

}; // namespace

// FLOATmatrix3D class methods
namespace SqMatrix {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, FLOATmatrix3D &val) {
  SQFloat f;

  if (ctArgs > 0 && SQ_SUCCEEDED(sq_getfloat(v, 2, &f))) {
    val = FLOATmatrix3D(f);
  } else {
    val = FLOATmatrix3D(0);
  }

  return 0;
};

static SQInteger Clone(HSQUIRRELVM v, FLOATmatrix3D &val, FLOATmatrix3D &valOther) {
  val = valOther;
  return 0;
};

static SQInteger Add(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  GetInstanceValueVerify(FLOATmatrix3D, pOther, v, idxOther);

  *pm = val + *pOther;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  GetInstanceValueVerify(FLOATmatrix3D, pOther, v, idxOther);

  *pm = val - *pOther;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  GetInstanceValue(FLOATmatrix3D, pOtherMat, v, idxOther);

  if (pOtherMat != NULL) {
    *pm = val * *pOtherMat;
    return 1;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected FLOATmatrix3D or float value");

  *pm = val * f;
  return 1;
};

static SQInteger Div(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected float value");

  *pm = val / f;
  return 1;
};

static SQInteger UnaryMinus(HSQUIRRELVM v, FLOATmatrix3D &val) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  *pm = !val;
  return 1;
};

static SQInteger ToString(HSQUIRRELVM v, FLOATmatrix3D &val) {
  CTString str(0, "[%g, %g, %g; %g, %g, %g; %g, %g, %g]",
    val(1, 1), val(1, 2), val(1, 3),
    val(2, 1), val(2, 2), val(2, 3),
    val(3, 1), val(3, 2), val(3, 3));
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

SQCLASS_GETSET_FLOAT(Get11, Set11, FLOATmatrix3D, val(1, 1), val(1, 1));
SQCLASS_GETSET_FLOAT(Get12, Set12, FLOATmatrix3D, val(1, 2), val(1, 2));
SQCLASS_GETSET_FLOAT(Get13, Set13, FLOATmatrix3D, val(1, 3), val(1, 3));

SQCLASS_GETSET_FLOAT(Get21, Set21, FLOATmatrix3D, val(2, 1), val(2, 1));
SQCLASS_GETSET_FLOAT(Get22, Set22, FLOATmatrix3D, val(2, 2), val(2, 2));
SQCLASS_GETSET_FLOAT(Get23, Set23, FLOATmatrix3D, val(2, 3), val(2, 3));

SQCLASS_GETSET_FLOAT(Get31, Set31, FLOATmatrix3D, val(3, 1), val(3, 1));
SQCLASS_GETSET_FLOAT(Get32, Set32, FLOATmatrix3D, val(3, 2), val(3, 2));
SQCLASS_GETSET_FLOAT(Get33, Set33, FLOATmatrix3D, val(3, 3), val(3, 3));

static SQInteger Diagonal(HSQUIRRELVM v, int, FLOATmatrix3D &val) {
  // Get a vector value
  GetInstanceValue(FLOAT3D, pOtherVec, v, 2);

  if (pOtherVec != NULL) {
    val.Diagonal(*pOtherVec);
    return 0;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, 2, &f))) return sq_throwerror(v, "expected FLOAT3D or float value");

  val.Diagonal(f);
  return 0;
};

static SQInteger GetRow(HSQUIRRELVM v, int, FLOATmatrix3D &val) {
  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.GetRow(i);
  return 1;
};

static SQInteger GetColumn(HSQUIRRELVM v, int, FLOATmatrix3D &val) {
  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.GetColumn(i);
  return 1;
};

static Method<FLOATmatrix3D> _aMethods[] = {
  { "Diagonal",  &Diagonal,  2, ".n|x" },
  { "GetRow",    &GetRow,    2, ".n" },
  { "GetColumn", &GetColumn, 2, ".n" },
};

}; // namespace

// FLOATaabbox3D class methods
namespace SqBox {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, FLOATaabbox3D &val) {
  if (ctArgs <= 0) return 0;

  // Try to get the starting point as the first argument
  GetInstanceValueVerify(FLOAT3D, pPoint, v, 2);

  if (ctArgs > 1) {
    GetInstanceValue(FLOAT3D, pMax, v, 3);
    SQFloat fRadius;

    // Try creating a box with min and max values
    if (pMax != NULL) {
      val = FLOATaabbox3D(*pPoint, *pMax);
      return 0;

    // Or a box with a center point and a radius
    } else if (SQ_SUCCEEDED(sq_getfloat(v, 3, &fRadius))) {
      val = FLOATaabbox3D(*pPoint, fRadius);
      return 0;
    }

    return sq_throwerror(v, "expected FLOAT3D or number value in argument 2");
  }

  // Or just a simple point
  val = FLOATaabbox3D(*pPoint);
  return 0;
};

static SQInteger Clone(HSQUIRRELVM v, FLOATaabbox3D &val, FLOATaabbox3D &valOther) {
  val = valOther;
  return 0;
};

static SQInteger AddPos(HSQUIRRELVM v, FLOATaabbox3D &val, SQInteger idxOther) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  *pbox = val;
  *pbox += *pOther;
  return 1;
};

static SQInteger SubPos(HSQUIRRELVM v, FLOATaabbox3D &val, SQInteger idxOther) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, idxOther);

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  *pbox = val;
  *pbox -= *pOther;
  return 1;
};

static SQInteger Union(HSQUIRRELVM v, FLOATaabbox3D &val, SQInteger idxOther) {
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, idxOther);

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  *pbox = val;
  *pbox |= *pOther;
  return 1;
};

static SQInteger Intersection(HSQUIRRELVM v, FLOATaabbox3D &val, SQInteger idxOther) {
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, idxOther);

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  *pbox = val;
  *pbox &= *pOther;
  return 1;
};

static SQInteger ToString(HSQUIRRELVM v, FLOATaabbox3D &val) {
  CTString str(0, "box[%g, %g, %g; %g, %g, %g]",
    val.Min()(1), val.Min()(2), val.Min()(3),
    val.Max()(1), val.Max()(2), val.Max()(3));
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

SQCLASS_GET_INSTANCE(GetMin, FLOATaabbox3D, FLOAT3D, val.minvect);
SQCLASS_SET_INSTANCE(SetMin, FLOATaabbox3D, FLOAT3D, val.minvect);

SQCLASS_GET_INSTANCE(GetMax, FLOATaabbox3D, FLOAT3D, val.maxvect);
SQCLASS_SET_INSTANCE(SetMax, FLOATaabbox3D, FLOAT3D, val.maxvect);

static SQInteger Equal(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, 2);
  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger SetToNormalizedEmpty(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  val.SetToNormalizedEmpty();
  return 0;
};

static SQInteger Size(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.Size();
  return 1;
};

static SQInteger Center(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val.Center();
  return 1;
};

static SQInteger IsEmpty(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  sq_pushbool(v, val.IsEmpty());
  return 1;
};

static SQInteger Contains(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, 2);
  sq_pushbool(v, (val >= *pOther));
  return 1;
};

static SQInteger IsInside(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, 2);
  sq_pushbool(v, (val <= *pOther));
  return 1;
};

static SQInteger HasContactWith(HSQUIRRELVM v, int ctArgs, FLOATaabbox3D &val) {
  // Get other box
  GetInstanceValueVerify(FLOATaabbox3D, pOther, v, 2);

  // Get optional epsilon
  SQFloat fEpsilon = 0.0f;
  if (ctArgs > 1) sq_getfloat(v, 3, &fEpsilon);

  sq_pushbool(v, val.HasContactWith(*pOther, fEpsilon));
  return 1;
};

static SQInteger TouchesSphere(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  // Get sphere center
  GetInstanceValueVerify(FLOAT3D, pvCenter, v, 2);

  // Get sphere radius
  SQFloat fRadius;
  sq_getfloat(v, 3, &fRadius);

  sq_pushbool(v, val.TouchesSphere(*pvCenter, fRadius));
  return 1;
};

static SQInteger Expand(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  val.Expand(f);
  return 0;
};

static SQInteger ExpandByFactor(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  val.ExpandByFactor(f);
  return 0;
};

static SQInteger StretchByFactor(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  val.StretchByFactor(f);
  return 0;
};

static SQInteger StretchByVector(HSQUIRRELVM v, int, FLOATaabbox3D &val) {
  GetInstanceValueVerify(FLOAT3D, pv, v, 2);
  val.StretchByVector(*pv);
  return 0;
};

static Method<FLOATaabbox3D> _aMethods[] = {
  { "Equal", &Equal, 2, ".x" },
  { "SetToNormalizedEmpty", &SetToNormalizedEmpty, 1, "." },

  { "Size",    &Size,    1, "." },
  { "Center",  &Center,  1, "." },
  { "IsEmpty", &IsEmpty, 1, "." },

  { "Contains",       &Contains,        2, ".x" },
  { "IsInside",       &IsInside,        2, ".x" },
  { "HasContactWith", &HasContactWith, -2, ".xn" },
  { "TouchesSphere",  &TouchesSphere,   3, ".xn" },

  { "Expand",           &Expand,          2, ".n" },
  { "ExpandByFactor",   &ExpandByFactor,  2, ".n" },
  { "StretchByFactor",  &StretchByFactor, 2, ".n" },
  { "StretchByVector",  &StretchByVector, 2, ".x" },
};

}; // namespace

// CPlacement3D class methods
namespace SqPlacement {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CPlacement3D &val) {
  val = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

  if (ctArgs > 0) {
    GetInstanceValueVerify(FLOAT3D, pPos, v, 2);
    if (pPos != NULL) val.pl_PositionVector = *pPos;
  }

  if (ctArgs > 1) {
    GetInstanceValueVerify(FLOAT3D, pRot, v, 3);
    if (pRot != NULL) val.pl_OrientationAngle = *pRot;
  }

  return 0;
};

static SQInteger Clone(HSQUIRRELVM v, CPlacement3D &val, CPlacement3D &valOther) {
  val = valOther;
  return 0;
};

static SQInteger ToString(HSQUIRRELVM v, CPlacement3D &val) {
  CTString str(0, "pl([%g, %g, %g]; [%g, %g, %g])",
    val.pl_PositionVector(1), val.pl_PositionVector(2), val.pl_PositionVector(3),
    val.pl_OrientationAngle(1), val.pl_OrientationAngle(2), val.pl_OrientationAngle(3));
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

SQCLASS_GET_INSTANCE(GetPos, CPlacement3D, FLOAT3D, val.pl_PositionVector);
SQCLASS_SET_INSTANCE(SetPos, CPlacement3D, FLOAT3D, val.pl_PositionVector);
SQCLASS_GET_INSTANCE(GetRot, CPlacement3D, FLOAT3D, val.pl_OrientationAngle);
SQCLASS_SET_INSTANCE(SetRot, CPlacement3D, FLOAT3D, val.pl_OrientationAngle);

static SQInteger Rotate_TrackBall(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  val.Rotate_TrackBall(*pOther);
  return 0;
};

static SQInteger Rotate_Airplane(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  val.Rotate_Airplane(*pOther);
  return 0;
};

static SQInteger Rotate_HPB(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  val.Rotate_HPB(*pOther);
  return 0;
};

static SQInteger Translate_OwnSystem(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  val.Translate_OwnSystem(*pOther);
  return 0;
};

static SQInteger Translate_AbsoluteSystem(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(FLOAT3D, pOther, v, 2);
  val.Translate_AbsoluteSystem(*pOther);
  return 0;
};

static SQInteger GetDirectionVector(HSQUIRRELVM v, int, CPlacement3D &val) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  val.GetDirectionVector(*pv);
  return 1;
};

static SQInteger AbsoluteToRelative(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther, v, 2);
  val.AbsoluteToRelative(*pOther);
  return 0;
};

static SQInteger AbsoluteToRelativeSmooth(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther, v, 2);
  val.AbsoluteToRelativeSmooth(*pOther);
  return 0;
};

static SQInteger RelativeToAbsolute(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther, v, 2);
  val.RelativeToAbsolute(*pOther);
  return 0;
};

static SQInteger RelativeToAbsoluteSmooth(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther, v, 2);
  val.RelativeToAbsoluteSmooth(*pOther);
  return 0;
};

static SQInteger RelativeToRelative(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther1, v, 2);
  GetInstanceValueVerify(CPlacement3D, pOther2, v, 3);
  val.RelativeToRelative(*pOther1, *pOther2);
  return 0;
};

static SQInteger RelativeToRelativeSmooth(HSQUIRRELVM v, int, CPlacement3D &val) {
  GetInstanceValueVerify(CPlacement3D, pOther1, v, 2);
  GetInstanceValueVerify(CPlacement3D, pOther2, v, 3);
  val.RelativeToRelativeSmooth(*pOther1, *pOther2);
  return 0;
};

static SQInteger Lerp(HSQUIRRELVM v, int, CPlacement3D &val) {
  // Get two placement values
  GetInstanceValueVerify(CPlacement3D, pOther1, v, 2);
  GetInstanceValueVerify(CPlacement3D, pOther2, v, 3);

  // Get a float value
  SQFloat fFactor;
  if (SQ_FAILED(sq_getfloat(v, 4, &fFactor))) return sq_throwerror(v, "expected float value");

  val.Lerp(*pOther1, *pOther2, fFactor);
  return 0;
};

static Method<CPlacement3D> _aMethods[] = {
  { "Rotate_TrackBall",         &Rotate_TrackBall,         2, ".x" },
  { "Rotate_Airplane",          &Rotate_Airplane,          2, ".x" },
  { "Rotate_HPB",               &Rotate_HPB,               2, ".x" },
  { "Translate_OwnSystem",      &Translate_OwnSystem,      2, ".x" },
  { "Translate_AbsoluteSystem", &Translate_AbsoluteSystem, 2, ".x" },
  { "GetDirectionVector",       &GetDirectionVector,       1, "." },
  { "AbsoluteToRelative",       &AbsoluteToRelative,       2, ".x" },
  { "AbsoluteToRelativeSmooth", &AbsoluteToRelativeSmooth, 2, ".x" },
  { "RelativeToAbsolute",       &RelativeToAbsolute,       2, ".x" },
  { "RelativeToAbsoluteSmooth", &RelativeToAbsoluteSmooth, 2, ".x" },
  { "RelativeToRelative",       &RelativeToRelative,       3, ".xx" },
  { "RelativeToRelativeSmooth", &RelativeToRelativeSmooth, 3, ".xx" },
  { "Lerp",                     &Lerp,                     4, ".xxn" },
};

}; // namespace

namespace Math {

static SQInteger NormFloatToByte(HSQUIRRELVM v) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  sq_pushinteger(v, ::NormFloatToByte(f));
  return 1;
};

static SQInteger NormByteToFloat(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushfloat(v, ::NormByteToFloat(i));
  return 1;
};

static SQInteger WrapAngle(HSQUIRRELVM v) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  sq_pushfloat(v, ::WrapAngle(f));
  return 1;
};

static SQInteger NormalizeAngle(HSQUIRRELVM v) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  sq_pushfloat(v, ::NormalizeAngle(f));
  return 1;
};

static SQInteger RadToDeg(HSQUIRRELVM v) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  sq_pushfloat(v, ::AngleRad(f));
  return 1;
};

static SQInteger DegToRad(HSQUIRRELVM v) {
  SQFloat f;
  sq_getfloat(v, 2, &f);
  sq_pushfloat(v, ::RadAngle(f));
  return 1;
};

static SQInteger MakeRotationMatrix(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pv, v, 2);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrix(*pm, *pv);
  return 1;
};

static SQInteger MakeRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pv, v, 2);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrixFast(*pm, *pv);
  return 1;
};

static SQInteger MakeInverseRotationMatrix(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pv, v, 2);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrix(*pm, *pv);
  return 1;
};

static SQInteger MakeInverseRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pv, v, 2);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrixFast(*pm, *pv);
  return 1;
};

static SQInteger DecomposeRotationMatrix(HSQUIRRELVM v) {
  // Get a matrix value
  GetInstanceValueVerify(FLOATmatrix3D, pm, v, 2);

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrix(*pv, *pm);
  return 1;
};

static SQInteger DecomposeRotationMatrixNoSnap(HSQUIRRELVM v) {
  // Get a matrix value
  GetInstanceValueVerify(FLOATmatrix3D, pm, v, 2);

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrixNoSnap(*pv, *pm);
  return 1;
};

static SQInteger AnglesToDirectionVector(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pvAngles, v, 2);

  // Create a vector instance
  FLOAT3D *pvDir;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvDir)) return SQ_ERROR;

  ::AnglesToDirectionVector(*pvAngles, *pvDir);
  return 1;
};

static SQInteger DirectionVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAngles(*pvDir, *pvAngles);
  return 1;
};

static SQInteger DirectionVectorToAnglesNoSnap(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAnglesNoSnap(*pvDir, *pvAngles);
  return 1;
};

static SQInteger UpVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  GetInstanceValueVerify(FLOAT3D, pvDir, v, 2);

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::UpVectorToAngles(*pvDir, *pvAngles);
  return 1;
};

}; // namespace

// "Math" namespace functions
static SQRegFunction _aMathFuncs[] = {
  { "NormFloatToByte", &Math::NormFloatToByte, 2, ".n" },
  { "NormByteToFloat", &Math::NormByteToFloat, 2, ".n" },
  { "WrapAngle",       &Math::WrapAngle, 2, ".n" },
  { "NormalizeAngle",  &Math::NormalizeAngle, 2, ".n" },
  { "RadToDeg",        &Math::RadToDeg, 2, ".n" },
  { "DegToRad",        &Math::DegToRad, 2, ".n" },

  { "MakeRotationMatrix",            &Math::MakeRotationMatrix, 2, ".x" },
  { "MakeRotationMatrixFast",        &Math::MakeRotationMatrixFast, 2, ".x" },
  { "MakeInverseRotationMatrix",     &Math::MakeInverseRotationMatrix, 2, ".x" },
  { "MakeInverseRotationMatrixFast", &Math::MakeInverseRotationMatrixFast, 2, ".x" },
  { "DecomposeRotationMatrix",       &Math::DecomposeRotationMatrix, 2, ".x" },
  { "DecomposeRotationMatrixNoSnap", &Math::DecomposeRotationMatrixNoSnap, 2, ".x" },
  { "AnglesToDirectionVector",       &Math::AnglesToDirectionVector, 2, ".x" },
  { "DirectionVectorToAngles",       &Math::DirectionVectorToAngles, 2, ".x" },
  { "DirectionVectorToAnglesNoSnap", &Math::DirectionVectorToAnglesNoSnap, 2, ".x" },
  { "UpVectorToAngles",              &Math::UpVectorToAngles, 2, ".x" },
};

void VM::RegisterMath(void) {
  Table sqtMath = Root().RegisterTable("Math");
  INDEX i;

  // Register classes
  {
    Class<FLOAT3D> sqcVector(GetVM(), "FLOAT3D", &SqVector::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqVector::_aMethods); i++) {
      sqcVector.RegisterMethod(SqVector::_aMethods[i]);
    }

    // Metamethods
    sqcVector.RegisterMetamethod(E_MM_CLONED, &SqVector::Clone);
    sqcVector.RegisterMetamethod(E_MM_ADD, &SqVector::Add);
    sqcVector.RegisterMetamethod(E_MM_SUB, &SqVector::Sub);
    sqcVector.RegisterMetamethod(E_MM_MUL, &SqVector::Mul);
    sqcVector.RegisterMetamethod(E_MM_DIV, &SqVector::Div);
    sqcVector.RegisterMetamethod(E_MM_MODULO, &SqVector::Mod);
    sqcVector.RegisterMetamethod(E_MM_UNM, &SqVector::UnaryMinus);
    sqcVector.RegisterMetamethod(E_MM_TOSTRING, &SqVector::ToString);

    // Position axes
    sqcVector.RegisterVar("x", &SqVector::GetX, &SqVector::SetX);
    sqcVector.RegisterVar("y", &SqVector::GetY, &SqVector::SetY);
    sqcVector.RegisterVar("z", &SqVector::GetZ, &SqVector::SetZ);

    // Rotation angles
    sqcVector.RegisterVar("h", &SqVector::GetX, &SqVector::SetX);
    sqcVector.RegisterVar("p", &SqVector::GetY, &SqVector::SetY);
    sqcVector.RegisterVar("b", &SqVector::GetZ, &SqVector::SetZ);

    // Indices
    sqcVector.RegisterVar(1, &SqVector::GetX, &SqVector::SetX);
    sqcVector.RegisterVar(2, &SqVector::GetY, &SqVector::SetY);
    sqcVector.RegisterVar(3, &SqVector::GetZ, &SqVector::SetZ);

    Root().AddClass(sqcVector);

    // Derive the plane from the vector
    DerivedClass<FLOATplane3D> sqcPlane(GetVM(), "FLOATplane3D", sqcVector, &SqPlane::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqPlane::_aMethods); i++) {
      sqcPlane.RegisterMethod(SqPlane::_aMethods[i]);
    }

    // Metamethods
    sqcPlane.RegisterMetamethod(E_MM_CLONED, &SqPlane::Clone);
    sqcPlane.RegisterMetamethod(E_MM_ADD, &SqPlane::Add);
    sqcPlane.RegisterMetamethod(E_MM_SUB, &SqPlane::Sub);
    sqcPlane.RegisterMetamethod(E_MM_MUL, &SqPlane::Mul);
    sqcPlane.RegisterMetamethod(E_MM_UNM, &SqPlane::UnaryMinus);
    sqcPlane.RegisterMetamethod(E_MM_TOSTRING, &SqPlane::ToString);

    // Plane fields
    sqcPlane.RegisterVar("vec",  &SqPlane::GetVector, &SqPlane::SetVector);
    sqcPlane.RegisterVar("dist", &SqPlane::GetDist,   &SqPlane::SetDist);

    Root().AddClass(sqcPlane);
  }
  {
    Class<FLOATmatrix3D> sqcMatrix(GetVM(), "FLOATmatrix3D", &SqMatrix::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqMatrix::_aMethods); i++) {
      sqcMatrix.RegisterMethod(SqMatrix::_aMethods[i]);
    }

    // Metamethods
    sqcMatrix.RegisterMetamethod(E_MM_CLONED, &SqMatrix::Clone);
    sqcMatrix.RegisterMetamethod(E_MM_ADD, &SqMatrix::Add);
    sqcMatrix.RegisterMetamethod(E_MM_SUB, &SqMatrix::Sub);
    sqcMatrix.RegisterMetamethod(E_MM_MUL, &SqMatrix::Mul);
    sqcMatrix.RegisterMetamethod(E_MM_DIV, &SqMatrix::Div);
    sqcMatrix.RegisterMetamethod(E_MM_UNM, &SqMatrix::UnaryMinus);
    sqcMatrix.RegisterMetamethod(E_MM_TOSTRING, &SqMatrix::ToString);

    // Row and column values
    sqcMatrix.RegisterVar("m11", &SqMatrix::Get11, &SqMatrix::Set11);
    sqcMatrix.RegisterVar("m12", &SqMatrix::Get12, &SqMatrix::Set12);
    sqcMatrix.RegisterVar("m13", &SqMatrix::Get13, &SqMatrix::Set13);

    sqcMatrix.RegisterVar("m21", &SqMatrix::Get21, &SqMatrix::Set21);
    sqcMatrix.RegisterVar("m22", &SqMatrix::Get22, &SqMatrix::Set22);
    sqcMatrix.RegisterVar("m23", &SqMatrix::Get23, &SqMatrix::Set23);

    sqcMatrix.RegisterVar("m31", &SqMatrix::Get31, &SqMatrix::Set31);
    sqcMatrix.RegisterVar("m32", &SqMatrix::Get32, &SqMatrix::Set32);
    sqcMatrix.RegisterVar("m33", &SqMatrix::Get33, &SqMatrix::Set33);

    Root().AddClass(sqcMatrix);
  }
  {
    Class<FLOATaabbox3D> sqcBox(GetVM(), "FLOATaabbox3D", &SqBox::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqBox::_aMethods); i++) {
      sqcBox.RegisterMethod(SqBox::_aMethods[i]);
    }

    // Metamethods
    sqcBox.RegisterMetamethod(E_MM_CLONED, &SqBox::Clone);
    sqcBox.RegisterMetamethod(E_MM_ADD, &SqBox::AddPos);
    sqcBox.RegisterMetamethod(E_MM_SUB, &SqBox::SubPos);
    sqcBox.RegisterMetamethod(E_MM_MUL, &SqBox::Union);
    sqcBox.RegisterMetamethod(E_MM_DIV, &SqBox::Intersection);
    sqcBox.RegisterMetamethod(E_MM_TOSTRING, &SqBox::ToString);

    // Min and max points
    sqcBox.RegisterVar("min", &SqBox::GetMin, &SqBox::SetMin);
    sqcBox.RegisterVar("max", &SqBox::GetMax, &SqBox::SetMax);

    Root().AddClass(sqcBox);
  }
  {
    Class<CPlacement3D> sqcPlacement(GetVM(), "CPlacement3D", &SqPlacement::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqPlacement::_aMethods); i++) {
      sqcPlacement.RegisterMethod(SqPlacement::_aMethods[i]);
    }

    // Metamethods
    sqcPlacement.RegisterMetamethod(E_MM_CLONED, &SqPlacement::Clone);
    sqcPlacement.RegisterMetamethod(E_MM_TOSTRING, &SqPlacement::ToString);

    // Position and rotation
    sqcPlacement.RegisterVar("pos", &SqPlacement::GetPos, &SqPlacement::SetPos);
    sqcPlacement.RegisterVar("rot", &SqPlacement::GetRot, &SqPlacement::SetRot);

    sqcPlacement.RegisterVar("pl_PositionVector",   &SqPlacement::GetPos, &SqPlacement::SetPos);
    sqcPlacement.RegisterVar("pl_OrientationAngle", &SqPlacement::GetRot, &SqPlacement::SetRot);

    Root().AddClass(sqcPlacement);
  }

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aMathFuncs); i++) {
    sqtMath.RegisterFunc(_aMathFuncs[i]);
  }
};

}; // namespace
