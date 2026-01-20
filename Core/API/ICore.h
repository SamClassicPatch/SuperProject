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

#ifndef CECIL_INCL_COREINTERFACE_H
#define CECIL_INCL_COREINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Pre-1.10 variables removed from Revolution engine
#if SE1_GAME == SS_REV
  CORE_API extern CTString _strModName;
  CORE_API extern CTString _strModURL;
  CORE_API extern CTString _strModExt;
#endif

// Common game variables
CORE_API extern CTString sam_strFirstLevel;
CORE_API extern CTString sam_strIntroLevel;
CORE_API extern CTString sam_strGameName;
CORE_API extern CTString sam_strVersion;

// Temporary password for connecting to some server
CORE_API extern CTString cli_strConnectPassword;

// Current values of input axes
CORE_API extern FLOAT inp_afAxisValues[MAX_OVERALL_AXES];

namespace ICore {

// Own constants to replace engine ones
const INDEX MAX_SERVER_CLIENTS = 16;                 // 16 in vanilla as SERVER_CLIENTS
const INDEX MAX_GAME_COMPUTERS = MAX_SERVER_CLIENTS; // 16 in vanilla as NET_MAXGAMECOMPUTERS
const INDEX MAX_GAME_PLAYERS   = MAX_SERVER_CLIENTS; // 16 in vanilla as NET_MAXGAMEPLAYERS
const INDEX MAX_LOCAL_PLAYERS  = MAX_SERVER_CLIENTS; //  4 in vanilla as NET_MAXLOCALPLAYERS

// Toggle vanilla query manager
CORE_API void DisableGameSpy(void);

// Reinitialize console in the engine
CORE_API void ReinitConsole(INDEX ctCharsPerLine, INDEX ctLines);

}; // namespace

#endif
