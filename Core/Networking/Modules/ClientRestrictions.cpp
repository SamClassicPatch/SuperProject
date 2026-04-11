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

// Records of all client restrictions
static CDynamicContainer<CClientRestriction> _cClientRestrictions;

// Set new ban time
void CClientRestriction::SetBanTime(CTimerValue tvTime) {
  // Indefinite ban
  if (tvTime.tv_llValue < 0) {
    tvBanExpiration.tv_llValue = -1;
    return;
  }

  tvBanExpiration = _pTimer->GetHighPrecisionTimer() + tvTime;
};

// Get remaining ban time
CTimerValue CClientRestriction::GetBanTime(void) const {
  return tvBanExpiration - _pTimer->GetHighPrecisionTimer();
};

// Check if banned
BOOL CClientRestriction::IsBanned(void) const {
  // Indefinite ban
  if (tvBanExpiration.tv_llValue < 0) {
    return TRUE;
  }

  return (GetBanTime().tv_llValue > 0);
};

// Print out remaining ban time
void CClientRestriction::PrintBanTime(CTString &str) const {
  // Indefinite time
  if (tvBanExpiration.tv_llValue < 0) {
    str = TRANS("indefinite time");
    return;
  }

  IData::PrintDetailedTime(str, GetBanTime());
};

// Set new mute time
void CClientRestriction::SetMuteTime(CTimerValue tvTime) {
  // Indefinite mute
  if (tvTime.tv_llValue < 0) {
    tvMuteExpiration.tv_llValue = -1;
    return;
  }

  tvMuteExpiration = _pTimer->GetHighPrecisionTimer() + tvTime;
};

// Get remaining mute time
CTimerValue CClientRestriction::GetMuteTime(void) const {
  return tvMuteExpiration - _pTimer->GetHighPrecisionTimer();
};

// Check if muted
BOOL CClientRestriction::IsMuted(void) const {
  // Indefinite mute
  if (tvMuteExpiration.tv_llValue < 0) {
    return TRUE;
  }

  return (GetMuteTime().tv_llValue > 0);
};

// Print out remaining mute time
void CClientRestriction::PrintMuteTime(CTString &str) const {
  // Indefinite time
  if (tvMuteExpiration.tv_llValue < 0) {
    str = TRANS("indefinite time");
    return;
  }

  IData::PrintDetailedTime(str, GetMuteTime());
};

// Ban a specific client by the identity index
CTString CClientRestriction::BanClient(INDEX iIdentity, FLOAT fTime) {
  // Get restriction record (create if there isn't one)
  CClientIdentity *pci = &_aClientIdentities[iIdentity];
  CClientRestriction &cr = pci->crRestrictions;

  // Update ban time and print it out
  cr.SetBanTime((DOUBLE)fTime);

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
CTString CClientRestriction::MuteClient(INDEX iIdentity, FLOAT fTime) {
  // Get restriction record (create if there isn't one)
  CClientIdentity *pci = &_aClientIdentities[iIdentity];
  CClientRestriction &cr = pci->crRestrictions;

  // Update mute time and print it out
  cr.SetMuteTime((DOUBLE)fTime);

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
