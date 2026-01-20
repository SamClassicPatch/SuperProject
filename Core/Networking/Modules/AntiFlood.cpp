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

#include "AntiFlood.h"
#include "ClientLogging.h"
#include "Networking/NetworkFunctions.h"

// Kick clients for attempted packet flood
INDEX ser_bEnableAntiFlood = TRUE;

// Allowed messages threshold before treating it as packet flood
INDEX ser_iPacketFloodThreshold = 10;

// Allowed messages from client per second
INDEX ser_iMaxMessagesPerSecond = 2;

// Detect potential packet flood and deal with it
static BOOL DetectPacketFlood(INDEX iClient)
{
  // Get packet count of this client
  CActiveClient &acClient = _aActiveClients[iClient];
  INDEX ctPackets = acClient.ctLastSecPackets;

  // Client haven't exceeded the flood threshold
  if (ser_iPacketFloodThreshold < 0 || ctPackets <= ser_iPacketFloodThreshold) {
    return FALSE;
  }

  // Kick for attempted packet flood
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2; // Force disconnect

  CTString strChatMessage;
  strChatMessage.PrintF("^cff0000 Client %d has been kicked for a packet flood attempt!", iClient);
  _pNetwork->SendChat(0, -1, strChatMessage);

  // Detected
  return TRUE;
};

// Handle character changes from a client
BOOL IAntiFlood::HandleCharacterChange(INDEX iClient)
{
  // Not using anti-flood system or it's a server client
  if (!ser_bEnableAntiFlood || GetComm().Server_IsClientLocal(iClient)) {
    return FALSE;
  }

  // Count one packet from the client
  CActiveClient &acClient = _aActiveClients[iClient];
  acClient.ctLastSecPackets++;

  // Deal with packet flood
  return DetectPacketFlood(iClient);
};

// Handle chat messages from a client
BOOL IAntiFlood::HandleChatMessage(INDEX iClient)
{
  // Get client identity and active client
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);
  CActiveClient &acClient = _aActiveClients[iClient];

  // Ignore administrators
  if (CActiveClient::IsAdmin(iClient)) {
    return FALSE;
  }

  // Count one chat message from the client
  acClient.ctLastSecPackets++;
  acClient.ctLastSecMessages++;

  // Don't show the message if detected packet flood
  if (DetectPacketFlood(iClient)) {
    return TRUE;
  }

  static CTString strKickWarning = TRANS("\n^cffffffFurther attempts may lead to a kick!");

  // Check if the client is muted
  CClientRestriction *pcr = CClientRestriction::IsMuted(pci);

  if (pcr != NULL) {
    // Notify the client about being muted
    CTString strTime, strWarning;
    pcr->PrintMuteTime(strTime);

    strWarning.PrintF(TRANS("You are not allowed to chat for %s!"), strTime);

    if (ser_iPacketFloodThreshold >= 0) {
      strWarning += strKickWarning;
    }

    INetwork::SendChatToClient(iClient, "Server", strWarning);

    // Don't show the message in chat
    return TRUE;
  }

  // Quit if not using anti-flood system or no further message limit
  if (!ser_bEnableAntiFlood || ser_iMaxMessagesPerSecond < 0) {
    return FALSE;
  }

  // If client sent too many messages in the past second
  if (acClient.ctLastSecMessages > ser_iMaxMessagesPerSecond) {
    // Notify the client about spam
    CTString strWarning;

    if (ser_iMaxMessagesPerSecond > 0) {
      strWarning = TRANS("Too many chat messages at once!");
    } else {
      strWarning = TRANS("Chatting is disabled in this session!");
    }

    if (ser_iPacketFloodThreshold >= 0) {
      strWarning += strKickWarning;
    }

    INetwork::SendChatToClient(iClient, "Server", strWarning);

    // Don't show the message in chat
    return TRUE;
  }

  // No packet flood
  return FALSE;
};

// Reset packet counters for each client
void IAntiFlood::ResetCounters(void)
{
  FOREACHINSTATICARRAY(_aActiveClients, CActiveClient, itac) {
    itac->ResetPacketCounters();
  }
};
