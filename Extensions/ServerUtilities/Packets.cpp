/* Copyright (c) 2024 Dreamy Cecil
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

#include <CoreLib/Networking/Modules/ActiveClients.h>

// Handle third person option on character change
static void HandleThirdPerson(INDEX iClient, CPlayerCharacter &pc)
{
  // Disallow third person view
  if (_psThirdPerson.GetIndex() == -1) return;

  CPlayerSettings *pps = (CPlayerSettings *)pc.pc_aubAppearance;

  // Check if client keeps trying to change the option
  if (pps->ps_ulFlags & PSF_PREFER3RDPERSON) {
    _aActiveClients[iClient].ctAnnoyanceLevel += 10;
  }

  pps->ps_ulFlags &= ~PSF_PREFER3RDPERSON;
};

void IPacketEvents_OnCharacterConnect(INDEX iClient, CPlayerCharacter &pc)
{
  HandleThirdPerson(iClient, pc);
};

BOOL IPacketEvents_OnCharacterChange(INDEX iClient, INDEX iPlayer, CPlayerCharacter &pc)
{
  HandleThirdPerson(iClient, pc);
  return TRUE;
};

void IPacketEvents_OnPlayerAction(INDEX iClient, INDEX iPlayer, CPlayerAction &pa, INDEX iResent)
{
  // Lock disabled buttons
  pa.pa_ulButtons &= _psActionButtons.GetIndex();

  // Disallow toggling third person view using a specified button
  INDEX i3rdPersonButton = _psThirdPerson.GetIndex();

  if (i3rdPersonButton != -1) {
    const INDEX iBit = (1 << i3rdPersonButton);

    // Check if client keeps pressing the button
    if (iResent == -1 && pa.pa_ulButtons & iBit) {
      _aActiveClients[iClient].ctAnnoyanceLevel += 10;
    }

    pa.pa_ulButtons &= ~iBit;
  }
};
