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

  // Get vector value
  Instance<FLOAT3D> *pOther = (Instance<FLOAT3D> *)InstanceAny::OfType(v, idxOther, typeid(FLOAT3D).raw_name());
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val + pOther->val;
  return 1;
};

static SQInteger Sub(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get vector value
  Instance<FLOAT3D> *pOther = (Instance<FLOAT3D> *)InstanceAny::OfType(v, idxOther, typeid(FLOAT3D).raw_name());
  if (pOther == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  *pv = val - pOther->val;
  return 1;
};

static SQInteger Mul(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get vector value
  Instance<FLOAT3D> *pOther = (Instance<FLOAT3D> *)InstanceAny::OfType(v, idxOther, typeid(FLOAT3D).raw_name());

  // Or a float value
  if (pOther == NULL) {
    SQFloat f;
    if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected FLOAT3D or float value");

    *pv = val * f;
    return 1;
  }

  *pv = val * pOther->val;
  return 1;
};

static SQInteger Div(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  VM &vm = GetVMClass(v);

  // Create a vector instance
  FLOAT3D *pv;
  if (!vm.Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  // Get float value
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxOther, &f))) return sq_throwerror(v, "expected float value");

  *pv = val / f;
  return 1;
};

static SQInteger Mod(HSQUIRRELVM v, FLOAT3D &val, SQInteger idxOther) {
  Instance<FLOAT3D> *pOther = (Instance<FLOAT3D> *)InstanceAny::OfType(v, idxOther, typeid(FLOAT3D).raw_name());
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
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.Length());
  return 1;
};

static SQInteger ManhattanNorm(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.ManhattanNorm());
  return 1;
};

static SQInteger MaxNorm(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
  if (pInstance == NULL) return SQ_ERROR;

  sq_pushfloat(v, pInstance->val.MaxNorm());
  return 1;
};

static SQInteger Normalize(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
  if (pInstance == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.Normalize();
  return 1;
};

static SQInteger SafeNormalize(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
  if (pInstance == NULL) return SQ_ERROR;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = pInstance->val.SafeNormalize();
  return 1;
};

static SQInteger Flip(HSQUIRRELVM v) {
  Instance<FLOAT3D> *pInstance = (Instance<FLOAT3D> *)InstanceAny::OfType(v, 1, typeid(FLOAT3D).raw_name());
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

    Root().SetClass(sqcVector);
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
