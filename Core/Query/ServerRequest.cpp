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

#include "StdH.h"

#if _PATCHCONFIG_NEW_QUERY

#include "ServerRequest.h"
#include "QueryManager.h"

// Add a new server request
void SServerRequest::AddRequest(const sockaddr_in &addr) {
  SServerRequest &req = IQuery::aRequests.Push();
  req.ulAddress = addr.sin_addr.s_addr;
  req.uwPort = addr.sin_port;
  req.tvRequestTime = _pTimer->GetHighPrecisionTimer();
};

// Find server request with a matching the socket address
SServerRequest *SServerRequest::Find(const sockaddr_in &addr) {
  // Search for the status request
  for (INDEX i = 0; i < IQuery::aRequests.Count(); i++) {
    SServerRequest &req = IQuery::aRequests[i];

    // Found matching address
    if (req.ulAddress == addr.sin_addr.s_addr && req.uwPort == addr.sin_port) {
      return &req;
    }
  }

  // None found
  return NULL;
};

// Get time from a server request and discard it, if found for this socket address
CTimerValue SServerRequest::PopRequestTime(const sockaddr_in &addr) {
  // Find server request for this socket address
  SServerRequest *preq = Find(addr);

  // If found
  if (preq != NULL) {
    // Get its time and clear it
    CTimerValue tvTime = preq->tvRequestTime;
    preq->Clear();

    return tvTime;
  }

  return __int64(-1);
};

#endif // _PATCHCONFIG_NEW_QUERY
