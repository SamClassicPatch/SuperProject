/* Copyright (c) 2022-2025 Dreamy Cecil
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

#include "StockCommands.h"
#include "Networking/NetworkFunctions.h"
#include "ClientLogging.h"

// Parse command as a normal chat message for clients without administrator rights
#define IGNORE_CLIENTS { if (!CActiveClient::IsAdmin(iClient)) return FALSE; }

// Display name of the current map
BOOL IStockCommands::CurrentMap(CTString &strResult, INDEX, const CTString &) {
  CWorld &wo = *IWorld::GetWorld();
  strResult.PrintF("Current map: \"%s^r\"\n- %s", wo.wo_strName, wo.wo_fnmFileName.str_String);

  return TRUE;
};

// Log in as administrator
BOOL IStockCommands::PasswordLogin(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  // Server client is always an operator
  if (GetComm().Server_IsClientLocal(iClient)) {
    strResult = TRANS("You are already a server operator!");
    return TRUE;
  }

  // Matches operator password
  if (ser_strOperatorPassword != "" && ser_strOperatorPassword == strArguments) {
    _aActiveClients[iClient].eRole = CActiveClient::E_OPERATOR;
    strResult = TRANS("Successfully authorized as a server operator!");

  // Matches admin password
  } else if (ser_strAdminPassword != "" && ser_strAdminPassword == strArguments) {
    _aActiveClients[iClient].eRole = CActiveClient::E_ADMIN;
    strResult = TRANS("Successfully authorized as a server administrator!");

  // No passwords set or they mismatch
  } else {
    strResult = TRANS("Password is incorrect! Login attempt was logged.");
  }

  // Log attempts from non-server clients
  if (!GetComm().Server_IsClientLocal(iClient)) {
    INDEX iIdentity = _aClientIdentities.Index(_aActiveClients[iClient].pClient);

    CPrintF(TRANS("Client '%s' (identity %d) is attempting to authorize as an admin with '%s':\n"),
            GetComm().Server_GetClientName(iClient), iIdentity, strArguments);
    CPrintF("  %s\n", strResult);
  }

  return TRUE;
};

// Remote console command
BOOL IStockCommands::RemoteConsole(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  const CActiveClient &ac = _aActiveClients[iClient];

  const CTString strClientName = GetComm().Server_GetClientName(iClient);
  const INDEX iIdentity = _aClientIdentities.Index(ac.pClient);

  if (!GetComm().Server_IsClientLocal(iClient)) {
    // Not a server operator
    if (ac.eRole != CActiveClient::E_OPERATOR) {
      CPrintF(TRANS("Client '%s' (identity %d) is trying to execute a command:\n"), strClientName, iIdentity);
      CPrintF("> %s\n", strArguments);

      strResult = TRANS("You are not a server operator!");
      return TRUE;
    }

    // Log commands from non-server clients
    CPrintF(TRANS("Client '%s' (identity %d) is executing a command:\n"), strClientName, iIdentity);
    CPrintF("> %s\n", strArguments);
  }

  _pShell->Execute(strArguments + ";");
  strResult = TRANS("See server log for the command output.");

  return TRUE;
};

// Save game remotely
BOOL IStockCommands::RemoteSave(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  const CActiveClient &ac = _aActiveClients[iClient];

  // Not a server operator
  if (!GetComm().Server_IsClientLocal(iClient)
   && _aActiveClients[iClient].eRole != CActiveClient::E_OPERATOR)
  {
    strResult = TRANS("You are not a server operator!");
    return TRUE;
  }

  CTString strSaveFile = "SaveGame\\Network\\";

  if (strArguments == "") {
    strSaveFile += "RemoteSave.sav";
  } else {
    strSaveFile += strArguments + ".sav";
  }

  _pNetwork->Save_t(strSaveFile);
  strResult.PrintF(TRANS("Saved game: %s"), strSaveFile);

  return TRUE;
};

// Print client restrictions
static void PrintIdentityRestrictions(CClientIdentity &ci, CTString &strInfo) {
  // Check for a ban
  CClientRestriction *pcr = CClientRestriction::IsBanned(&ci);

  if (pcr != NULL) {
    CTString strTime;
    pcr->PrintBanTime(strTime);

    strInfo += CTString(0, "\n  ^caa0000BANNED for %s!", strTime);
  }

  // Check for a mute
  pcr = CClientRestriction::IsMuted(&ci);

  if (pcr != NULL) {
    CTString strTime;
    pcr->PrintMuteTime(strTime);

    strInfo += CTString(0, "\n  ^c7f7f7fMUTED for %s!", strTime);
  }
};

// Display minimal information about a specific identity
static void PrintIdentityInfoMinimal(CTString &strResult, INDEX iIdentity) {
  CClientIdentity &ci = _aClientIdentities[iIdentity];
  strResult += CTString(0, "\nClient %d: ", iIdentity);

  // Print relevant information
  CTString strInfo = "";

  // Check current activity
  CActiveClient::List cClients;
  CActiveClient::GetActiveClients(cClients, &ci);

  BOOL bNoActivePlayers = TRUE;

  // List all active characters
  FOREACHINDYNAMICCONTAINER(cClients, CActiveClient, itac) {
    FOREACHINDYNAMICCONTAINER(itac->cPlayers, CPlayerBuffer, itplb) {
      const CPlayerCharacter &pc = itplb->plb_pcCharacter;

      // Add color in the beginning and a comma later on
      if (bNoActivePlayers) {
        strInfo += "^cffffff";
      } else {
        strInfo += ", ";
      }
        
      // Names in a row
      strInfo += pc.GetName().Undecorated();
      bNoActivePlayers = FALSE;
    }
  }

  // No active players found
  if (bNoActivePlayers) {
    strInfo += "^caaaaaaOffline";
  }

  PrintIdentityRestrictions(ci, strInfo);

  // Display relevant information
  if (strInfo != "") {
    strResult += strInfo;
  }
};

// Display full information about a specific identity
static void PrintIdentityInfoFull(CTString &strResult, INDEX iIdentity, INDEX iCharacter) {
  CClientIdentity &ci = _aClientIdentities[iIdentity];
  iCharacter = ClampUp(iCharacter, ci.aCharacters.Count());

  if (iCharacter > 0) {
    strResult = CTString(0, "\nClient %d, Character %d: ", iIdentity, iCharacter);

    const CPlayerCharacter &pc = ci.aCharacters[iCharacter - 1];
    const UBYTE *pGUID = pc.pc_aubGUID;

    // GUID and name
    strResult += CTString(0, "\n GUID: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n Name: %s",
      pGUID[0], pGUID[1], pGUID[2], pGUID[3], pGUID[4], pGUID[5], pGUID[6], pGUID[7],
      pGUID[8], pGUID[9], pGUID[10], pGUID[11], pGUID[12], pGUID[13], pGUID[14], pGUID[15], pc.GetNameForPrinting());

    return;

  } else {
    // List addresses
    strResult = "\nAddresses:";

    for (INDEX iAddr = 0; iAddr < ci.aAddresses.Count(); iAddr++) {
      strResult += CTString(0, "\n%d %s", iAddr + 1, ci.aAddresses[iAddr].GetHost());
    }

    // List characters
    strResult += "\nCharacters:";

    for (INDEX iChar = 0; iChar < ci.aCharacters.Count(); iChar++) {
      const CPlayerCharacter &pc = ci.aCharacters[iChar];

      // Undecorated name
      strResult += CTString(0, "\n%d %s", iChar + 1, pc.GetName().Undecorated());
    }
  }

  // Print relevant information
  CTString strInfo = "";

  // Check current activity
  CActiveClient::List cClients;
  CActiveClient::GetActiveClients(cClients, &ci);

  BOOL bNoActivePlayers = TRUE;

  FOREACHINDYNAMICCONTAINER(cClients, CActiveClient, itac) {
    CDynamicContainer<CPlayerBuffer> &aPlayers = itac->cPlayers;
    strInfo += CTString(0, "\n^cffff00Active %d: %s\n ", itac.GetIndex(), itac->addr.GetHost());

    BOOL bNext = FALSE;

    // List active characters
    FOREACHINDYNAMICCONTAINER(aPlayers, CPlayerBuffer, itplb) {
      const CPlayerCharacter &pc = itplb->plb_pcCharacter;

      // Find index of this character
      for (INDEX iCompare = 0; iCompare < ci.aCharacters.Count(); iCompare++) {
        const CPlayerCharacter &pcCompare = ci.aCharacters[iCompare];

        if (pc == pcCompare) {
          if (bNext) strInfo += ", ";

          // Character indices in a row
          strInfo += CTString(0, "%d", iCompare + 1);
          bNext = TRUE;
          break;
        }
      }
    }
  }

  PrintIdentityRestrictions(ci, strInfo);

  // Display relevant information
  if (strInfo != "") {
    strResult += strInfo;
  }
};

// Print log of all clients
void PrintClientLog(CTString &strResult, INDEX iIdentity, INDEX iCharacter) {
  if (iIdentity >= _aClientIdentities.Count()) {
    strResult = "Invalid client index!";
    return;
  }

  // Print info about a specific identity (and optionally a specific character)
  if (iIdentity >= 0) {
    PrintIdentityInfoFull(strResult, iIdentity, iCharacter);
    return;
  }

  // Print info about all identities
  const INDEX ct = _aClientIdentities.Count();

  for (INDEX i = 0; i < ct; i++) {
    PrintIdentityInfoMinimal(strResult, i);
  }
};

// Display log of all clients
BOOL IStockCommands::ClientLog(CTString &strResult, INDEX iClient, const CTString &strArgs) {
  IGNORE_CLIENTS;

  // No clients
  if (_aClientIdentities.Count() == 0) {
    strResult = "No clients have been logged!";
    return TRUE;
  }

  // Display info about a specific identity and character
  INDEX iIdentity, iCharacter;
  INDEX iScan = const_cast<CTString &>(strArgs).ScanF("%d %d", &iIdentity, &iCharacter);

  if (iScan > 0) {
    if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
      strResult = "Invalid client index!";
      return TRUE;
    }

    // No character index scanned
    if (iScan == 1) {
      iCharacter = -1;
    }

  // No identity index scanned
  } else {
    iIdentity = -1;
    iCharacter = -1;
  }

  PrintClientLog(strResult, iIdentity, iCharacter);
  return TRUE;
};

// Parse arguments of a timed action aimed at some client
static CTString TimedClientAction(INDEX &iIdentity, FLOAT &fTime, CTString strArgs) {
  // Get identity index and time from the arguments
  INDEX iScan = strArgs.ScanF("%d %f", &iIdentity, &fTime);

  if (iScan < 1) {
    return "Couldn't parse the client index!";
  }

  // Invalid index
  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    return "Invalid client index!";
  }

  // Set default time (5 minutes)
  if (iScan < 2) {
    fTime = 300.0f;
  }

  return "";
};

// Parse arguments of a reasoned action aimed at some client
static CTString ReasonedClientAction(INDEX &iIdentity, CTString &strReason, CTString strArgs) {
  // Get identity index from the arguments
  if (strArgs.ScanF("%d", &iIdentity) < 1) {
    return "Couldn't parse the client index!";
  }

  // Invalid index
  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    return "Invalid client index!";
  }

  // Find first whitespace
  const ULONG ulWhitespace = IData::FindChar(strArgs, ' ');

  // If found
  if (ulWhitespace != -1) {
    // Remove everything before the whitespace and trim the rest
    strArgs.TrimLeft(strArgs.Length() - ulWhitespace);
    strArgs.TrimSpacesLeft();

    // Use first 100 characters as a reason
    strArgs.TrimRight(100);
    strReason = strArgs;

  } else {
    // Otherwise no reason
    strReason = "";
  }

  return "";
};

// Ban a specific client
BOOL IStockCommands::BanClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  FLOAT fTime;

  strResult = TimedClientAction(iIdentity, fTime, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Ban client
  strResult = CClientRestriction::BanClient(iIdentity, fTime);
  return TRUE;
};

// Mute a specific client
BOOL IStockCommands::MuteClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  FLOAT fTime;

  strResult = TimedClientAction(iIdentity, fTime, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Mute client
  strResult = CClientRestriction::MuteClient(iIdentity, fTime);
  return TRUE;
};

// Kick a specific client
BOOL IStockCommands::KickClient(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  IGNORE_CLIENTS;

  // Get arguments
  INDEX iIdentity;
  CTString strReason;

  strResult = ReasonedClientAction(iIdentity, strReason, strArguments);

  // Some error has occurred
  if (strResult != "") {
    return TRUE;
  }

  // Ban client
  strResult = CClientRestriction::KickClient(iIdentity, strReason);
  return TRUE;
};
