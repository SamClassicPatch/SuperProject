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

#ifndef CECIL_INCL_CLIENTRESTRICTIONS_H
#define CECIL_INCL_CLIENTRESTRICTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// One record of client's restriction
class CORE_API CClientRestriction {
  private:
    __int64 llBanExpiration;  // Time when the ban expires
    __int64 llMuteExpiration; // Time when the mute expires

  public:
    // Default constructor
    CClientRestriction() : llBanExpiration(0), llMuteExpiration(0) {};

  // Ban methods
  public:

    // Set new ban time in seconds
    void SetBanTime(__int64 llSeconds);

    // Get remaining ban time in seconds
    __int64 GetBanTime(void) const;

    // Check if banned
    BOOL IsBanned(void) const;

    // Print out remaining ban time
    void PrintBanTime(CTString &str) const;

  // Mute methods
  public:

    // Set new mute time in seconds
    void SetMuteTime(__int64 llSeconds);

    // Get remaining mute time in seconds
    __int64 GetMuteTime(void) const;

    // Check if muted
    BOOL IsMuted(void) const;

    // Print out remaining mute time
    void PrintMuteTime(CTString &str) const;

  // Static methods
  public:

    // Ban a specific client by the identity index
    static CTString BanClient(INDEX iIdentity, DOUBLE fTime);

    // Mute a specific client by the identity index
    static CTString MuteClient(INDEX iIdentity, DOUBLE fTime);

    // Kick a specific client by the identity index
    static CTString KickClient(INDEX iIdentity, const CTString &strReason);
};

#endif
