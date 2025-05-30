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

// Temporary holder of any Squirrel value outside any VM
class Value {
  private:
    // SQObject is repurposed as an 'any' class that only holds constants
    HSQOBJECT m_obj;

    // For holding strings
    CTString m_str;

  public:
    // Constructor for a null value
    Value() {
      sq_resetobject(&m_obj);
    };

    // Constructor from a bool
    Value(SQBool bSet) {
      m_obj._type = OT_BOOL;
      m_obj._unVal.nInteger = bSet;
    };

    // Constructor from a float
    Value(SQFloat fSet) {
      m_obj._type = OT_FLOAT;
      m_obj._unVal.fFloat = fSet;
    };

    // Constructor from an integer
    Value(SQInteger iSet) {
      m_obj._type = OT_INTEGER;
      m_obj._unVal.nInteger = iSet;
    };

    // Constructor from a string
    Value(const SQChar *strSet) {
      m_obj._type = OT_STRING;
      m_str = strSet;
    };

    // Constructor from an engine string
    Value(const CTString &strSet) {
      m_obj._type = OT_STRING;
      m_str = strSet;
    };

    // Get current value type
    __forceinline SQObjectType GetType(void) const {
      return m_obj._type;
    };

    // Assignment operator
    inline Value &operator=(const Value &other) {
      if (&other == this) return *this;

      m_obj = other.m_obj;
      m_str = other.m_str;

      return *this;
    };

  // VM interactions
  public:

    // Push value onto the stack
    inline void Push(HSQUIRRELVM v) const {
      switch (m_obj._type) {
        case OT_BOOL:    sq_pushbool(v, GetBool()); return;
        case OT_FLOAT:   sq_pushfloat(v, GetFloat()); return;
        case OT_INTEGER: sq_pushinteger(v, GetInt()); return;
        case OT_STRING:  sq_pushstring(v, GetString(), -1); return;

        default:
          ASSERTALWAYS("Invalid value type");
          sq_pushnull(v);
          return;
      }
    };

    // Get value from the stack
    inline void Get(HSQUIRRELVM v, SQInteger idx) {
      m_obj._type = sq_gettype(v, idx);

      switch (m_obj._type) {
        case OT_BOOL:
        case OT_INTEGER: {
          sq_getinteger(v, idx, &m_obj._unVal.nInteger);
        } return;

        case OT_FLOAT: {
          sq_getfloat(v, idx, &m_obj._unVal.fFloat);
        } return;

        case OT_STRING: {
          const SQChar *strGet;
          sq_getstring(v, idx, &strGet);

          m_str = strGet;
        } return;

        default: {
          ASSERTALWAYS("Invalid value type");
          sq_resetobject(&m_obj);
        } return;
      }
    };

  // Value access
  public:

    __forceinline SQBool GetBool(void) const {
      ASSERT(GetType() == OT_BOOL);
      return m_obj._unVal.nInteger ? SQTrue : SQFalse;
    };

    __forceinline SQFloat GetFloat(void) const {
      ASSERT(GetType() == OT_FLOAT);
      return m_obj._unVal.fFloat;
    };

    __forceinline SQInteger GetInt(void) const {
      ASSERT(GetType() == OT_INTEGER);
      return m_obj._unVal.nInteger;
    };

    __forceinline const SQChar *GetString(void) const {
      ASSERT(GetType() == OT_STRING);
      return (const SQChar *)m_str.str_String;
    };
};

}; // namespace

#endif
