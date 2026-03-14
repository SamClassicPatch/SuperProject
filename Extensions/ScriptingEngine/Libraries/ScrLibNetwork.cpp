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

#include <Core/Definitions/NetworkDefs.inl>
#include <Core/Networking/ExtPackets.h>
#include <Core/Networking/NetworkFunctions.h>

#include <Extras/XGizmo/Interfaces/Sounds.h>

// Make sure the client is currently running a server
#define ASSERT_SERVER { if (!_pNetwork->IsServer()) return sq_throwerror(v, "cannot call this function while not hosting a game"); }

namespace sq {

// CPlayerAction methods
namespace SqPlayerAction {

SQCLASS_GET_INSTANCE(GetTranslation, CPlayerAction, FLOAT3D, val.pa_vTranslation);
SQCLASS_SET_INSTANCE(SetTranslation, CPlayerAction, FLOAT3D, val.pa_vTranslation);

SQCLASS_GET_INSTANCE(GetRotation, CPlayerAction, FLOAT3D, val.pa_aRotation);
SQCLASS_SET_INSTANCE(SetRotation, CPlayerAction, FLOAT3D, val.pa_aRotation);

SQCLASS_GET_INSTANCE(GetViewRotation, CPlayerAction, FLOAT3D, val.pa_aViewRotation);
SQCLASS_SET_INSTANCE(SetViewRotation, CPlayerAction, FLOAT3D, val.pa_aViewRotation);

SQCLASS_GETSET_INT(GetButtons, SetButtons, CPlayerAction, val.pa_ulButtons, val.pa_ulButtons);

static SQInteger Clear(HSQUIRRELVM v, int, CPlayerAction &val) {
  val.Clear();
  return 0;
};

static SQInteger Lerp(HSQUIRRELVM v, int, CPlayerAction &val) {
  GetInstanceValueVerify(CPlayerAction, ppa0, v, 2);
  GetInstanceValueVerify(CPlayerAction, ppa1, v, 3);

  SQFloat fFactor;
  sq_getfloat(v, 4, &fFactor);

  val.Lerp(*ppa0, *ppa1, fFactor);
  return 0;
};

static Method<CPlayerAction> _aMethods[] = {
  { "Clear", &Clear, 1, "." },
  { "Lerp",  &Lerp,  4, ".xxn" },
};

}; // namespace

#if _PATCHCONFIG_EXT_PACKETS

// Session properties class methods
namespace SqProps {

// Simple holder of data bytes at a specific offset
struct SesProp {
  INDEX iOffset;
  UBYTE *pData;
  size_t iLength;

  SesProp() : iOffset(0), pData(NULL), iLength(0) {};

  ~SesProp() {
    if (pData != NULL) FreeMemory(pData);
  };

  inline SesProp &operator=(const SesProp &other) {
    SetData(other.iOffset, other.pData, other.iLength);
    return *this;
  };

  inline void SetData(INDEX iSetOffset, const void *pSetData, size_t iSetLength) {
    if (pData != NULL) FreeMemory(pData);

    iOffset = iSetOffset;
    pData = (UBYTE *)AllocMemory(iSetLength);
    iLength = iSetLength;

    memcpy(pData, pSetData, iSetLength);
  };
};

// Holder of a chunk of session properties to apply
struct SesPropsHolder {
  INDEX iBegin, iEnd;
  CStaticStackArray<SesProp> aProps;

  SesPropsHolder() : iBegin(-1), iEnd(-1) {};

  inline BOOL IsValid(void) {
    return (iBegin != -1 && iEnd != -1);
  };

  inline void Clear(void) {
    iBegin = iEnd = -1;
    aProps.PopAll();
  };

  inline void SetLimits(INDEX iSetBegin, INDEX iSetEnd) {
    if (iBegin == -1) {
      iBegin = iSetBegin;
    } else {
      iBegin = Min(iBegin, iSetBegin);
    }

    if (iEnd == -1) {
      iEnd = iSetEnd;
    } else {
      iEnd = Max(iEnd, iSetEnd);
    }
  };
};

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, SesPropsHolder &val) {
  if (ctArgs > 0) {
    GetInstanceValueVerifyN(SesPropsHolder, pOther, v, 2, "Network.SessionProperties");

    val.iBegin = pOther->iBegin;
    val.iEnd = pOther->iEnd;
    val.aProps.CopyArray(pOther->aProps);
  }

  return 0;
};

static SQInteger SetIndex(HSQUIRRELVM v, int, SesPropsHolder &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(INDEX)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  SQInteger i;
  sq_getinteger(v, 3, &i);

  SesProp &prop = val.aProps.Push();

  INDEX iSet = i;
  prop.SetData(iOffset, &iSet, sizeof(iSet));

  val.SetLimits(iOffset, iOffset + sizeof(iSet));
  return 0;
};

static SQInteger SetFloat(HSQUIRRELVM v, int, SesPropsHolder &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(FLOAT)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  SQFloat f;
  sq_getfloat(v, 3, &f);

  SesProp &prop = val.aProps.Push();

  FLOAT fSet = f;
  prop.SetData(iOffset, &fSet, sizeof(fSet));

  val.SetLimits(iOffset, iOffset + sizeof(fSet));
  return 0;
};

static SQInteger SetBool(HSQUIRRELVM v, int, SesPropsHolder &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(BOOL)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  SQBool b;
  sq_getbool(v, 3, &b);

  SesProp &prop = val.aProps.Push();

  BOOL bSet = b;
  prop.SetData(iOffset, &bSet, sizeof(bSet));

  val.SetLimits(iOffset, iOffset + sizeof(bSet));
  return 0;
};

static SQInteger SetString(HSQUIRRELVM v, int, SesPropsHolder &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  const SQChar *str;
  sq_getstring(v, 3, &str);
  const size_t ctLen = strlen(str);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - ctLen) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  SesProp &prop = val.aProps.Push();
  prop.SetData(iOffset, str, ctLen);

