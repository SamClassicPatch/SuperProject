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
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  Instance<FLOAT3D> *pOther = InstanceOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val + pOther->val;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a vector value
  Instance<FLOAT3D> *pOther = InstanceOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val - pOther->val;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a matrix value
  Instance<FLOATmatrix3D> *pOtherMat = InstanceOfType(v, idxOther, FLOATmatrix3D);

  if (pOtherMat != NULL) {
    *pv = val * pOtherMat->val;
    return 1;
  }

  // Or a vector value
  Instance<FLOAT3D> *pOtherVec = InstanceOfType(v, idxOther, FLOAT3D);

  if (pOtherVec != NULL) {
    *pv = val * pOtherVec->val;
    return 1;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected FLOATmatrix3D or FLOAT3D or float value");

  *pv = val * f;
  return 1;
};

static SQInteger Div(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected float value");

  *pv = val / f;
  return 1;
};

static SQInteger Mod(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  Instance<FLOAT3D> *pOther = InstanceOfType(v, idxOther, FLOAT3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  sq_pushfloat(v, val % pOther->val);
  return 1;
};

static SQInteger UnaryMinus(HSQUIRRELVM v, FLOAT3D &val) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = -val;
  return 1;
};

static SQInteger ToString(HSQUIRRELVM v, FLOAT3D &val) {
  CTString str(0, "[%g, %g, %g]", val(1), val(2), val(3));
  sq_pushstring(v, str.str_String, -1);
  return 1;
};

#define VECTOR_AXIS_FUNC(_Get, _Set, _AxisIndex) \
  static SQInteger _Get(HSQUIRRELVM v, FLOAT3D &val) { \
    sq_pushfloat(v, val(_AxisIndex)); \
    return 1; \
  }; \
  static SQInteger _Set(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxValue) { \
    if (SQ_FAILED(sq_getfloat(v, idxValue, &val(_AxisIndex)))) { \
      return sq_throwerror(v, "expected a number value for a vector axis"); \
    } \
    return 1; \
  };

VECTOR_AXIS_FUNC(GetX, SetX, 1);
VECTOR_AXIS_FUNC(GetY, SetY, 2);
VECTOR_AXIS_FUNC(GetZ, SetZ, 3);

static SQInteger Length(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.Length());
  return 1;
};

static SQInteger ManhattanNorm(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.ManhattanNorm());
  return 1;
};

static SQInteger MaxNorm(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.MaxNorm());
  return 1;
};

static SQInteger Normalize(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.Normalize();
  return 1;
};

static SQInteger SafeNormalize(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.SafeNormalize();
  return 1;
};

