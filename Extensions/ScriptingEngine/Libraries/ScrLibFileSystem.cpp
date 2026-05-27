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

#include "RawDataBuffer.h"

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

// CTFileStream class methods
namespace SqStream {

struct Stream {
  CTFileStream strm;
  bool bOpen; // Whether the stream is currently open
  bool bWriting; // Whether writing or reading

  Stream() : bOpen(false), bWriting(false) {};
};

static SQInteger Create(HSQUIRRELVM v, int, Stream &val) {
  const SQChar *strPath;

  if (SQ_FAILED(sq_getstring(v, 2, &strPath))) {
    return sq_throwerror(v, "expected a path to a file in argument 1");
  }

  if (val.bOpen) {
    return sq_throwerror(v, val.bWriting ? "stream is already open for writing" : "stream is already open for reading");
  }

  // Make sure the directory exists
  CTString fnm = strPath;
  IDir::CreateDir(fnm);

  try {
    val.strm.Create_t(fnm);
    val.bOpen = true;
    val.bWriting = true;

  } catch(char *strError) {
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger Open(HSQUIRRELVM v, int, Stream &val) {
  const SQChar *strPath;

  if (SQ_FAILED(sq_getstring(v, 2, &strPath))) {
    return sq_throwerror(v, "expected a path to a file in argument 1");
  }

  if (val.bOpen) {
    return sq_throwerror(v, val.bWriting ? "stream is already open for writing" : "stream is already open for reading");
  }

  try {
    val.strm.Open_t(CTString(strPath));
    val.bOpen = true;
    val.bWriting = false;

  } catch(char *strError) {
    return sq_throwerror(v, strError);
  }

  return 0;
};

static SQInteger Close(HSQUIRRELVM v, int, Stream &val) {
  if (!val.bOpen) return 0;

  val.strm.Close();
  val.bOpen = false;
  return 0;
};

static SQInteger IsOpen(HSQUIRRELVM v, int, Stream &val) {
  sq_pushbool(v, val.bOpen);
  return 1;
};

#define ASSERT_OPEN      { if (!val.bOpen) return sq_throwerror(v, "stream isn't open"); }
#define ASSERT_CAN_WRITE { if (!val.bOpen || !val.bWriting) return sq_throwerror(v, "stream isn't open for writing"); }
#define ASSERT_CAN_READ  { if (!val.bOpen ||  val.bWriting) return sq_throwerror(v, "stream isn't open for reading"); }

static SQInteger GetDescription(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;
  sq_pushstring(v, val.strm.GetDescription().str_String, -1);
  return 1;
};

static SQInteger Seek(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;

  SQInteger iOffset, iSeekDir;
  sq_getinteger(v, 2, &iOffset);
  sq_getinteger(v, 3, &iSeekDir);

  try {
    val.strm.Seek_t(iOffset, (CTStream::SeekDir)iSeekDir);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger SetPos(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;

  SQInteger iPosition;
  sq_getinteger(v, 2, &iPosition);

  try {
    val.strm.SetPos_t(iPosition);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger GetPos(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;
  SLONG slPos;

  try {
    slPos = val.strm.GetPos_t();
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, slPos);
  return 1;
};

static SQInteger AtEOF(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;
  sq_pushbool(v, val.strm.AtEOF());
  return 1;
};

static SQInteger GetSize(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;
  sq_pushinteger(v, val.strm.GetStreamSize());
  return 1;
};

static SQInteger GetCRC32(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_OPEN;
  ULONG ulCRC;

  try {
    ulCRC = val.strm.GetStreamCRC32_t();
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, ulCRC);
  return 1;
};

static SQInteger WriteFloat(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQFloat f;
  sq_getfloat(v, 2, &f);

  try {
    val.strm << (FLOAT)f;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteDouble(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQFloat f;
  sq_getfloat(v, 2, &f);

  try {
    val.strm << (DOUBLE)f;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteU8(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (UBYTE)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteU16(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (UWORD)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteU32(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (ULONG)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteU64(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  unsigned __int64 iU64 = i;

  try {
    val.strm.Write_t(&iU64, sizeof(iU64));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteS8(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (SBYTE)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteS16(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (SWORD)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteS32(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  try {
    val.strm << (SLONG)i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteS64(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQInteger i;
  sq_getinteger(v, 2, &i);

  __int64 iS64 = i;

  try {
    val.strm.Write_t(&iS64, sizeof(iS64));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteBool(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  SQBool b;
  sq_getbool(v, 2, &b);

  try {
    val.strm << (BOOL)b;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteString(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm << CTString(str);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteStringNull(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm.Write_t(str, strlen(str) + 1);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteFilename(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm << CTFileName(CTString(str));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteBuffer(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  GetInstanceValueVerify(CRawDataBuffer, pbuf, v, 2);

  try {
    val.strm.Write_t(pbuf->aData.sa_Array, pbuf->aData.Count());
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger ReadFloat(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  FLOAT f;

  try {
    val.strm >> f;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushfloat(v, f);
  return 1;
};

static SQInteger ReadDouble(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  DOUBLE f;

  try {
    val.strm >> f;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushfloat(v, f);
  return 1;
};

static SQInteger ReadU8(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  UBYTE i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadU16(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  UWORD i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadU32(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  ULONG i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadU64(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  unsigned __int64 i;

  try {
    val.strm.Read_t(&i, sizeof(i));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadS8(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  SBYTE i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadS16(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  SWORD i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadS32(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  SLONG i;

  try {
    val.strm >> i;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadS64(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  __int64 i;

  try {
    val.strm.Read_t(&i, sizeof(i));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushinteger(v, i);
  return 1;
};

static SQInteger ReadBool(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  BOOL b;

  try {
    val.strm >> b;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushbool(v, b);
  return 1;
};

static SQInteger ReadString(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  CTString str;

  try {
    val.strm >> str;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger ReadStringNull(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  UBYTE ub;
  SLONG slPos, slLen;
  CTString str = "";

  try {
    slPos = val.strm.GetPos_t();

    do {
      val.strm >> ub;
    } while (ub != 0);

    slLen = val.strm.GetPos_t() - slPos;
    val.strm.SetPos_t(slPos);

    char *pBuffer = (char *)AllocMemory(slLen);
    val.strm.Read_t(pBuffer, slLen);
    str = pBuffer;
    FreeMemory(pBuffer);

  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger ReadFilename(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  CTFileName fnm;

  try {
    val.strm >> fnm;
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, fnm.str_String, -1);
  return 1;
};

static SQInteger ReadBuffer(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  SQInteger ct;
  sq_getinteger(v, 2, &ct);

  VM &vm = GetVMClass(v);

  vm.SetArgumentBypass(true);
  PushNewInstance(CRawDataBuffer, pbuf, vm.Root(), "CRawDataBuffer");
  vm.SetArgumentBypass(false);

  pbuf->New(ct);

  try {
    val.strm.Read_t(pbuf->aData.sa_Array, ct);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 1;
};

static SQInteger PutLine(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm.PutLine_t(str);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger PutString(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;
  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm.PutString_t(str);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger GetLine(HSQUIRRELVM v, int ctArgs, Stream &val) {
  ASSERT_CAN_READ;

  SQInteger iDelimiter = '\n';
  if (ctArgs > 0) sq_getinteger(v, 2, &iDelimiter);

  CTString str;

  try {
    val.strm.GetLine_t(str, (char)(UBYTE)iDelimiter);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, str.str_String, -1);
  return 1;
};

static SQInteger ExpectKeyword(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;

  const SQChar *str;
  sq_getstring(v, 2, &str);

  try {
    val.strm.ExpectKeyword_t(str);
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger WriteID(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_WRITE;

  const SQChar *strID;
  sq_getstring(v, 2, &strID);

  try {
    val.strm.WriteID_t(CChunkID(strID));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger ExpectID(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;

  const SQChar *strID;
  sq_getstring(v, 2, &strID);

  try {
    val.strm.ExpectID_t(CChunkID(strID));
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }
  return 0;
};

static SQInteger PeekID(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  CChunkID cid;

  try {
    cid = val.strm.PeekID_t();
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, cid.cid_ID, 4);
  return 1;
};

static SQInteger GetID(HSQUIRRELVM v, int, Stream &val) {
  ASSERT_CAN_READ;
  CChunkID cid;

  try {
    cid = val.strm.GetID_t();
  } catch (char *strError) {
    return sq_throwerror(v, strError);
  }

  sq_pushstring(v, cid.cid_ID, 4);
  return 1;
};

static Method<Stream> _aMethods[] = {
  { "Create", &Create, 2, ".s" },
  { "Open",   &Open,   2, ".s" },
  { "Close",  &Close,  1, "." },
  { "IsOpen", &IsOpen, 1, "." },
  { "GetDescription", &GetDescription, 1, "." },

  // Stream navigation
  { "Seek",     &Seek,     3, ".nn" },
  { "SetPos",   &SetPos,   2, ".n" },
  { "GetPos",   &GetPos,   1, "." },
  { "AtEOF",    &AtEOF,    1, "." },
  { "GetSize",  &GetSize,  1, "." },
  { "GetCRC32", &GetCRC32, 1, "." },

  // Binary writing
  { "WriteFloat",      &WriteFloat,      2, ".n" },
  { "WriteDouble",     &WriteDouble,     2, ".n" },
  { "WriteU8",         &WriteU8,         2, ".n" },
  { "WriteU16",        &WriteU16,        2, ".n" },
  { "WriteU32",        &WriteU32,        2, ".n" },
  { "WriteU64",        &WriteU64,        2, ".n" },
  { "WriteS8",         &WriteS8,         2, ".n" },
  { "WriteS16",        &WriteS16,        2, ".n" },
  { "WriteS32",        &WriteS32,        2, ".n" },
  { "WriteS64",        &WriteS64,        2, ".n" },
  { "WriteBool",       &WriteBool,       2, ".b" },
  { "WriteString",     &WriteString,     2, ".s" },
  { "WriteStringNull", &WriteStringNull, 2, ".s" },
  { "WriteFilename",   &WriteFilename,   2, ".s" },
  { "WriteBuffer",     &WriteBuffer,     2, ".x" },

  // Binary reading
  { "ReadFloat",      &ReadFloat,      1, "." },
  { "ReadDouble",     &ReadDouble,     1, "." },
  { "ReadU8",         &ReadU8,         1, "." },
  { "ReadU16",        &ReadU16,        1, "." },
  { "ReadU32",        &ReadU32,        1, "." },
  { "ReadU64",        &ReadU64,        1, "." },
  { "ReadS8",         &ReadS8,         1, "." },
  { "ReadS16",        &ReadS16,        1, "." },
  { "ReadS32",        &ReadS32,        1, "." },
  { "ReadS64",        &ReadS64,        1, "." },
  { "ReadBool",       &ReadBool,       1, "." },
  { "ReadString",     &ReadString,     1, "." },
  { "ReadStringNull", &ReadStringNull, 1, "." },
  { "ReadFilename",   &ReadFilename,   1, "." },
  { "ReadBuffer",     &ReadBuffer,     2, ".n" },

  // Text
  { "PutLine",       &PutLine,       2, ".s" },
  { "PutString",     &PutString,     2, ".s" },
  { "GetLine",       &GetLine,      -1, ".n" },
  { "ExpectKeyword", &ExpectKeyword, 2, ".s" },

  // Chunk IDs
  { "WriteID",  &WriteID,  2, ".s" },
  { "ExpectID", &ExpectID, 2, ".s" },
  { "PeekID",   &PeekID,   1, "." },
  { "GetID",    &GetID,    1, "." },
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
  {
    Class<SqStream::Stream> sqcWriter(GetVM(), "Stream", NULL);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqStream::_aMethods); i++) {
      sqcWriter.RegisterMethod(SqStream::_aMethods[i]);
    }

    sqtFileSystem.AddClass(sqcWriter);
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

  // Seek directions
  Enumeration enSeekDir(GetVM());

#define ADD_SEEKDIR(_DirName) enSeekDir.RegisterValue(#_DirName, (SQInteger)SEEK_##_DirName)
  ADD_SEEKDIR(SET);
  ADD_SEEKDIR(CUR);
  ADD_SEEKDIR(END);
#undef ADD_SEEKDIR

  Const().AddEnum("SEEK", enSeekDir);
};

}; // namespace
