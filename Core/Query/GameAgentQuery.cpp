/* Copyright (c) 2002-2012 Croteam Ltd.
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

#if _PATCHCONFIG_NEW_QUERY

#include "QueryManager.h"
#include "Networking/NetworkFunctions.h"

static void ClientParsePacket(INDEX iLength) {
  // String of data
  const char *strData = IQuery::pBuffer;

  // Server status request
  if (*strData == 's') {
    _pNetwork->ga_strEnumerationStatus = "";

    const INDEX iAddrLength = 6;
    const char *pServers = strData + 1;

    // As long as there's enough data for an address
    while (iLength >= iAddrLength) {
      // Add server request from the address at the current position
      IQuery::Address addr = *(IQuery::Address *)pServers;
      addr.AddServerRequest(&pServers, iLength, htons(addr.uwPort + 1), "\x02");
    }
    return;
  }

  // Status response
  if (*strData == '0') {
    // Skip packet index with a separator
    strData += 2;

    // Key and value strings
    CTString strKey;
    CTString strValue;

    // Values for reading
    INDEX ctPlayers, ctMaxPlayers;
    CTString strLevel, strGameType, strVersion, strGameName, strSessionName;

    BOOL bReadValue = FALSE;

    // Go until the end of the string
    while (strData != NULL && *strData != '\0')
    {
      switch (*strData) {
        // Data separator
        case ';': {
          // Fill server listing with data (without session name)
          if (bReadValue && strKey != "sessionname")
          {
            if (strKey == "players") {
              ctPlayers = atoi(strValue);

            } else if (strKey == "maxplayers") {
              ctMaxPlayers = atoi(strValue);

            } else if (strKey == "level") {
              strLevel = strValue;

            } else if (strKey == "gametype") {
              strGameType = strValue;

            } else if (strKey == "version") {
              strVersion = strValue;

            } else if (strKey == "gamename") {
              strGameName = strValue;
            }

            // Reset key and value
            strKey = "";
            strValue = "";
          }

          // Toggle between the key and the value
          bReadValue = !bReadValue;

          // Proceed
          strData++;
        } break;

        // Data
        default: {
          // Set session name at the very end
          if (strKey == "sessionname") {
            strSessionName = strData;

            // Stop parsing
            strData = NULL;
            break;
          }

          // Extract substring until a separator or the end
          ULONG ulSep = IData::FindChar(strData, ';');
          CTString strExtracted = IData::ExtractSubstr(strData, 0, ulSep);

          // Set new key or value
          if (bReadValue) {
            strValue = strExtracted;
          } else {
            strKey = strExtracted;
          }

          // Advance the packet data
          strData += strExtracted.Length();
        } break;
      }
    }

    // Get request time from some server request
    CTimerValue tvPingTime = SServerRequest::PopRequestTime(IQuery::sinFrom);

    // Server status hasn't been requested
    if (tvPingTime.tv_llValue == -1) {
      return;
    }

    // Get ping in seconds
    const FLOAT tmPingTime = (_pTimer->GetHighPrecisionTimer() - tvPingTime).GetMilliseconds() / 1000.0f;

    // Create a new server listing
    CNetworkSession &ns = *new CNetworkSession;
    _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);

    ns.ns_strAddress.PrintF("%s:%d", inet_ntoa(IQuery::sinFrom.sin_addr), htons(IQuery::sinFrom.sin_port) - 1);
    ns.ns_tmPing = tmPingTime;

    ns.ns_strSession = strSessionName;
    ns.ns_strWorld = strLevel;
    ns.ns_ctPlayers = ctPlayers;
    ns.ns_ctMaxPlayers = ctMaxPlayers;

    ns.ns_strGameType = strGameType;
    ns.ns_strMod = strGameName;
    ns.ns_strVer = strVersion;
    return;
  }

  // Unknown packet
  CPrintF("Unknown enumeration packet ID: 0x%X\n", *strData);
};

void IGameAgent::BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge)
{
  strPacket.PrintF("0;challenge;%d;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;product;%s",
    iChallenge, INetwork::CountPlayers(FALSE), _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
    IWorld::GetWorld()->wo_strName, GetGameAPI()->GetCurrentGameTypeNameSS(), _SE_VER_STRING, sam_strGameName);
};

void IGameAgent::EnumTrigger(BOOL bInternet) {
  // Reset requests
  IQuery::aRequests.Clear();

  // Initialize as a client
  IQuery::bServer = FALSE;
  IQuery::bInitialized = TRUE;

  // Send request for enumeration
  IQuery::SendPacket("e");
  IQuery::SetStatus(".");
};

void IGameAgent::EnumUpdate(void) {
  int iLength = IQuery::ReceivePacket();

  if (iLength == -1) {
    return;
  }

  // End with a null terminator
  IQuery::pBuffer[iLength] = '\0';
  ClientParsePacket(iLength);
};

void IGameAgent::ServerParsePacket(INDEX iLength) {
  // Data buffer and player count
  const char *pData = IQuery::pBuffer;
  const INDEX ctPlayers = INetwork::CountPlayers(FALSE);

  // Response data packet
  CTString strPacket;

  // Received packet ID
  switch (pData[0])
  {
    // Server join request
    case 1: {
      // Send the challenge
      INDEX iChallenge = *(INDEX *)(pData + 1);
      IMasterServer::SendHeartbeat(iChallenge);
    } break;

    // Server status request
    case 2: {
      // Send status response
      strPacket.PrintF("0;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;gamename;%s;sessionname;%s",
        ctPlayers, _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
        IWorld::GetWorld()->wo_strName, GetGameAPI()->GetCurrentGameTypeNameSS(),
        _SE_VER_STRING, sam_strGameName, GetGameAPI()->SessionName());

      IQuery::SendReply(strPacket);
    } break;

    // Player status request
    case 3: {
      // Compose player status response
      strPacket.PrintF("\x01players\x02%d\x03", ctPlayers);

      // Go through server players
      for (INDEX i = 0; i < ctPlayers; i++) {
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
        CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];

        if (plt.plt_bActive) {
          // Get info about an individual player
          CTString strPlayer;
          plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

          // If not enough space for the next player info
          if (strPacket.Length() + strPlayer.Length() > 2048) {
            // Send existing packet and reset it
            IQuery::SendReply(strPacket);
            strPacket = "";
          }

          // Append player info
          strPacket += strPlayer;
        }
      }

      // Send the packet
      strPacket += "\x04";
      IQuery::SendReply(strPacket);
    } break;

    // Ping request
    case 4: {
      // Send 1 byte and the player count (useful in some cases for external scripts)
      strPacket.PrintF("\x04%d", ctPlayers);
      IQuery::SendReply(strPacket);
    } break;
  }
};

#endif // _PATCHCONFIG_NEW_QUERY
