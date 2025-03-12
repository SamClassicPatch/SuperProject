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

#include "Networking/NetworkFunctions.h"
#include "Networking/Modules/StockCommands.h"
#include "Networking/Modules/VotingSystem.h"
#include "Networking/Modules/ClientLogging.h"

// Prefix that the chat commands start with
CTString ser_strCommandPrefix = "!";

// Passwords for authorizing as administrator and operator
CTString ser_strAdminPassword = "";
CTString ser_strOperatorPassword = "";

// Chat command function holder
struct ChatCommand_t {
  BOOL bPure;

  union {
    FEngineChatCommand pEngineHandler;
    FPureChatCommand pPureHandler;
  };

  ChatCommand_t() : bPure(FALSE), pEngineHandler(NULL) {};

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
    // Execute it
    CTString strOut = "";
    BOOL bHandled;

    if (it->second.bPure) {
      ChatCommandResultStr strBufferOut = { 0 };
      bHandled = it->second.pPureHandler(strBufferOut, iClient, strArguments.str_String);
      strOut = strBufferOut;

    } else {
      bHandled = it->second.pEngineHandler(strOut, iClient, strArguments);
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

void ClassicsChat_RegisterCommand(const char *strName, FEngineChatCommand pFunction)
{
  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = FALSE;
  com.pEngineHandler = pFunction;
};

void ClassicsChat_RegisterCommandPure(const char *strName, FPureChatCommand pFunction)
{
  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = TRUE;
  com.pPureHandler = pFunction;
};

void ClassicsChat_UnregisterCommand(const char *strName)
{
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);

  if (itCommand != _mapChatCommands.end()) {
    _mapChatCommands.remove(*itCommand);
  }
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
  ClassicsChat_RegisterCommand("map",   &IStockCommands::CurrentMap);
  ClassicsChat_RegisterCommand("login", &IStockCommands::PasswordLogin);
  ClassicsChat_RegisterCommand("rcon",  &IStockCommands::RemoteConsole);
  ClassicsChat_RegisterCommand("save",  &IStockCommands::RemoteSave);
  ClassicsChat_RegisterCommand("log",   &IStockCommands::ClientLog);
  ClassicsChat_RegisterCommand("ban",   &IStockCommands::BanClient);
  ClassicsChat_RegisterCommand("mute",  &IStockCommands::MuteClient);
  ClassicsChat_RegisterCommand("kick",  &IStockCommands::KickClient);

  // Voting
  if (ClassicsCore_IsServerApp()) {
    ClassicsChat_RegisterCommand("voteskip", &IVotingSystem::Chat::VoteSkip);
  }

  ClassicsChat_RegisterCommand("votemap",  &IVotingSystem::Chat::VoteMap);
  ClassicsChat_RegisterCommand("votekick", &IVotingSystem::Chat::VoteKick);
  ClassicsChat_RegisterCommand("votemute", &IVotingSystem::Chat::VoteMute);
  ClassicsChat_RegisterCommand("y", &IVotingSystem::Chat::VoteYes);
  ClassicsChat_RegisterCommand("n", &IVotingSystem::Chat::VoteNo);

  // Local interaction with the client log
  _pShell->DeclareSymbol("user void ClientLog(INDEX, INDEX);", &ClientLogInConsole);
  _pShell->DeclareSymbol("user void ClientLogDelete(INDEX, INDEX);", &ClientLogDelete);
  _pShell->DeclareSymbol("user void ClientLogSave(void);", &ClientLogSave);
  _pShell->DeclareSymbol("user void ClientLogLoad(void);", &ClientLogLoad);
};

}; // namespace