  val.SetLimits(iOffset, iOffset + ctLen);
  return 0;
};

static SQInteger SetByte(HSQUIRRELVM v, int, SesPropsHolder &val) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(UBYTE)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  SQInteger i;
  sq_getinteger(v, 3, &i);

  SesProp &prop = val.aProps.Push();

  UBYTE iSet = i & 0xFF;
  prop.SetData(iOffset, &iSet, sizeof(iSet));

  val.SetLimits(iOffset, iOffset + sizeof(iSet));
  return 0;
};

static SQInteger Reset(HSQUIRRELVM v, int, SesPropsHolder &val) {
  val.Clear();
  return 0;
};

static SQInteger Apply(HSQUIRRELVM v, int, SesPropsHolder &val) {
  ASSERT_SERVER;

  if (!val.IsValid()) {
    return sq_throwerror(v, "no data to apply to session properties");
  }

  const INDEX iOffset = val.iBegin;

  CExtSessionProps pck;
  pck.GetOffset() = iOffset;
  pck.GetSize() = val.iEnd - iOffset;

  // First copy a chunk of the current properties as to not overwrite them with zeros if there are any gaps in the data
  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  memcpy(pck.sp, pubProps + iOffset, pck.GetSize());

  // Then copy new data that needs to be changed from all the properties
  const INDEX ctProps = val.aProps.Count();

  for (INDEX i = 0; i < ctProps; i++) {
    const SesProp &prop = val.aProps[i];
    memcpy(pck.sp + prop.iOffset - iOffset, prop.pData, prop.iLength);
  }

  pck.SendToClients();
  return 0;
};

static Method<SesPropsHolder> _aMethods[] = {
  { "SetIndex",  &SetIndex,  3, ".nn" },
  { "SetFloat",  &SetFloat,  3, ".nn" },
  { "SetBool",   &SetBool,   3, ".nb" },
  { "SetString", &SetString, 3, ".ns" },
  { "SetByte",   &SetByte,   3, ".nn" },

  { "Reset", &Reset, 1, "." },
  { "Apply", &Apply, 1, "." },
};

}; // namespace

// Sound class methods
namespace SqSound {

struct SoundSetup {
  CTString strFile;

  INDEX iChannel;
  ULONG ulFlags;

  FLOAT fDelay, fOffset, fVolumeL, fVolumeR, fFilterL, fFilterR, fPitch;

  inline SoundSetup() : iChannel(0), ulFlags(SOF_NONE), fDelay(0.0f), fOffset(0.0f),
    fVolumeL(1.0f), fVolumeR(1.0f), fFilterL(1.0f), fFilterR(1.0f), fPitch(1.0f) {};

  inline void SetChannel(INDEX i) { iChannel = Clamp(i, (INDEX)0, (INDEX)31); };

