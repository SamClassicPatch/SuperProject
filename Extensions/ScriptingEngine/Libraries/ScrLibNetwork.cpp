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
#include <Core/Networking/NetworkFunctions.h>

// Make sure the client is currently running a server
#define ASSERT_SERVER { if (!_pNetwork->IsServer()) return sq_throwerror(v, "cannot call this function while not hosting a game"); }

namespace sq {

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

  try {
    _pNetwork->StartDemoRec_t(CTString(strPath));
  } catch (char *strError) {
    // Don't throw script error
    CPrintF(LOCALIZE("Cannot start recording: %s\n"), strError);
  }

  return 0;
};

static SQInteger StopDemoRec(HSQUIRRELVM v) {
  _pNetwork->StopDemoRec();
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

static SQInteger GetPlayerEntityByName(HSQUIRRELVM v) {
  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  const SQChar *strName;
  sq_getstring(v, 2, &strName);

  *ppen = _pNetwork->GetPlayerEntityByName(strName);
  return 1;
};

static SQInteger CountEntitiesWithName(HSQUIRRELVM v) {
  const SQChar *strName;
  sq_getstring(v, 2, &strName);
  sq_pushinteger(v, _pNetwork->GetNumberOfEntitiesWithName(strName));
  return 1;
};

static SQInteger GetEntityWithName(HSQUIRRELVM v) {
  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  const SQChar *strName;
  sq_getstring(v, 2, &strName);

  SQInteger iEntity;
  sq_getinteger(v, 3, &iEntity);

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
};

void VM::RegisterNetwork(void) {
  Table sqtNetwork = Root().RegisterTable("Network");
  INDEX i;

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aNetworkFuncs); i++) {
    sqtNetwork.RegisterFunc(_aNetworkFuncs[i]);
  }
};

}; // namespace
