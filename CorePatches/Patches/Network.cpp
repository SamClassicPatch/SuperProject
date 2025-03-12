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

#if _PATCHCONFIG_ENGINEPATCHES

#include "Network.h"

#include <Core/Query/QueryManager.h>
#include <Core/Networking/NetworkFunctions.h>

#include <Core/Definitions/ActionBufferDefs.inl>
#include <Core/Definitions/PlayerCharacterDefs.inl>
#include <Core/Definitions/PlayerTargetDefs.inl>

#if _PATCHCONFIG_EXTEND_NETWORK

// Original function pointers
void (CCommunicationInterface::*pServerInit)(void) = NULL;
void (CCommunicationInterface::*pServerClose)(void) = NULL;

#if _PATCHCONFIG_NEW_QUERY

void CComIntPatch::P_EndWinsock(void) {
  // New query manager
  if (!ms_bVanillaQuery) {
    // Stop master server enumeration
    if (ms_bDebugOutput) {
      CPutString("CCommunicationInterface::EndWinsock() -> IMasterServer::EnumCancel()\n");
    }

    IMasterServer::EnumCancel();
  }

  // Original function code
  #if SE1_VER >= SE1_107
    if (!cci_bWinSockOpen) return;
  #endif

  int iResult = WSACleanup();
  ASSERT(iResult == 0);

  #if SE1_VER >= SE1_107
    cci_bWinSockOpen = FALSE;
  #endif
};

#endif // _PATCHCONFIG_NEW_QUERY

void CComIntPatch::P_ServerInit(void) {
  // Proceed to the original function
  (this->*pServerInit)();

#if _PATCHCONFIG_GUID_MASKING
  IProcessPacket::ClearSyncChecks();
#endif

#if _PATCHCONFIG_NEW_QUERY
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  if (ms_bDebugOutput) {
    CPutString("CCommunicationInterface::Server_Init_t()\n");
  }

  // Start new master server
  static CSymbolPtr symptr("ser_bEnumeration");

  if (symptr.GetIndex() && GetComm().IsNetworkEnabled()) {
    IMasterServer::OnServerStart();
  }
#endif // _PATCHCONFIG_NEW_QUERY
};

void CComIntPatch::P_ServerClose(void) {
  // Proceed to the original function
  (this->*pServerClose)();

#if _PATCHCONFIG_GUID_MASKING
  IProcessPacket::ClearSyncChecks();
#endif

#if _PATCHCONFIG_NEW_QUERY
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  if (ms_bDebugOutput) {
    CPutString("CCommunicationInterface::Server_Close()\n");
  }

  // Stop new master server
  static CSymbolPtr symptr("ser_bEnumeration");

  if (symptr.GetIndex()) {
    IMasterServer::OnServerEnd();
  }
#endif // _PATCHCONFIG_NEW_QUERY
};

// Send a reliable packet to the server
void CMessageDisPatch::P_SendToServerReliable(const CNetworkMessage &nmMessage) {
  // [Cecil] Append extra info to the session state to distinguish patch users from normal clients
  if (nmMessage.GetType() == MSG_REQ_CONNECTREMOTESESSIONSTATE) {
    CNetworkMessage &nmWriteable = const_cast<CNetworkMessage &>(nmMessage);

    // Patch identification tag and the release version
    nmWriteable.Write(_aSessionStatePatchTag, sizeof(_aSessionStatePatchTag));
    nmWriteable << (ULONG)ClassicsCore_GetVersion();
  }

  GetComm().Client_Send_Reliable((void *)nmMessage.nm_pubMessage, nmMessage.nm_slSize);

  // Relevant inline reimplementation of UpdateSentMessageStats()
  static CSymbolPtr pbReport("net_bReportTraffic");

  if (pbReport.GetIndex()) {
    CPrintF("Sent: %d\n", nmMessage.nm_slSize);
  }
};

