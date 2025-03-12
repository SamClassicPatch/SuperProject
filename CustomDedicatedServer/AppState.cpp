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

#include "StdH.h"

// Application state variables
BOOL _bRunning = TRUE;
BOOL _bForceRestart = FALSE;
BOOL _bForceNextMap = FALSE;

// Stop running the server
void QuitGame(void) {
  _bRunning = FALSE;
};

// Restart the server
void RestartGame(void) {
  _bForceRestart = TRUE;
};

// Start next server map
void NextMap(void) {
  _bForceNextMap = TRUE;
};

#define REFRESHTIME (0.1f)

// Loading hook handler
static void LoadingHook_t(CProgressHookInfo *pphi)
{
  // measure time since last call
  static CTimerValue tvLast(0I64);
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  if (!_bRunning) {
    ThrowF_t(LOCALIZE("User break!"));
  }

  // if not first or final update, and not enough time passed
  if (pphi->phi_fCompleted != 0 && pphi->phi_fCompleted != 1
   && (tvNow - tvLast).GetSeconds() < REFRESHTIME) {
    // do nothing
    return;
  }

  tvLast = tvNow;

  // print status text
  CTString strRes;
  printf("\r                                                                      ");
  printf("\r%s : %3.0f%%\r", pphi->phi_strDescription, pphi->phi_fCompleted * 100);
};

// Set loading hook
void EnableLoadingHook(void)
{
  printf("\n");
  SetProgressHook(LoadingHook_t);
};

// Reset loading hook
void DisableLoadingHook(void)
{
  SetProgressHook(NULL);
  printf("\n");
};
