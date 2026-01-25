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

#include "ActiveClients.h"
#include "Networking/CommInterface.h"
#include "Networking/NetworkFunctions.h"

// Active clients by client IDs on the server
CStaticArray<CActiveClient> _aActiveClients;

// Setup the client to be active
void CActiveClient::Set(CClientIdentity *pci, const SClientAddress &addrSet) {
  pClient = pci;
  addr = addrSet;
  eRole = E_CLIENT;
};

// Reset the client to be inactive
void CActiveClient::Reset(void) {
  pClient = NULL;
  cPlayers.Clear();
  addr.SetIP(0);
  eRole = E_CLIENT;

  ResetPacketCounters();
};

// Reset anti-flood counters
void CActiveClient::ResetPacketCounters(void) {
  ctLastSecPackets = 0;
  ctLastSecMessages = 0;
  ctAnnoyanceLevel = 0;
};

// Check if client is active right now
BOOL CActiveClient::IsActive(void) const {
  // Client identity exists
  return pClient != NULL;
};

// Add a new player
void CActiveClient::AddPlayer(CPlayerBuffer *pplb) {
  cPlayers.Add(pplb);
};

// List players of this client
CTString CActiveClient::ListPlayers(void) const {
  const INDEX ct = cPlayers.Count();
  if (ct == 0) return "";

  CTString str = "";

  for (INDEX i = 0; i < ct; i++) {
    if (i != 0) {
      str += ", ";
    }

    str += cPlayers[i].plb_pcCharacter.GetNameForPrinting();
  }

  return str;
};

// Get active clients with a specific identity
void CActiveClient::GetActiveClients(CActiveClient::List &cClients, CClientIdentity *pci) {
  // Go through active clients
  FOREACHINSTATICARRAY(_aActiveClients, CActiveClient, itac)
  {
    // If found matching identity
    if (itac->pClient == pci) {
      // Add it to the list
      cClients.Add(itac);
    }
  }
};

// Deactivate some client
void CActiveClient::DeactivateClient(INDEX iClient) {
  // Make sure the server client isn't being deactivated
  ASSERT(!GetComm().Server_IsClientLocal(iClient));
  _aActiveClients[iClient].Reset();
};

// Check on annoying clients
void CActiveClient::CheckAnnoyingClients(void)
{
  const INDEX ct = _aActiveClients.Count();

  for (INDEX i = 0; i < ct; i++) {
    const CActiveClient &ac = _aActiveClients[i];

    // Not quite annoying
    if (ac.ctAnnoyanceLevel < 100 || IsAdmin(i)) continue;

    // Kick for being annoying
    INetwork::SendDisconnectMessage(i, TRANS("Stop being annoying!"), FALSE);
  }
};

// Reset all clients to be inactive
void CActiveClient::ResetAll(void)
{
  FOREACHINSTATICARRAY(_aActiveClients, CActiveClient, itac) {
    itac->Reset();
  }
};

// Check if some client has administrator rights
BOOL CActiveClient::IsAdmin(INDEX iClient) {
  // Authorized as an admin or the operator
  return (_aActiveClients[iClient].eRole != CActiveClient::E_CLIENT
       || GetComm().Server_IsClientLocal(iClient));
};

// Check if some client has operator rights
BOOL CActiveClient::IsOperator(INDEX iClient) {
  // Authorized as an admin or the operator
  return (_aActiveClients[iClient].eRole == CActiveClient::E_OPERATOR
       || GetComm().Server_IsClientLocal(iClient));
};