  inline void SetDelay(FLOAT fValue)   { fDelay   = ClampDn(fValue, 0.0f); };
  inline void SetOffset(FLOAT fValue)  { fOffset  = fValue; };
  inline void SetVolumeL(FLOAT fValue) { fVolumeL = Clamp(fValue, 0.0f, 4.0f); };
  inline void SetVolumeR(FLOAT fValue) { fVolumeR = Clamp(fValue, 0.0f, 4.0f); };
  inline void SetFilterL(FLOAT fValue) { fFilterL = Clamp(fValue, 1.0f, 500.0f); };
  inline void SetFilterR(FLOAT fValue) { fFilterR = Clamp(fValue, 1.0f, 500.0f); };
  inline void SetPitch(FLOAT fValue)   { fPitch   = Clamp(fValue, 0.0f, 10.0f); };
};

static SQInteger Constructor(HSQUIRRELVM v, int ctArgs, SoundSetup &val) {
  // Minimal setup through the construction with a file, channel and volume
  if (ctArgs > 0) {
    const SQChar *str;
    if (SQ_FAILED(sq_getstring(v, 2, &str))) return sq_throwerror(v, "expected path to a sound file in argument 1");
    val.strFile = str;
  }

  if (ctArgs > 1) {
    SQInteger iChannel;
    if (SQ_FAILED(sq_getinteger(v, 3, &iChannel))) return sq_throwerror(v, "expected channel index in argument 2");
    val.SetChannel(iChannel);
  }

  if (ctArgs > 2) {
    SQFloat fVolume;
    if (SQ_FAILED(sq_getfloat(v, 4, &fVolume))) return sq_throwerror(v, "expected volume in argument 3");
    val.SetVolumeL(fVolume);
    val.SetVolumeR(fVolume);
  }

  return 0;
};

static SQInteger GetFile(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushstring(v, val.strFile.str_String, -1);
  return 1;
};

static SQInteger SetFile(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  const SQChar *str;
  if (SQ_FAILED(sq_getstring(v, idxValue, &str))) return sq_throwerror(v, "expected a number value");
  val.strFile = str;
  return 0;
};

static SQInteger GetChannel(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushinteger(v, val.iChannel);
  return 1;
};

static SQInteger SetChannel(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, idxValue, &i))) return sq_throwerror(v, "expected a number value");
  val.SetChannel(i);
  return 0;
};

static SQInteger GetFlags(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushinteger(v, val.ulFlags);
  return 1;
};

static SQInteger SetFlags(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQInteger i;
  if (SQ_FAILED(sq_getinteger(v, idxValue, &i))) return sq_throwerror(v, "expected a number value");
  val.ulFlags = (ULONG)i;
  return 0;
};

static SQInteger GetDelay(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fDelay);
  return 1;
};

static SQInteger SetDelay(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetDelay(f);
  return 0;
};

static SQInteger GetOffset(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fOffset);
  return 1;
};

static SQInteger SetOffset(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetOffset(f);
  return 0;
};

static SQInteger GetVolumeL(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fVolumeL);
  return 1;
};

static SQInteger SetVolumeL(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetVolumeL(f);
  return 0;
};

static SQInteger GetVolumeR(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fVolumeR);
  return 1;
};

static SQInteger SetVolumeR(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetVolumeR(f);
  return 0;
};

static SQInteger SetVolumes(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetVolumeL(f);
  val.SetVolumeR(f);
  return 0;
};

static SQInteger GetFilterL(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fFilterL);
  return 1;
};

static SQInteger SetFilterL(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetFilterL(f);
  return 0;
};

static SQInteger GetFilterR(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fFilterR);
  return 1;
};

static SQInteger SetFilterR(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetFilterR(f);
  return 0;
};

static SQInteger SetFilters(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetFilterL(f);
  val.SetFilterR(f);
  return 0;
};

static SQInteger GetPitch(HSQUIRRELVM v, SoundSetup &val) {
  sq_pushfloat(v, val.fPitch);
  return 1;
};

static SQInteger SetPitch(HSQUIRRELVM v, SoundSetup &val, SQInteger idxValue) {
  SQFloat f;
  if (SQ_FAILED(sq_getfloat(v, idxValue, &f))) return sq_throwerror(v, "expected a number value");
  val.SetPitch(f);
  return 0;
};

static SQInteger IsPlaying(HSQUIRRELVM v, int, SoundSetup &val) {
  sq_pushbool(v, CExtPlaySound::GetChannel(val.iChannel)->IsPlaying());
  return 1;
};

static SQInteger IsPaused(HSQUIRRELVM v, int, SoundSetup &val) {
  sq_pushbool(v, CExtPlaySound::GetChannel(val.iChannel)->IsPaused());
  return 1;
};

static SQInteger Play(HSQUIRRELVM v, int ctArgs, SoundSetup &val) {
  SQBool bLocal = SQFalse;
  if (ctArgs > 0) sq_getbool(v, 2, &bLocal);

  // Play the sound locally
  if (bLocal) {
    CSoundObject &so = *CExtPlaySound::GetChannel(val.iChannel);

    // Set sound parameters for a specific channel
    so.SetDelay(val.fDelay);
    so.SetVolume(val.fVolumeL, val.fVolumeR);
    so.SetFilter(val.fFilterL, val.fFilterR);
    so.SetPitch(val.fPitch);

    // Play a new sound
    if (val.strFile != "") {
      try {
        so.Play_t(val.strFile, val.ulFlags);
      } catch (char *strError) {
        CPrintF(TRANS("Cannot play '%s' sound: %s\n"), val.strFile.str_String, strError);
      }
    }

    // Set offset after playing the sound
    ISounds::SetOffset(so, val.fOffset, val.fOffset);

  // Send packet to all clients to play the sound
  } else {
    ASSERT_SERVER;

    CExtPlaySound pck;
    pck("strFile", val.strFile);
    pck("iChannel", (int)val.iChannel);
    pck("ulFlags", (int)val.ulFlags);
    pck("fDelay", val.fDelay);
    pck("fOffset", val.fOffset);

    pck("fVolumeL", val.fVolumeL);
    pck("fVolumeR", val.fVolumeR);
    pck("fFilterL", val.fFilterL);
    pck("fFilterR", val.fFilterR);
    pck("fPitch", val.fPitch);
    pck.SendToClients();
  }
  return 0;
};

