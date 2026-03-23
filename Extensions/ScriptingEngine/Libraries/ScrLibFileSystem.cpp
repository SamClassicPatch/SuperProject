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

// CTextureObject class methods
namespace SqTexture {

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, CTextureObject &val) {
  const SQChar *strPath;

  if (ctArgs == 0 || SQ_FAILED(sq_getstring(v, 2, &strPath))) {
    return sq_throwerror(v, "expected a path to a texture in argument 1");
  }

  try {
    val.SetData_t(CTString(strPath));
    ((CTextureData *)val.GetData())->Force(TEX_CONSTANT);

  } catch(char *strError) {
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger NextAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  val.NextAnim();
  return 0;
};

static SQInteger PrevAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  val.PrevAnim();
  return 0;
};

static SQInteger NextFrame(HSQUIRRELVM v, int, CTextureObject &val) {
  val.NextFrame();
  return 0;
};

static SQInteger PrevFrame(HSQUIRRELVM v, int, CTextureObject &val) {
  val.PrevFrame();
  return 0;
};

static SQInteger FirstFrame(HSQUIRRELVM v, int, CTextureObject &val) {
  val.FirstFrame();
  return 0;
};

static SQInteger LastFrame(HSQUIRRELVM v, int, CTextureObject &val) {
  val.LastFrame();
  return 0;
};

static SQInteger GetCurrentFrame(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, val.GetFrame());
  return 1;
};

static SQInteger GetInBetweenFrames(HSQUIRRELVM v, int, CTextureObject &val) {
  INDEX iFrame0, iFrame1;
  FLOAT fRatio;
  val.GetFrame(iFrame0, iFrame1, fRatio);

  sq_newarray(v, 0);
  sq_pushinteger(v, iFrame0);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, iFrame1);
  sq_arrayappend(v, -2);
  sq_pushfloat(v, fRatio);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger GetAnimCount(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, val.GetAnimsCt());
  return 1;
};

static SQInteger GetAnimInfo(HSQUIRRELVM v, int, CTextureObject &val) {
  SQInteger iAnim;
  sq_getinteger(v, 2, &iAnim);

  CAnimInfo ai;
  val.GetAnimInfo(iAnim, ai);

  sq_newarray(v, 0);
  sq_pushstring(v, ai.ai_AnimName, -1);
  sq_arrayappend(v, -2);
  sq_pushinteger(v, ai.ai_NumberOfFrames);
  sq_arrayappend(v, -2);
  sq_pushfloat(v, ai.ai_SecsPerFrame);
  sq_arrayappend(v, -2);
  return 1;
};

static SQInteger GetAnimLength(HSQUIRRELVM v, int, CTextureObject &val) {
  SQInteger iAnim;
  sq_getinteger(v, 2, &iAnim);
  sq_pushfloat(v, val.GetAnimLength(iAnim));
  return 1;
};

static SQInteger GetCurrentAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, val.GetAnim());
  return 1;
};

static SQInteger GetCurrentAnimFrames(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, val.GetFramesInCurrentAnim());
  return 1;
};

static SQInteger GetCurrentAnimLength(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushfloat(v, val.GetCurrentAnimLength());
  return 1;
};

static SQInteger SetSpeed(HSQUIRRELVM v, int, CTextureObject &val) {
  SQInteger iAnim;
  sq_getinteger(v, 2, &iAnim);
  SQFloat fSpeed;
  sq_getfloat(v, 3, &fSpeed);
  val.GetData()->SetSpeed(iAnim, fSpeed);
  return 0;
};

static SQInteger PlayAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  SQInteger iAnim, iFlags;
  sq_getinteger(v, 2, &iAnim);
  sq_getinteger(v, 3, &iFlags);
  val.PlayAnim(iAnim, iFlags);
  return 0;
};

static SQInteger SwitchToAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  SQInteger iAnim;
  sq_getinteger(v, 2, &iAnim);
  val.SwitchToAnim(iAnim);
  return 0;
};

static SQInteger PauseAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  val.PauseAnim();
  return 0;
};

static SQInteger ContinueAnim(HSQUIRRELVM v, int, CTextureObject &val) {
  val.ContinueAnim();
  return 0;
};

static SQInteger IsPaused(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushbool(v, val.IsPaused());
  return 1;
};

