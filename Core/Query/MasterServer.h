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

#ifndef CECIL_INCL_MASTERSERVER_H
#define CECIL_INCL_MASTERSERVER_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Master server functionality
namespace IMasterServer {

// Get current master server protocol
CORE_API INDEX GetProtocol(void);

// Start the server
CORE_API void OnServerStart(void);

// Stop the server
CORE_API void OnServerEnd(void);

// Server update step
CORE_API void OnServerUpdate(void);

// Server state has changed
CORE_API void OnServerStateChanged(void);

// Send heartbeat to the master server
CORE_API void SendHeartbeat(INDEX iChallenge);

// Request server list enumeration
CORE_API void EnumTrigger(BOOL bInternet);

// Replacement for CNetworkLibrary::EnumSessions()
CORE_API void EnumSessions(BOOL bInternet);

// Update enumerations from the server
CORE_API void EnumUpdate(void);

// Cancel master server enumeration
CORE_API void EnumCancel(void);

};

#endif
