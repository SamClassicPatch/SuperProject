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

#include "Networking/NetworkFunctions.h"
#include "Networking/Modules/StockCommands.h"
#include "Networking/Modules/VotingSystem.h"
#include "Networking/Modules/ClientLogging.h"

#include <STLIncludesBegin.h>
#include <map>
#include <STLIncludesEnd.h>

// Prefix that the chat commands start with
CTString ser_strCommandPrefix = "!";

// Passwords for authorizing as administrator and operator
CTString ser_strAdminPassword = "";
CTString ser_strOperatorPassword = "";

// Chat command function holder
struct ChatCommand_t {
  CTString strArgumentList;
  CTString strDescription;
  EChatCommandAccessLevel eAccess;

  BOOL bPure;
  BOOL bHidden; // Whether to hide the command from the help

  union {
    FEngineChatCommand pEngineHandler;
    FPureChatCommand pPureHandler;
  };

  ChatCommand_t() : eAccess(k_EChatCommandAccessLevel_Everyone), bPure(FALSE), bHidden(FALSE), pEngineHandler(NULL) {};

  inline bool operator==(const ChatCommand_t &other) const {
    return bPure == other.bPure && pEngineHandler == other.pEngineHandler;
  };
};

// List of chat commands
typedef se1::map<CTString, ChatCommand_t> CChatCommands;
static CChatCommands _mapChatCommands;

// Extract command name from the string
static INDEX ExtractCommand(CTString &strCommand) {
  const INDEX ct = strCommand.Length();

  // Parse until the end of the string
  INDEX i;

  for (i = 0; i < ct; i++) {
    char ch = strCommand[i];

    // Anything before and including space is a delimiter
    if (ch <= ' ') break;
  }

  // Same length
  if (i == ct) {
    return -1;
  }

  // Cut off the string at the last parsed character
  strCommand.TrimRight(i);
  return i;
};

// Interface for chat commands
BOOL HandleChatCommand(INDEX iClient, const CTString &strCommand)
{
  // Copy full command for extracting arguments
  CTString strArguments = strCommand;

  // Try to remove the command prefix
  if (!strArguments.RemovePrefix(ser_strCommandPrefix)) {
    return TRUE;
  }

  // Extract command name
  CTString strCommandName = strArguments;
  INDEX iCutOff = ExtractCommand(strCommandName);

  // Remove command name from the string
  if (iCutOff != -1) {
    strArguments.RemovePrefix(strCommandName);
    strArguments.TrimSpacesLeft();

  // Nothing left
  } else {
    strArguments = "";
  }

  // Find desired command
  CChatCommands::const_iterator it = _mapChatCommands.find(strCommandName);

  if (it != _mapChatCommands.end()) {
    const ChatCommand_t &com = it->second;

    // Process as a normal chat message if a non-operator tries executing an operator command
    if (!CActiveClient::IsOperator(iClient) && com.eAccess == k_EChatCommandAccessLevel_Operator) {
      return TRUE;
    }

    // Process as a normal chat message if a non-admin tries executing an admin command
    if (!CActiveClient::IsAdmin(iClient) && com.eAccess == k_EChatCommandAccessLevel_Admin) {
      return TRUE;
    }

    // Execute it
    CTString strOut = "";
    BOOL bHandled;

    if (com.bPure) {
      ChatCommandResultStr strBufferOut = { 0 };
      bHandled = com.pPureHandler(strBufferOut, iClient, strArguments.str_String);
      strOut = strBufferOut;

    } else {
      bHandled = com.pEngineHandler(strOut, iClient, strArguments);
    }

    // Process as a normal chat message upon failure
    if (!bHandled) {
      return TRUE;
    }

    // Reply to the client with the inputted command
    const CTString strReply = strCommand + "\n" + strOut;
    INetwork::SendChatToClient(iClient, "Chat command", strReply);

    // Don't process as a chat message
    return FALSE;
  }

  return TRUE;
};

// Map of references to chat commands that need to be listed stored in the alphabetical order
typedef std::map<CTString, const ChatCommand_t *> CListedCommands;

namespace std {
  // Sort strings alphabetically
  template<>
  struct less<CTString> : binary_function<CTString, CTString, bool> {
    bool operator()(const CTString &str1, const CTString &str2) const {
      return strcmp(str1, str2) < 0;
    };
  };
};