// Server receives a speciifc packet
BOOL CMessageDisPatch::ReceiveFromClientSpecific(INDEX iClient, CNetworkMessage &nmMessage, CReceiveFunc pFunc) {
  // Receive message in static buffer
  nmMessage.nm_slSize = nmMessage.nm_slMaxSize;

  // Receive using a specific method
  BOOL bReceived = (GetComm().*pFunc)(iClient, (void *)nmMessage.nm_pubMessage, nmMessage.nm_slSize);

  // If there is message
  if (bReceived) {
    // Init the message structure
    nmMessage.nm_pubPointer = nmMessage.nm_pubMessage;
    nmMessage.nm_iBit = 0;

    UBYTE ubType;
    nmMessage.Read(&ubType, sizeof(ubType));
    nmMessage.nm_mtType = (MESSAGETYPE)ubType;

    // Process received packet
    return TRUE;
  }

  return bReceived;
};

// Server receives a packet
BOOL CMessageDisPatch::P_ReceiveFromClient(INDEX iClient, CNetworkMessage &nmMessage) {
  FOREVER {
    // Process unreliable message
    if (ReceiveFromClientSpecific(iClient, nmMessage, &CCommunicationInterface::Server_Receive_Unreliable)) {
      // Set client that's being handled
      IProcessPacket::_iHandlingClient = iClient;
      BOOL bPass = INetwork::ServerHandle(this, iClient, nmMessage);

      // Exit to process through engine's CServer::Handle()
      if (bPass) return TRUE;

    // No more messages
    } else {
      break;
    }
  }

  // Exit engine's loop
  return FALSE;
};

// Server receives a reliable packet
BOOL CMessageDisPatch::P_ReceiveFromClientReliable(INDEX iClient, CNetworkMessage &nmMessage) {
  FOREVER {
    // Process reliable message
    if (ReceiveFromClientSpecific(iClient, nmMessage, &CCommunicationInterface::Server_Receive_Reliable)) {
      // Set client that's being handled
      IProcessPacket::_iHandlingClient = iClient;
      BOOL bPass = INetwork::ServerHandle(this, iClient, nmMessage);

      // Exit to process through engine's CServer::Handle()
      if (bPass) return TRUE;

    // Proceed to unreliable messages
    } else {
      break;
    }
  }

  // Exit engine's loop
  return FALSE;
};

// Original function pointers
void (CSessionState::*pFlushPredictions)(void) = NULL;
void (CSessionState::*pStartAtClient)(INDEX) = NULL;
void (CSessionState::*pReadSessionState)(CTStream *) = NULL;
void (CSessionState::*pWriteSessionState)(CTStream *) = NULL;

void (CNetworkLibrary::*pLoadGame)(const CTFileName &) = NULL;
void (CNetworkLibrary::*pStopGame)(void) = NULL;
CNetworkPatch::CStartP2PFunc pStartPeerToPeer = NULL;
void (CNetworkLibrary::*pStartDemoPlay)(const CTFileName &) = NULL;

// Read or write server info after the session state (for saves & demos)
static BOOL _bSerializeServerInfo = FALSE;

// Enable info serialization and auto-disable it upon exiting the function
struct SerializeServerInfoNow {
  int iDummy;

  SerializeServerInfoNow() {
    _bSerializeServerInfo = TRUE;
  };

  ~SerializeServerInfoNow() {
    _bSerializeServerInfo = FALSE;
  };
};

void (CNetworkLibrary::*pChangeLevel)(void) = NULL;

// Go through the level changing process
void CNetworkPatch::P_ChangeLevelInternal(void) {
  // Proceed to the original function
  (this->*pChangeLevel)();

#if _PATCHCONFIG_GUID_MASKING
  // Clear sync checks for each client on a new level
  if (IsServer()) {
    IProcessPacket::ClearSyncChecks();
  }
#endif // _PATCHCONFIG_GUID_MASKING

  // Change level for Core
  IHooks::OnChangeLevel();
};

// Save current game
void CNetworkPatch::P_Save(const CTFileName &fnmGame) {
  // Synchronize access to network
  CTSingleLock slNetwork(&ga_csNetwork, TRUE);

  // Must be the server
  if (!IsServer()) {
    ThrowF_t(LOCALIZE("Cannot save game - not a server!\n"));
  }

  // Currently saving
  IProcessPacket::_iHandlingClient = IProcessPacket::CLT_SAVE;

  // [Cecil] Write server info
  SerializeServerInfoNow serInfo;

  // Create the file
  CTFileStream strmFile;
  strmFile.Create_t(fnmGame);

  // Write game to stream
  strmFile.WriteID_t("GAME");
  ga_sesSessionState.Write_t(&strmFile);
  strmFile.WriteID_t("GEND"); // Game end

  // [Cecil] Save game for Core
  IHooks::OnGameSave(fnmGame);
};

