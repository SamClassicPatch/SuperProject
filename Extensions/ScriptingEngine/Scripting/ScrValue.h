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

#ifndef CECIL_INCL_SQVALUE_H
#define CECIL_INCL_SQVALUE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

namespace sq {

// Abstract base for a holder of any Squirrel value
template<class Type>
struct Value {
  // Must be explicitly defined for specific template classes
  void Push(HSQUIRRELVM v) const;
  bool Get(HSQUIRRELVM v, SQInteger idx);
};

// Define a simple value class
#define SQ_SIMPLEVALUE(_Type, _SqType, _PushFunc, _GetFunc)                               \
  template<>                                                                              \
  struct Value< _Type > {                                                                 \
    _Type val;                                                                            \
    Value(_Type valSet) : val(valSet) {};                                                 \
    inline void Push(HSQUIRRELVM v) const { _PushFunc(v, static_cast< _SqType >(val)); }; \
    inline bool Get(HSQUIRRELVM v, SQInteger idx) {                                       \
      _SqType sqvalue;                                                                    \
      bool res = SQ_SUCCEEDED(_GetFunc(v, idx, &sqvalue));                                \
      val = static_cast< _Type >(sqvalue);                                                \
      return res;                                                                         \
    };                                                                                    \
  };

#pragma warning(push)
#pragma warning(disable : 4800) // For SQBool -> bool truncation

// Define integral types
SQ_SIMPLEVALUE(UBYTE,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(SBYTE,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(UWORD,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(SWORD,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(ULONG,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(SLONG,  SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(FLOAT,  SQFloat,   sq_pushfloat,   sq_getfloat);
SQ_SIMPLEVALUE(DOUBLE, SQFloat,   sq_pushfloat,   sq_getfloat);
SQ_SIMPLEVALUE(bool,   SQBool,    sq_pushbool,    sq_getbool);

// Define possible SQInteger types
SQ_SIMPLEVALUE(int,     SQInteger, sq_pushinteger, sq_getinteger);
SQ_SIMPLEVALUE(__int64, SQInteger, sq_pushinteger, sq_getinteger);

#pragma warning(pop)

// Define null type
template<>
struct Value<void> {
  inline void Push(HSQUIRRELVM v) const { sq_pushnull(v); };
  inline bool Get(HSQUIRRELVM v, SQInteger idx) { return true; };
};

// Define string type
template<>
struct Value<CTString> {
  CTString val;

  Value(const SQChar *strSet) : val(strSet) {};
  Value(const CTString &strSet) : val(strSet) {};

  inline void Push(HSQUIRRELVM v) const {
    sq_pushstring(v, val.str_String, -1);
  };

  inline bool Get(HSQUIRRELVM v, SQInteger idx) {
    const SQChar *str = "";
    bool res = SQ_SUCCEEDED(sq_getstring(v, idx, &str));
    val = str;
    return res;
  };
};

}; // namespace

#endif