// List available chat commands
BOOL IStockCommands::ListCommands(CTString &strResult, INDEX iClient, const CTString &strArguments) {
  // Gather available commands for a specific client
  CListedCommands mapList;
  CChatCommands::const_iterator it;

  INDEX ctCommands = 0;

  for (it = _mapChatCommands.begin(); it != _mapChatCommands.end(); it++) {
    const ChatCommand_t &con = it->second;

    // Don't list higher access commands for clients without elevated privileges
    if (!CActiveClient::IsOperator(iClient) && con.eAccess == k_EChatCommandAccessLevel_Operator) continue;
    if (!CActiveClient::IsAdmin(iClient) && con.eAccess == k_EChatCommandAccessLevel_Admin) continue;

    // Don't list hidden commands for non-admins
    if (!CActiveClient::IsAdmin(iClient) && con.bHidden) continue;

    mapList[it->first] = &con;
    ctCommands++;
  }

  if (ctCommands == 0) {
    strResult = TRANS("No chat commands available");
    return TRUE;
  }

  // How many commands to list per page
  INDEX ctPerPage = 8;

  const INDEX ctPages = ceil((FLOAT)ctCommands / (FLOAT)ctPerPage);

  // Determine the current page
  INDEX iPage = 1;

  if (((CTString &)strArguments).ScanF("%d", &iPage) >= 1) {
    iPage = Clamp(iPage, (INDEX)1, ctPages);
  }

  strResult.PrintF(TRANS("^cffffffAvailable commands (page %d/%d):"), iPage, ctPages);
  strResult += "\n^cffffff--------------------------------";

  CListedCommands::const_iterator itList = mapList.begin();
  std::advance(itList, (iPage - 1) * ctPerPage);

  while (--ctPerPage >= 0 && itList != mapList.end()) {
    // Append argument list
    CTString strArgs = "";

    if (itList->second->strArgumentList != "") {
      strArgs = itList->second->strArgumentList;
      strArgs.TrimSpacesLeft();
      strArgs = " " + strArgs;
    }

    strResult += CTString(0, "\n  ^cffdf00%s%s%s^r - %s", ser_strCommandPrefix.str_String, itList->first.str_String,
      strArgs.str_String, itList->second->strDescription);

    itList++;
  }

  return TRUE;
};

void ClassicsChat_RegisterCommand(const char *strName, FEngineChatCommand pFunction)
{
  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = FALSE;
  com.pEngineHandler = pFunction;

  com.strArgumentList = "";
  com.strDescription = TRANS("No description");
  com.eAccess = k_EChatCommandAccessLevel_Everyone;
};

void ClassicsChat_RegisterCommandPure(const char *strName, FPureChatCommand pFunction)
{
  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = TRUE;
  com.pPureHandler = pFunction;

  com.strArgumentList = "";
  com.strDescription = TRANS("No description");
  com.eAccess = k_EChatCommandAccessLevel_Everyone;
};

void ClassicsChat_UnregisterCommand(const char *strName)
{
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return;

  _mapChatCommands.remove(*itCommand);
};

BOOL ClassicsChat_SetCommandAccess(const char *strName, EChatCommandAccessLevel eAccess, BOOL bHidden)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return FALSE;

  itCommand->second.eAccess = eAccess;
  itCommand->second.bHidden = bHidden;
  return TRUE;
};

BOOL ClassicsChat_SetCommandInfo(const char *strName, const char *strArgumentList, const char *strDescription)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return FALSE;

  itCommand->second.strArgumentList = strArgumentList;
  itCommand->second.strDescription = strDescription;
  return TRUE;
};

extern void PrintClientLog(CTString &strResult, INDEX iIdentity, INDEX iCharacter);

// Interface initialization
namespace IInitAPI {

// Output log of a specific identity and optionally a character
static void ClientLogInConsole(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  INDEX iCharacter = NEXT_ARG(INDEX);

  CTString strLog;
  PrintClientLog(strLog, iIdentity, iCharacter);

  CPutString(strLog + "\n");
};

// Delete specific character or an identity as a whole from the log
static void ClientLogDelete(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  INDEX iCharacter = NEXT_ARG(INDEX);

  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    CPutString("Invalid client index!\n");
    return;
  }

  CClientIdentity &ci = _aClientIdentities[iIdentity];

  // Delete entire identity
  if (iCharacter == -1) {
    _aClientIdentities.Delete(&ci);
    return;
  }

  if (iCharacter <= 0 || iCharacter > ci.aCharacters.Count()) {
    CPutString("Invalid character index!\n");
    return;
  }

  // Delete character
  CPlayerCharacter &pc = ci.aCharacters[iCharacter - 1];
  ci.aCharacters.Delete(&pc);
};

// Resave client log
static void ClientLogSave(void) {
  IClientLogging::SaveLog();
};

// Reload client log
static void ClientLogLoad(void) {
  _aClientIdentities.Clear();
  IClientLogging::LoadLog();
};

