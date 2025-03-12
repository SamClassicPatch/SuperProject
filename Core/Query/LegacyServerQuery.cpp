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

#if _PATCHCONFIG_NEW_QUERY

#include "QueryManager.h"
#include "Networking/NetworkFunctions.h"

extern unsigned char *gsseckey(u_char *secure, u_char *key, int enctype);

// Status response for formatting
static const char *_strStatusResponseFormat =
  "\\gamename\\%s\\gamever\\%s\\location\\%s\\hostname\\%s\\hostport\\%hu"
  "\\mapname\\%s\\gametype\\%s\\activemod\\"
  "\\numplayers\\%d\\maxplayers\\%d\\gamemode\\openplaying\\difficulty\\Normal"
  "\\friendlyfire\\%d\\weaponsstay\\%d\\ammosstay\\%d"
  "\\healthandarmorstays\\%d\\allowhealth\\%d\\allowarmor\\%d\\infiniteammo\\%d\\respawninplace\\%d"
  "\\password\\0\\vipplayers\\1";

void ILegacy::BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge) {
  strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_piNetPort.GetIndex() + 1), SAM_MS_NAME);
};

void ILegacy::ServerParsePacket(INDEX iLength) {
  // End with a null terminator
  IQuery::pBuffer[iLength] = '\0';

  // String of data
  const char *strData = IQuery::pBuffer;

  // Check for packet types
  const char *pStatus  = strstr(strData, "\\status\\");
  const char *pInfo    = strstr(strData, "\\info\\");
  const char *pBasic   = strstr(strData, "\\basic\\");
  const char *pPlayers = strstr(strData, "\\players\\");
  const char *pSecure  = strstr(strData, "\\secure\\");

  if (ms_bDebugOutput) {
    CPrintF("Received data (%d bytes):\n%s\n", iLength, IQuery::pBuffer);
  }

  // Player count
  const INDEX ctPlayers = INetwork::CountPlayers(FALSE);
  const INDEX ctMaxPlayers = _pNetwork->ga_sesSessionState.ses_ctMaxPlayers;

  // Status request
  if (pStatus != NULL) {
    // Get location
    CTString strLocation;
    strLocation = _pstrLocalHost.GetString();

    if (strLocation == "") {
      strLocation = "Heartland";
    }

    // Retrieve symbols once
    static CSymbolPtr symptrFF("gam_bFriendlyFire");
    static CSymbolPtr symptrWeap("gam_bWeaponsStay");
    static CSymbolPtr symptrAmmo("gam_bAmmoStays");
    static CSymbolPtr symptrVital("gam_bHealthArmorStays");
    static CSymbolPtr symptrHP("gam_bAllowHealth");
    static CSymbolPtr symptrAR("gam_bAllowArmor");
    static CSymbolPtr symptrIA("gam_bInfiniteAmmo");
    static CSymbolPtr symptrResp("gam_bRespawnInPlace");

    // Compose status response
    CTString strPacket;
    strPacket.PrintF(_strStatusResponseFormat,
      sam_strGameName, _SE_VER_STRING, strLocation, GetGameAPI()->SessionName(), _piNetPort.GetIndex(),
      IWorld::GetWorld()->wo_strName, GetGameAPI()->GetCurrentGameTypeNameSS(),
      ctPlayers, ctMaxPlayers, symptrFF.GetIndex(), symptrWeap.GetIndex(), symptrAmmo.GetIndex(),
      symptrVital.GetIndex(), symptrHP.GetIndex(), symptrAR.GetIndex(), symptrIA.GetIndex(), symptrResp.GetIndex());

    // Go through server players
    for (INDEX i = 0; i < ctPlayers; i++) {
      CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
      CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];

      if (plt.plt_bActive) {
        // Get info about an individual player
        CTString strPlayer;
        plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

        // If not enough space for the next player info
        if (strPacket.Length() + strPlayer.Length() > 2048) {
          // Send existing packet and reset it
          IQuery::SendReply(strPacket);
          strPacket = "";
        }

        // Append player info
        strPacket += strPlayer;
      }
    }

    // Send the packet
    strPacket += "\\final\\\\queryid\\333.1";
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending status answer:\n%s\n", strPacket);
    }

  // Information request
  } else if (pInfo != NULL) {
    // Send information response
    CTString strPacket;
    strPacket.PrintF("\\hostname\\%s\\hostport\\%hu\\mapname\\%s\\gametype\\%s"
      "\\numplayers\\%d\\maxplayers\\%d\\gamemode\\openplaying\\final\\"
      "\\queryid\\8.1",
      GetGameAPI()->SessionName(), _piNetPort.GetIndex(),
      IWorld::GetWorld()->wo_strName, GetGameAPI()->GetCurrentGameTypeNameSS(),
      ctPlayers, ctMaxPlayers);

    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending info answer:\n%s\n", strPacket);
    }

  // Basic request
  } else if (pBasic != NULL) {
    // Get location
    CTString strLocation;
    strLocation = _pstrLocalHost.GetString();

    if (strLocation == "") {
      strLocation = "Heartland";
    }

    // Send basic response
    CTString strPacket;
    strPacket.PrintF("\\gamename\\%s\\gamever\\%s\\location\\EU\\final\\" "\\queryid\\1.1",
      sam_strGameName, _SE_VER_STRING, strLocation); // [Cecil] NOTE: Unused location

    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending basic answer:\n%s\n", strPacket);
    }

  // Player status request
  } else if (pPlayers != NULL) {
    // Compose player status response
    CTString strPacket;
    strPacket = "";

    // Go through server players
    for (INDEX i = 0; i < ctPlayers; i++) {
      CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
      CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];

      if (plt.plt_bActive) {
        // Get info about an individual player
        CTString strPlayer;
        plt.plt_penPlayerEntity->GetGameSpyPlayerInfo(plb.plb_Index, strPlayer);

        // If not enough space for the next player info
        if (strPacket.Length() + strPlayer.Length() > 2048) {
          // Send existing packet and reset it
          IQuery::SendReply(strPacket);
          strPacket = "";
        }

        // Append player info
        strPacket += strPlayer;
      }
    }

    // Send the packet
    strPacket += "\\final\\\\queryid\\6.1";
    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending players answer:\n%s\n", strPacket);
    }

  // Validation request
  } else if (pSecure != NULL) {
    UBYTE *pValidateKey = gsseckey((UBYTE *)(strData + 8), (UBYTE *)SAM_MS_KEY, 0);

    // Send validation response
    CTString strPacket;
    strPacket.PrintF("\\validate\\%s\\final\\" "\\queryid\\2.1", pValidateKey);

    IQuery::SendReply(strPacket);

    if (ms_bDebugOutput) {
      CPrintF("Sending validation answer:\n%s\n", strPacket);
    }

  // Unknown request
  } else if (ms_bDebugOutput) {
    CPrintF("Unknown query server request!\n"
            "Data (%d bytes): %s\n", iLength, strData);
  }
};

#endif // _PATCHCONFIG_NEW_QUERY
