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

#include "MessageProcessing.h"
#include "NetworkFunctions.h"
#include "Modules.h"

#include "Query/QueryManager.h"

// Which client sent last packet to the server
INDEX IProcessPacket::_iHandlingClient = IProcessPacket::CLT_NONE;

// Notify clients whenever they desync
INDEX IProcessPacket::_bReportSyncBadToClients = FALSE;

// Prevent clients from joining unless they have the same patch installed
INDEX IProcessPacket::_bForbidVanilla = FALSE;

#if _PATCHCONFIG_GAMEPLAY_EXT

// Gameplay extensions (reset to recommended settings)
IConfig::GameplayExt IProcessPacket::_gexSetup(FALSE);

// Check if gameplay extensions are enabled for the server
BOOL IProcessPacket::GameplayExtEnabled(void) {
  return _gexSetup[k_EGameplayExt_Enable].IsTrue();
};

#else

// Gameplay extensions don't exist
BOOL IProcessPacket::GameplayExtEnabled(void) {
  return FALSE;
};

#endif // _PATCHCONFIG_GAMEPLAY_EXT

// Allow changing value of a symbol unless currently running a server
BOOL IProcessPacket::UpdateSymbolValue(void *pSymbol) {
  // Cannot change the value while running the game as a server
  if (_pNetwork->IsServer()) {
    CPutString(TRANS("Cannot change the value of this shell symbol while the server is running!\n"));
    return FALSE;
  }

  // Safe to change
  return TRUE;
};

// Register commands for packet processing
void IProcessPacket::RegisterCommands(void) {
  _pShell->DeclareSymbol("INDEX UpdateServerSymbolValue(INDEX);", &UpdateSymbolValue);

  _pShell->DeclareSymbol("persistent user INDEX ser_bReportSyncBadToClients;", &_bReportSyncBadToClients);
  _pShell->DeclareSymbol("persistent user INDEX ser_bForbidVanilla pre:UpdateServerSymbolValue;", &_bForbidVanilla);

#if _PATCHCONFIG_GAMEPLAY_EXT
  // Gameplay extensions
  _gexSetup.DeclareSymbols();
#endif
};

#if _PATCHCONFIG_GUID_MASKING

// Arrays of sync checks per client
CStaticArray<IProcessPacket::CSyncCheckArray> IProcessPacket::_aClientChecks;

// Should mask player GUIDs or not
BOOL IProcessPacket::_bMaskGUIDs = FALSE;

// Check if should use GUID masking
BOOL IProcessPacket::ShouldMaskGUIDs(void) {
  // Setting is on; multiplayer game
  return IProcessPacket::_bMaskGUIDs && INetwork::IsHostingMultiplayer();
};

// Clear arrays with sync checks
void IProcessPacket::ClearSyncChecks(void)
{
  FOREACHINSTATICARRAY(IProcessPacket::_aClientChecks, IProcessPacket::CSyncCheckArray, itar) {
    itar->Clear();
  }
};

#endif // _PATCHCONFIG_GUID_MASKING

// Buffer sync check for the server
void IProcessPacket::AddSyncCheck(const INDEX iClient, const CSyncCheck &sc)
{
#if _PATCHCONFIG_GUID_MASKING
  // Use the first array if not masking
  CSyncCheckArray &aChecks = _aClientChecks[ShouldMaskGUIDs() ? iClient : 0];
#else
  CSyncCheckArray &aChecks = _pNetwork->ga_srvServer.srv_ascChecks;
#endif

  // Recreate the buffer if the size differs
  static CSymbolPtr symptr("ser_iSyncCheckBuffer");
  INDEX &iBuffer = symptr.GetIndex();

  iBuffer = ClampDn(iBuffer, (INDEX)1);

  if (aChecks.Count() != iBuffer) {
    aChecks.Clear();
    aChecks.New(iBuffer);
  }

  // Find the oldest one
  INDEX iOldest = 0;

  for (INDEX i = 1; i < aChecks.Count(); i++) {
    if (aChecks[i].sc_tmTick < aChecks[iOldest].sc_tmTick) {
      iOldest = i;
    }
  }

  // Overwrite it
  aChecks[iOldest] = sc;
};