static SQInteger Stop(HSQUIRRELVM v, int ctArgs, SoundSetup &val) {
  SQBool bLocal = SQFalse;
  if (ctArgs > 0) sq_getbool(v, 2, &bLocal);

  // Stop the sound locally
  if (bLocal) {
    CSoundObject *pso = CExtPlaySound::GetChannel(val.iChannel);
    pso->Stop();

  // Send packet to all clients to stop the sound
  } else {
    ASSERT_SERVER;

    CExtPlaySound pck;
    pck("strFile", "/stop/");
    pck("iChannel", (int)val.iChannel);
    pck.SendToClients();
  }
  return 0;
};

static Method<SoundSetup> _aMethods[] = {
  { "IsPlaying", &IsPlaying, 1, "." },
  { "IsPaused",  &IsPaused,  1, "." },
  { "Play",      &Play,     -1, ".b" },
  { "Stop",      &Stop,     -1, ".b" },
};

}; // namespace

#endif // _PATCHCONFIG_EXT_PACKETS

namespace Network {

static SQInteger IsHost(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsServer());
  return 1;
};

static SQInteger IsOnline(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsNetworkEnabled());
  return 1;
};

static SQInteger IsOnlineServer(HSQUIRRELVM v) {
  sq_pushbool(v, INetwork::IsHostingMultiplayer());
  return 1;
};

static SQInteger IsObserving(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsNetworkEnabled() && !IWorld::AnyLocalPlayers());
  return 1;
};

static SQInteger IsPredicting(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsPredicting());
  return 1;
};

static SQInteger GetMaxPlayers(HSQUIRRELVM v) {
  sq_pushinteger(v, _pNetwork->ga_sesSessionState.ses_ctMaxPlayers);
  return 1;
};

static SQInteger StartDemoRec(HSQUIRRELVM v) {
  const SQChar *strPath;
  sq_getstring(v, 2, &strPath);
  GetVMClass(v).StartDemoRec(strPath);
  return 0;
};

static SQInteger StopDemoRec(HSQUIRRELVM v) {
  GetVMClass(v).StopDemoRec();
  return 0;
};

static SQInteger IsPlayingDemo(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsPlayingDemo());
  return 1;
};

static SQInteger IsRecordingDemo(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsRecordingDemo());
  return 1;
};

static SQInteger IsDemoPlayFinished(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsDemoPlayFinished());
  return 1;
};

static SQInteger IsWaitingForPlayers(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsWaitingForPlayers());
  return 1;
};

static SQInteger IsWaitingForServer(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsWaitingForServer());
  return 1;
};

static SQInteger IsConnectionStable(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsConnectionStable());
  return 1;
};

static SQInteger IsDisconnected(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsDisconnected());
  return 1;
};

static SQInteger WhyDisconnected(HSQUIRRELVM v) {
  sq_pushstring(v, _pNetwork->WhyDisconnected().str_String, -1);
  return 1;
};

static SQInteger IsGameFinished(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsGameFinished());
  return 1;
};

static SQInteger GetRealTimeFactor(HSQUIRRELVM v) {
  sq_pushfloat(v, _pNetwork->GetRealTimeFactor());
  return 1;
};

static SQInteger GetCurrentWorld(HSQUIRRELVM v) {
  sq_pushstring(v, _pNetwork->GetCurrentWorld().str_String, -1);
  return 1;
};

static SQInteger GetGameTime(HSQUIRRELVM v) {
  sq_pushfloat(v, _pNetwork->GetGameTime());
  return 1;
};

static SQInteger GetPlayerEntity(HSQUIRRELVM v) {
  SQ_RESTRICT(v);

  SQInteger iPlayer;
  sq_getinteger(v, 2, &iPlayer);

  CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[iPlayer];

  if (!plt.IsActive()) {
    SQChar strError[256];
    scsprintf(strError, 256, "player %d is inactive", iPlayer);
    return sq_throwerror(v, strError);
  }

  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = plt.plt_penPlayerEntity;
  return 1;
};

static SQInteger GetPlayerEntityByName(HSQUIRRELVM v) {
  SQ_RESTRICT(v);

  const SQChar *strName;
  sq_getstring(v, 2, &strName);

  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = _pNetwork->GetPlayerEntityByName(strName);
  return 1;
};

static SQInteger CountEntitiesWithName(HSQUIRRELVM v) {
  SQ_RESTRICT(v);

  const SQChar *strName;
  sq_getstring(v, 2, &strName);
  sq_pushinteger(v, _pNetwork->GetNumberOfEntitiesWithName(strName));
  return 1;
};

