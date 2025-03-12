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

#ifndef CECIL_INCL_SERVERREQUEST_H
#define CECIL_INCL_SERVERREQUEST_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Server request for receiving server pings
struct SServerRequest {
  ULONG ulAddress;
  UWORD uwPort;
  CTimerValue tvRequestTime;

  // Constructor
  SServerRequest() {
    Clear();
  };

  // Clear data in the dynamic stack array
  void Clear(void) {
    ulAddress = 0;
    uwPort = 0;
    tvRequestTime.Clear();
  };

  // Add a new server request
  static void AddRequest(const sockaddr_in &addr);

  // Find server request with a matching the socket address
  static SServerRequest *Find(const sockaddr_in &addr);

  // Get time from a server request and discard it, if found for this socket address
  static CTimerValue PopRequestTime(const sockaddr_in &addr);
};

#endif
