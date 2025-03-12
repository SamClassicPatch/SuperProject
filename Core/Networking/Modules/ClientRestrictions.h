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

#ifndef CECIL_INCL_CLIENTRESTRICTIONS_H
#define CECIL_INCL_CLIENTRESTRICTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ClientIdentity.h"

// One record of client's restriction
class CORE_API CClientRestriction {
  public:
    CClientIdentity *pciClient; // Whose record this is

    CTimerValue tvBanExpiration; // Time when the ban expires
    CTimerValue tvMuteExpiration; // Time when the mute expires

  public:
    // Default constructor
    CClientRestriction() : pciClient(NULL) {
      tvBanExpiration.Clear();
      tvMuteExpiration.Clear();
    };

  // Ban methods
  public:

    // Set new ban time
    void SetBanTime(CTimerValue tvTime);

    // Get remaining ban time
    CTimerValue GetBanTime(void) const;

    // Check if banned
    BOOL IsBanned(void) const;

    // Print out remaining ban time
    void PrintBanTime(CTString &str) const;

  // Mute methods
  public:

    // Set new mute time
    void SetMuteTime(CTimerValue tvTime);

    // Get remaining mute time
    CTimerValue GetMuteTime(void) const;

    // Check if muted
    BOOL IsMuted(void) const;

    // Print out remaining mute time
    void PrintMuteTime(CTString &str) const;

  // Static methods
  public:

    // Check if any records have expired and remove them from the list
    static void UpdateExpirations(void);

    // Add new restriction for a specific client identity
    static CClientRestriction *AddNew(CClientIdentity *pci);

    // Check if some client is banned and return a record with the ban
    static CClientRestriction *IsBanned(CClientIdentity *pci);

    // Check if some client is muted and return a record with the mute
    static CClientRestriction *IsMuted(CClientIdentity *pci);

    // Ban a specific client by the identity index
    static CTString BanClient(INDEX iIdentity, FLOAT fTime);

    // Mute a specific client by the identity index
    static CTString MuteClient(INDEX iIdentity, FLOAT fTime);

    // Kick a specific client by the identity index
    static CTString KickClient(INDEX iIdentity, const CTString &strReason);
};

#endif
