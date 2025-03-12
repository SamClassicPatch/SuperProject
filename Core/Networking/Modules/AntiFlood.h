/* Copyright (c) 2022-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_ANTIFLOOD_H
#define CECIL_INCL_ANTIFLOOD_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Kick clients for attempted packet flood
CORE_API extern INDEX ser_bEnableAntiFlood;

// Allowed messages threshold before treating it as packet flood
CORE_API extern INDEX ser_iPacketFloodThreshold;

// Allowed messages from client per second
CORE_API extern INDEX ser_iMaxMessagesPerSecond;

// Interface for anti-flood system
class IAntiFlood {
  public:
    // Handle character changes from a client
    static BOOL HandleCharacterChange(INDEX iClient);

    // Handle chat messages from a client
    static BOOL HandleChatMessage(INDEX iClient);

    // Reset packet counters for each client
    static void ResetCounters(void);
};

#endif
