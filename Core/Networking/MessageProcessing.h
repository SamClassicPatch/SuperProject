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

#ifndef CECIL_INCL_MESSAGEPROCESSING_H
#define CECIL_INCL_MESSAGEPROCESSING_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Identification tag to append at the end of the session state to distinguish patch users from normal clients
static const char _aSessionStatePatchTag[8] = {
  (char)0x43, (char)0x65, (char)0x63, (char)0x69,
  (char)0x6C, (char)0xBB, (char)0xD1, (char)0xEB,
};

// Interface with custom message processing methods
class CORE_API IProcessPacket {
  public:
    typedef CStaticArray<CSyncCheck> CSyncCheckArray;

  // Helper variables and functions
  public:
    // Special values for handling client index (never use positive integers!)
    enum {
      CLT_NONE = -1, // No client
      CLT_SAVE = -2, // In the process of saving the game
    };

    // Which client sent last packet to the server
    static INDEX _iHandlingClient;

    // Notify clients whenever they desync
    static INDEX _bReportSyncBadToClients;

    // Prevent clients from joining unless they have the same patch installed
    static INDEX _bForbidVanilla;

  #if _PATCHCONFIG_GAMEPLAY_EXT

    // Gameplay extensions
    static IConfig::GameplayExt _gexSetup;

  #endif // _PATCHCONFIG_GAMEPLAY_EXT

    // Check if gameplay extensions are enabled for the server
    static BOOL GameplayExtEnabled(void);

    // Allow changing value of a symbol unless currently running a server
    static BOOL UpdateSymbolValue(void *pSymbol);

    // Register commands for packet processing
    static void RegisterCommands(void);

  #if _PATCHCONFIG_GUID_MASKING

    // Arrays of sync checks per client
    static CStaticArray<CSyncCheckArray> _aClientChecks;

    // Should mask player GUIDs or not
    static BOOL _bMaskGUIDs;

    // Check if should use GUID masking
    static BOOL ShouldMaskGUIDs(void);

    // Clear arrays with sync checks
    static void ClearSyncChecks(void);

  #endif // _PATCHCONFIG_GUID_MASKING

    // Buffer sync check for the server
    static void AddSyncCheck(const INDEX iClient, const CSyncCheck &sc);

    // Find buffered sync check for a given tick
    static INDEX FindSyncCheck(const INDEX iClient, TIME tmTick, CSyncCheck &sc);

    // Mask player GUID using data from the player buffer
    static void MaskGUID(UBYTE *aubGUID, CPlayerBuffer &plb);

    // Check if character can be changed for a specific player
    static BOOL CanChangeCharacter(CPlayerEntity *pen);

  // Session state data
  public:

    // Write patch identification tag into a stream
    static void WritePatchTag(CTStream &strm);

    // Read patch identification tag from a stream and verify it
    static BOOL ReadPatchTag(CTStream &strm, ULONG *pulReadVersion);

    // Reset data before starting any session
    static void ResetSessionData(BOOL bNewSetup);

    // Append extra info about the patched server
    static void WriteServerInfoToSessionState(CTStream &strm);

    // Read extra info about the patched server
    static void ReadServerInfoFromSessionState(CTStream &strm);

  // Message processors (if they return TRUE, they also get processed by CServer afterwards)
  public:

    // Client confirming the disconnection
    static void OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage);

    // Client requesting the session state
    static void OnConnectRemoteSessionStateRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client requesting the connection to the server
    static void OnPlayerConnectRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client changing the character
    static void OnCharacterChangeRequest(INDEX iClient, CNetworkMessage &nmMessage);

    // Client sending player actions
    static void OnPlayerAction(INDEX iClient, CNetworkMessage &nmMessage);

    // Client sends a CRC check
    static void OnSyncCheck(INDEX iClient, CNetworkMessage &nmMessage);

    // Client sending a chat message
    static BOOL OnChatInRequest(INDEX iClient, CNetworkMessage &nmMessage);
};

#endif