// Find buffered sync check for a given tick
INDEX IProcessPacket::FindSyncCheck(const INDEX iClient, TIME tmTick, CSyncCheck &sc)
{
#if _PATCHCONFIG_GUID_MASKING
  // Use the first array if not masking
  CSyncCheckArray &aChecks = _aClientChecks[ShouldMaskGUIDs() ? iClient : 0];
#else
  CSyncCheckArray &aChecks = _pNetwork->ga_srvServer.srv_ascChecks;
#endif

  BOOL bHasEarlier = FALSE;
  BOOL bHasLater = FALSE;

  for (INDEX i = 0; i < aChecks.Count(); i++) {
    TIME tmInTable = aChecks[i].sc_tmTick;

    if (tmInTable == tmTick) {
      sc = aChecks[i];
      return 0;

    } else if (tmInTable < tmTick) {
      bHasEarlier = TRUE;

    } else if (tmInTable > tmTick) {
      bHasLater = TRUE;
    }
  }

  if (!bHasEarlier) {
    ASSERT(bHasLater);
    return -1;

  } else if (!bHasLater) {
    ASSERT(bHasEarlier);
    return +1;
  }

  // Cannot have earlier, later and not found all at once
  ASSERT(FALSE);
  return +1;
};

// Mask player GUID using data from the player buffer
void IProcessPacket::MaskGUID(UBYTE *aubGUID, CPlayerBuffer &plb) {
  // Clear GUID
  memset(aubGUID, 0, 16);

  // Use client and buffer indices for uniqueness
  aubGUID[0] = plb.plb_iClient;
  aubGUID[1] = plb.plb_Index;
};

// Check if character can be changed for a specific player
BOOL IProcessPacket::CanChangeCharacter(CPlayerEntity *pen) {
  // Retrieve CPlayer::m_penAnimator
  static CPropertyPtr pptrAnimator(pen);

  // Check if player has a player animator to handle the character change
  if (pptrAnimator.ByVariable("CPlayer", "m_penAnimator"))
  {
    CEntity *penAnimator = ENTITYPROPERTY(pen, pptrAnimator.Offset(), CEntityPointer);
    return (penAnimator != NULL);
  }

  // Just proceed if cannot determine
  return TRUE;
};

// Client confirming the disconnection
void IProcessPacket::OnClientDisconnect(INDEX iClient, CNetworkMessage &nmMessage) {
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.sso_iDisconnectedState = 2;

  // Make client inactive
  CActiveClient::DeactivateClient(iClient);
};

// Try checking if the client is running the same patch version as the server
static BOOL CheckClientPatch(INDEX iClient, CNetworkMessage &nmMessage) {
  const CTString strClient = GetComm().Server_GetClientName(iClient);

  // Tag length and client version
  const ULONG ctTagLen = sizeof(_aSessionStatePatchTag);
  ULONG ulClientVer;

  // Desired position in the packet vs the last possible position
  const UBYTE *pubDesired = nmMessage.nm_pubPointer + ctTagLen + sizeof(ulClientVer);
  const UBYTE *pubEnd = nmMessage.nm_pubMessage + nmMessage.nm_slSize;

  // If there's enough space for the tag and the version
  if (pubDesired <= pubEnd) {
    // Read them
    char aClientTag[ctTagLen];

    nmMessage.Read(aClientTag, ctTagLen);
    nmMessage >> ulClientVer;

    // Verify client tag and version
    const BOOL bTagMatch = (memcmp(aClientTag, _aSessionStatePatchTag, ctTagLen) == 0);
    const BOOL bVersionMatch = (ulClientVer == ClassicsCore_GetVersion());

    CPrintF(TRANS("Server: Client '%s' has provided an identification tag:\n"
                  "  Tag match: %d | Version match: %d\n"), strClient.str_String, bTagMatch, bVersionMatch);

    // Client is running the right patch
    return (bTagMatch && bVersionMatch);

  } else {
    CPrintF(TRANS("Server: Client '%s' hasn't provided an identification tag\n"), strClient.str_String);
  }

  // Client isn't running the right patch
  return FALSE;
};

