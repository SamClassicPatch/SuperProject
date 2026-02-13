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

static SQInteger Constructor(HSQUIRRELVM v, FLOAT3D &val) {
  val = FLOAT3D(0, 0, 0);

  SQFloat f;
  if (SQ_SUCCEEDED(sq_getfloat(v, 2, &f))) val(1) = f;
  if (SQ_SUCCEEDED(sq_getfloat(v, 3, &f))) val(2) = f;
  if (SQ_SUCCEEDED(sq_getfloat(v, 4, &f))) val(3) = f;

  return 0;
};

static SQInteger Add(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val + *pOther;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val - *pOther;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a matrix value
  FLOATmatrix3D *pOtherMat = InstanceValueOfType(v, idxOther, FLOATmatrix3D);

  if (pOtherMat != NULL) {
    *pv = val * *pOtherMat;
    return 1;
  }

  // Or a vector value
  FLOAT3D *pOtherVec = InstanceValueOfType(v, idxOther, FLOAT3D);

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
  FLOAT3D *pOther = InstanceValueOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

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

static SQInteger Length(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  sq_pushfloat(v, pVal->Length());
  return 1;
};

static SQInteger ManhattanNorm(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  sq_pushfloat(v, pVal->ManhattanNorm());
  return 1;
};

static SQInteger MaxNorm(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  sq_pushfloat(v, pVal->MaxNorm());
  return 1;
};

static SQInteger Normalize(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pVal->Normalize();
  return 1;
};

static SQInteger SafeNormalize(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pVal->SafeNormalize();
  return 1;
};

static SQInteger Flip(HSQUIRRELVM v) {
  FLOAT3D *pVal = InstanceValueOfType(v, 1, FLOAT3D);
  if (pVal == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pVal->Flip();
  return 1;
};

static SQRegFunction _aMethods[] = {
  { "Length",        &Length, 1, "." },
  { "ManhattanNorm", &ManhattanNorm, 1, "." },
  { "MaxNorm",       &MaxNorm, 1, "." },
  { "Normalize",     &Normalize, 1, "." },
  { "SafeNormalize", &SafeNormalize, 1, "." },
  { "Flip",          &Flip, 1, "." },
};

}; // namespace

// FLOATmatrix3D class methods
namespace SqMatrix {

static SQInteger Constructor(HSQUIRRELVM v, FLOATmatrix3D &val) {
  SQFloat f;

  if (SQ_SUCCEEDED(sq_getfloat(v, 2, &f))) {
    val = FLOATmatrix3D(f);
  } else {
    val = FLOATmatrix3D(0);
  }

  return 0;
};

static SQInteger Add(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  FLOATmatrix3D *pOther = InstanceValueOfType(v, idxOther, FLOATmatrix3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  *pm = val + *pOther;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  FLOATmatrix3D *pOther = InstanceValueOfType(v, idxOther, FLOATmatrix3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  *pm = val - *pOther;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  FLOATmatrix3D *pOtherMat = InstanceValueOfType(v, idxOther, FLOATmatrix3D);

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

static SQInteger Diagonal(HSQUIRRELVM v) {
  FLOATmatrix3D *pVal = InstanceValueOfType(v, 1, FLOATmatrix3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOtherVec = InstanceValueOfType(v, 2, FLOAT3D);

  if (pOtherVec != NULL) {
    pVal->Diagonal(*pOtherVec);
    return 0;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, 2, &f))) return sq_throwerror(v, "expected FLOAT3D or float value");

  pVal->Diagonal(f);
  return 0;
};

static SQInteger GetRow(HSQUIRRELVM v) {
  FLOATmatrix3D *pVal = InstanceValueOfType(v, 1, FLOATmatrix3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pVal->GetRow(i);
  return 1;
};

static SQInteger GetColumn(HSQUIRRELVM v) {
  FLOATmatrix3D *pVal = InstanceValueOfType(v, 1, FLOATmatrix3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pVal->GetColumn(i);
  return 1;
};

static SQRegFunction _aMethods[] = {
  { "Diagonal",  &Diagonal, 2, ".n|x" },
  { "GetRow",    &GetRow, 2, ".n" },
  { "GetColumn", &GetColumn, 2, ".n" },
};

}; // namespace

// CPlacement3D class methods
namespace SqPlacement {

static SQInteger Constructor(HSQUIRRELVM v, CPlacement3D &val) {
  val = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

  FLOAT3D *pPos = InstanceValueOfType(v, 2, FLOAT3D);
  if (pPos != NULL) val.pl_PositionVector = *pPos;

  FLOAT3D *pRot = InstanceValueOfType(v, 3, FLOAT3D);
  if (pRot != NULL) val.pl_OrientationAngle = *pRot;

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

static SQInteger Rotate_TrackBall(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, 2, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  pVal->Rotate_TrackBall(*pOther);
  return 0;
};

static SQInteger Rotate_Airplane(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, 2, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  pVal->Rotate_Airplane(*pOther);
  return 0;
};

static SQInteger Rotate_HPB(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, 2, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  pVal->Rotate_HPB(*pOther);
  return 0;
};

static SQInteger Translate_OwnSystem(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, 2, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  pVal->Translate_OwnSystem(*pOther);
  return 0;
};

static SQInteger Translate_AbsoluteSystem(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a vector value
  FLOAT3D *pOther = InstanceValueOfType(v, 2, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  pVal->Translate_AbsoluteSystem(*pOther);
  return 0;
};

static SQInteger GetDirectionVector(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  pVal->GetDirectionVector(*pv);
  return 1;
};

static SQInteger AbsoluteToRelative(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a placement value
  CPlacement3D *pOther = InstanceValueOfType(v, 2, CPlacement3D);
  if (pOther == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  pVal->AbsoluteToRelative(*pOther);
  return 0;
};

static SQInteger AbsoluteToRelativeSmooth(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a placement value
  CPlacement3D *pOther = InstanceValueOfType(v, 2, CPlacement3D);
  if (pOther == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  pVal->AbsoluteToRelativeSmooth(*pOther);
  return 0;
};

static SQInteger RelativeToAbsolute(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a placement value
  CPlacement3D *pOther = InstanceValueOfType(v, 2, CPlacement3D);
  if (pOther == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  pVal->RelativeToAbsolute(*pOther);
  return 0;
};

static SQInteger RelativeToAbsoluteSmooth(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get a placement value
  CPlacement3D *pOther = InstanceValueOfType(v, 2, CPlacement3D);
  if (pOther == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  pVal->RelativeToAbsoluteSmooth(*pOther);
  return 0;
};

static SQInteger RelativeToRelative(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get two placement values
  CPlacement3D *pOther1 = InstanceValueOfType(v, 2, CPlacement3D);
  CPlacement3D *pOther2 = InstanceValueOfType(v, 3, CPlacement3D);
  if (pOther1 == NULL || pOther2 == NULL) return sq_throwerror(v, "expected two CPlacement3D values");

  pVal->RelativeToRelative(*pOther1, *pOther2);
  return 0;
};

static SQInteger RelativeToRelativeSmooth(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get two placement values
  CPlacement3D *pOther1 = InstanceValueOfType(v, 2, CPlacement3D);
  CPlacement3D *pOther2 = InstanceValueOfType(v, 3, CPlacement3D);
  if (pOther1 == NULL || pOther2 == NULL) return sq_throwerror(v, "expected two CPlacement3D values");

  pVal->RelativeToRelativeSmooth(*pOther1, *pOther2);
  return 0;
};

static SQInteger Lerp(HSQUIRRELVM v) {
  CPlacement3D *pVal = InstanceValueOfType(v, 1, CPlacement3D);
  if (pVal == NULL) return SQ_ERROR;

  // Get two placement values
  CPlacement3D *pOther1 = InstanceValueOfType(v, 2, CPlacement3D);
  if (pOther1 == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  CPlacement3D *pOther2 = InstanceValueOfType(v, 3, CPlacement3D);
  if (pOther2 == NULL) return sq_throwerror(v, "expected CPlacement3D value");

  // Get a float value
  SQFloat fFactor;
  if (SQ_FAILED(sq_getfloat(v, 4, &fFactor))) return sq_throwerror(v, "expected float value");

  pVal->Lerp(*pOther1, *pOther2, fFactor);
  return 0;
};

static SQRegFunction _aMethods[] = {
  { "Rotate_TrackBall",         &Rotate_TrackBall, 2, ".x" },
  { "Rotate_Airplane",          &Rotate_Airplane, 2, ".x" },
  { "Rotate_HPB",               &Rotate_HPB, 2, ".x" },
  { "Translate_OwnSystem",      &Translate_OwnSystem, 2, ".x" },
  { "Translate_AbsoluteSystem", &Translate_AbsoluteSystem, 2, ".x" },
  { "GetDirectionVector",       &GetDirectionVector, 1, "." },
  { "AbsoluteToRelative",       &AbsoluteToRelative, 2, ".x" },
  { "AbsoluteToRelativeSmooth", &AbsoluteToRelativeSmooth, 2, ".x" },
  { "RelativeToAbsolute",       &RelativeToAbsolute, 2, ".x" },
  { "RelativeToAbsoluteSmooth", &RelativeToAbsoluteSmooth, 2, ".x" },
  { "RelativeToRelative",       &RelativeToRelative, 3, ".xx" },
  { "RelativeToRelativeSmooth", &RelativeToRelativeSmooth, 3, ".xx" },
  { "Lerp",                     &Lerp, 4, ".xxn" },
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
  FLOAT3D *pv = InstanceValueOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrix(*pm, *pv);
  return 1;
};

static SQInteger MakeRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pv = InstanceValueOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrixFast(*pm, *pv);
  return 1;
};

static SQInteger MakeInverseRotationMatrix(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pv = InstanceValueOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrix(*pm, *pv);
  return 1;
};

static SQInteger MakeInverseRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pv = InstanceValueOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrixFast(*pm, *pv);
  return 1;
};

static SQInteger DecomposeRotationMatrix(HSQUIRRELVM v) {
  // Get a matrix value
  FLOATmatrix3D *pm = InstanceValueOfType(v, 2, FLOATmatrix3D);
  if (pm == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrix(*pv, *pm);
  return 1;
};

static SQInteger DecomposeRotationMatrixNoSnap(HSQUIRRELVM v) {
  // Get a matrix value
  FLOATmatrix3D *pm = InstanceValueOfType(v, 2, FLOATmatrix3D);
  if (pm == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrixNoSnap(*pv, *pm);
  return 1;
};

static SQInteger AnglesToDirectionVector(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pvAngles = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvAngles == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvDir;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvDir)) return SQ_ERROR;

  ::AnglesToDirectionVector(*pvAngles, *pvDir);
  return 1;
};

static SQInteger DirectionVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pvDir = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAngles(*pvDir, *pvAngles);
  return 1;
};

static SQInteger DirectionVectorToAnglesNoSnap(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pvDir = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAnglesNoSnap(*pvDir, *pvAngles);
  return 1;
};

static SQInteger UpVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  FLOAT3D *pvDir = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

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
  Table sqtMath = Root().AddTable("Math");
  INDEX i;

  // Register classes
  {
    Class<FLOAT3D> sqcVector(GetVM(), "FLOAT3D", &SqVector::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqVector::_aMethods); i++) {
      sqcVector.RegisterFunc(SqVector::_aMethods[i]);
    }

    // Metamethods
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
    sqcVector.RegisterIndex(1, &SqVector::GetX, &SqVector::SetX);
    sqcVector.RegisterIndex(2, &SqVector::GetY, &SqVector::SetY);
    sqcVector.RegisterIndex(3, &SqVector::GetZ, &SqVector::SetZ);

    Root().SetClass(sqcVector);
  }
  {
    Class<FLOATmatrix3D> sqcMatrix(GetVM(), "FLOATmatrix3D", &SqMatrix::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqMatrix::_aMethods); i++) {
      sqcMatrix.RegisterFunc(SqMatrix::_aMethods[i]);
    }

    // Metamethods
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

    Root().SetClass(sqcMatrix);
  }
  {
    Class<CPlacement3D> sqcPlacement(GetVM(), "CPlacement3D", &SqPlacement::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqPlacement::_aMethods); i++) {
      sqcPlacement.RegisterFunc(SqPlacement::_aMethods[i]);
    }

    // Metamethods
    sqcPlacement.RegisterMetamethod(E_MM_TOSTRING, &SqPlacement::ToString);

    // Position and rotation
    sqcPlacement.RegisterVar("pos", &SqPlacement::GetPos, &SqPlacement::SetPos);
    sqcPlacement.RegisterVar("rot", &SqPlacement::GetRot, &SqPlacement::SetRot);

    sqcPlacement.RegisterVar("pl_PositionVector",   &SqPlacement::GetPos, &SqPlacement::SetPos);
    sqcPlacement.RegisterVar("pl_OrientationAngle", &SqPlacement::GetRot, &SqPlacement::SetRot);

    Root().SetClass(sqcPlacement);
  }

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aMathFuncs); i++) {
    sqtMath.RegisterFunc(_aMathFuncs[i]);
  }
};

}; // namespace