// Load saved game
void CNetworkPatch::P_Load(const CTFileName &fnmGame) {
  // Reset data and read server info
  IProcessPacket::ResetSessionData(FALSE); // Load game
  SerializeServerInfoNow serInfo;

  // Proceed to the original function
  (this->*pLoadGame)(fnmGame);

  // Load game for Core
  IHooks::OnGameLoad(fnmGame);
};

// Stop current game
void CNetworkPatch::P_StopGame(void) {
  // Stop game for Core
  IHooks::OnGameStop();

  // Proceed to the original function
  (this->*pStopGame)();

  // Make sure there is enough space for local players
  ga_aplsPlayers.Clear();
  ga_aplsPlayers.New(ICore::MAX_LOCAL_PLAYERS);
};

// Start new game session
void CNetworkPatch::P_StartPeerToPeer(const CTString &strSessionName, const CTFileName &fnmWorld,
#if SE1_GAME != SS_REV
  ULONG ulSpawnFlags, INDEX ctMaxPlayers, BOOL bWaitAllPlayers, void *pSesProps)
#else
  ULONG ulSpawnFlags, INDEX ctMaxPlayers, BOOL bWaitAllPlayers, void *pSesProps, const CTString &strGamemode, const CTString &strTags)
#endif
{
  // Reset data before starting
  IProcessPacket::ResetSessionData(TRUE); // Server start

  // Proceed to the original function
  #if SE1_GAME != SS_REV
    (this->*pStartPeerToPeer)(strSessionName, fnmWorld, ulSpawnFlags, ctMaxPlayers, bWaitAllPlayers, pSesProps);
  #else
    (this->*pStartPeerToPeer)(strSessionName, fnmWorld, ulSpawnFlags, ctMaxPlayers, bWaitAllPlayers, pSesProps, strGamemode, strTags);
  #endif
};

// Start playing a demo
void CNetworkPatch::P_StartDemoPlay(const CTFileName &fnDemo) {
  // Reset data and read server info
  IProcessPacket::ResetSessionData(FALSE); // Play demo
  SerializeServerInfoNow serInfo;

  // Proceed to the original function
  (this->*pStartDemoPlay)(fnDemo);

  // Play demo for Core
  IHooks::OnDemoPlay(fnDemo);
};

// Start recording a demo
void CNetworkPatch::P_StartDemoRec(const CTFileName &fnDemo) {
  // Synchronize access to network
  CTSingleLock slNetwork(&ga_csNetwork, TRUE);

  // Already recording
  if (ga_bDemoRec) {
    throw LOCALIZE("Already recording a demo!");
  }

  // [Cecil] Write server info
  SerializeServerInfoNow serInfo;

  // Create the file
  ga_strmDemoRec.Create_t(fnDemo);

  // Write initial info to stream
  ga_strmDemoRec.WriteID_t("DEMO");
  ga_strmDemoRec.WriteID_t("MVER");
  ga_strmDemoRec << ULONG(_SE_BUILD_MINOR);
  ga_sesSessionState.Write_t(&ga_strmDemoRec);

  // Set demo recording state
  ga_bDemoRec = TRUE;

  // [Cecil] Start demo recording for Core
  IHooks::OnDemoStart(fnDemo);
};

// Stop recording a demo
void CNetworkPatch::P_StopDemoRec(void) {
  // Synchronize access to network
  CTSingleLock slNetwork(&ga_csNetwork, TRUE);

  // Not recording
  if (!ga_bDemoRec) return;

  // Write terminal info to the stream and close the file
  ga_strmDemoRec.WriteID_t("DEND"); // Demo end
  ga_strmDemoRec.Close();

  // Set demo recording state
  ga_bDemoRec = FALSE;

  // [Cecil] Stop demo recording for Core
  IHooks::OnDemoStop();
};

