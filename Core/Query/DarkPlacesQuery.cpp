/* Copyright (c) 2022-2025 Dreamy Cecil
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

// Used DarkPlaces protocol
static const INDEX _iProtocolVersion = 3;

// Parse server list data from the master server
static void ParseServerList(const char *pData, INDEX iLength, BOOL bExtended) {
  if (ms_bDebugOutput) {
    CPrintF("Data length: %d\n", iLength);
  }

  // As long as there's enough data
  while (iLength >= 7)
  {
    // Retrieve IPv4 address
    if (pData[0] == '\\') {
      pData++;
      iLength++;

      // Add server request from the address at the current position
      IQuery::Address addr = *(IQuery::Address *)pData;
      addr.AddServerRequest(&pData, iLength, htons(addr.uwPort), "\xFF\xFF\xFF\xFFgetstatus");
      continue;
    }

    // Skip IPv6 address
    if (bExtended && iLength >= 19 && pData[0] == '/') {
      pData += 19;
      iLength -= 19;
      continue;
    }

    // Unknown data
    if (ms_bDebugOutput) {
      CPutString("Received unknown data upon parsing the server list!\n");
    }
    break;
  }
};

static void ClientParsePacket(INDEX iLength) {
  // String of data
  const char *strData = IQuery::pBuffer;

  // Not long enough or doesn't start with full bytes
  if (iLength <= 4 || memcmp(strData, "\xFF\xFF\xFF\xFF", 4) != 0) {
    return;
  }

  strData += 4;
  iLength -= 4;

  // Regular response
  if (iLength >= 18 && memcmp(strData, "getserversResponse", 18) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'getserversResponse'!\n");
    }

    ParseServerList(strData + 18, iLength - 18, FALSE);
    return;
  }

  // Extended response
  if (iLength >= 21 && memcmp(strData, "getserversExtResponse", 21) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'getserversExtResponse'!\n");
    }

    ParseServerList(strData + 21, iLength - 21, TRUE);
    return;
  }

  // Status response from one of the servers in the list
  if (iLength >= 15 && memcmp(strData, "statusResponse\x0A", 15) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'statusResponse'!\n");
    }

    strData += 15;
    iLength -= 15;

    if (ms_bDebugOutput) {
      CPrintF("Data (%d bytes): %s\n", iLength, strData);
    }

    // Skip initial separator
    if (*strData == '\\') {
      strData++;
    }

    // Key and value strings
    CTString strKey;
    CTString strValue;

    // Values for reading
    CTString strGameName, strGameVer, strHostName, strMode, strMapName;
    INDEX ctClients, ctMaxClients;

    BOOL bReadValue = FALSE;

    // Go until the end of the string
    while (*strData != '\0')
    {
      switch (*strData) {
        // Data separator
        case '\\': {
          // Fill server listing with data
          if (bReadValue)
          {
            if (strKey == "gamename") {
              strGameName = strValue;

            } else if (strKey == "gameversion") {
              strGameVer = strValue;

            } else if (strKey == "hostname") {
              strHostName = strValue;

            } else if (strKey == "qcstatus") {
              strMode = strValue;

            } else if (strKey == "mapname") {
              strMapName = strValue;

            } else if (strKey == "clients") {
              ctClients = atoi(strValue);

            } else if (strKey == "sv_maxclients") {
              ctMaxClients = atoi(strValue);

            // [Cecil] NOTE: Unused
            } else if (strKey == "location" || strKey == "hostport" || strKey == "modname"
                    || strKey == "bots" || strKey == "protocol") {
              NOTHING;
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
          // Extract substring until a separator or the end
          ULONG ulSep = IData::FindChar(strData, '\\');
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

    ns.ns_strSession = strHostName;
    ns.ns_strWorld = strMapName;
    ns.ns_ctPlayers = ctClients;
    ns.ns_ctMaxPlayers = ctMaxClients;

    ns.ns_strMod = strGameName;
    ns.ns_strVer = strGameVer;

    // If there's any gamemode, find a separator
    ULONG ulSep = IData::FindChar(strMode, ':');

    // Extract game type before the separator
    if (ulSep != -1) {
      ns.ns_strGameType = IData::ExtractSubstr(strMode, 0, ulSep);
    }
    return;
  }

  if (ms_bDebugOutput) {
    CPutString("Received unknown packet!\n");
    CPrintF("Data (%d bytes): %s\n", iLength, strData);
  }
};

// Compose status response packet
static void ComposeStatusPacket(CTString &strPacket, const char *strChallenge, BOOL bFullStatus) {
  const INDEX ctMaxPlayers = _pNetwork->ga_sesSessionState.ses_ctMaxPlayers;
  const INDEX ctClients = INetwork::CountClients(FALSE);

  // Compose the packet
  strPacket.PrintF("\xFF\xFF\xFF\xFF%s\x0A"
    "\\gamename\\%s\\modname\\%s\\gameversion\\%s"
    "\\sv_maxclients\\%d\\clients\\%d\\bots\\%d\\mapname\\%s\\hostname\\%s\\protocol\\%d"
    "\\qcstatus\\%s:%s:P%d:S%d:F%d:M%s::score!!",
    // Response header
    (bFullStatus ? "statusResponse" : "infoResponse"),
    // Game info
    sam_strGameName, "", _SE_VER_STRING,
    // Server info
    ctMaxPlayers, ctClients, 0, IWorld::GetWorld()->wo_strName, GetGameAPI()->SessionName(), _iProtocolVersion,
    // Server status
    GetGameAPI()->GetCurrentGameTypeNameSS(), "0.8.2", 0, ctMaxPlayers - ctClients, 0, sam_strGameName);

  // Optional challenge
  if (strChallenge != NULL) {
    strPacket += "\\challenge\\" + CTString(strChallenge);
  }

  // [Cecil] NOTE: Unused extras (3 strings)
  if (FALSE) {
    strPacket += CTString(0, "%s%s%s", "", "", "");
  }
};

void IDarkPlaces::BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge) {
  strPacket.PrintF("\xFF\xFF\xFF\xFFheartbeat DarkPlaces\x0A");
};

void IDarkPlaces::EnumTrigger(BOOL bInternet) {
  // Reset requests
  IQuery::aRequests.Clear();

  // Initialize as a client
  IQuery::bServer = FALSE;
  IQuery::bInitialized = TRUE;

  // Send request for enumeration
  CTString strPacket;
  strPacket.PrintF("\xFF\xFF\xFF\xFFgetservers %s %u empty full", sam_strGameName, _iProtocolVersion);

  IQuery::SendPacket(strPacket);
  IQuery::SetStatus(".");
};

void IDarkPlaces::EnumUpdate(void) {
  int iLength = IQuery::ReceivePacket();

  if (iLength == -1) {
    return;
  }

  ClientParsePacket(iLength);
};

void IDarkPlaces::ServerParsePacket(INDEX iLength) {
  // String of data
  const char *strData = IQuery::pBuffer;

  // Not long enough or doesn't start with full bytes
  if (iLength <= 4 || memcmp(strData, "\xFF\xFF\xFF\xFF", 4) != 0) {
    return;
  }

  strData += 4;
  iLength -= 4;

  // Challenge and packet strings
  const char *strChallenge = NULL;
  CTString strPacket;

  // Requesting short information
  if (iLength >= 7 && memcmp(strData, "getinfo", 7) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'getinfo'!\n");
    }

    // Use the rest of the data as challenge
    if (iLength > 8 && strData[7] == ' ') {
      strChallenge = strData + 8;
    }

    ComposeStatusPacket(strPacket, strChallenge, FALSE);
    IQuery::SendPacket(strPacket);
    return;
  }

  // Requesting full information
  if (iLength >= 9 && memcmp(strData, "getstatus", 9) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'getstatus'!\n");
    }

    // Use the rest of the data as challenge
    if (iLength > 10 && strData[9] == ' ') {
      strChallenge = strData + 10;
    }

    ComposeStatusPacket(strPacket, strChallenge, TRUE);
    IQuery::SendReply(strPacket);
    return;
  }

  // Requesting challenge
  if (iLength >= 12 && memcmp(strData, "getchallenge", 12) == 0)
  {
    if (ms_bDebugOutput) {
      CPutString("Received 'getchallenge'!\n");
    }

    // No challenge to reply with
    strPacket.PrintF("\xFF\xFF\xFF\xFFreject No challenge!");
    IQuery::SendReply(strPacket);
    return;
  }

  // Unknown request
  if (ms_bDebugOutput) {
    CPutString("Received unknown packet!\n");
    CPrintF("Data (%d bytes): %s\n", iLength, strData);
  }
};

#endif // _PATCHCONFIG_NEW_QUERY