void Chat(void) {
  _pShell->DeclareSymbol("persistent user CTString ser_strCommandPrefix;", &ser_strCommandPrefix);
  _pShell->DeclareSymbol("user CTString ser_strAdminPassword;", &ser_strAdminPassword);
  _pShell->DeclareSymbol("user CTString ser_strOperatorPassword;", &ser_strOperatorPassword);

  // Register default chat commands
  ClassicsChat_RegisterCommand("help", &IStockCommands::ListCommands);
  ClassicsChat_SetCommandInfo("help", "[page]", TRANS("Display a list of available commands on a specific page"));

  ClassicsChat_RegisterCommand("map", &IStockCommands::CurrentMap);
  ClassicsChat_SetCommandInfo("map", "", TRANS("Display the name of the current level and its world file"));

  ClassicsChat_RegisterCommand("login", &IStockCommands::PasswordLogin);
  ClassicsChat_SetCommandInfo("login", "<password>", TRANS("Log in as a server administrator, or as a server operator"));

  ClassicsChat_RegisterCommand("rcon", &IStockCommands::RemoteConsole);
  ClassicsChat_SetCommandInfo("rcon", "<command>", TRANS("Execute any server-side console commands"));
  ClassicsChat_SetCommandAccess("rcon", k_EChatCommandAccessLevel_Operator, FALSE);

  ClassicsChat_RegisterCommand("save", &IStockCommands::RemoteSave);
  ClassicsChat_SetCommandInfo("save", "[filename]", TRANS("Save the current game on the same computer where the dedicated server is running"));
  ClassicsChat_SetCommandAccess("save", k_EChatCommandAccessLevel_Operator, FALSE);

  ClassicsChat_RegisterCommand("log", &IStockCommands::ClientLog);
  ClassicsChat_SetCommandInfo("log", "[client] [character]", TRANS("Display a log of clients that have ever connected to the server"));
  ClassicsChat_SetCommandAccess("log", k_EChatCommandAccessLevel_Admin, FALSE);

  ClassicsChat_RegisterCommand("ban", &IStockCommands::BanClient);
  ClassicsChat_SetCommandInfo("ban", "<client> [time]", TRANS("Ban all clients under a specific identity index for a specific amount of time"));
  ClassicsChat_SetCommandAccess("ban", k_EChatCommandAccessLevel_Admin, FALSE);

  ClassicsChat_RegisterCommand("mute", &IStockCommands::MuteClient);
  ClassicsChat_SetCommandInfo("mute", "<client> [time]", TRANS("Mute all clients under a specific identity index for a specific amount of time"));
  ClassicsChat_SetCommandAccess("mute", k_EChatCommandAccessLevel_Admin, FALSE);

  ClassicsChat_RegisterCommand("kick", &IStockCommands::KickClient);
  ClassicsChat_SetCommandInfo("kick", "<client> [reason]", TRANS("Kick all clients under a specific identity index, with an optional reason"));
  ClassicsChat_SetCommandAccess("kick", k_EChatCommandAccessLevel_Admin, FALSE);

  // Voting
  if (ClassicsCore_IsServerApp()) {
    ClassicsChat_RegisterCommand("voteskip", &IVotingSystem::Chat::VoteSkip);
    ClassicsChat_SetCommandInfo("voteskip", "", TRANS("Initiate a voting process to skip the current round on a dedicated server"));
  }

  ClassicsChat_RegisterCommand("votemap", &IVotingSystem::Chat::VoteMap);
  ClassicsChat_SetCommandInfo("votemap", "[map index]", TRANS("Initiate a voting process to change the current map with another one from the current map pool"));

  ClassicsChat_RegisterCommand("votekick", &IVotingSystem::Chat::VoteKick);
  ClassicsChat_SetCommandInfo("votekick", "[client]", TRANS("Initiate a voting process to kick a specific client under a specific index"));

  ClassicsChat_RegisterCommand("votemute", &IVotingSystem::Chat::VoteMute);
  ClassicsChat_SetCommandInfo("votemute", "[client]", TRANS("Initiate a voting process to mute a specific client under a specific index"));

  ClassicsChat_RegisterCommand("y", &IVotingSystem::Chat::VoteYes);
  ClassicsChat_SetCommandInfo("y", "", TRANS("Vote \"yes\" during active voting"));

  ClassicsChat_RegisterCommand("n", &IVotingSystem::Chat::VoteNo);
  ClassicsChat_SetCommandInfo("n", "", TRANS("Vote \"no\" during active voting"));

  // Local interaction with the client log
  _pShell->DeclareSymbol("user void ClientLog(INDEX, INDEX);", &ClientLogInConsole);
  _pShell->DeclareSymbol("user void ClientLogDelete(INDEX, INDEX);", &ClientLogDelete);
  _pShell->DeclareSymbol("user void ClientLogSave(void);", &ClientLogSave);
  _pShell->DeclareSymbol("user void ClientLogLoad(void);", &ClientLogLoad);
};

}; // namespace