// Client requesting the session state
void IProcessPacket::OnConnectRemoteSessionStateRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // [Cecil] Get identity of a remote client
  ASSERT(iClient > 0);
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // [Cecil] Check if the client is banned
  CClientRestriction *pcr = CClientRestriction::IsBanned(pci);
  BOOL bBanned = (pcr != NULL);

  static CSymbolPtr pbWhiteList("ser_bInverseBanning");

  // Not allowed on the server
  if (bBanned == !pbWhiteList.GetIndex()) {
    // No specific ban record
    if (!bBanned) {
      INetwork::SendDisconnectMessage(iClient, TRANS("You are not allowed on this server!"), TRUE);
      return;
    }

    CTString strTime, strReason;
    pcr->PrintBanTime(strTime);

    strReason.PrintF(TRANS("You have been banned for %s!"), strTime);
    INetwork::SendDisconnectMessage(iClient, strReason, TRUE);
    return;
  }

  // Original function code
  static CSymbolPtr pstrIPMask("ser_strIPMask");

  // IP address is banned
  if (IData::MatchesMask(GetComm().Server_GetClientName(iClient), pstrIPMask.GetString()) == !pbWhiteList.GetIndex()) {
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("You are banned from this server"), TRUE);
    return;
  }

  // Default version info
  INDEX iMajor = 109;
  INDEX iMinor = 1;

  // Read version tag
  INDEX iTag;
  nmMessage >> iTag;

  if (iTag == 'VTAG') {
    nmMessage >> iMajor >> iMinor;
  }

  // Disconnect if mismatching version
  if (iMajor != _SE_BUILD_MAJOR || iMinor != _SE_BUILD_MINOR) {
    CTString strExplanation;
    strExplanation.PrintF(LOCALIZE(
      "This server runs version %d.%d, your version is %d.%d.\n"
      "Please visit http://www.croteam.com for information on version updating."),
      _SE_BUILD_MAJOR, _SE_BUILD_MINOR, iMajor, iMinor);

    INetwork::SendDisconnectMessage(iClient, strExplanation, TRUE);
    return;
  }

  // Check mod
  CTString strGivenMod;
  nmMessage >> strGivenMod;

  // Disconnect with a special non-translatable mod tag
  if (_strModName != strGivenMod) {
    CTString strMod(0, "MOD:%s\\%s", _strModName, _strModURL);
    INetwork::SendDisconnectMessage(iClient, strMod, TRUE);
    return;
  }

  CTString strGivenPassword;
  nmMessage >> strGivenPassword;

  INDEX ctWantedLocalPlayers;
  nmMessage >> ctWantedLocalPlayers;

  // [Cecil] Read socket parameters in advance
  CSessionSocketParams sspClient;
  nmMessage >> sspClient;

  // [Cecil] Check if vanilla clients are forbidden or using incompatible gameplay extensions
  const BOOL bForbid = (_bForbidVanilla || GameplayExtEnabled());

  // [Cecil] Disconnect unless the client has the right patch version installed
  if (bForbid && !CheckClientPatch(iClient, nmMessage)) {
    // Prompt to download the right patch version
    const CTString strVer = ClassicsCore_GetVersionName();
    const CTString strMod = "MOD:Classics Patch " + strVer + "\\" + CLASSICSPATCH_URL_TAGRELEASE(strVer);

    INetwork::SendDisconnectMessage(iClient, strMod, TRUE);
    return;
  }

  // [Cecil] Check for connecting clients with split-screen
  if (!CheckSplitScreenClients(iClient, ctWantedLocalPlayers)) return;

  static CSymbolPtr pstrConnectPassword("net_strConnectPassword");
  static CSymbolPtr pstrVIPPassword("net_strVIPPassword");
  static CSymbolPtr pstrObserverPassword("net_strObserverPassword");
  static CSymbolPtr piVIPReserve("net_iVIPReserve");
  static CSymbolPtr piMaxObservers("net_iMaxObservers");
  static CSymbolPtr piMaxClients("net_iMaxClients");

  const CTString strPwdConnect  = pstrConnectPassword.GetString();
  const CTString strPwdVIP      = pstrVIPPassword.GetString();
  const CTString strPwdObserver = pstrObserverPassword.GetString();

  // Count allowed players, clients and VIPs, then check if allowed to connect
  INDEX ctMaxAllowedPlayers = _pNetwork->ga_sesSessionState.ses_ctMaxPlayers;
  INDEX ctMaxAllowedClients = ctMaxAllowedPlayers;

  if (piMaxClients.GetIndex() > 0) {
    ctMaxAllowedClients = ClampUp(piMaxClients.GetIndex(), ICore::MAX_GAME_COMPUTERS);
  }

  INDEX ctMaxAllowedVIPPlayers = 0;
  INDEX ctMaxAllowedVIPClients = 0;

  if (piVIPReserve.GetIndex() > 0 && strPwdVIP != "") {
    ctMaxAllowedVIPPlayers = ClampDn(piVIPReserve.GetIndex() - INetwork::CountPlayers(TRUE), 0L);
    ctMaxAllowedVIPClients = ClampDn(piVIPReserve.GetIndex() - INetwork::CountClients(TRUE), 0L);
  }

  const INDEX ctMaxAllowedObservers = piMaxObservers.GetIndex();

  // Get current amounts
  const INDEX ctCurrentPlayers = INetwork::CountPlayers(FALSE);
  const INDEX ctCurrentClients = INetwork::CountClients(FALSE);
  const INDEX ctCurrentObservers = INetwork::CountObservers();

  // Check which passwords this client can satisfy
  BOOL bAutorizedAsVIP = FALSE;
  BOOL bAutorizedAsObserver = FALSE;
  BOOL bAutorizedAsPlayer = FALSE;

  if (strPwdVIP != "" && strPwdVIP == strGivenPassword) {
    bAutorizedAsVIP = TRUE;
    bAutorizedAsPlayer = TRUE;
    bAutorizedAsObserver = TRUE;
  }

  if (strPwdConnect == "" || strPwdConnect == strGivenPassword) {
    bAutorizedAsPlayer = TRUE;
  }

  if ((strPwdObserver == "" && bAutorizedAsPlayer) || strPwdObserver == strGivenPassword) {
    bAutorizedAsObserver = TRUE;
  }

  // Artificially decrease allowed number of players and clients for VIPs
  if (!bAutorizedAsVIP) {
    ctMaxAllowedPlayers = ClampDn(ctMaxAllowedPlayers - ctMaxAllowedVIPPlayers, 0L);
    ctMaxAllowedClients = ClampDn(ctMaxAllowedClients - ctMaxAllowedVIPClients, 0L);
  }

  // Disconnect if too many clients or players
  if (ctCurrentPlayers + ctWantedLocalPlayers > ctMaxAllowedPlayers
   || ctCurrentClients + 1 > ctMaxAllowedClients) {
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("Server full!"), TRUE);
    return;
  }

  // Disconnect observers
  if (ctWantedLocalPlayers == 0) {
    // If too many
    if (ctCurrentObservers >= ctMaxAllowedObservers && !bAutorizedAsVIP) {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("Too many observers!"), TRUE);
      return;
    }

    // If password is wrong
    if (!bAutorizedAsObserver) {
      if (strGivenPassword == "") {
        INetwork::SendDisconnectMessage(iClient, LOCALIZE("This server requires password for observers!"), TRUE);
      } else {
        INetwork::SendDisconnectMessage(iClient, LOCALIZE("Wrong observer password!"), TRUE);
      }
      return;
    }

  // Disconnect players if password is wrong
  } else if (!bAutorizedAsPlayer) {
    if (strGivenPassword == "") {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("This server requires password to connect!"), TRUE);
    } else {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("Wrong password!"), TRUE);
    }
    return;
  }

  // Activate client socket and read parameters for it
  CSessionSocket &sso = _pNetwork->ga_srvServer.srv_assoSessions[iClient];
  sso.Activate();

  sso.sso_ctLocalPlayers = ctWantedLocalPlayers;
  sso.sso_bVIP = bAutorizedAsVIP;
  sso.sso_sspParams = sspClient;

  // Try to send base info
  try {
    static CSymbolPtr pstrMOTD("ser_strMOTD");

    CTMemoryStream strmInfo;
    strmInfo << INDEX(MSG_REP_CONNECTREMOTESESSIONSTATE);
    strmInfo << pstrMOTD.GetString();
    strmInfo << _pNetwork->ga_World.wo_fnmFileName;
    strmInfo << _pNetwork->ga_sesSessionState.ses_ulSpawnFlags;
    strmInfo.Write_t(_pNetwork->ga_aubDefaultProperties, NET_MAXSESSIONPROPERTIES);

    // [Cecil] Append extra info to the session state for patched clients
    WriteServerInfoToSessionState(strmInfo);

    const SLONG slSize = strmInfo.GetStreamSize() >> 10; // Size in KB

    // Send reply to the remote session state
    _pNetwork->SendToClientReliable(iClient, strmInfo);

    CPrintF(LOCALIZE("Server: Sent initialization info to '%s' (%dk)\n"),
      GetComm().Server_GetClientName(iClient).str_String, slSize);

  // Abort on error
  } catch (char *strError) {
    sso.Deactivate();
    CPrintF(LOCALIZE("Server: Cannot prepare connection data: %s\n"), strError);
  }
};

