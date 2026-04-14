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

// Chat command output colors
INDEX ser_iChatCommandColor  = 0xFFDF00FF;
INDEX ser_iChatCommandColor2 = 0xFF5F3FFF;

// Currently processed chat command by name
static const CTString *_pstrCurrentChatCommand = NULL;

// Execute the command function
BOOL ChatCommand_t::Execute(const CTString &strCommand, CTString &strOut, INDEX iClient, const CTString &strArguments) const {
  BOOL bHandled;
  _pstrCurrentChatCommand = &strCommand; // Set current command

  if (bPure) {
    ChatCommandResultStr strBufferOut = { 0 };
    bHandled = pPureHandler(strBufferOut, iClient, strArguments.str_String);
    strOut = strBufferOut;

  } else {
    bHandled = pEngineHandler(strOut, iClient, strArguments);
  }

  _pstrCurrentChatCommand = NULL; // Reset current command
  return bHandled;
};

// Check whether the command is usable by a specific client
bool ChatCommand_t::CheckCommand(const CTString &strCommand, INDEX iClient) const {
  // Non-operator tries executing an operator command
  if (!CActiveClient::IsOperator(iClient) && eAccess == k_EChatCommandAccessLevel_Operator) return false;

  // Non-admin tries executing an admin command
  if (!CActiveClient::IsAdmin(iClient) && eAccess == k_EChatCommandAccessLevel_Admin) return false;

  // Check the command using specified callback, if it exists
  bool bResult = true;

  if (pCheckFunc != NULL) {
    _pstrCurrentChatCommand = &strCommand; // Set current command
    bResult = !!pCheckFunc(iClient);
    _pstrCurrentChatCommand = NULL; // Reset current command
  }

  return bResult;
};

// List of chat commands
CChatCommands _mapChatCommands;

// Primary chat command color
const CTString &GetChatCommandColor(void) {
  static INDEX _iLastColor = 0xFFFFFFFF;
  static CTString _strColorTag = "^cFFFFFF^aFF";

  // Cache the color tag string
  if (_iLastColor != ser_iChatCommandColor) {
    _iLastColor = ser_iChatCommandColor;

    const ULONG ulColor = ULONG(ser_iChatCommandColor) >> 8;
    const ULONG ulAlpha = ser_iChatCommandColor & 0xFF;
    _strColorTag.PrintF("^c%06X^a%02X", ulColor, ulAlpha);
  }

  return _strColorTag;
};

// Secondary chat command color
const CTString &GetChatCommandColor2(void) {
  static INDEX _iLastColor2 = 0xFFFFFFFF;
  static CTString _strColorTag2 = "^cFFFFFF^aFF";

  // Cache the color tag string
  if (_iLastColor2 != ser_iChatCommandColor2) {
    _iLastColor2 = ser_iChatCommandColor2;

    const ULONG ulColor = ULONG(ser_iChatCommandColor2) >> 8;
    const ULONG ulAlpha = ser_iChatCommandColor2 & 0xFF;
    _strColorTag2.PrintF("^c%06X^a%02X", ulColor, ulAlpha);
  }

  return _strColorTag2;
};

