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

#ifndef CECIL_INCL_ACTIVECLIENTS_H
#define CECIL_INCL_ACTIVECLIENTS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ClientIdentity.h"

// Currently active client
class CORE_API CActiveClient {
  public:
    // List of pointers to active clients
    typedef CDynamicContainer<CActiveClient> List;

    enum ClientRole {
      E_CLIENT,   // Normal player/spectator client
      E_ADMIN,    // Server administrator with access to admin commands
      E_OPERATOR, // Server operator with access to server commands
    };

  public:
    // Client identity that's using this client (none, if inactive)
    CClientIdentity *pClient;

    // Players of this client (none, if observer)
    CDynamicContainer<CPlayerBuffer> cPlayers;

    // Address the client is playing from
    SClientAddress addr;

    // Level of client privileges
    ClientRole eRole;

    // Anti-flood system
    INDEX ctLastSecPackets; // Packets sent in the past second
    INDEX ctLastSecMessages; // Chat messages sent in the past second
    INDEX ctAnnoyanceLevel; // For kicking clients deemed too annoying in the past second (set by user; up to 100)

  public:
    // Default constructor
    CActiveClient() : pClient(NULL), eRole(E_CLIENT)
    {
      ResetPacketCounters();
    };

    // Setup the client to be active
    void Set(CClientIdentity *pci, const SClientAddress &addrSet);

    // Reset the client to be inactive
    void Reset(void);

    // Reset anti-flood counters
    void ResetPacketCounters(void);

    // Check if client is active right now
    BOOL IsActive(void) const;

    // Add a new player
    void AddPlayer(CPlayerBuffer *pplb);

    // List players of this client
    CTString ListPlayers(void) const;

  public:
    // Get active clients with a specific identity
    static void GetActiveClients(CActiveClient::List &cClients, CClientIdentity *pci);

    // Deactivate some client
    static void DeactivateClient(INDEX iClient);

    // Check on annoying clients
    static void CheckAnnoyingClients(void);

    // Reset all clients to be inactive
    static void ResetAll(void);

    // Check if some client has administrator rights
    static BOOL IsAdmin(INDEX iClient);
};

// Active clients by client IDs on the server
CORE_API extern CStaticArray<CActiveClient> _aActiveClients;

#endif