// Client requesting the connection to the server
void IProcessPacket::OnPlayerConnectRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Read character data
  CPlayerCharacter pcCharacter;
  nmMessage >> pcCharacter;

  // Get client identity
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // Add new character to the identity
  pci->AddNewCharacter(pcCharacter);

  // Check for blacklisted/whitelisted character names
  if (!GetComm().Server_IsClientLocal(iClient)) {
    static CSymbolPtr pstrNameMask("ser_strNameMask");
    static CSymbolPtr pbWhiteList("ser_bInverseBanning");

    // Character name is banned
    if (IData::MatchesMask(pcCharacter.GetName(), pstrNameMask.GetString()) == !pbWhiteList.GetIndex()) {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("You are banned from this server"), FALSE);
      return;
    }
  }

  CServer &srv = _pNetwork->ga_srvServer;
  CSessionSocket &sso = srv.srv_assoSessions[iClient];

  // Check if someone's connecting with too many players
  if (iClient > 0 && INetwork::CountClientPlayers(iClient) >= sso.sso_ctLocalPlayers) {
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("Protocol violation"), FALSE);
    return;
  }

  // If there's a used character already
  if (INetwork::IsCharacterUsed(pcCharacter)) {
    // Refuse connection
    CTString strMessage;
    strMessage.PrintF(LOCALIZE("Player character '%s' already exists in this session."), pcCharacter.GetName());

    INetwork::SendDisconnectMessage(iClient, strMessage, FALSE);
    return;
  }

  // Find inactive player for the client
  CPlayerBuffer *pplbNew = INetwork::FirstInactivePlayer();

  // If able to add new players
  if (pplbNew != NULL) {
    // Activate new player
    pplbNew->Activate(iClient);
    INDEX iNewPlayer = pplbNew->plb_Index;

    // Let plugins handle characters
    FOREACHPLUGIN(itPlugin) {
      if (itPlugin->pm_events.m_packet->OnCharacterConnect == NULL) continue;

      itPlugin->pm_events.m_packet->OnCharacterConnect(iClient, pcCharacter);
    }

    // Remember the character
    pplbNew->plb_pcCharacter = pcCharacter;

    const INDEX iLastSequence = ++srv.srv_iLastProcessedSequence;

  #if _PATCHCONFIG_GUID_MASKING
    if (ShouldMaskGUIDs()) {
      // Send message back to this client about adding a new player
      if (iClient == 0 || sso.sso_bActive) {
        CNetStreamBlock nsbClientPlayer(MSG_SEQ_ADDPLAYER, iLastSequence);
        nsbClientPlayer << iNewPlayer;
        nsbClientPlayer << pcCharacter;

        INetwork::AddBlockToSession(nsbClientPlayer, iClient);
      }

      // Mask player GUID for other clients
      MaskGUID(pcCharacter.pc_aubGUID, *pplbNew);
    }
  #endif // _PATCHCONFIG_GUID_MASKING

    // Send message to other clients about adding a new player
    CNetStreamBlock nsbAddClientData(MSG_SEQ_ADDPLAYER, iLastSequence);
    nsbAddClientData << iNewPlayer;
    nsbAddClientData << pcCharacter;

  #if _PATCHCONFIG_GUID_MASKING
    // Send to other clients
    if (ShouldMaskGUIDs()) {
      for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
        CSessionSocket &ssoBlock = srv.srv_assoSessions[iSession];

        if (iSession == iClient || (iSession > 0 && !ssoBlock.IsActive())) {
          continue;
        }

        INetwork::AddBlockToSession(nsbAddClientData, iSession);
      }

    } else
  #endif // _PATCHCONFIG_GUID_MASKING
    {
      // Send to everyone
      INetwork::AddBlockToAllSessions(nsbAddClientData);
    }

    // Don't wait for any more players
    _pShell->Execute("ser_bWaitFirstPlayer = 0;");

    // Reply to this client about adding a new player
    CNetworkMessage nmPlayerRegistered(MSG_REP_CONNECTPLAYER);
    nmPlayerRegistered << iNewPlayer;

    _pNetwork->SendToClientReliable(iClient, nmPlayerRegistered);

    // Add new player to the active client
    CActiveClient &acClient = _aActiveClients[iClient];
    ASSERT(acClient.IsActive() && acClient.pClient == pci);

    acClient.AddPlayer(pplbNew);

    // Notify master server that a player is connecting
    static CSymbolPtr symptr("ser_bEnumeration");

    if (symptr.GetIndex()) {
      IMasterServer::OnServerStateChanged();
    }

  // If too many players
  } else {
    // Refuse connection
    INetwork::SendDisconnectMessage(iClient, LOCALIZE("Too many players in session."), FALSE);
  }
};

