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

#include "MasterServer.h"
#include "QueryManager.h"
#include "Networking/CommInterface.h"

#if _PATCHCONFIG_NEW_QUERY

// When the last heartbeat has been sent
static TIME _tmLastHeartbeat = -1.0f;

extern INDEX ms_iProtocol;

namespace IMasterServer {

// Get current master server protocol
INDEX GetProtocol(void) {
  // Old query manager is just like Legacy
  if (ms_bVanillaQuery) return E_MS_LEGACY;

  if (ms_iProtocol < E_MS_LEGACY || ms_iProtocol >= E_MS_MAX) {
    return E_MS_LEGACY;
  }

  return ms_iProtocol;
};

// Start the server
void OnServerStart(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerStart()\n");
  }

  // Initialize as a server
  IQuery::bServer = TRUE;
  IQuery::bInitialized = TRUE;

  // Send opening packet to the master server
  switch (GetProtocol()) {
    case E_MS_LEGACY: {
      CTString strPacket;
      strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);

      IQuery::SendPacket(strPacket);
    } break;

    case E_MS_DARKPLACES: {
      CTString strPacket;
      strPacket.PrintF("\xFF\xFF\xFF\xFFheartbeat DarkPlaces\x0A");

      IQuery::SendPacket(strPacket);
    } break;

    case E_MS_GAMEAGENT: {
      IQuery::SendPacket("q");
    } break;
  }
};

// Stop the server
void OnServerEnd(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerEnd()\n");
  }

  const INDEX iProtocol = GetProtocol();

  // Send double heartbeat for Dark Places
  if (iProtocol == E_MS_DARKPLACES) {
    SendHeartbeat(0);
    SendHeartbeat(0);

  // Send server closing packet to anything but GameAgent
  } else if (iProtocol == E_MS_LEGACY) {
    CTString strPacket;
    strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s\\statechanged", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);
    IQuery::SendPacket(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Server end:\n%s\n", strPacket);
    }
  }

  // Close the socket
  IQuery::CloseWinsock();
  IQuery::bInitialized = FALSE;
};

// Server update step
void OnServerUpdate(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Not usable
  if (!IQuery::IsSocketUsable()) {
    return;
  }

  // Receive new packet
  memset(&IQuery::pBuffer[0], 0, 2050);
  INDEX iLength = IQuery::ReceivePacket();

  // If there's any data
  if (iLength > 0) {
    if (ms_bDebugOutput) {
      CPrintF("Received packet (%d bytes)\n", iLength);
    }

    // Parse received packet
    _aProtocols[GetProtocol()]->ServerParsePacket(iLength);
  }

  // Send a heartbeat every 150 seconds
  if (_pTimer->GetRealTimeTick() - _tmLastHeartbeat >= 150.0f) {
    SendHeartbeat(0);
  }
};

// Server state has changed
void OnServerStateChanged(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  if (ms_bDebugOutput) {
    CPutString("  IMasterServer::OnServerStateChanged()\n");
  }

  // Notify master server about the state change
  switch (GetProtocol()) {
    // Legacy
    case E_MS_LEGACY: {
      CTString strPacket;
      strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s\\statechanged", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);

      IQuery::SendPacket(strPacket);

      if (ms_bDebugOutput) {
        CPrintF("Sending state change:\n%s\n", strPacket);
      }
    } break;

    // Nothing for Dark Places

    // GameAgent
    case E_MS_GAMEAGENT: {
      IQuery::SendPacket("u");
    } break;
  }
};

// Send heartbeat to the master server
void SendHeartbeat(INDEX iChallenge) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  CTString strPacket;

  // Build heartbeat packet for a specific master server
  _aProtocols[GetProtocol()]->BuildHearthbeatPacket(strPacket, iChallenge);

  if (ms_bDebugOutput) {
    CPrintF("Sending heartbeat:\n%s\n", strPacket);
  }

  // Send heartbeat to the master server
  IQuery::SendPacket(strPacket);
  _tmLastHeartbeat = _pTimer->GetRealTimeTick();
};

// Request server list enumeration
void EnumTrigger(BOOL bInternet) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // The list has changed
  if (_pNetwork->ga_bEnumerationChange) {
    return;
  }

  // Request for a specific master server
  _aProtocols[GetProtocol()]->EnumTrigger(bInternet);
};

// Replacement for CNetworkLibrary::EnumSessions()
void EnumSessions(BOOL bInternet) {
  // Keep using old query manager
  if (ms_bVanillaQuery) {
    _pNetwork->EnumSessions(bInternet);
    return;
  }

  // Clear old sessions
  FORDELETELIST(CNetworkSession, ns_lnNode, _pNetwork->ga_lhEnumeratedSessions, itns) {
    delete &*itns;
  }

  if (!GetComm().IsNetworkEnabled()) {
    // Have to enumerate as server
    GetComm().PrepareForUse(TRUE, FALSE);
  }

  // Enumerate sessions using new query manager
  EnumTrigger(bInternet);
};

// Update enumerations from the server
void EnumUpdate(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Not usable
  if (!IQuery::IsSocketUsable()) {
    return;
  }

  // Call update method for a specific master server
  _aProtocols[GetProtocol()]->EnumUpdate();
};

// Cancel master server enumeration
void EnumCancel(void) {
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Not initialized
  if (!IQuery::bInitialized) {
    return;
  }

  // Delete server requests and close the socket
  IQuery::aRequests.Clear();
  IQuery::CloseWinsock();
};

}; // namespace

#else

namespace IMasterServer {

INDEX GetProtocol(void) {
  return E_MS_LEGACY;
};

void OnServerStart(void)
{
};

void OnServerEnd(void)
{
};

void OnServerUpdate(void)
{
};

void OnServerStateChanged(void)
{
};

void SendHeartbeat(INDEX iChallenge)
{
};

void EnumTrigger(BOOL bInternet)
{
};

void EnumSessions(BOOL bInternet) {
  _pNetwork->EnumSessions(bInternet);
};

void EnumUpdate(void)
{
};

void EnumCancel(void)
{
};

}; // namespace

#endif // _PATCHCONFIG_NEW_QUERY
