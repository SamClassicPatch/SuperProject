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

// Timer.Value class methods
namespace SqTimerValue {

static SQInteger Constructor(HSQUIRRELVM v, CTimerValue &tv) {
  SQFloat fOptSec;

  // Optional value
  if (SQ_SUCCEEDED(sq_getfloat(v, 2, &fOptSec))) {
    tv = CTimerValue((DOUBLE)fOptSec);

  // Default value
  } else {
    tv.tv_llValue = 0;
  }

  // No error
  return 0;
};

static SQInteger Sec(HSQUIRRELVM v, CTimerValue &tv) {
  sq_pushfloat(v, tv.GetSeconds());
  return 1;
};

static SQInteger MSec(HSQUIRRELVM v, CTimerValue &tv) {
  sq_pushinteger(v, tv.GetMilliseconds());
  return 1;
};

}; // namespace

namespace Timer {

static SQInteger GetRealTimeTick(HSQUIRRELVM v) {
  sq_pushfloat(v, _pTimer->GetRealTimeTick());
  return 1;
};

static SQInteger CurrentTick(HSQUIRRELVM v) {
  sq_pushfloat(v, _pTimer->CurrentTick());
  return 1;
};

static SQInteger GetLerpedCurrentTick(HSQUIRRELVM v) {
  sq_pushfloat(v, _pTimer->GetLerpedCurrentTick());
  return 1;
};

static SQInteger GetLerpFactor(HSQUIRRELVM v) {
  sq_pushfloat(v, _pTimer->GetLerpFactor());
  return 1;
};

static SQInteger GetLerpFactor2(HSQUIRRELVM v) {
  sq_pushfloat(v, _pTimer->GetLerpFactor2());
  return 1;
};

static SQInteger GetHighPrecisionTimer(HSQUIRRELVM v) {
  Table sqtTimer(v, 1);

  // Create a timer value instance
  CTimerValue *ptv;
  if (!sqtTimer.CreateInstanceOf("Value", &ptv)) return SQ_ERROR;

  // And set its value
  *ptv = _pTimer->GetHighPrecisionTimer();
  return 1;
};

}; // namespace

// "Timer" namespace functions
static SQRegFunction _aTimerFuncs[] = {
  { "GetRealTimeTick",       &Timer::GetRealTimeTick, 1, "." },
  { "CurrentTick",           &Timer::CurrentTick, 1, "." },
  { "GetLerpedCurrentTick",  &Timer::GetLerpedCurrentTick, 1, "." },
  { "GetLerpFactor",         &Timer::GetLerpFactor, 1, "." },
  { "GetLerpFactor2",        &Timer::GetLerpFactor2, 1, "." },
  { "GetHighPrecisionTimer", &Timer::GetHighPrecisionTimer, 1, "." },
};

void VM::RegisterTimer(void) {
  Table sqtTimer = Root().AddTable("Timer");

  // Register classes
  Class<CTimerValue> sqcTimerValue(GetVM(), "Value", &SqTimerValue::Constructor);
  sqcTimerValue.RegisterVar("sec",  &SqTimerValue::Sec, NULL);
  sqcTimerValue.RegisterVar("msec", &SqTimerValue::MSec, NULL);
  sqtTimer.SetClass(sqcTimerValue);

  // Register functions
  for (INDEX i = 0; i < ARRAYCOUNT(_aTimerFuncs); i++) {
    sqtTimer.RegisterFunc(_aTimerFuncs[i]);
  }

  // Register variables
  Const().SetValue("TickQuantum", _pTimer->TickQuantum);
};

}; // namespace