// Client changing the character
void IProcessPacket::OnCharacterChangeRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Read character data
  INDEX iPlayer;
  CPlayerCharacter pcCharacter;
  nmMessage >> iPlayer >> pcCharacter;

  // Get client identity
  CClientIdentity *pci = IClientLogging::GetIdentity(iClient);

  // Add new character to the identity
  pci->AddNewCharacter(pcCharacter);

  // Skip character changes blocked by the anti-flood system
  if (IAntiFlood::HandleCharacterChange(iClient)) {
    return;
  }

  CServer &srv = _pNetwork->ga_srvServer;

  // Invalid player
  if (iPlayer < 0 || iPlayer > srv.srv_aplbPlayers.Count() ) {
    return;
  }

  CPlayerBuffer &plb = srv.srv_aplbPlayers[iPlayer];

  // Wrong client or character
  if (plb.plb_iClient != iClient || !(plb.plb_pcCharacter == pcCharacter)) {
    return;
  }

  // [Cecil] Check if the entity is even capable of changing its appearance
  CPlayerEntity *penPlayer = _pNetwork->ga_sesSessionState.ses_apltPlayers[iPlayer].plt_penPlayerEntity;

  if (!CanChangeCharacter(penPlayer)) {
    INetwork::SendChatToClient(iClient, "Server", TRANS("Please wait until you are fully connected to change your character."));
    return;
  }

  // Let plugins handle characters
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_packet->OnCharacterChange == NULL) continue;

    // Quit if cannot change the character
    if (!itPlugin->pm_events.m_packet->OnCharacterChange(iClient, iPlayer, pcCharacter)) {
      return;
    }
  }

  // Remember the character
  plb.plb_pcCharacter = pcCharacter;

  const INDEX iLastSequence = ++srv.srv_iLastProcessedSequence;

