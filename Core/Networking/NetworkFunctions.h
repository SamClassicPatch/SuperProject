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

#ifndef CECIL_INCL_NETWORKFUNCTIONS_H
#define CECIL_INCL_NETWORKFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "CommInterface.h"
#include "StreamBlock.h"
#include "MessageCompression.h"

// Interface of network methods
class CORE_API INetwork {
  public:
    // New network packet types
    enum ENetPackets {
      // Backported MSG_REP_DISCONNECTED from 1.07
      PCK_REP_DISCONNECTED = 48,

      // Start with 49 to continue the MESSAGETYPE / NetworkMessageType list
      PCK_DUMMY_NETWORK_PACKET = 49,

      // Allow 49-62 range for mod packets

    #if _PATCHCONFIG_EXT_PACKETS
      // Occupy the last index for new packets (cannot go above 63)
      PCK_EXTENSION = 63,
    #endif
    };

  public:
  #if _PATCHCONFIG_EXT_PACKETS

    // Create packet to send to a server
    static inline CNetworkMessage CreateClientPacket(const ULONG ulType) {
      CNetworkMessage nmClient((MESSAGETYPE)PCK_EXTENSION);
      INetCompress::Integer(nmClient, ulType); // Packet type

      return nmClient;
    };

    // Create packet to send to all clients
    static inline CNetStreamBlock CreateServerPacket(const ULONG ulType) {
      CServer &srv = _pNetwork->ga_srvServer;

      CNetStreamBlock nsbServer(PCK_EXTENSION, ++srv.srv_iLastProcessedSequence);
      INetCompress::Integer(nsbServer, ulType); // Packet type

      return nsbServer;
    };

  #endif // _PATCHCONFIG_EXT_PACKETS

    // Add stream block to one session
    static inline void AddBlockToSession(CNetStreamBlock &nsb, INDEX iSession) {
      CServer &srv = _pNetwork->ga_srvServer;
      CSessionSocket &sso = srv.srv_assoSessions[iSession];

      // Add block to the buffer if client is active (server client always is)
      if (iSession == 0 || sso.IsActive()) {
        ((CNetStream &)sso.sso_nsBuffer).AddBlock(nsb);
      }
    };

    // Add stream block to all sessions
    static inline void AddBlockToAllSessions(CNetStreamBlock &nsb) {
      // For each active session
      for (INDEX i = 0; i < _pNetwork->ga_srvServer.srv_assoSessions.Count(); i++) {
        AddBlockToSession(nsb, i);
      }
    };

    // Initialize networking
    static void Initialize(void);

    // Handle packets coming from a client
    // If output is TRUE, it will pass packets into engine's CServer::Handle()
    static BOOL ServerHandle(CMessageDispatcher *pmd, INDEX iClient, CNetworkMessage &nmReceived);

    // Handle packets coming from a server
    // If output is TRUE, it will pass packets into engine's CSessionState::ProcessGameStreamBlock()
    static BOOL ClientHandle(CSessionState *pses, CNetworkMessage &nmMessage);

    // Send disconnect message to a client (CServer::SendDisconnectMessage reimplementation)
    static void SendDisconnectMessage(INDEX iClient, const char *strExplanation, BOOL bStream);

    // Send chat message to a client with custom name of a sender
    static void SendChatToClient(INDEX iClient, const CTString &strFromName, const CTString &strMessage);

    // Check if hosting an online multiplayer game
    static inline BOOL IsHostingMultiplayer(void) {
      // Non-local game; running a server; with more than one player
      return _pNetwork->IsNetworkEnabled() && _pNetwork->IsServer()
        && _pNetwork->ga_sesSessionState.ses_ctMaxPlayers > 1;
    };

  // CServer method reimplementations
  public:

    // Get number of active players
    // Reimplementation of CServer::GetPlayersCount() and CServer::GetVIPPlayersCount() methods
    static inline INDEX CountPlayers(BOOL bOnlyVIP) {
      CServer &srv = _pNetwork->ga_srvServer;
      INDEX ctPlayers = 0;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Skip inactive players
        if (!itplb->IsActive()) continue;

        // Any or VIP
        if (!bOnlyVIP || srv.srv_assoSessions[itplb->plb_iClient].sso_bVIP) {
          ctPlayers++;
        }
      }

      return ctPlayers;
    };

    // Get number of active clients
    // Reimplementation of CServer::GetClientsCount() and CServer::GetVIPClientsCount() methods
    static inline INDEX CountClients(BOOL bOnlyVIP) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctSessions = srv.srv_assoSessions.Count();
      INDEX ctClients = 0;

      for (INDEX i = 0; i < ctSessions; i++) {
        CSessionSocket &sso = srv.srv_assoSessions[i];

        // Skip inactive clients
        if (i > 0 && !sso.sso_bActive) continue;

        // Any or VIP
        if (!bOnlyVIP || sso.sso_bVIP) {
          ctClients++;
        }
      }

      return ctClients;
    };

    // Get number of active observers
    // Reimplementation of CServer::GetObserversCount() method
    static inline INDEX CountObservers(void) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctSessions = srv.srv_assoSessions.Count();
      INDEX ctClients = 0;

      for (INDEX i = 0; i < ctSessions; i++) {
        CSessionSocket &sso = srv.srv_assoSessions[i];

        // Skip inactive clients
        if (i > 0 && !sso.IsActive()) continue;

        // Client with no players
        if (sso.sso_ctLocalPlayers == 0) {
          ctClients++;
        }
      }

      return ctClients;
    };

    // Get number of active players of a specific client
    static inline INDEX CountClientPlayers(INDEX iClient) {
      CServer &srv = _pNetwork->ga_srvServer;
      INDEX ctPlayers = 0;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Active player with a matching client index
        if (itplb->IsActive() && itplb->plb_iClient == iClient) {
          ctPlayers++;
        }
      }

      return ctPlayers;
    };

    // Find first inactive client
    static inline CPlayerBuffer *FirstInactivePlayer(void) {
      CServer &srv = _pNetwork->ga_srvServer;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Found inactive player
        if (!itplb->IsActive()) {
          return itplb;
        }
      }

      // No inactive players
      return NULL;
    };

    // Check if some character already exists in this session
    static inline BOOL IsCharacterUsed(const CPlayerCharacter &pc) {
      CServer &srv = _pNetwork->ga_srvServer;

      FOREACHINSTATICARRAY(srv.srv_aplbPlayers, CPlayerBuffer, itplb)
      {
        // Active player with a matching character
        if (itplb->IsActive() && itplb->plb_pcCharacter == pc) {
          return TRUE;
        }
      }

      // No matching character found
      return FALSE;
    };

    // Compose a bit mask of all players of a specific client
    static inline ULONG MaskOfClientPlayers(INDEX iClient) {
      CServer &srv = _pNetwork->ga_srvServer;

      const INDEX ctPlayers = srv.srv_aplbPlayers.Count();
      ULONG ulMask = 0;

      for (INDEX i = 0; i < ctPlayers; i++) {
        CPlayerBuffer &plb = srv.srv_aplbPlayers[i];

        // Active player with a matching client index
        if (plb.IsActive() && plb.plb_iClient == iClient) {
          ulMask |= (1UL << i);
        }
      }

      return ulMask;
    };
};

#endif