static SQInteger GetEntityWithName(HSQUIRRELVM v) {
  SQ_RESTRICT(v);

  const SQChar *strName;
  sq_getstring(v, 2, &strName);

  SQInteger iEntity;
  sq_getinteger(v, 3, &iEntity);

  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = _pNetwork->GetEntityWithName(strName, iEntity);
  return 1;
};

static SQInteger IsPlayerLocal(HSQUIRRELVM v) {
  GetInstanceValueVerify(CEntityPointer, ppen, v, 2);
  sq_pushbool(v, _pNetwork->IsPlayerLocal(*ppen));
  return 1;
};

static SQInteger TogglePause(HSQUIRRELVM v) {
  _pNetwork->TogglePause();
  return 0;
};

static SQInteger IsPaused(HSQUIRRELVM v) {
  sq_pushbool(v, _pNetwork->IsPaused() || _pNetwork->GetLocalPause());
  return 1;
};

static SQInteger SendChat(HSQUIRRELVM v) {
  SQInteger iFrom, iTo;
  sq_getinteger(v, 2, &iFrom);
  sq_getinteger(v, 3, &iTo);

  const SQChar *strMessage;
  sq_getstring(v, 4, &strMessage);

  _pNetwork->SendChat(iFrom, iTo, strMessage);
  return 0;
};

static SQInteger SendChatToClient(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iClient;
  sq_getinteger(v, 2, &iClient);

  if (iClient < 0 || iClient >= _pNetwork->ga_srvServer.srv_assoSessions.Count()) {
    return sq_throwerror(v, "client index is out of bounds");
  }

  const SQChar *strFromName;
  const SQChar *strMessage;
  sq_getstring(v, 3, &strFromName);
  sq_getstring(v, 4, &strMessage);

  INetwork::SendChatToClient(iClient, strFromName, strMessage);
  return 0;
};

static SQInteger KickClient(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iClient;
  sq_getinteger(v, 2, &iClient);

  if (iClient < 0 || iClient >= _pNetwork->ga_srvServer.srv_assoSessions.Count()) {
    return sq_throwerror(v, "client index is out of bounds");
  }

  const SQChar *strExplanation;
  sq_getstring(v, 3, &strExplanation);

  INetwork::SendDisconnectMessage(iClient, strExplanation, FALSE);
  return 0;
};

static SQInteger GetHostName(HSQUIRRELVM v) {
  ASSERT_SERVER;

#if SE1_GAME != SS_REV
  CTString strName, strAddress;
  _pNetwork->GetHostName(strName, strAddress);
  sq_pushstring(v, strName.str_String, -1);
  return 1;
#else
  return sq_throwerror(v, "GetHostName() is unavailable in Revolution");
#endif
};

static SQInteger GetHostAddress(HSQUIRRELVM v) {
  ASSERT_SERVER;

#if SE1_GAME != SS_REV
  CTString strName, strAddress;
  _pNetwork->GetHostName(strName, strAddress);
  sq_pushstring(v, strAddress.str_String, -1);
  return 1;
#else
  return sq_throwerror(v, "GetHostAddress() is unavailable in Revolution");
#endif
};

static SQInteger GetClients(HSQUIRRELVM v) {
  ASSERT_SERVER;

  // Gather indices of active clients into an array
  CServer &srv = _pNetwork->ga_srvServer;
  const INDEX ctSessions = srv.srv_assoSessions.Count();

  sq_newarray(v, 0);

  for (INDEX i = 0; i < ctSessions; i++) {
    if (i == 0 || srv.srv_assoSessions[i].sso_bActive) {
      sq_pushinteger(v, i);
      sq_arrayappend(v, -2);
    }
  }

  return 1;
};

static SQInteger GetMaxClients(HSQUIRRELVM v) {
  ASSERT_SERVER;

  CServer &srv = _pNetwork->ga_srvServer;
  sq_pushinteger(v, srv.srv_assoSessions.Count());
  return 1;
};

static SQInteger IsClientActive(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iClient;
  sq_getinteger(v, 2, &iClient);

  CServer &srv = _pNetwork->ga_srvServer;

  if (iClient < 0 || iClient >= srv.srv_assoSessions.Count()) {
    return sq_throwerror(v, "client index is out of bounds");
  }

  sq_pushbool(v, srv.srv_assoSessions[iClient].sso_bActive);
  return 1;
};

static SQInteger CountPlayers(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQBool bOnlyVIP;
  sq_getbool(v, 2, &bOnlyVIP);
  sq_pushinteger(v, INetwork::CountPlayers(bOnlyVIP));
  return 1;
};

static SQInteger CountClients(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQBool bOnlyVIP;
  sq_getbool(v, 2, &bOnlyVIP);
  sq_pushinteger(v, INetwork::CountClients(bOnlyVIP));
  return 1;
};

static SQInteger CountObservers(HSQUIRRELVM v) {
  ASSERT_SERVER;
  sq_pushinteger(v, INetwork::CountObservers());
  return 1;
};