#if _PATCHCONFIG_GUID_MASKING
  if (ShouldMaskGUIDs()) {
    // Send character change back to this client
    if (iClient == 0 || srv.srv_assoSessions[iClient].sso_bActive) {
      CNetStreamBlock nsbClientChar(MSG_SEQ_CHARACTERCHANGE, iLastSequence);
      nsbClientChar << iPlayer;
      nsbClientChar << pcCharacter;

      INetwork::AddBlockToSession(nsbClientChar, iClient);
    }

    // Mask player GUID for other clients
    MaskGUID(pcCharacter.pc_aubGUID, plb);
  }
#endif // _PATCHCONFIG_GUID_MASKING

  // Send character change to other clients
  CNetStreamBlock nsbChangeChar(MSG_SEQ_CHARACTERCHANGE, iLastSequence);
  nsbChangeChar << iPlayer;
  nsbChangeChar << pcCharacter;

#if _PATCHCONFIG_GUID_MASKING
  // Send to other clients
  if (ShouldMaskGUIDs()) {
    for (INDEX iSession = 0; iSession < srv.srv_assoSessions.Count(); iSession++) {
      CSessionSocket &ssoBlock = srv.srv_assoSessions[iSession];

      if (iSession == iClient || (iSession > 0 && !ssoBlock.IsActive())) {
        continue;
      }

      INetwork::AddBlockToSession(nsbChangeChar, iSession);
    }

  } else
#endif // _PATCHCONFIG_GUID_MASKING
  {
    // Send to everyone
    INetwork::AddBlockToAllSessions(nsbChangeChar);
  }
};