static SQInteger Flip(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = InstanceOfType(v, 1, FLOAT3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.Flip();
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
  VM &vm = GetVMClass(v);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!vm.Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  Instance<FLOATmatrix3D> *pOther = InstanceOfType(v, idxOther, FLOATmatrix3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  *pm = val + pOther->val;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!vm.Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  Instance<FLOATmatrix3D> *pOther = InstanceOfType(v, idxOther, FLOATmatrix3D);
  if (pOther == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  *pm = val - pOther->val;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!vm.Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a matrix value
  Instance<FLOATmatrix3D> *pOtherMat = InstanceOfType(v, idxOther, FLOATmatrix3D);

  if (pOtherMat != NULL) {
    *pm = val * pOtherMat->val;
    return 1;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected FLOATmatrix3D or float value");

  *pm = val * f;
  return 1;
};

static SQInteger Div(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!vm.Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  // Get a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected float value");

  *pm = val / f;
  return 1;
};

static SQInteger UnaryMinus(HSQUIRRELVM v, FLOATmatrix3D &val) {
  VM &vm = GetVMClass(v);

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!vm.Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

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

#define MATRIX_VALUE_FUNC(_Get, _Set, _RowIndex, _ColumnIndex) \
  static SQInteger _Get(HSQUIRRELVM v, FLOATmatrix3D &val) { \
    sq_pushfloat(v, val(_RowIndex, _ColumnIndex)); \
    return 1; \
  }; \
  static SQInteger _Set(HSQUIRRELVM v, FLOATmatrix3D &val, SQInteger idxValue) { \
    if (SQ_FAILED(sq_getfloat(v, idxValue, &val(_RowIndex, _ColumnIndex)))) { \
      return sq_throwerror(v, "expected a number value for a matrix value"); \
    } \
    return 1; \
  };

MATRIX_VALUE_FUNC(Get11, Set11, 1, 1);
MATRIX_VALUE_FUNC(Get12, Set12, 1, 2);
MATRIX_VALUE_FUNC(Get13, Set13, 1, 3);

MATRIX_VALUE_FUNC(Get21, Set21, 2, 1);
MATRIX_VALUE_FUNC(Get22, Set22, 2, 2);
MATRIX_VALUE_FUNC(Get23, Set23, 2, 3);

MATRIX_VALUE_FUNC(Get31, Set31, 3, 1);
MATRIX_VALUE_FUNC(Get32, Set32, 3, 2);
MATRIX_VALUE_FUNC(Get33, Set33, 3, 3);

static SQInteger Diagonal(HSQUIRRELVM v) {
  Instance<FLOATmatrix3D> *pInstance = InstanceOfType(v, 1, FLOATmatrix3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Get a vector value
  Instance<FLOAT3D> *pOtherVec = InstanceOfType(v, 2, FLOAT3D);

  if (pOtherVec != NULL) {
    pInstance->val.Diagonal(pOtherVec->val);
    return 0;
  }

  // Or a float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, 2, &f))) return sq_throwerror(v, "expected FLOAT3D or float value");

  pInstance->val.Diagonal(f);
  return 0;
};

static SQInteger GetRow(HSQUIRRELVM v) {
  Instance<FLOATmatrix3D> *pInstance = InstanceOfType(v, 1, FLOATmatrix3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.GetRow(i);
  return 1;
};

static SQInteger GetColumn(HSQUIRRELVM v) {
  Instance<FLOATmatrix3D> *pInstance = InstanceOfType(v, 1, FLOATmatrix3D);
  if (pInstance == NULL) return SQ_ERROR;

  // Get an index
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, 2, &i))) return sq_throwerror(v, "expected number value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.GetColumn(i);
  return 1;
};

static SQRegFunction _aMethods[] = {
  { "Diagonal",  &Diagonal, 2, ".n|x" },
  { "GetRow",    &GetRow, 2, ".n" },
  { "GetColumn", &GetColumn, 2, ".n" },
};

}; // namespace

namespace Utils {

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
  Instance<FLOAT3D> *pv = InstanceOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrix(*pm, pv->val);
  return 1;
};

static SQInteger MakeRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pv = InstanceOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeRotationMatrixFast(*pm, pv->val);
  return 1;
};

static SQInteger MakeInverseRotationMatrix(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pv = InstanceOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrix(*pm, pv->val);
  return 1;
};

static SQInteger MakeInverseRotationMatrixFast(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pv = InstanceOfType(v, 2, FLOAT3D);
  if (pv == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  ::MakeInverseRotationMatrixFast(*pm, pv->val);
  return 1;
};

static SQInteger DecomposeRotationMatrix(HSQUIRRELVM v) {
  // Get a matrix value
  Instance<FLOATmatrix3D> *pm = InstanceOfType(v, 2, FLOATmatrix3D);
  if (pm == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrix(*pv, pm->val);
  return 1;
};

static SQInteger DecomposeRotationMatrixNoSnap(HSQUIRRELVM v) {
  // Get a matrix value
  Instance<FLOATmatrix3D> *pm = InstanceOfType(v, 2, FLOATmatrix3D);
  if (pm == NULL) return sq_throwerror(v, "expected FLOATmatrix3D value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  ::DecomposeRotationMatrixNoSnap(*pv, pm->val);
  return 1;
};

static SQInteger AnglesToDirectionVector(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pvAngles = InstanceOfType(v, 2, FLOAT3D);
  if (pvAngles == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvDir;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvDir)) return SQ_ERROR;

  ::AnglesToDirectionVector(pvAngles->val, *pvDir);
  return 1;
};

static SQInteger DirectionVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pvDir = InstanceOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAngles(pvDir->val, *pvAngles);
  return 1;
};

static SQInteger DirectionVectorToAnglesNoSnap(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pvDir = InstanceOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::DirectionVectorToAnglesNoSnap(pvDir->val, *pvAngles);
  return 1;
};

static SQInteger UpVectorToAngles(HSQUIRRELVM v) {
  // Get a vector value
  Instance<FLOAT3D> *pvDir = InstanceOfType(v, 2, FLOAT3D);
  if (pvDir == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pvAngles;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pvAngles)) return SQ_ERROR;

  ::UpVectorToAngles(pvDir->val, *pvAngles);
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
  vm.CompileFromFile(strFile);

  if (!vm.CanBeExecuted()) {
    // Pass compilation error
    return sq_throwerror(v, vm.GetError());
  }

  // Return compiled closure
  return 1;
};

}; // namespace

// "Utils" namespace functions
static SQRegFunction _aUtilsFuncs[] = {
  { "NormFloatToByte", &Utils::NormFloatToByte, 2, ".n" },
  { "NormByteToFloat", &Utils::NormByteToFloat, 2, ".n" },
  { "WrapAngle",       &Utils::WrapAngle, 2, ".n" },
  { "NormalizeAngle",  &Utils::NormalizeAngle, 2, ".n" },
  { "RadToDeg",        &Utils::RadToDeg, 2, ".n" },
  { "DegToRad",        &Utils::DegToRad, 2, ".n" },

  { "MakeRotationMatrix",            &Utils::MakeRotationMatrix, 2, ".x" },
  { "MakeRotationMatrixFast",        &Utils::MakeRotationMatrixFast, 2, ".x" },
  { "MakeInverseRotationMatrix",     &Utils::MakeInverseRotationMatrix, 2, ".x" },
  { "MakeInverseRotationMatrixFast", &Utils::MakeInverseRotationMatrixFast, 2, ".x" },
  { "DecomposeRotationMatrix",       &Utils::DecomposeRotationMatrix, 2, ".x" },
  { "DecomposeRotationMatrixNoSnap", &Utils::DecomposeRotationMatrixNoSnap, 2, ".x" },
  { "AnglesToDirectionVector",       &Utils::AnglesToDirectionVector, 2, ".x" },
  { "DirectionVectorToAngles",       &Utils::DirectionVectorToAngles, 2, ".x" },
  { "DirectionVectorToAnglesNoSnap", &Utils::DirectionVectorToAnglesNoSnap, 2, ".x" },
  { "UpVectorToAngles",              &Utils::UpVectorToAngles, 2, ".x" },
};

// Global functions
static SQRegFunction _aGlobalFuncs[] = {
  { "IncludeScript", &Utils::IncludeScript, 2, ".s" },
  { "CompileScript", &Utils::CompileScript, 2, ".s" },
};

void VM::RegisterUtils(void) {
  Table sqtUtils = Root().AddTable("Utils");
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

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aUtilsFuncs); i++) {
    sqtUtils.RegisterFunc(_aUtilsFuncs[i]);
  }

  for (i = 0; i < ARRAYCOUNT(_aGlobalFuncs); i++) {
    Root().RegisterFunc(_aGlobalFuncs[i]);
  }
};

}; // namespace