static SQInteger CountClientPlayers(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iClient;
  sq_getinteger(v, 2, &iClient);
  sq_pushinteger(v, INetwork::CountClientPlayers(iClient));
  return 1;
};

static SQInteger GetGameplayExtension(HSQUIRRELVM v) {
#if _PATCHCONFIG_GAMEPLAY_EXT
  const SQChar *str;
  sq_getstring(v, 2, &str);

  CTString strVar(str);
  if (strVar == "") return sq_throwerror(v, "gameplay extension variable is an empty string");

  IConfig::NamedValue *pEntry = NULL;

  for (int i = 0; i < k_EGameplayExt_Max; i++) {
    if (strVar == IConfig::gex.props[i].strKey) {
      pEntry = &IConfig::gex.props[i];
      break;
    }
  }

  if (pEntry == NULL) {
    SQChar strError[256];
    scsprintf(strError, 256, "gameplay extension '%s' doesn't exist", str);
    return sq_throwerror(v, strError);
  }

  switch (pEntry->val.GetType()) {
    case CAnyValue::E_VAL_BOOL:
      sq_pushbool(v, pEntry->val.GetIndex());
      return 1;

    case CAnyValue::E_VAL_INDEX:
      sq_pushinteger(v, pEntry->val.GetIndex());
      return 1;

    case CAnyValue::E_VAL_FLOAT:
      sq_pushfloat(v, pEntry->val.GetFloat());
      return 1;

    case CAnyValue::E_VAL_STRING:
      sq_pushstring(v, pEntry->val.GetString().str_String, -1);
      return 1;
  }

  return sq_throwerror(v, "unsupported gameplay extension value type");

#else
  return sq_throwerror(v, GAMEPLAY_EXT_ASSERT_MSG);
#endif // _PATCHCONFIG_GAMEPLAY_EXT
};

#if _PATCHCONFIG_EXT_PACKETS

static SQInteger SetGameplayExtension(HSQUIRRELVM v) {
  ASSERT_SERVER;

  const SQChar *strVar;
  sq_getstring(v, 2, &strVar);

  const SQChar *strValue = NULL;
  SQFloat fValue = 0.0f;

  if (sq_gettype(v, 3) == OT_STRING) {
    sq_getstring(v, 3, &strValue);

  } else if (SQ_FAILED(sq_getfloat(v, 3, &fValue))) {
    return sq_throwerror(v, "expected a string or a number in argument 2");
  }

  CExtGameplayExt pck;

  if (strValue != NULL) {
    pck.SetValue(strVar, strValue);
  } else {
    pck.SetValue(strVar, (DOUBLE)fValue);
  }

  pck.SendToClients();
  return 0;
};

static SQInteger ChangeLevel(HSQUIRRELVM v) {
  ASSERT_SERVER;

  const SQChar *strWorld;
  sq_getstring(v, 2, &strWorld);

  CExtChangeLevel pck;
  pck("strWorld", strWorld);
  pck.SendToClients();
  return 0;
};

static SQInteger ChangeWorld(HSQUIRRELVM v) {
  ASSERT_SERVER;

  const SQChar *strWorld;
  sq_getstring(v, 2, &strWorld);

  CExtChangeWorld pck;
  pck("strWorld", strWorld);
  pck.SendToClients();
  return 0;
};

static SQInteger StopSound(HSQUIRRELVM v) {
  ASSERT_SERVER;

  SQInteger iChannel;
  sq_getinteger(v, 2, &iChannel);

  CExtPlaySound pck;
  pck("strFile", "/stop/");
  pck("iChannel", (int)iChannel);
  pck.SendToClients();
  return 0;
};

static SQInteger GetSound(HSQUIRRELVM v) {
  Table sqtNetwork(v, 1);

  SQInteger iChannel;
  sq_getinteger(v, 2, &iChannel);
  iChannel = Clamp(iChannel, (SQInteger)0, (SQInteger)31);

  // Create a sound instance
  PushNewInstance(SqSound::SoundSetup, psnd, sqtNetwork, "Sound");

  // Copy channel data
  CSoundObject *pso = CExtPlaySound::GetChannel(iChannel);
  psnd->strFile = "";
  psnd->iChannel = iChannel;
  psnd->ulFlags = pso->so_slFlags;

  CSoundParameters &sp = pso->so_sp; // Current parameters instead of set up ones (so_spNew)
  psnd->SetDelay(sp.sp_fDelay);
  psnd->SetOffset(pso->so_fLeftOffset); // Left and right offsets should be synced
  psnd->SetVolumeL(sp.sp_fLeftVolume);
  psnd->SetVolumeR(sp.sp_fRightVolume);
  psnd->SetFilterL(32767.0f / (FLOAT)sp.sp_slLeftFilter);
  psnd->SetFilterR(32767.0f / (FLOAT)sp.sp_slRightFilter);
  psnd->SetPitch(sp.sp_fPitchShift);
  return 1;
};

#endif // _PATCHCONFIG_EXT_PACKETS