// Receive action packet from one player of a client
static void ReceiveActionsForPlayer(INDEX iClient, INDEX iPlayer, CNetworkMessage &nm, INDEX iMaxBuffer)
{
  CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[iPlayer];
  ASSERT(plb.plb_Active);

  // Receive new action
  CPlayerAction pa;
  nm >> pa;

  // Let plugins handle actions
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_packet->OnPlayerAction == NULL) continue;

    itPlugin->pm_events.m_packet->OnPlayerAction(iClient, iPlayer, pa, -1);
  }

  // Buffer it
  plb.plb_abReceived.AddAction(pa);

  INDEX iSendBehind = 0;
  nm.ReadBits(&iSendBehind, 2);

  // Add resent actions
  for (INDEX i = 0; i < iSendBehind; i++) {
    CPlayerAction paOld;
    nm >> paOld;

    // Let plugins handle actions
    FOREACHPLUGIN(itPlugin) {
      if (itPlugin->pm_events.m_packet->OnPlayerAction == NULL) continue;

      itPlugin->pm_events.m_packet->OnPlayerAction(iClient, iPlayer, pa, i);
    }

    if (paOld.pa_llCreated > plb.plb_paLastAction.pa_llCreated) {
      plb.plb_abReceived.AddAction(paOld);
    }
  }

  // If there are too many actions buffered
  while (plb.plb_abReceived.GetCount() > iMaxBuffer) {
    // Purge the oldest one
    plb.plb_abReceived.RemoveOldest();
  }
};

// Client sending player actions
void IProcessPacket::OnPlayerAction(INDEX iClient, CNetworkMessage &nmMessage)
{
  CServer &srv = _pNetwork->ga_srvServer;
  CSessionSocket &sso = srv.srv_assoSessions[iClient];

  // For each possible player on that client
  for (INDEX i = 0; i < ICore::MAX_LOCAL_PLAYERS; i++) {
    // [Cecil] Stop reading actions if there are less than 4 bytes of data left
    // 15 bits / 2 bytes for bSaved, iPlayer and plb_iPing and then CPlayerAction (more than 2 extra bytes)
    const INDEX ctBytesLeft = nmMessage.nm_slSize - INDEX(nmMessage.nm_pubPointer - nmMessage.nm_pubMessage);
    if (ctBytesLeft < 4) break;

    // See if saved in the message
    BOOL bSaved = 0;
    nmMessage.ReadBits(&bSaved, 1);

    if (!bSaved) continue;

    // Read client index
    INDEX iPlayer = 0;
    nmMessage.ReadBits(&iPlayer, 4);

    CPlayerBuffer &plb = srv.srv_aplbPlayers[iPlayer];

    // If there is no player on that client
    if (plb.plb_iClient != iClient) {
      // Consider the entire message invalid
      CPrintF("Wrong Client! (expected: %d, got: %d from srv_aplbPlayers[%d])\n", iClient, plb.plb_iClient, iPlayer);
      break;
    }

    // Read ping
    plb.plb_iPing = 0;
    nmMessage.ReadBits(&plb.plb_iPing, 10);

    // Let the corresponding client buffer receive the message
    INDEX iMaxBuffer = sso.sso_sspParams.ssp_iBufferActions;

    static CSymbolPtr symptr("cli_bPredictIfServer");

    if (iClient == 0 && !symptr.GetIndex()) {
      iMaxBuffer = 1;
    }

    ReceiveActionsForPlayer(iClient, iPlayer, nmMessage, iMaxBuffer);
  }
};