void CSessionStatePatch::P_FlushProcessedPredictions(void) {
  // Proceed to the original function
  (this->*pFlushPredictions)();

#if _PATCHCONFIG_NEW_QUERY
  // Keep using old query manager
  if (ms_bVanillaQuery) return;

  // Update server for the master server
  static CSymbolPtr symptr("ser_bEnumeration");

  if (GetComm().IsNetworkEnabled() && symptr.GetIndex()) {
    if (ms_bDebugOutput) {
      //CPutString("CSessionState::FlushProcessedPredictions() -> IMasterServer::OnServerUpdate()\n");
    }
    IMasterServer::OnServerUpdate();
  }
#endif // _PATCHCONFIG_NEW_QUERY
};

// Client processes received packet from the server
void CSessionStatePatch::P_ProcessGameStreamBlock(CNetworkMessage &nmMessage) {
  // Copy the tick to process into tick used for all tasks
  _pTimer->SetCurrentTick(ses_tmLastProcessedTick);

  // Call API every simulation tick
  IHooks::OnTick();

  // Quit if don't need to process standard packets
  if (!INetwork::ClientHandle(this, nmMessage)) {
    return;
  }

  switch (nmMessage.GetType())
  {
    // Adding a new player
    case MSG_SEQ_ADDPLAYER: {
      // Non-action sequence
      _pNetwork->AddNetGraphValue(NGET_NONACTION, 1.0f);

      // Read player index and the character
      INDEX iNewPlayer;
      CPlayerCharacter pcCharacter;
      nmMessage >> iNewPlayer >> pcCharacter;

      CPlayerTarget &plt = ses_apltPlayers[iNewPlayer];

      // Delete all predictors
      _pNetwork->ga_World.DeletePredictors();

      // Activate the player
      plt.Activate();

      // Find entity with this character
      CPlayerEntity *penNewPlayer = _pNetwork->ga_World.FindEntityWithCharacter(pcCharacter);

      // If none found
      if (penNewPlayer == NULL) {
        // Create a new player entity
        const CPlacement3D pl(FLOAT3D(0.0f, 0.0f, 0.0f), ANGLE3D(0.0f, 0.0f, 0.0f));

        try {
          static const CTString strPlayerClass = "Classes\\Player.ecl";
          penNewPlayer = (CPlayerEntity *)_pNetwork->ga_World.CreateEntity_t(pl, strPlayerClass);

          // Attach entity to client data
          plt.AttachEntity(penNewPlayer);

          // Attach character to it
          penNewPlayer->en_pcCharacter = pcCharacter;

          // Prepare the entity
          penNewPlayer->Initialize();

        } catch (char *strError) {
          FatalError(LOCALIZE("Cannot load Player class:\n%s"), strError);
        }

        if (!_pNetwork->IsPlayerLocal(penNewPlayer)) {
          CPrintF(LOCALIZE("%s joined\n"), penNewPlayer->GetPlayerName());
        }

      // If found some entity
      } else {
        // Attach entity to client data
        plt.AttachEntity(penNewPlayer);

        // Update its character
        penNewPlayer->CharacterChanged(pcCharacter);

        if (!_pNetwork->IsPlayerLocal(penNewPlayer)) {
          CPrintF(LOCALIZE("%s rejoined\n"), penNewPlayer->GetPlayerName());
        }
      }

      // [Cecil] Call player addition for Core
      IHooks::OnAddPlayer(plt, _pNetwork->IsPlayerLocal(penNewPlayer));
    } break;

    // Removing a player
    case MSG_SEQ_REMPLAYER: {
      // Non-action sequence
      _pNetwork->AddNetGraphValue(NGET_NONACTION, 1.0f);

      // Read player index
      INDEX iPlayer;
      nmMessage >> iPlayer;

      CPlayerTarget &plt = ses_apltPlayers[iPlayer];
      CPlayerEntity *penRemPlayer = plt.plt_penPlayerEntity;

      // Delete all predictors
      _pNetwork->ga_World.DeletePredictors();

      // [Cecil] Call player removal for Core
      IHooks::OnRemovePlayer(plt, _pNetwork->IsPlayerLocal(penRemPlayer));

      // Inform entity of disconnnection
      CPrintF(LOCALIZE("%s left\n"), penRemPlayer->GetPlayerName());
      penRemPlayer->Disconnect();

      // Deactivate the player
      plt.Deactivate();

      // Handle sent entity events
      ses_bAllowRandom = TRUE;
      CEntity::HandleSentEvents();
      ses_bAllowRandom = FALSE;
    } break;

    // Character change
    case MSG_SEQ_CHARACTERCHANGE: {
      // Non-action sequence
      _pNetwork->AddNetGraphValue(NGET_NONACTION, 1.0f);

      // Read player index and the character
      INDEX iPlayer;
      CPlayerCharacter pcCharacter;
      nmMessage >> iPlayer >> pcCharacter;

      CPlayerEntity *penPlayer = ses_apltPlayers[iPlayer].plt_penPlayerEntity;

      // Delete all predictors
      _pNetwork->ga_World.DeletePredictors();

      // [Cecil] Check if the entity is even capable of changing its appearance
      if (IProcessPacket::CanChangeCharacter(penPlayer)) {
        // Change the character
        penPlayer->CharacterChanged(pcCharacter);

      } else {
        static CTString strWarning = TRANS("Please wait until you are fully connected to change your character.");
        CPutString(strWarning + "\n");
      }

      // Handle sent entity events
      ses_bAllowRandom = TRUE;
      CEntity::HandleSentEvents();
      ses_bAllowRandom = FALSE;
    } break;

    // Client actions
    case MSG_SEQ_ALLACTIONS: {
      // Read packet time
      TIME tmPacket;
      nmMessage >> tmPacket;

      // Time must be greater than what has been previously received
      TIME tmTickQuantum = _pTimer->TickQuantum;
      TIME tmPacketDelta = tmPacket - ses_tmLastProcessedTick;

      // Report debug info upon mistimed actions
      if (Abs(tmPacketDelta - tmTickQuantum) >= tmTickQuantum / 10.0f) {
        CPrintF(LOCALIZE("Session state: Mistimed MSG_ALLACTIONS: Last received tick %g, this tick %g\n"),
          ses_tmLastProcessedTick, tmPacket);
      }

      // Remember received tick
      ses_tmLastProcessedTick = tmPacket;

      // Don't wait for new players anymore
      ses_bWaitAllPlayers = FALSE;

      // Delete all predictors
      _pNetwork->ga_World.DeletePredictors();

      // Process the tick
      ProcessGameTick(nmMessage, tmPacket);
    } break;

    // Pause message
    case MSG_SEQ_PAUSE: {
      // Non-action sequence
      _pNetwork->AddNetGraphValue(NGET_NONACTION, 1.0f);

      // Delete all predictors
      _pNetwork->ga_World.DeletePredictors();

      BOOL bPauseBefore = ses_bPause;

      // Read pause state and the client
      nmMessage >> (INDEX &)ses_bPause;

      CTString strPauser;
      nmMessage >> strPauser;

      // Report who paused
      if (ses_bPause != bPauseBefore && strPauser != LOCALIZE("Local machine")) {
        if (ses_bPause) {
          CPrintF(LOCALIZE("Paused by '%s'\n"), strPauser);
        } else {
          CPrintF(LOCALIZE("Unpaused by '%s'\n"), strPauser);
        }
      }

      // Must keep wanting current state
      ses_bWantPause = ses_bPause;
    } break;

    // Invalid packet
    default: ASSERT(FALSE);
  }
};