static SQInteger GetSessionPropIndex(HSQUIRRELVM v) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(INDEX)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  sq_pushinteger(v, *(const INDEX *)(pubProps + iOffset));
  return 1;
};

static SQInteger GetSessionPropFloat(HSQUIRRELVM v) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(FLOAT)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  sq_pushfloat(v, *(const FLOAT *)(pubProps + iOffset));
  return 1;
};

static SQInteger GetSessionPropBool(HSQUIRRELVM v) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(BOOL)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  sq_pushbool(v, *(const BOOL *)(pubProps + iOffset));
  return 1;
};

static SQInteger GetSessionPropString(HSQUIRRELVM v) {
  SQInteger iOffset, iLength;
  sq_getinteger(v, 2, &iOffset);
  sq_getinteger(v, 3, &iLength);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - iLength) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  sq_pushstring(v, (const SQChar *)(pubProps + iOffset), iLength);
  return 1;
};

static SQInteger GetSessionPropByte(HSQUIRRELVM v) {
  SQInteger iOffset;
  sq_getinteger(v, 2, &iOffset);

  if (iOffset < 0 || iOffset > sizeof(CSesPropsContainer) - sizeof(UBYTE)) {
    return sq_throwerror(v, "offset is out of bounds");
  }

  const UBYTE *pubProps = (const UBYTE *)_pNetwork->GetSessionProperties();
  sq_pushinteger(v, *(pubProps + iOffset));
  return 1;
};

}; // namespace

// "Network" namespace functions
static SQRegFunction _aNetworkFuncs[] = {
  // Network states
  { "IsHost",                &Network::IsHost,               1, "." },
  { "IsOnline",              &Network::IsOnline,             1, "." },
  { "IsOnlineServer",        &Network::IsOnlineServer,       1, "." },
  { "IsObserving",           &Network::IsObserving,          1, "." },
  { "IsPredicting",          &Network::IsPredicting,         1, "." },
  { "GetMaxPlayers",         &Network::GetMaxPlayers,        1, "." },

  // Demos
  { "StartDemoRec",          &Network::StartDemoRec,       2, ".s" },
  { "StopDemoRec",           &Network::StopDemoRec,        1, "." },
  { "IsPlayingDemo",         &Network::IsPlayingDemo,      1, "." },
  { "IsRecordingDemo",       &Network::IsRecordingDemo,    1, "." },
  { "IsDemoPlayFinished",    &Network::IsDemoPlayFinished, 1, "." },

  // Session states
  { "IsWaitingForPlayers",   &Network::IsWaitingForPlayers, 1, "." },
  { "IsWaitingForServer",    &Network::IsWaitingForServer,  1, "." },
  { "IsConnectionStable",    &Network::IsConnectionStable,  1, "." },
  { "IsDisconnected",        &Network::IsDisconnected,      1, "." },
  { "WhyDisconnected",       &Network::WhyDisconnected,     1, "." },

  { "IsGameFinished",        &Network::IsGameFinished,    1, "." },
  { "GetRealTimeFactor",     &Network::GetRealTimeFactor, 1, "." },
  { "GetCurrentWorld",       &Network::GetCurrentWorld,   1, "." },
  { "GetGameTime",           &Network::GetGameTime,       1, "." },

  // Entities
  { "GetPlayerEntity",       &Network::GetPlayerEntity,       2, ".n" },
  { "GetPlayerEntityByName", &Network::GetPlayerEntityByName, 2, ".s" },
  { "CountEntitiesWithName", &Network::CountEntitiesWithName, 2, ".s" },
  { "GetEntityWithName",     &Network::GetEntityWithName,     3, ".sn" },
  { "IsPlayerLocal",         &Network::IsPlayerLocal,         2, ".x" },

  // Client-specific
  { "TogglePause",           &Network::TogglePause, 1, "." },
  { "IsPaused",              &Network::IsPaused,    1, "." },
  { "SendChat",              &Network::SendChat,    4, ".nns" },

  // Server-specific
  { "SendChatToClient",      &Network::SendChatToClient, 4, ".nss" },
  { "KickClient",            &Network::KickClient,       3, ".ns" },
  { "GetHostName",           &Network::GetHostName,      1, "." },
  { "GetHostAddress",        &Network::GetHostAddress,   1, "." },

  { "GetClients",            &Network::GetClients,         1, "." },
  { "GetMaxClients",         &Network::GetMaxClients,      1, "." },
  { "IsClientActive",        &Network::IsClientActive,     2, ".n" },
  { "CountPlayers",          &Network::CountPlayers,       2, ".b" },
  { "CountClients",          &Network::CountClients,       2, ".b" },
  { "CountObservers",        &Network::CountObservers,     1, "." },
  { "CountClientPlayers",    &Network::CountClientPlayers, 2, ".n" },

  // Extension packets
  { "GetGameplayExtension", &Network::GetGameplayExtension, 2, ".s" },
#if _PATCHCONFIG_EXT_PACKETS
  { "SetGameplayExtension", &Network::SetGameplayExtension, 3, ".ss|n|b" },
  { "ChangeLevel",          &Network::ChangeLevel,          2, ".s" },
  { "ChangeWorld",          &Network::ChangeWorld,          2, ".s" },
  { "StopSound",            &Network::StopSound,            2, ".n" },
  { "GetSound",             &Network::GetSound,             2, ".n" },
#endif // _PATCHCONFIG_EXT_PACKETS

  // Current session properties
  { "GetSessionPropIndex",  &Network::GetSessionPropIndex,  2, ".n" },
  { "GetSessionPropFloat",  &Network::GetSessionPropFloat,  2, ".n" },
  { "GetSessionPropBool",   &Network::GetSessionPropBool,   2, ".n" },
  { "GetSessionPropString", &Network::GetSessionPropString, 3, ".nn" },
  { "GetSessionPropByte",   &Network::GetSessionPropByte,   2, ".n" },
};

