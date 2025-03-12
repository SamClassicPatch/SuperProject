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

#include "ClientLogging.h"
#include "Networking/CommInterface.h"

// Get client's address by the client ID on the server
void IClientLogging::GetAddress(SClientAddress &addr, INDEX iClient) {
  const BOOL bServer = GetComm().Server_IsClientLocal(iClient);
  const CTString strHost = (bServer ? "127.0.0.1" : GetComm().Server_GetClientName(iClient));

  addr = SClientAddress(strHost);
};

// Get client identity by the client ID on the server
CClientIdentity *IClientLogging::GetIdentity(INDEX iClient) {
  // Get active client
  CActiveClient &acClient = _aActiveClients[iClient];

  // Return its identity if it's active
  if (acClient.IsActive()) {
    return acClient.pClient;
  }

  // Get client's address
  SClientAddress addr;
  IClientLogging::GetAddress(addr, iClient);

  // Find client identity by an address
  INDEX iIdentity;
  FindByAddress(iIdentity, addr);

  // If it's found
  if (iIdentity != -1) {
    // Activate a new client
    CClientIdentity *pci = &_aClientIdentities[iIdentity];
    _aActiveClients[iClient].Set(pci, addr);

    // Return it
    return &_aClientIdentities[iIdentity];
  }

  // Otherwise create a new client identity with this address
  CClientIdentity *pci = &_aClientIdentities.Push();
  pci->aAddresses.Push() = addr;

  // Activate a new client
  _aActiveClients[iClient].Set(pci, addr);

  return pci;
};

// Find client index in the list from an address and return address index
INDEX IClientLogging::FindByAddress(INDEX &iClient, const SClientAddress &addr) {
  const INDEX ctClients = _aClientIdentities.Count();

  // Go through clients
  for (INDEX i = 0; i < ctClients; i++) {
    const INDEX iAddr = _aClientIdentities[i].FindAddress(addr);

    // Found address
    if (iAddr != -1) {
      iClient = i;
      return iAddr;
    }
  }

  // No client under this address
  iClient = -1;
  return -1;
};

// Find client index in the list from a character and return character index
INDEX IClientLogging::FindByCharacter(INDEX &iClient, const CPlayerCharacter &pc) {
  const INDEX ctClients = _aClientIdentities.Count();

  // Go through clients
  for (INDEX i = 0; i < ctClients; i++) {
    const INDEX iChar = _aClientIdentities[i].FindCharacter(pc);

    // Found character
    if (iChar != -1) {
      iClient = i;
      return iChar;
    }
  }

  // No client with this character
  iClient = -1;
  return -1;
};

// Client log file
static const CTString _strClientLogFile = "Data\\ClassicsPatch\\ClientLog.dat";

// Save client log
void IClientLogging::SaveLog(void) {
  // Make sure the directory exists
  IDir::CreateDir(_strClientLogFile);

  try {
    CTFileStream strm;
    strm.Create_t(_strClientLogFile);

    strm.WriteID_t("CLLG"); // CLient LoG

    // Write clients
    const INDEX ctClients = _aClientIdentities.Count();
    strm << ctClients;

    for (INDEX i = 0; i < ctClients; i++) {
      CClientIdentity &ci = _aClientIdentities[i];
      ci.Write(&strm);
    }

    strm.Close();

  } catch (char *strError) {
    CPrintF(TRANS("Cannot save client log file: %s\n"), strError);
  }
};

// Load client log
void IClientLogging::LoadLog(void) {
  // No log file
  if (!FileExists(_strClientLogFile)) return;

  try {
    CTFileStream strm;
    strm.Open_t(_strClientLogFile);

    strm.ExpectID_t("CLLG"); // CLient LoG

    // Read clients
    INDEX ctClients;
    strm >> ctClients;

    // Warning for safety
    if (ctClients == 0) {
      ThrowF_t(TRANS("Client count is zero"));
    }

    CClientIdentity *aci = _aClientIdentities.Push(ctClients);

    for (INDEX i = 0; i < ctClients; i++) {
      aci[i].Read(&strm);
    }

    strm.Close();

  } catch (char *strError) {
    CPrintF(TRANS("Cannot load client log file: %s\n"), strError);
  }
};
