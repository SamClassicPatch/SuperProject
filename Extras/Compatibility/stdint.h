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

#ifndef CECIL_INCL_COMPATIBILITY_STDINT_H
#define CECIL_INCL_COMPATIBILITY_STDINT_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

#ifdef _WIN64
  typedef __int64 intptr_t;
#else
  typedef int intptr_t;
#endif

#ifdef _WIN64
  typedef unsigned __int64 uintptr_t;
#else
  typedef unsigned int uintptr_t;
#endif

#define INT8_MIN   (-127 - 1)
#define INT16_MIN  (-32767 - 1)
#define INT32_MIN  (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807 - 1)
#define INT8_MAX   127
#define INT16_MAX  32767
#define INT32_MAX  2147483647
#define INT64_MAX  9223372036854775807
#define UINT8_MAX  0xffu
#define UINT16_MAX 0xffffu
#define UINT32_MAX 0xffffffffu
#define UINT64_MAX 0xffffffffffffffffu

#endif
