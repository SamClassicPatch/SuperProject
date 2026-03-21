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

// CFontData class methods
namespace SqFontData {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CFontData &val) {
  const SQChar *strPath;

  if (ctArgs == 0 || SQ_FAILED(sq_getstring(v, 2, &strPath))) {
    return sq_throwerror(v, "expected a path to a font in argument 1");
  }

  try {
    val.Load_t(CTString(strPath));
  } catch(char *strError) {
    return sq_throwerror(v, strError);
  }

  return 0;
};

SQCLASS_GETSET_INT(GetCharSpacing, SetCharSpacing, CFontData, val.fd_pixCharSpacing, val.fd_pixCharSpacing);
SQCLASS_GETSET_INT(GetLineSpacing, SetLineSpacing, CFontData, val.fd_pixLineSpacing, val.fd_pixLineSpacing);
SQCLASS_GETSET_BOOL(GetFixedWidth, SetFixedWidth, CFontData, val.fd_bFixedWidth, val.fd_bFixedWidth);

static SQInteger GetTextureFile(HSQUIRRELVM v, int, CFontData &val) {
  sq_pushstring(v, val.fd_fnTexture.str_String, -1);
  return 1;
};

static SQInteger GetWidth(HSQUIRRELVM v, int, CFontData &val) {
  sq_pushinteger(v, val.GetWidth());
  return 1;
};

static SQInteger GetHeight(HSQUIRRELVM v, int, CFontData &val) {
  sq_pushinteger(v, val.GetHeight());
  return 1;
};

static Method<CFontData> _aMethods[] = {
  { "GetTextureFile", &GetTextureFile, 1, "." },
  { "GetWidth",       &GetWidth,       1, "." },
  { "GetHeight",      &GetHeight,      1, "." },
};

}; // namespace

namespace FileSystem {

}; // namespace

void VM::RegisterFileSystem(void) {
  Table sqtFileSystem = Root().RegisterTable("FileSystem");
  INDEX i;

  // Register classes
  {
    Class<CFontData> sqcFont(GetVM(), "CFontData", &SqFontData::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqFontData::_aMethods); i++) {
      sqcFont.RegisterMethod(SqFontData::_aMethods[i]);
    }

    sqcFont.RegisterVar("pixCharSpacing", &SqFontData::GetCharSpacing, &SqFontData::SetCharSpacing);
    sqcFont.RegisterVar("pixLineSpacing", &SqFontData::GetLineSpacing, &SqFontData::SetLineSpacing);
    sqcFont.RegisterVar("bFixedWidth",    &SqFontData::GetFixedWidth,  &SqFontData::SetFixedWidth);

    Root().AddClass(sqcFont);
  }
};

}; // namespace