// Vote message color
const CTString &GetVoteMessageColor(void) {
  static const CTString _strVoteColorTag = "^cFFFF00";
  return _strVoteColorTag;
};

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

    // Process as a normal chat message if the command is currently unusable
    if (!com.CheckCommand(it->first, iClient)) return TRUE;

    // Execute it
    CTString strOut = "";
    const BOOL bHandled = com.Execute(it->first, strOut, iClient, strArguments);

    // Process as a normal chat message upon failure
    if (!bHandled) return TRUE;

    // Reply to the client with the inputted command
    INetwork::SendChatToClient(iClient, TRANS("Chat command"), strCommand);

    // Separate command output by lines and send each line as the sender name with an empty chat message
    CStringStack astrLines;
    IData::GetStrings(astrLines, strOut, '\n');
    const INDEX ctLines = astrLines.Count();

    for (INDEX iLine = 0; iLine < ctLines; iLine++) {
      INetwork::SendChatToClient(iClient, "^a7F", "^r" + astrLines[iLine]);
    }

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
    const ChatCommand_t &com = it->second;

    // Don't list unusable commands
    if (!com.CheckCommand(it->first, iClient)) continue;

    // Don't list hidden commands for non-admins
    if (!CActiveClient::IsAdmin(iClient) && com.bHidden) continue;

    mapList[it->first] = &com;
    ctCommands++;
  }

  if (ctCommands == 0) {
    strResult = TRANS("No chat commands available");
    return TRUE;
  }

  // How many commands to list per page
  INDEX ctPerPage = 20;

  const INDEX ctPages = ceil((FLOAT)ctCommands / (FLOAT)ctPerPage);

  // Determine the current page
  INDEX iPage = 1;

  if (((CTString &)strArguments).ScanF("%d", &iPage) >= 1) {
    iPage = Clamp(iPage, (INDEX)1, ctPages);
  }

  strResult.PrintF(TRANS("^cffffffAvailable commands (page %d/%d):"), iPage, ctPages);
  strResult += "\n^cffffff--------------------------------";

  // Get the first command on the specified page
  CListedCommands::const_iterator itList = mapList.begin();
  std::advance(itList, (iPage - 1) * ctPerPage);

  // Print the commands until the counter per page reaches zero or until the end of the command list
  while (--ctPerPage >= 0 && itList != mapList.end()) {
    // Append argument list
    CTString strArgs = "";

    if (itList->second->strArgumentList != "") {
      strArgs = itList->second->strArgumentList;
      strArgs.TrimSpacesLeft();
      strArgs = " " + strArgs;
    }

    strResult += "\n" + GetChatCommandColor() + ser_strCommandPrefix + itList->first + strArgs + "^r - " + itList->second->strDescription;
    itList++;
  }

  return TRUE;
};

void ClassicsChat_RegisterCommand(const char *strName, FEngineChatCommand pFunction)
{
  // Already registered under this name
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand != _mapChatCommands.end()) return;

  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = FALSE;
  com.pEngineHandler = pFunction;

  com.strArgumentList = "";
  com.strDescription = TRANS("No description");
  com.eAccess = k_EChatCommandAccessLevel_Everyone;
};

void ClassicsChat_RegisterCommandPure(const char *strName, FPureChatCommand pFunction)
{
  // Already registered under this name
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand != _mapChatCommands.end()) return;

  ChatCommand_t &com = _mapChatCommands[strName];
  com.bPure = TRUE;
  com.pPureHandler = pFunction;

  com.strArgumentList = "";
  com.strDescription = TRANS("No description");
  com.eAccess = k_EChatCommandAccessLevel_Everyone;
};

void ClassicsChat_UnregisterCommand(const char *strName)
{
  // Nothing registered under this name
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return;

  _mapChatCommands.remove(*itCommand);
};

BOOL ClassicsChat_CommandExists(const char *strName)
{
  CChatCommands::const_iterator itCommand = _mapChatCommands.find(strName);
  return itCommand != _mapChatCommands.end();
};

BOOL ClassicsChat_SetCommandAccess(const char *strName, EChatCommandAccessLevel eAccess, BOOL bHidden)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return FALSE;

  itCommand->second.eAccess = eAccess;
  itCommand->second.bHidden = bHidden;
  return TRUE;
};

BOOL ClassicsChat_SetCommandCheck(const char *strName, FCheckChatCommand pFunction)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return FALSE;

  itCommand->second.pCheckFunc = pFunction;
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

const char *ClassicsChat_CurrentCommand(void)
{
  if (_pstrCurrentChatCommand == NULL) return NULL;
  return _pstrCurrentChatCommand->str_String;
};

BOOL ClassicsChat_SetCommandUserData(const char *strName, void *pUserData)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return FALSE;

  itCommand->second.pUserData = pUserData;
  return TRUE;
};