// Start session as a client
void CSessionStatePatch::P_Start_AtClient(INDEX ctLocalPlayers) {
  // Get passwords
  static CSymbolPtr pstrPwd1("net_strConnectPassword");
  static CSymbolPtr pstrPwd2("net_strVIPPassword");

  const CTString strOldPwd1 = pstrPwd1.GetString();
  const CTString strOldPwd2 = pstrPwd2.GetString();

  // Set temporary passwords for client connection
  if (cli_strConnectPassword != "") {
    pstrPwd1.GetString() = cli_strConnectPassword;
    pstrPwd2.GetString() = cli_strConnectPassword;
  }

  // Reset data before starting
  IProcessPacket::ResetSessionData(FALSE); // Client start

  // Proceed to the original function
  (this->*pStartAtClient)(ctLocalPlayers);

  // Restore passwords
  pstrPwd1.GetString() = strOldPwd1;
  pstrPwd2.GetString() = strOldPwd2;
};

// Wait for a stream from a server
void CSessionStatePatch::P_WaitStream(CTMemoryStream &strmMessage, const CTString &strName, INDEX iMsgCode) {
  // Start waiting for server's response
  SetProgressDescription(LOCALIZE("waiting for ") + strName);
  CallProgressHook_t(0.0f);

  SLONG slReceivedLast = 0;

  // [Cecil] NOTE: This code doesn't set '_bRunNetUpdates' variable from the engine because it's not exported and
  // it isn't used for any checks; it's some kind of leftover from 1.07 netcode changes (it doesn't exist in 1.05)
  static CSymbolPtr pfTimeout("net_tmConnectionTimeout");

  // Repeat until timed out
  for (TIME tmWait = 0; tmWait < pfTimeout.GetFloat() * 1000;
    Sleep(NET_WAITMESSAGE_DELAY), tmWait += NET_WAITMESSAGE_DELAY)
  {
    // Update network connection sockets
    #if SE1_VER >= SE1_107
      if (!GetComm().Client_Update()) break;
    #else
      GetComm().Client_Update();
    #endif

    // Check how much is received so far
    SLONG slExpectedSize, slReceivedSize;
    GetComm().Client_PeekSize_Reliable(slExpectedSize, slReceivedSize);

    // If nothing received yet
    if (slExpectedSize == 0) {
      // Progress with waiting
      CallProgressHook_t(tmWait / (pfTimeout.GetFloat() * 1000));

    // If something is received
    } else {
      // Remember new data
      if (slReceivedSize != slReceivedLast) {
        slReceivedLast = slReceivedSize;

        // Reset timeout
        tmWait = 0;
      }

      // Progress with receiving
      SetProgressDescription(LOCALIZE("receiving ") + strName + "  ");
      CallProgressHook_t((FLOAT)slReceivedSize / (FLOAT)slExpectedSize);
    }

    // Continue waiting if not everything received yet
    if (!_pNetwork->ReceiveFromServerReliable(strmMessage)) {
      continue;
    }

    // Read message identifier
    strmMessage.SetPos_t(0);

    INDEX iID;
    strmMessage >> iID;

    // Received the message
    if (iID == iMsgCode) {
      CallProgressHook_t(1.0f);

      // [Cecil] If received session state data
      if (iMsgCode == MSG_REP_CONNECTREMOTESESSIONSTATE) {
        // Remember stream position
        const SLONG slStreamPos = strmMessage.GetPos_t();

        try {
          // Skip strings
          CTString strMOTD;
          CTFileName fnmWorld;
          strmMessage >> strMOTD;
          strmMessage >> fnmWorld;

          // Skip flags (4) and session properties (2048)
          strmMessage.Seek_t(sizeof(ULONG) + NET_MAXSESSIONPROPERTIES, CTStream::SD_CUR);

          // Read server info
          IProcessPacket::ReadServerInfoFromSessionState(strmMessage);

        // Catch and report random errors, just in case
        } catch (char *strError) {
          CPrintF(TRANS("Cannot read server info from session state:\n%s\n"), strError);
        }

        // Reset stream position
        strmMessage.SetPos_t(slStreamPos);
      }
      return;

    // If disconnected
    } else if (iID == MSG_INF_DISCONNECTED) {
      // Confirm disconnection
      CNetworkMessage nmConfirmDisconnect((MESSAGETYPE)INetwork::PCK_REP_DISCONNECTED);
      _pNetwork->SendToServerReliable(nmConfirmDisconnect);

      // Report the reason
      CTString strReason;
      strmMessage >> strReason;
      ses_strDisconnected = strReason;

      ThrowF_t(LOCALIZE("Disconnected: %s\n"), strReason);

    // Otherwise it's an invalid message
    } else {
      ThrowF_t(LOCALIZE("Invalid stream while waiting for %s"), strName);
    }

    // [Cecil] NOTE: Unreachable code?
    #if SE1_VER >= SE1_107
      // Quit if client is disconnected
      if (!GetComm().Client_IsConnected()) {
        ThrowF_t(LOCALIZE("Client disconnected"));
      }
    #endif
  }

  ThrowF_t(LOCALIZE("Timeout while waiting for %s"), strName);
};

