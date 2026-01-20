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

#ifndef CECIL_INCL_CLIENTLOGGING_H
#define CECIL_INCL_CLIENTLOGGING_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// IP address of a client with their host name
struct CORE_API SClientAddress {
  private:
    ULONG ulIP; // IP address
    CTString strHost; // Host name

  public:
    // Default constructor
    SClientAddress() {
      Clear();
    };

    // Constructor from an address
    SClientAddress(ULONG ulSetIP) {
      SetIP(ulSetIP);
    };

    // Constructor from a host name
    SClientAddress(const CTString &strSetHost) {
      SetHost(strSetHost);
    };

    // Clear address
    inline void Clear(void) {
      ulIP = 0;
      strHost = "0.0.0.0";
    };

    // Get IP address
    inline ULONG GetIP(void) const {
      return ulIP;
    };

    // Get host name
    inline CTString GetHost(void) const {
      return strHost;
    };

  public:
    // Set a new IP address
    inline void SetIP(ULONG ulSetIP) {
      ulIP = ulSetIP;
      strHost = AddressToString(ulIP);
    };

    // Set a new host name
    inline void SetHost(const CTString &strSetHost) {
      strHost = strSetHost;
      ulIP = StringToAddress(strHost);
    };

    // Equality check
    inline BOOL operator==(const SClientAddress &addrOther) const {
      return (ulIP == addrOther.ulIP);
    };

    // Inequality check
    inline BOOL operator!=(const SClientAddress &addrOther) const {
      return (ulIP != addrOther.ulIP);
    };

    // Assignment operator
    inline SClientAddress &operator=(const SClientAddress &addrOther) {
      ulIP = addrOther.ulIP;
      strHost = addrOther.strHost;

      return *this;
    };

    // Write address
    inline void Write(CTStream *strm) {
      *strm << ulIP;
      *strm << strHost;
    };

    // Read address
    inline void Read(CTStream *strm) {
      *strm >> ulIP;
      *strm >> strHost;
    };
};

// Interface with methods for client logging
class CORE_API IClientLogging {
  public:
    // Get client's address by the client ID on the server
    static void GetAddress(SClientAddress &addr, INDEX iClient);

    // Get client identity by the client ID on the server
    static class CClientIdentity *GetIdentity(INDEX iClient);

  public:
    // Find client index in the list from an address and return address index
    static INDEX FindByAddress(INDEX &iClient, const SClientAddress &addr);

    // Find client index in the list from a character and return character index
    static INDEX FindByCharacter(INDEX &iClient, const CPlayerCharacter &pc);

  public:
    // Save client log
    static void SaveLog(void);

    // Load client log
    static void LoadLog(void);
};

// Declare all elements of client logging system
#include "ClientIdentity.h"
#include "ClientRestrictions.h"
#include "ActiveClients.h"

#endif