void *ClassicsChat_GetCommandUserData(const char *strName)
{
  CChatCommands::iterator itCommand = _mapChatCommands.find(strName);
  if (itCommand == _mapChatCommands.end()) return NULL;

  return itCommand->second.pUserData;
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

// Ban client under a specific identity for the specified amount of time
static void ClientLogBan(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  FLOAT fTime = NEXT_ARG(FLOAT);

  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    CPutString("Invalid client index!\n");
    return;
  }

  CClientRestriction::BanClient(iIdentity, fTime);
};

// Mute client under a specific identity for the specified amount of time
static void ClientLogMute(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  FLOAT fTime = NEXT_ARG(FLOAT);

  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    CPutString("Invalid client index!\n");
    return;
  }

  CClientRestriction::MuteClient(iIdentity, fTime);
};

void Chat(void) {
  _pShell->DeclareSymbol("persistent user CTString ser_strCommandPrefix;", &ser_strCommandPrefix);
  _pShell->DeclareSymbol("persistent user INDEX ser_iChatCommandColor;",  &ser_iChatCommandColor);
  _pShell->DeclareSymbol("persistent user INDEX ser_iChatCommandColor2;", &ser_iChatCommandColor2);
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
  ClassicsChat_RegisterCommand("votemap", &IVotingSystem::Chat::VoteMap);
  ClassicsChat_SetCommandInfo("votemap", "[map index]", TRANS("Initiate a voting process to change the current map with another one from the current map pool"));
  ClassicsChat_SetCommandCheck("votemap", &IVotingSystem::Chat::CheckMapVoteCommand);

  ClassicsChat_RegisterCommand("votekick", &IVotingSystem::Chat::VoteKick);
  ClassicsChat_SetCommandInfo("votekick", "[client]", TRANS("Initiate a voting process to kick a specific client under a specific index"));
  ClassicsChat_SetCommandCheck("votekick", &IVotingSystem::Chat::CheckKickVoteCommand);

  ClassicsChat_RegisterCommand("votemute", &IVotingSystem::Chat::VoteMute);
  ClassicsChat_SetCommandInfo("votemute", "[client]", TRANS("Initiate a voting process to mute a specific client under a specific index"));
  ClassicsChat_SetCommandCheck("votemute", &IVotingSystem::Chat::CheckMuteVoteCommand);

  ClassicsChat_RegisterCommand("voteskip", &IVotingSystem::Chat::VoteSkip);
  ClassicsChat_SetCommandInfo("voteskip", "", TRANS("Initiate a voting process to skip the current round on a dedicated server"));
  ClassicsChat_SetCommandCheck("voteskip", &IVotingSystem::Chat::CheckSkipVoteCommand);

  ClassicsChat_RegisterCommand("y", &IVotingSystem::Chat::VoteYes);
  ClassicsChat_SetCommandInfo("y", "", TRANS("Vote \"yes\" during active voting"));
  ClassicsChat_SetCommandCheck("y", &IVotingSystem::Chat::CheckGenericVoteCommand);

  ClassicsChat_RegisterCommand("n", &IVotingSystem::Chat::VoteNo);
  ClassicsChat_SetCommandInfo("n", "", TRANS("Vote \"no\" during active voting"));
  ClassicsChat_SetCommandCheck("n", &IVotingSystem::Chat::CheckGenericVoteCommand);

  // Local interactions with the client log
  _pShell->DeclareSymbol("user void ClientLog(INDEX, INDEX);", &ClientLogInConsole);
  _pShell->DeclareSymbol("user void ClientLogDelete(INDEX, INDEX);", &ClientLogDelete);
  _pShell->DeclareSymbol("user void ClientLogSave(void);", &ClientLogSave);
  _pShell->DeclareSymbol("user void ClientLogLoad(void);", &ClientLogLoad);
  _pShell->DeclareSymbol("user void ClientLogBan(INDEX, FLOAT);", &ClientLogBan);
  _pShell->DeclareSymbol("user void ClientLogMute(INDEX, FLOAT);", &ClientLogMute);

  _pShell->DeclareSymbol("persistent user INDEX ser_bAutoSaveClientLog;", &ser_bAutoSaveClientLog);
};

}; // namespace