void VM::RegisterNetwork(void) {
  Table sqtNetwork = Root().RegisterTable("Network");
  INDEX i;

  // Register classes
  {
    Class<CPlayerAction> sqcAction(GetVM(), "CPlayerAction", NULL);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqPlayerAction::_aMethods); i++) {
      sqcAction.RegisterMethod(SqPlayerAction::_aMethods[i]);
    }

    sqcAction.RegisterVar("aTranslation",  &SqPlayerAction::GetTranslation,  &SqPlayerAction::SetTranslation);
    sqcAction.RegisterVar("aRotation",     &SqPlayerAction::GetRotation,     &SqPlayerAction::SetRotation);
    sqcAction.RegisterVar("aViewRotation", &SqPlayerAction::GetViewRotation, &SqPlayerAction::SetViewRotation);
    sqcAction.RegisterVar("ulButtons",     &SqPlayerAction::GetButtons,      &SqPlayerAction::SetButtons);

    Root().AddClass(sqcAction);
  }
#if _PATCHCONFIG_EXT_PACKETS
  {
    Class<SqProps::SesPropsHolder> sqcProps(GetVM(), "SessionProperties", &SqProps::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqProps::_aMethods); i++) {
      sqcProps.RegisterMethod(SqProps::_aMethods[i]);
    }

    sqtNetwork.AddClass(sqcProps);
  }
  {
    Class<SqSound::SoundSetup> sqcSound(GetVM(), "Sound", &SqSound::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqSound::_aMethods); i++) {
      sqcSound.RegisterMethod(SqSound::_aMethods[i]);
    }

    sqcSound.RegisterVar("file",    &SqSound::GetFile, &SqSound::SetFile);
    sqcSound.RegisterVar("channel", &SqSound::GetChannel, &SqSound::SetChannel);
    sqcSound.RegisterVar("flags",   &SqSound::GetFlags, &SqSound::SetFlags);
    sqcSound.RegisterVar("delay",   &SqSound::GetDelay, &SqSound::SetDelay);
    sqcSound.RegisterVar("offset",  &SqSound::GetOffset, &SqSound::SetOffset);
    sqcSound.RegisterVar("volumeL", &SqSound::GetVolumeL, &SqSound::SetVolumeL);
    sqcSound.RegisterVar("volumeR", &SqSound::GetVolumeR, &SqSound::SetVolumeR);
    sqcSound.RegisterVar("volume",  NULL, &SqSound::SetVolumes);
    sqcSound.RegisterVar("filterL", &SqSound::GetFilterL, &SqSound::SetFilterL);
    sqcSound.RegisterVar("filterR", &SqSound::GetFilterR, &SqSound::SetFilterR);
    sqcSound.RegisterVar("filter",  NULL, &SqSound::SetFilters);
    sqcSound.RegisterVar("pitch",   &SqSound::GetPitch, &SqSound::SetPitch);

    sqtNetwork.AddClass(sqcSound);
  }
#endif // _PATCHCONFIG_EXT_PACKETS

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aNetworkFuncs); i++) {
    sqtNetwork.RegisterFunc(_aNetworkFuncs[i]);
  }

  // Sound flags
  Enumeration enSoundFlags(GetVM());

#define ADD_SOF(_FlagName) enSoundFlags.RegisterValue(#_FlagName, (SQInteger)SOF_##_FlagName)
  ADD_SOF(NONE);
  ADD_SOF(LOOP);
  ADD_SOF(3D);
  ADD_SOF(VOLUMETRIC);
  ADD_SOF(SURROUND);
  ADD_SOF(LOCAL);
  ADD_SOF(SMOOTHCHANGE);
  ADD_SOF(MUSIC);
  ADD_SOF(NONGAME);
  ADD_SOF(NOFILTER);
  ADD_SOF(PAUSED);
  ADD_SOF(LOADED);
  ADD_SOF(PREPARE);
  ADD_SOF(PLAY);
#undef ADD_SOF

  Const().AddEnum("SOF", enSoundFlags);
};

}; // namespace
