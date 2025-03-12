/* C/* Copyright (c) 2002-2012 Croteam Ltd.
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
#include "MGChangePlayer.h"

extern CSoundData *_psdPress;

void CMGChangePlayer::OnActivate(void) {
  PlayMenuSound(_psdPress);
  _iLocalPlayer = mg_iLocalPlayer;

  if (GetGameAPI()->GetProfileForMenu(mg_iLocalPlayer) < 0) {
    GetGameAPI()->SetProfileForMenu(mg_iLocalPlayer, 0);
  }

  _pGUIM->gmPlayerProfile.gm_piCurrentPlayer = &GetGameAPI()->aiMenuLocalPlayers[mg_iLocalPlayer];

  extern BOOL _bPlayerMenuFromSinglePlayer;
  _bPlayerMenuFromSinglePlayer = FALSE;
  ChangeToMenu(&_pGUIM->gmPlayerProfile);
}

void CMGChangePlayer::SetPlayerText(void) {
  INDEX iPlayer = GetGameAPI()->GetProfileForMenu(mg_iLocalPlayer);
  CPlayerCharacter &pc = *GetGameAPI()->GetPlayerCharacter(iPlayer);

  if (iPlayer < 0 || iPlayer >= GetGameAPI()->GetProfileCount()) {
    SetText("????");
  } else {
    CTString strPlayer;
    strPlayer.PrintF(LOCALIZE("Player %d: %s\n"), mg_iLocalPlayer + 1, pc.GetNameForPrinting());

    SetText(strPlayer);
  }
}