void CSessionStatePatch::P_Stop(void) {
  // Original function code
  ses_bKeepingUpWithTime = TRUE;
  ses_tmLastUpdated = -100;
  ses_bAllowRandom = TRUE;
  ses_bPredicting = FALSE;
  ses_tmPredictionHeadTick = -2.0f;
  ses_tmLastSyncCheck = 0;
  ses_bPause = FALSE;
  ses_bWantPause = FALSE;
  ses_bGameFinished = FALSE;
#if SE1_GAME == SS_REV
  ses_ulField4 = 0;
#endif
  ses_bWaitingForServer = FALSE;
  ses_strDisconnected = "";
  ses_ctMaxPlayers = 1;
  ses_fRealTimeFactor = 1.0f;
  ses_bWaitAllPlayers = FALSE;
  ses_apeEvents.PopAll();

  _pTimer->DisableLerp();

  // Notify server about the disconnection
  if (IsCommInitialized()) {
    CNetworkMessage nmConfirmDisconnect((MESSAGETYPE)INetwork::PCK_REP_DISCONNECTED);
    _pNetwork->SendToServerReliable(nmConfirmDisconnect);
  }

  GetComm().Client_Close();
  ForgetOldLevels();

  ses_apltPlayers.Clear();
  ses_apltPlayers.New(ICore::MAX_GAME_PLAYERS);
};

