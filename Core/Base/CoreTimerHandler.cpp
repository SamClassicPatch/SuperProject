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

#include "CoreTimerHandler.h"

#include "Networking/Modules.h"
#include "Networking/ExtPackets.h"

// Define pointer to the timer handler
CCoreTimerHandler *_pTimerHandler = NULL;

// This is called every CTimer::TickQuantum seconds
void CCoreTimerHandler::HandleTimer(void) {
  // Update observer camera controls during the game
  BOOL bInput = _pInput->IsInputEnabled() && GetGameAPI()->IsHooked()
             && !GetGameAPI()->IsMenuOn() && GetGameAPI()->IsGameOn();

  if (bInput) {
    GetGameAPI()->GetCamera().UpdateControls();
  }

  // Called every game tick, even if no session was started and while in pause
  static CTimerValue _tvLastSecCheck(-1.0f);

  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  if ((tvNow - _tvLastSecCheck).GetSeconds() >= 1.0f) {
    _tvLastSecCheck = tvNow;

    // Call per-second functions
    OnSecond();
  }

  // Call per-tick functions
  OnTick();
};

// Called every game tick
void CCoreTimerHandler::OnTick(void)
{
  // Check on annoying clients
  CActiveClient::CheckAnnoyingClients();

  // Update client restriction records
  CClientRestriction::UpdateExpirations();

  // Call per-tick function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_timer->OnTick == NULL) continue;

    itPlugin->pm_events.m_timer->OnTick();
  }

#if _PATCHCONFIG_EXT_PACKETS
  // Stop extension packet sounds when the game isn't active
  if (!GetGameAPI()->IsHooked() || !GetGameAPI()->IsGameOn()) {
    CExtPlaySound::StopAllSounds();
  }
#endif
};

// Called every game second
void CCoreTimerHandler::OnSecond(void)
{
  // Reset anti-flood counters
  IAntiFlood::ResetCounters();

  // Call per-second function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_timer->OnSecond == NULL) continue;

    itPlugin->pm_events.m_timer->OnSecond();
  }
};