// Client sends a CRC check
void IProcessPacket::OnSyncCheck(INDEX iClient, CNetworkMessage &nmMessage) {
  static CSymbolPtr pbReportSyncOK("ser_bReportSyncOK");
  static CSymbolPtr pbReportSyncBad("ser_bReportSyncBad");
  static CSymbolPtr pbReportSyncLate("ser_bReportSyncLate");
  static CSymbolPtr pbReportSyncEarly("ser_bReportSyncEarly");
  static CSymbolPtr pbPauseOnSyncBad("ser_bPauseOnSyncBad");
  static CSymbolPtr piKickOnSyncBad("ser_iKickOnSyncBad");

  CServer &srv = _pNetwork->ga_srvServer;

  // Read sync check from the packet
  CSyncCheck scRemote;
  nmMessage.Read(&scRemote, sizeof(scRemote));

  const TIME tmTick = scRemote.sc_tmTick;

  // Try to find it in the buffer
  CSyncCheck scLocal;
  INDEX iFound = FindSyncCheck(iClient, tmTick, scLocal);

  CSessionSocket &sso = srv.srv_assoSessions[iClient];
  TIME &tmLastSync = sso.sso_tmLastSyncReceived;

  const CTString strClientName = GetComm().Server_GetClientName(iClient);

  // Sync on time
  if (iFound == 0) {
    // Flush stream buffer up to that sequence
    ((CNetStream &)sso.sso_nsBuffer).RemoveOlderBlocksBySequence(scRemote.sc_iSequence);

    // Disconnect if the level has changed
    if (scLocal.sc_iLevel != scRemote.sc_iLevel) {
      INetwork::SendDisconnectMessage(iClient, LOCALIZE("Level change in progress. Please retry."), FALSE);

    // Wrong CRC
    } else if (scLocal.sc_ulCRC != scRemote.sc_ulCRC) {
      sso.sso_ctBadSyncs++;

      if (pbReportSyncBad.GetIndex()) {
        CTString strSyncBad;
        strSyncBad.PrintF(LOCALIZE("SYNCBAD: Client '%s', Sequence %d Tick %.2f - bad %d\n"),
          strClientName, scRemote.sc_iSequence, tmTick, sso.sso_ctBadSyncs);

        CPutString(strSyncBad);

        // [Cecil] Report bad syncs to the client
        if (_bReportSyncBadToClients) {
          strSyncBad.PrintF(TRANS("SYNCBAD: Sequence %d Tick %.2f - bad %d"),
            scRemote.sc_iSequence, tmTick, sso.sso_ctBadSyncs);

          INetwork::SendChatToClient(iClient, "Server", strSyncBad);
        }
      }

      // Kick from too many bad sync
      if (piKickOnSyncBad.GetIndex() > 0) {
        if (sso.sso_ctBadSyncs >= piKickOnSyncBad.GetIndex()) {
          INetwork::SendDisconnectMessage(iClient, LOCALIZE("Too many bad syncs"), FALSE);
        }

      // Pause on any bad sync
      } else if (pbPauseOnSyncBad.GetIndex()) {
        _pNetwork->ga_sesSessionState.ses_bWantPause = TRUE;
      }

    // Clear bad syncs
    } else {
      sso.sso_ctBadSyncs = 0;

      if (pbReportSyncOK.GetIndex()) {
        CPrintF(LOCALIZE("SYNCOK: Client '%s', Tick %.2f\n"), strClientName, tmTick);
      }
    }

    // Remember that this sync is for this tick
    if (tmLastSync < tmTick) tmLastSync = tmTick;

  // Too old
  } else if (iFound < 0) {
    // Only report if syncs are okay (to avoid late syncs on level change)
    if (pbReportSyncLate.GetIndex() && tmLastSync > 0) {
      CPrintF(LOCALIZE("SYNCLATE: Client '%s', Tick %.2f\n"), strClientName, tmTick);
    }

  // Too new
  } else {
    if (pbReportSyncEarly.GetIndex()) {
      CPrintF(LOCALIZE("SYNCEARLY: Client '%s', Tick %.2f\n"), strClientName, tmTick);
    }

    // Remember that this sync was ahead of time
    if (tmLastSync < tmTick) tmLastSync = tmTick;
  }
};

// Client sending a chat message
BOOL IProcessPacket::OnChatInRequest(INDEX iClient, CNetworkMessage &nmMessage)
{
  // Skip messages blocked by the anti-flood system
  if (IAntiFlood::HandleChatMessage(iClient)) {
    return FALSE;
  }

  ULONG ulFrom, ulTo;
  CTString strMessage;

  nmMessage >> ulFrom >> ulTo >> strMessage;
  nmMessage.Rewind();

  // Let plugins handle chat messages
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_packet->OnChatMessage == NULL) continue;

    // Quit if it's not a regular chat message
    if (!itPlugin->pm_events.m_packet->OnChatMessage(iClient, ulFrom, ulTo, strMessage)) {
      return FALSE;
    }
  }

  // Handle chat command if the message starts with a command prefix
  if (strMessage.HasPrefix(ser_strCommandPrefix)) {
    return HandleChatCommand(iClient, strMessage);
  }

  return TRUE;
};
