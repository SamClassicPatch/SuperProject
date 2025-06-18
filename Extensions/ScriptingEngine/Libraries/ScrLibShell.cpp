/* Copyright (c) 2025 Dreamy Cecil
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

namespace Shell {

static SQInteger Execute(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  _pShell->Execute(str + CTString(";"));
  return 1;
};

static SQInteger GetFloat(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  sq_pushfloat(v, _pShell->GetFLOAT(str));
  return 1;
};

static SQInteger SetFloat(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  SQFloat fValue;
  sq_getfloat(v, 3, &fValue);

  _pShell->SetFLOAT(str, fValue);
  return 1;
};

static SQInteger GetIndex(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  sq_pushinteger(v, _pShell->GetINDEX(str));
  return 1;
};

static SQInteger SetIndex(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  SQInteger iValue;
  sq_getinteger(v, 3, &iValue);

  _pShell->SetINDEX(str, iValue);
  return 1;
};

static SQInteger GetString(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  sq_pushstring(v, _pShell->GetString(str).str_String, -1);
  return 1;
};

static SQInteger SetString(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  const SQChar *strValue = "";
  sq_getstring(v, 3, &strValue);

  _pShell->SetString(str, strValue);
  return 1;
};

static SQInteger GetValue(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  sq_pushstring(v, _pShell->GetValue(str).str_String, -1);
  return 1;
};

static SQInteger SetValue(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  const SQChar *strValue = "";
  sq_getstring(v, 3, &strValue);

  _pShell->SetValue(str, strValue);
  return 1;
};

}; // namespace

// "Shell" namespace functions
static SQRegFunction _aShellFuncs[] = {
  { "Execute",   &Shell::Execute, 2, ".s" },
  { "GetFloat",  &Shell::GetFloat, 2, ".s" },
  { "SetFloat",  &Shell::SetFloat, 3, ".sn" },
  { "GetIndex",  &Shell::GetIndex, 2, ".s" },
  { "SetIndex",  &Shell::SetIndex, 3, ".sn|b" },
  { "GetString", &Shell::GetString, 2, ".s" },
  { "SetString", &Shell::SetString, 3, ".ss" },
  { "GetValue",  &Shell::GetValue, 2, ".s" },
  { "SetValue",  &Shell::SetValue, 3, ".ss" },
};

void VM::RegisterShell(void) {
  Table sqtShell = Root().AddTable("Shell");

  // Register functions
  for (INDEX i = 0; i < ARRAYCOUNT(_aShellFuncs); i++) {
    sqtShell.RegisterFunc(_aShellFuncs[i]);
  }
};

}; // namespace
