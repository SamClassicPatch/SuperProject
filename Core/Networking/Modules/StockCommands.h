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

#ifndef CECIL_INCL_STOCKCOMMANDS_H
#define CECIL_INCL_STOCKCOMMANDS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface with built-in chat commands
class CORE_API IStockCommands {
  public:
    // List available chat commands
    static BOOL ListCommands(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Display name of the current map
    static BOOL CurrentMap(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Log in as administrator
    static BOOL PasswordLogin(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Remote console command
    static BOOL RemoteConsole(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Save game remotely
    static BOOL RemoteSave(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Display log of all clients
    static BOOL ClientLog(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Kick a specific client
    static BOOL KickClient(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Ban a specific client
    static BOOL BanClient(CTString &strResult, INDEX iClient, const CTString &strArguments);

    // Mute a specific client
    static BOOL MuteClient(CTString &strResult, INDEX iClient, const CTString &strArguments);
};

#endif
