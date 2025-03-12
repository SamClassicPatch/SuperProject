/* Copyright (c) 2002-2012 Croteam Ltd. 
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

// [Cecil] Definitions of unexported methods of various networking classes from the engine

#ifndef CECIL_INCL_NETWORK_DEFS_H
#define CECIL_INCL_NETWORK_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Network/SessionSocket.h>
#include <Engine/Network/SessionState.h>

#include <Core/Networking/StreamBlock.h>

#if _PATCHCONFIG_FUNC_DEFINITIONS

// CNetworkStream

CNetworkStream::CNetworkStream(void)
{
};

CNetworkStream::~CNetworkStream(void) {
  Clear();
};

// Remove all blocks from the list and delete them
void CNetworkStream::Clear(void)
{
  FORDELETELIST(CNetworkStreamBlock, nsb_lnInStream, ns_lhBlocks, itnsbInList) {
    itnsbInList->nsb_lnInStream.Remove();
    delete &*itnsbInList;
  }
};

// CSessionSocketParams

CSessionSocketParams::CSessionSocketParams(void) {
  Clear();
};

void CSessionSocketParams::Clear(void) {
  ssp_iBufferActions = 2;
  ssp_iMaxBPS = 4000;
  ssp_iMinBPS = 1000;
};

// Acquire shell symbols and return their clamped values
static void ClampParams(INDEX &iActions, INDEX &iMaxBPS, INDEX &iMinBPS) {
  static CSymbolPtr piActions("cli_iBufferActions");
  static CSymbolPtr piMaxBPS("cli_iMaxBPS");
  static CSymbolPtr piMinBPS("cli_iMinBPS");

  iActions = Clamp(piActions.GetIndex(), INDEX(1), INDEX(20));
  iMaxBPS = Clamp(piMaxBPS.GetIndex(), INDEX(100), INDEX(1000000));
  iMinBPS = Clamp(piMinBPS.GetIndex(), INDEX(100), INDEX(1000000));
};

// Check if up-to-date with current params
BOOL CSessionSocketParams::IsUpToDate(void) {
  INDEX iActions, iMaxBPS, iMinBPS;
  ClampParams(iActions, iMaxBPS, iMinBPS);

  return ssp_iBufferActions == iActions
      && ssp_iMaxBPS == iMaxBPS
      && ssp_iMinBPS == iMinBPS;
};

// Update params
void CSessionSocketParams::Update(void) {
  ClampParams(ssp_iBufferActions, ssp_iMaxBPS, ssp_iMinBPS);
};

// Write into a packet
CNetworkMessage &operator<<(CNetworkMessage &nm, CSessionSocketParams &ssp) {
  nm << ssp.ssp_iBufferActions << ssp.ssp_iMaxBPS << ssp.ssp_iMinBPS;
  return nm;
};

// Read from a packet
CNetworkMessage &operator>>(CNetworkMessage &nm, CSessionSocketParams &ssp) {
  nm >> ssp.ssp_iBufferActions >> ssp.ssp_iMaxBPS >> ssp.ssp_iMinBPS;
  return nm;
};

// CSessionSocketParams

// Reset common variables
static __forceinline void ResetSessionSocketState(CSessionSocket *psso, BOOL bActive) {
  psso->sso_bActive = bActive;
  psso->sso_bVIP = FALSE;
  psso->sso_bSendStream = FALSE;
  psso->sso_iDisconnectedState = 0;
  psso->sso_iLastSentSequence = -1;
  psso->sso_ctBadSyncs = 0;
  psso->sso_tvLastMessageSent.Clear();
  psso->sso_tvLastPingSent.Clear();
};

CSessionSocket::CSessionSocket(void) {
  ResetSessionSocketState(this, FALSE);
};

CSessionSocket::~CSessionSocket(void) {
  ResetSessionSocketState(this, FALSE);
};

void CSessionSocket::Clear(void) {
  ResetSessionSocketState(this, FALSE);

  sso_tvMessageReceived.Clear();
  sso_tmLastSyncReceived = -1.0f;
  sso_nsBuffer.Clear();
  sso_sspParams.Clear();
};

void CSessionSocket::Activate(void) {
  ASSERT(!sso_bActive);
  ResetSessionSocketState(this, TRUE);

  sso_tvMessageReceived.Clear();
  sso_tmLastSyncReceived = -1.0f;
  sso_sspParams.Clear();
};

void CSessionSocket::Deactivate(void) {
  ResetSessionSocketState(this, FALSE);

  sso_nsBuffer.Clear();
  sso_sspParams.Clear();
};

BOOL CSessionSocket::IsActive(void) {
  return sso_bActive;
};

// CPredictedEvent

CPredictedEvent::CPredictedEvent(void)
{
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
