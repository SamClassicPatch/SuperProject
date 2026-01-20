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

#ifndef CECIL_INCL_PATCHES_NETWORK_H
#define CECIL_INCL_PATCHES_NETWORK_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Core/Networking/CommInterface.h>
#include <Core/Networking/MessageProcessing.h>

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_EXTEND_NETWORK

class CComIntPatch : public CCommunicationInterface {
  public:
  #if _PATCHCONFIG_NEW_QUERY
    void P_EndWinsock(void);
  #endif

    void P_ServerInit(void);
    void P_ServerClose(void);
};

class CMessageDisPatch : public CMessageDispatcher {
  public:
    // Packet receiving method type
    typedef BOOL (CCommunicationInterface::*CReceiveFunc)(INDEX, void *, SLONG &);

  public:
    // Send a reliable packet to the server
    void P_SendToServerReliable(const CNetworkMessage &nmMessage);

    // Server receives a speciifc packet
    BOOL ReceiveFromClientSpecific(INDEX iClient, CNetworkMessage &nmMessage, CReceiveFunc pFunc);

    // Server receives a packet
    BOOL P_ReceiveFromClient(INDEX iClient, CNetworkMessage &nmMessage);

    // Server receives a reliable packet
    BOOL P_ReceiveFromClientReliable(INDEX iClient, CNetworkMessage &nmMessage);
};

class CNetworkPatch : public CNetworkLibrary {
  public:
    // Pointer type to CNetworkLibrary::StartPeerToPeer_t()
    #if SE1_GAME != SS_REV
      typedef void (CNetworkLibrary::*CStartP2PFunc)
        (const CTString &, const CTFileName &, ULONG, INDEX, BOOL, void *);
    #else
      typedef void (CNetworkLibrary::*CStartP2PFunc)
        (const CTString &, const CTFileName &, ULONG, INDEX, BOOL, void *, const CTString &, const CTString &);
    #endif

  public:
    // Go through the level changing process
    void P_ChangeLevelInternal(void);

    // Save current game
    void P_Save(const CTFileName &fnmGame);

    // Load saved game
    void P_Load(const CTFileName &fnmGame);

    // Stop current game
    void P_StopGame(void);

    // Start new game session
    void P_StartPeerToPeer(const CTString &strSessionName, const CTFileName &fnmWorld,
    #if SE1_GAME != SS_REV
      ULONG ulSpawnFlags, INDEX ctMaxPlayers, BOOL bWaitAllPlayers, void *pSesProps);
    #else
      ULONG ulSpawnFlags, INDEX ctMaxPlayers, BOOL bWaitAllPlayers, void *pSesProps, const CTString &strGamemode, const CTString &strTags);
    #endif

    // Start playing a demo
    void P_StartDemoPlay(const CTFileName &fnDemo);

    // Start recording a demo
    void P_StartDemoRec(const CTFileName &fnDemo);

    // Stop recording a demo
    void P_StopDemoRec(void);
};

class CSessionStatePatch : public CSessionState {
  public:
    void P_FlushProcessedPredictions(void);

    // Client processes received packet from the server
    void P_ProcessGameStreamBlock(CNetworkMessage &nmMessage);

    // Start session as a client
    void P_Start_AtClient(INDEX ctLocalPlayers);

    // Wait for a stream from a server
    void P_WaitStream(CTMemoryStream &strmMessage, const CTString &strName, INDEX iMsgCode);

    void P_Stop(void);

    // Read session state
    void P_Read(CTStream *pstr);

    // Write session state
    void P_Write(CTStream *pstr);

  #if _PATCHCONFIG_GUID_MASKING
    // Send synchronization packet to the server (as client) or add it to the buffer (as server)
    void P_MakeSynchronisationCheck(void);
  #endif
};

#if _PATCHCONFIG_GUID_MASKING

class CPlayerEntityPatch : public CPlayerEntity {
  public:
    void P_Write(CTStream *ostr);

    void P_ChecksumForSync(ULONG &ulCRC, INDEX iExtensiveSyncCheck);
};

#endif // _PATCHCONFIG_GUID_MASKING

#endif // _PATCHCONFIG_EXTEND_NETWORK

#endif
