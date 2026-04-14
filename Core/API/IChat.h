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

#ifndef CECIL_INCL_CHATINTERFACE_H
#define CECIL_INCL_CHATINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Prefix that the chat commands start with
CORE_API extern CTString ser_strCommandPrefix;

// Passwords for authorizing as administrator and operator
CORE_API extern CTString ser_strAdminPassword;
CORE_API extern CTString ser_strOperatorPassword;

// Chat command output colors
CORE_API extern INDEX ser_iChatCommandColor;
CORE_API extern INDEX ser_iChatCommandColor2;

// Chat command function holder
struct CORE_API ChatCommand_t {
  CTString strArgumentList;
  CTString strDescription;
  EChatCommandAccessLevel eAccess;

  BOOL bPure;
  BOOL bHidden; // Whether to hide the command from the help
  FCheckChatCommand pCheckFunc;
  void *pUserData;

  union {
    FEngineChatCommand pEngineHandler;
    FPureChatCommand pPureHandler;
  };

  ChatCommand_t() : eAccess(k_EChatCommandAccessLevel_Everyone), bPure(FALSE), bHidden(FALSE),
    pCheckFunc(NULL), pUserData(NULL), pEngineHandler(NULL) {};

  inline bool operator==(const ChatCommand_t &other) const {
    return bPure == other.bPure && pEngineHandler == other.pEngineHandler;
  };

  // Execute the command function
  BOOL Execute(const CTString &strCommand, CTString &strOut, INDEX iClient, const CTString &strArguments) const;

  // Check whether the command is usable by a specific client
  bool CheckCommand(const CTString &strCommand, INDEX iClient) const;
};

// List of chat commands
typedef se1::map<CTString, ChatCommand_t> CChatCommands;
CORE_API extern CChatCommands _mapChatCommands;

CORE_API const CTString &GetChatCommandColor(void);  // Primary chat command color
CORE_API const CTString &GetChatCommandColor2(void); // Secondary chat command color
CORE_API const CTString &GetVoteMessageColor(void);  // Vote message color

// Handle chat command from a client
CORE_API BOOL HandleChatCommand(INDEX iClient, const CTString &strCommand);

#endif