// Read session state
void CSessionStatePatch::P_Read(CTStream *pstr) {
  // Proceed to the original function
  (this->*pReadSessionState)(pstr);

  // Read server info, if needed
  if (_bSerializeServerInfo) {
    IProcessPacket::ReadServerInfoFromSessionState(*pstr);
  }

  _bSerializeServerInfo = FALSE;
};

// Write session state
void CSessionStatePatch::P_Write(CTStream *pstr) {
  // Proceed to the original function
  (this->*pWriteSessionState)(pstr);

  // Write server info, if needed
  if (_bSerializeServerInfo) {
    IProcessPacket::WriteServerInfoToSessionState(*pstr);
  }

  _bSerializeServerInfo = FALSE;
};

#if _PATCHCONFIG_GUID_MASKING

// Send synchronization packet to the server (as client) or add it to the buffer (as server)
void CSessionStatePatch::P_MakeSynchronisationCheck(void) {
  if (!IsCommInitialized()) return;

  // Don't check yet
  if (ses_tmLastSyncCheck + ses_tmSyncCheckFrequency > ses_tmLastProcessedTick) {
    return;
  }

  ses_tmLastSyncCheck = ses_tmLastProcessedTick;

  ULONG ulLocalCRC;
  CSyncCheck scLocal;

  // Buffer sync checks for the server
  if (IProcessPacket::ShouldMaskGUIDs()) {
    CServer &srv = _pNetwork->ga_srvServer;

    // Make local checksum for each session separately
    for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
      CSessionSocket &sso = srv.srv_assoSessions[iSession];

      if (iSession > 0 && !sso.sso_bActive) {
        continue;
      }

      IProcessPacket::_iHandlingClient = iSession;

      CRC_Start(ulLocalCRC);
      ChecksumForSync(ulLocalCRC, ses_iExtensiveSyncCheck);
      CRC_Finish(ulLocalCRC);

      // Create sync check
      CSyncCheck sc;
      sc.sc_tmTick = ses_tmLastSyncCheck;
      sc.sc_iSequence = ses_iLastProcessedSequence; 
      sc.sc_ulCRC = ulLocalCRC;
      sc.sc_iLevel = ses_iLevel;

      // Add this sync check to this client
      IProcessPacket::AddSyncCheck(iSession, sc);

      // Save local sync check for the server client
      if (iSession == 0) {
        scLocal = sc;
      }
    }

  // Local client sync check
  } else {
    CRC_Start(ulLocalCRC);
    ChecksumForSync(ulLocalCRC, ses_iExtensiveSyncCheck);
    CRC_Finish(ulLocalCRC);

    scLocal.sc_tmTick = ses_tmLastSyncCheck;
    scLocal.sc_iSequence = ses_iLastProcessedSequence; 
    scLocal.sc_ulCRC = ulLocalCRC;
    scLocal.sc_iLevel = ses_iLevel;

    // Add local sync check to the server if not masking
    if (_pNetwork->IsServer()) {
      IProcessPacket::AddSyncCheck(0, scLocal);
    }
  }

  IProcessPacket::_iHandlingClient = IProcessPacket::CLT_NONE;

  // Send sync check to the server (including the server client)
  CNetworkMessage nmSyncCheck(MSG_SYNCCHECK);
  nmSyncCheck.Write(&scLocal, sizeof(scLocal));

  _pNetwork->SendToServer(nmSyncCheck);
};

