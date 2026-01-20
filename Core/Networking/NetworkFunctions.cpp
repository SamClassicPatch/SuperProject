/* Copyright (c) 2022-2026 Dreamy Cecil
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

#include "NetworkFunctions.h"
#include "MessageProcessing.h"
#include "Modules.h"
#include "ExtPackets.h"

// Initialize networking
void INetwork::Initialize(void) {
  // Modeler applications don't need networking
  if (ClassicsCore_IsModelerApp()) return;

  // Load client log
  IClientLogging::LoadLog();

  // Server commands
  _pShell->DeclareSymbol("persistent user INDEX ser_bEnableAntiFlood;",      &ser_bEnableAntiFlood);
  _pShell->DeclareSymbol("persistent user INDEX ser_iPacketFloodThreshold;", &ser_iPacketFloodThreshold);
  _pShell->DeclareSymbol("persistent user INDEX ser_iMaxMessagesPerSecond;", &ser_iMaxMessagesPerSecond);
  _pShell->DeclareSymbol("persistent user INDEX ser_iMaxPlayersPerClient;",  &ser_iMaxPlayersPerClient);

  // Register commands for packet processing
  IProcessPacket::RegisterCommands();

#if _PATCHCONFIG_NEW_QUERY
  // Initialize query manager
  extern void InitQuery(void);
  InitQuery();
#endif

#if _PATCHCONFIG_EXT_PACKETS
  // Register extension packets
  CExtPacket::RegisterExtPackets();
#endif

  // Initialize voting system
  IVotingSystem::Initialize();

  _aActiveClients.New(ICore::MAX_SERVER_CLIENTS);

#if _PATCHCONFIG_GUID_MASKING
  IProcessPacket::_aClientChecks.New(ICore::MAX_GAME_PLAYERS);
#endif

  extern void InitHttp(void);
  InitHttp();

  // Make sure there is enough space for local players
  _pNetwork->ga_aplsPlayers.Clear();
  _pNetwork->ga_aplsPlayers.New(ICore::MAX_LOCAL_PLAYERS);
};

// Handle packets coming from a client
// If output is TRUE, it will pass packets into engine's CServer::Handle()
BOOL INetwork::ServerHandle(CMessageDispatcher *pmd, INDEX iClient, CNetworkMessage &nmMessage) {
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_tvMessageReceived = _pTimer->GetHighPrecisionTimer();

  MESSAGETYPE ePacket = nmMessage.GetType();

  // Process some default packets
  switch (ePacket) {
    // Client confirming the disconnection
    case PCK_REP_DISCONNECTED:
      IProcessPacket::OnClientDisconnect(iClient, nmMessage);
      return FALSE;

    // Client requesting the session state
    case MSG_REQ_CONNECTREMOTESESSIONSTATE:
      IProcessPacket::OnConnectRemoteSessionStateRequest(iClient, nmMessage);
      return FALSE;

    // Client requesting the connection to the server
    case MSG_REQ_CONNECTPLAYER:
      IProcessPacket::OnPlayerConnectRequest(iClient, nmMessage);
      return FALSE;

    // Client changing the character
    case MSG_REQ_CHARACTERCHANGE:
      IProcessPacket::OnCharacterChangeRequest(iClient, nmMessage);
      return FALSE;

    // Client sending player actions
    case MSG_ACTION:
      IProcessPacket::OnPlayerAction(iClient, nmMessage);
      return FALSE;

    // Client sending a CRC check
    case MSG_SYNCCHECK:
      IProcessPacket::OnSyncCheck(iClient, nmMessage);
      return FALSE;

    // Client sending a chat message
    case MSG_CHAT_IN:
      return IProcessPacket::OnChatInRequest(iClient, nmMessage);
  }

#if _PATCHCONFIG_EXT_PACKETS

  // Let CServer::Handle process packets of other types
  if (ePacket != PCK_EXTENSION) return TRUE;

  // Handle specific packet types
  ULONG ulType;
  INetDecompress::Integer(nmMessage, ulType);

  // Let plugins handle packets
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_network->OnServerPacket == NULL) continue;

    // Handle packet through this plugin handler
    if (itPlugin->pm_events.m_network->OnServerPacket(nmMessage, ulType)) {
      // Quit if packet has been handled
      return FALSE;
    }
  }

  CExtPacket *pPacket = NULL;

  // Only create packets that can come from clients
  if (ulType >= IClassicsExtPacket::k_EPacketType_FirstC2S) {
    pPacket = CExtPacket::CreatePacket((IClassicsExtPacket::EPacketType)ulType);
  }

  // No built-in packet under this index
  if (pPacket == NULL) {
    CPrintF(TRANS("Server received PCK_EXTENSION of an invalid (%u) type!\n"), ulType);
    ASSERT(FALSE);

    return FALSE;
  }

  // Read and process the packet
  pPacket->Read(nmMessage);
  pPacket->Process();

  // No extra processing needed
  delete pPacket;
  return FALSE;

#else

  // Let CServer::Handle process packets of other types
  return TRUE;

#endif // _PATCHCONFIG_EXT_PACKETS
};

// Handle packets coming from a server
// If output is TRUE, it will pass packets into engine's CSessionState::ProcessGameStreamBlock()
BOOL INetwork::ClientHandle(CSessionState *pses, CNetworkMessage &nmMessage) {
#if _PATCHCONFIG_EXT_PACKETS

  // Let default methods handle packets of other types
  if (nmMessage.GetType() != PCK_EXTENSION) return TRUE;

  // Handle specific packet types
  ULONG ulType;
  INetDecompress::Integer(nmMessage, ulType);

  // Let plugins handle packets
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_network->OnClientPacket == NULL) continue;

    // Handle packet through this plugin handler
    if (itPlugin->pm_events.m_network->OnClientPacket(nmMessage, ulType)) {
      // Quit if packet has been handled
      return FALSE;
    }
  }

  CExtPacket *pPacket = NULL;

  // Only create packets that can come from a server
  if (ulType <= IClassicsExtPacket::k_EPacketType_LastS2C) {
    pPacket = CExtPacket::CreatePacket((IClassicsExtPacket::EPacketType)ulType);
  }

  // No built-in packet under this index
  if (pPacket == NULL) {
    CPrintF(TRANS("Client received PCK_EXTENSION of an invalid (%u) type!\n"), ulType);
    ASSERT(FALSE);

    return FALSE;
  }

  // Read and process the packet
  pPacket->Read(nmMessage);
  pPacket->Process();

  // No extra processing needed
  delete pPacket;
  return FALSE;

#else

  // Let CSessionState::ProcessGameStreamBlock process packets of other types
  return TRUE;

#endif // _PATCHCONFIG_EXT_PACKETS
};

// Send disconnect message to a client (CServer::SendDisconnectMessage reimplementation)
void INetwork::SendDisconnectMessage(INDEX iClient, const char *strExplanation, BOOL bStream) {
  // Not a server
  if (!_pNetwork->IsServer()) {
    return;
  }

  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];

  if (!bStream) {
    // Send message
    CNetworkMessage nmDisconnect(MSG_INF_DISCONNECTED);
    nmDisconnect << CTString(strExplanation);

    _pNetwork->SendToClientReliable(iClient, nmDisconnect);

  } else {
    // Send stream
    CTMemoryStream strmDisconnect;
    strmDisconnect << INDEX(MSG_INF_DISCONNECTED);
    strmDisconnect << CTString(strExplanation);

    _pNetwork->SendToClientReliable(iClient, strmDisconnect);
  }

  // Report that the packet has been sent
  CPrintF(LOCALIZE("Client '%s' ordered to disconnect: %s\n"), GetComm().Server_GetClientName(iClient), strExplanation);

  // If not disconnected before
  if (sso.sso_iDisconnectedState == 0) {
    // Mark for disconnection
    sso.sso_iDisconnectedState = 1;

  // If the client is still hanging here
  } else {
    // Force disconnection
    CPrintF(LOCALIZE("Forcing client '%s' to disconnect\n"), GetComm().Server_GetClientName(iClient));

    sso.sso_iDisconnectedState = 2;
  }

  // Make client inactive
  CActiveClient::DeactivateClient(iClient);
};

// Send chat message to a client with custom name of a sender
void INetwork::SendChatToClient(INDEX iClient, const CTString &strFromName, const CTString &strMessage) {
  // Not a server
  if (!_pNetwork->IsServer()) {
    return;
  }

  CNetworkMessage nm(MSG_CHAT_OUT);
  nm << (INDEX)0;
  nm << strFromName;
  nm << strMessage;

  _pNetwork->SendToClient(iClient, nm);
};