static SQInteger IsAnimFinished(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushbool(v, val.IsAnimFinished());
  return 1;
};

static SQInteger SelectFrameInTime(HSQUIRRELVM v, int, CTextureObject &val) {
  SQFloat fOffset;
  sq_getfloat(v, 2, &fOffset);
  val.SelectFrameInTime(fOffset);
  return 0;
};

static SQInteger OffsetPhase(HSQUIRRELVM v, int, CTextureObject &val) {
  SQFloat fTime;
  sq_getfloat(v, 2, &fTime);
  val.OffsetPhase(fTime);
  return 0;
};

static SQInteger GetPassedTime(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushfloat(v, val.GetPassedTime());
  return 1;
};

static SQInteger GetWidth(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushfloat(v, val.GetWidth());
  return 1;
};

static SQInteger GetHeight(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushfloat(v, val.GetHeight());
  return 1;
};

static SQInteger GetPixWidth(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, ((CTextureData *)val.GetData())->GetPixWidth());
  return 1;
};

static SQInteger GetPixHeight(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, ((CTextureData *)val.GetData())->GetPixHeight());
  return 1;
};

static SQInteger GetFlags(HSQUIRRELVM v, int, CTextureObject &val) {
  sq_pushinteger(v, val.GetFlags());
  return 1;
};

static Method<CTextureObject> _aMethods[] = {
  // Animation frames
  { "NextAnim",           &NextAnim,           1, "." },
  { "PrevAnim",           &PrevAnim,           1, "." },
  { "NextFrame",          &NextFrame,          1, "." },
  { "PrevFrame",          &PrevFrame,          1, "." },
  { "FirstFrame",         &FirstFrame,         1, "." },
  { "LastFrame",          &LastFrame,          1, "." },
  { "GetCurrentFrame",    &GetCurrentFrame,    1, "." },
  { "GetInBetweenFrames", &GetInBetweenFrames, 1, "." },

  // Animation info
  { "GetAnimCount",  &GetAnimCount,  1, "." },
  { "GetAnimInfo",   &GetAnimInfo,   2, ".n" },
  { "GetAnimLength", &GetAnimLength, 2, ".n" },

  { "GetCurrentAnim",       &GetCurrentAnim,       1, "." },
  { "GetCurrentAnimFrames", &GetCurrentAnimFrames, 1, "." },
  { "GetCurrentAnimLength", &GetCurrentAnimLength, 1, "." },

  // Animations
  { "SetSpeed",       &SetSpeed,       3, ".nn" },
  { "PlayAnim",       &PlayAnim,       3, ".nn" },
  { "SwitchToAnim",   &SwitchToAnim,   2, ".n" },
  { "PauseAnim",      &PauseAnim,      1, "." },
  { "ContinueAnim",   &ContinueAnim,   1, "." },
  { "IsPaused",       &IsPaused,       1, "." },
  { "IsAnimFinished", &IsAnimFinished, 1, "." },

  { "SelectFrameInTime", &SelectFrameInTime, 2, ".n" },
  { "OffsetPhase",       &OffsetPhase,       2, ".n" },
  { "GetPassedTime",     &GetPassedTime,     1, "." },

  // Texture info
  { "GetWidth",     &GetWidth,     1, "." },
  { "GetHeight",    &GetHeight,    1, "." },
  { "GetPixWidth",  &GetPixWidth,  1, "." },
  { "GetPixHeight", &GetPixHeight, 1, "." },
  { "GetFlags",     &GetFlags,     1, "." },
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
  {
    Class<CTextureObject> sqcTexture(GetVM(), "CTextureObject", &SqTexture::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqTexture::_aMethods); i++) {
      sqcTexture.RegisterMethod(SqTexture::_aMethods[i]);
    }

    Root().AddClass(sqcTexture);
  }

  // Animation flags
  Enumeration enAnimFlags(GetVM());

#define ADD_AOF(_AnimFlagName) enAnimFlags.RegisterValue(#_AnimFlagName, (SQInteger)AOF_##_AnimFlagName)
  ADD_AOF(PAUSED);
  ADD_AOF(LOOPING);
  ADD_AOF(NORESTART);
  ADD_AOF(SMOOTHCHANGE);
#undef ADD_AOF

  Const().AddEnum("AOF", enAnimFlags);
};

}; // namespace