// Get player buffer from the server associated with a player entity in the world
static CPlayerBuffer *PlayerBufferFromEntity(CPlayerEntity *pen) {
  CStaticArray<CPlayerTarget> &aPlayerTargets = _pNetwork->ga_sesSessionState.ses_apltPlayers;

  for (INDEX i = 0; i < aPlayerTargets.Count(); i++) {
    if (aPlayerTargets[i].plt_penPlayerEntity == pen) {
      return &_pNetwork->ga_srvServer.srv_aplbPlayers[i];
    }
  }

  return NULL;
};

void CPlayerEntityPatch::P_Write(CTStream *ostr) {
  CMovableModelEntity::Write_t(ostr);
  const INDEX iClient = IProcessPacket::_iHandlingClient;

  // Normal writing for clients
  if (!IProcessPacket::ShouldMaskGUIDs() || iClient == IProcessPacket::CLT_NONE) {
    *ostr << en_pcCharacter << en_plViewpoint;
    return;
  }

  // Get player buffer for this entity
  CPlayerBuffer *pplb = PlayerBufferFromEntity(this);

  // Start with invalid GUID
  UBYTE aubGUID[16];
  memset(aubGUID, 0xFFFFFFFF, sizeof(aubGUID));

  // If there's an associated player buffer
  if (pplb != NULL) {
    // Only save GUID of the server client
    if (iClient == IProcessPacket::CLT_SAVE) {
      if (pplb->plb_iClient == 0) {
        memcpy(aubGUID, pplb->plb_pcCharacter.pc_aubGUID, sizeof(aubGUID));
      }

    // Use GUID for the current client
    } else if (iClient == pplb->plb_iClient) {
      memcpy(aubGUID, pplb->plb_pcCharacter.pc_aubGUID, sizeof(aubGUID));

    // Otherwise mask it
    } else {
      IProcessPacket::MaskGUID(aubGUID, *pplb);
    }
  }

  // Serialize CPlayerCharacter
  ostr->WriteID_t("PLC4");
  *ostr << en_pcCharacter.pc_strName << en_pcCharacter.pc_strTeam;
  ostr->Write_t(aubGUID, sizeof(aubGUID));
  ostr->Write_t(en_pcCharacter.pc_aubAppearance, sizeof(en_pcCharacter.pc_aubAppearance));

  *ostr << en_plViewpoint;
};

void CPlayerEntityPatch::P_ChecksumForSync(ULONG &ulCRC, INDEX iExtensiveSyncCheck) {
  CMovableModelEntity::ChecksumForSync(ulCRC, iExtensiveSyncCheck);
  const INDEX iClient = IProcessPacket::_iHandlingClient;

  // Normal check for clients
  if (!IProcessPacket::ShouldMaskGUIDs() || iClient == IProcessPacket::CLT_NONE) {
    CRC_AddBlock(ulCRC, en_pcCharacter.pc_aubGUID, sizeof(en_pcCharacter.pc_aubGUID));
    CRC_AddBlock(ulCRC, en_pcCharacter.pc_aubAppearance, sizeof(en_pcCharacter.pc_aubAppearance));
    return;
  }

  // Get player buffer for this entity
  CPlayerBuffer *pplb = PlayerBufferFromEntity(this);

  UBYTE aubGUID[16];

  // Use GUID from the buffer for the current client
  if (iClient == pplb->plb_iClient) {
    memcpy(aubGUID, pplb->plb_pcCharacter.pc_aubGUID, sizeof(aubGUID));

  } else {
    IProcessPacket::MaskGUID(aubGUID, *pplb);
  }

  CRC_AddBlock(ulCRC, aubGUID, sizeof(aubGUID));
  CRC_AddBlock(ulCRC, en_pcCharacter.pc_aubAppearance, sizeof(en_pcCharacter.pc_aubAppearance));
};

#endif // _PATCHCONFIG_GUID_MASKING

#endif // _PATCHCONFIG_EXTEND_NETWORK

#endif // _PATCHCONFIG_ENGINEPATCHES
