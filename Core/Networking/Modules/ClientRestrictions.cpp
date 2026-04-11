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

#include "ClientRestrictions.h"
#include "ActiveClients.h"
#include "Networking/NetworkFunctions.h"

static __int64 SecondsSinceEpoch(void) {
  typedef unsigned __int64 Ticks_t;

  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  Ticks_t ullTicks = ((Ticks_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

  // Difference between 1601-01-01 and 1970-01-01 in 100ns units (including leap years)
  const Ticks_t ullDiff = Ticks_t(116444736000000000);
  if (ullTicks < ullDiff) return 0; // Don't go below the epoch

  // Convert ticks since 1601-01-01 to seconds since Unix epoch
  Ticks_t ullEpoch100ns = ullTicks - ullDiff;
  return (__int64)(ullEpoch100ns / Ticks_t(10000000));
};

// Set new ban time
void CClientRestriction::SetBanTime(__int64 llSeconds) {
  // Indefinite ban
  if (llSeconds < 0) {
    llBanExpiration = -1;
    return;
  }

  llBanExpiration = SecondsSinceEpoch() + llSeconds;
};

// Get remaining ban time
__int64 CClientRestriction::GetBanTime(void) const {
  return llBanExpiration - SecondsSinceEpoch();
};

// Check if banned
BOOL CClientRestriction::IsBanned(void) const {
  // Indefinite ban
  if (llBanExpiration < 0) {
    return TRUE;
  }

  return (GetBanTime() > 0);
};

// Print out remaining ban time
void CClientRestriction::PrintBanTime(CTString &str) const {
  // Indefinite time
  if (llBanExpiration < 0) {
    str = TRANS("indefinite time");
    return;
  }

  IData::PrintDetailedTimeSec(str, GetBanTime());
};

// Set new mute time
void CClientRestriction::SetMuteTime(__int64 llSeconds) {
  // Indefinite mute
  if (llSeconds < 0) {
    llMuteExpiration = -1;
    return;
  }

  llMuteExpiration = SecondsSinceEpoch() + llSeconds;
};

// Get remaining mute time
__int64 CClientRestriction::GetMuteTime(void) const {
  return llMuteExpiration - SecondsSinceEpoch();
};

// Check if muted
BOOL CClientRestriction::IsMuted(void) const {
  // Indefinite mute
  if (llMuteExpiration < 0) {
    return TRUE;
  }

  return (GetMuteTime() > 0);
};

// Print out remaining mute time
void CClientRestriction::PrintMuteTime(CTString &str) const {
  // Indefinite time
  if (llMuteExpiration < 0) {
    str = TRANS("indefinite time");
    return;
  }

  IData::PrintDetailedTimeSec(str, GetMuteTime());
};

// Ban a specific client by the identity index
CTString CClientRestriction::BanClient(INDEX iIdentity, DOUBLE fTime) {
  // Get restriction record (create if there isn't one)
  CClientIdentity *pci = &_aClientIdentities[iIdentity];
  CClientRestriction &cr = pci->crRestrictions;

  // Update ban time and print it out
  cr.SetBanTime((__int64)ceil(fTime));

  CTString strTime;
  cr.PrintBanTime(strTime);

  // Get active clients of this identity
  CActiveClient::List cActive;
  CActiveClient::GetActiveClients(cActive, pci);

  // Disconnect active clients
  FOREACHINDYNAMICCONTAINER(cActive, CActiveClient, itac) {
    // Get active client index
    INDEX iBanClient = _aActiveClients.Index(itac);

    // Don't disconnect administrators
    if (CActiveClient::IsAdmin(iBanClient)) {
      continue;
    }

    CTString strReason;
    strReason.PrintF(TRANS("You have been banned for %s!"), strTime);

    INetwork::SendDisconnectMessage(iBanClient, strReason, FALSE);
  }

  // Report
  return CTString(0, TRANS("Client %d has been banned for %s!"), iIdentity, strTime);
};

// Mute a specific client by the identity index
CTString CClientRestriction::MuteClient(INDEX iIdentity, DOUBLE fTime) {
  // Get restriction record (create if there isn't one)
  CClientIdentity *pci = &_aClientIdentities[iIdentity];
  CClientRestriction &cr = pci->crRestrictions;

  // Update mute time and print it out
  cr.SetMuteTime((__int64)ceil(fTime));

  CTString strTime;
  cr.PrintMuteTime(strTime);

  // Report
  return CTString(0, TRANS("Client %d has been muted for %s!"), iIdentity, strTime);
};

// Kick a specific client by the identity index
CTString CClientRestriction::KickClient(INDEX iIdentity, const CTString &strReason) {
  // Get active clients of this identity
  CActiveClient::List cActive;
  CActiveClient::GetActiveClients(cActive, &_aClientIdentities[iIdentity]);

  // Disconnect active clients
  FOREACHINDYNAMICCONTAINER(cActive, CActiveClient, itac) {
    // Get active client index
    INDEX iKickClient = _aActiveClients.Index(itac);

    // Don't disconnect administrators
    if (CActiveClient::IsAdmin(iKickClient)) {
      continue;
    }

    // No reason specified
    if (strReason == "") {
      INetwork::SendDisconnectMessage(iKickClient, TRANS("You have been kicked!"), FALSE);

    } else {
      INetwork::SendDisconnectMessage(iKickClient, strReason, FALSE);
    }
  }

  // Report
  return CTString(0, TRANS("Client %d has been kicked! Reason: \"%s\""), iIdentity, strReason);
};
