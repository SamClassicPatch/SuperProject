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

#include "CmdLine.h"

extern CTString cmd_strWorld = "";    // world to load
extern INDEX cmd_iGoToMarker = -1;    // marker to go to
extern CTString cmd_strScript = "";   // script to execute
extern CTString cmd_strServer = "";   // server to connect to
extern INDEX cmd_iPort = -1;          // port to connect to
extern CTString cmd_strPassword = ""; // network password
extern CTString cmd_strOutput = "";   // output from parsing command line
extern BOOL cmd_bServer = FALSE;      // set to run as server
extern BOOL cmd_bQuickJoin = FALSE;   // do not ask for players and network settings

static CTString _strCmd;

// Get first next word or quoted string
CTString GetNextParam(void) {
  // strip leading spaces/tabs
  _strCmd.TrimSpacesLeft();

  // if nothing left
  if (_strCmd == "") {
    // no word to return
    return "";
  }

  // if the first char is quote
  if (_strCmd[0] == '"') {
    // find first next quote
    const char *pchClosingQuote = strchr(_strCmd + 1, '"');

    // if not found
    if (pchClosingQuote == NULL) {
      // error in command line
      cmd_strOutput += LOCALIZE("Command line error!\n");

      // finish parsing
      _strCmd = "";

      return "";
    }

    INDEX iQuote = pchClosingQuote - _strCmd;

    // get the quoted string
    CTString strWord;
    CTString strRest;
    _strCmd.Split(iQuote, strWord, strRest);

    // remove the quotes
    strWord.DeleteChar(0);
    strRest.DeleteChar(0);

    // get the word
    _strCmd = strRest;

    return strWord;

  // if the first char is not quote
  } else {
    // find first next space
    INDEX iSpace;
    INDEX ctChars = _strCmd.Length();

    for (iSpace = 0; iSpace < ctChars; iSpace++) {
      if (isspace(_strCmd[iSpace])) {
        break;
      }
    }

    // get the word string
    CTString strWord;
    CTString strRest;
    _strCmd.Split(iSpace, strWord, strRest);

    // remove the space
    strRest.DeleteChar(0);

    // get the word
    _strCmd = strRest;

    return strWord;
  }
}

// Parse command line parameters and set results to static variables
void ParseCommandLine(CTString strCmd, BOOL bOnLaunch) {
  cmd_strOutput = CTString(0, LOCALIZE("Command line: '%s'\n"), strCmd);

  // if no command line
  if (strCmd.Length() == 0) {
    // do nothing
    return;
  }

  _strCmd = strCmd;

  FOREVER {
    CTString strWord = GetNextParam();
    if (strWord == "") {
      cmd_strOutput += "\n";
      return;

    } else if (strWord == "+level") {
      cmd_strWorld = GetNextParam();

    } else if (strWord == "+server") {
      cmd_bServer = TRUE;

    } else if (strWord == "+quickjoin") {
      cmd_bQuickJoin = TRUE;

    } else if (strWord == "+password") {
      cmd_strPassword = GetNextParam();

    } else if (strWord == "+connect") {
      cmd_strServer = GetNextParam();
      const char *pcColon = strchr(cmd_strServer, ':');

      if (pcColon != NULL) {
        CTString strServer;
        CTString strPort;

        cmd_strServer.Split(pcColon - cmd_strServer, strServer, strPort);
        cmd_strServer = strServer;

        strPort.ScanF(":%d", &cmd_iPort);
      }

    } else if (strWord == "+goto") {
      GetNextParam().ScanF("%d", &cmd_iGoToMarker);

    // [Cecil] Commands only on launch
    } else if (bOnLaunch) {
      if (strWord == "+game") {
        CTString strMod = GetNextParam();

        // [Cecil] Ignore the custom mod as well, since it's essentially a vanilla game
        if (strMod != "SeriousSam" && strMod != CLASSICSPATCH_MODDIRNAME) { // (we ignore default mod - always use base dir in that case)
          _fnmMod = "Mods\\" + strMod + "\\";
        }

      } else if (strWord == "+cdpath") {
        _fnmCDPath = GetNextParam();

      } else if (bOnLaunch && strWord == "+script") {
        cmd_strScript = GetNextParam();

      } else if (bOnLaunch && strWord == "+logfile") {
        _strLogFile = GetNextParam();

      } else {
        // [Cecil] This is the first time I seriously use goto in C++ in my entire life
        goto UnknownOption;
      }

    } else {
    UnknownOption:
      cmd_strOutput += CTString(0, LOCALIZE("  Unknown option: '%s'\n"), strWord);
    }
  }
}

// [Cecil] Execute commands set by the command line
BOOL ExecuteCommandLine(void) {
  if (cmd_strPassword != "") {
    _pShell->SetString("net_strConnectPassword", cmd_strPassword);
  }

  // If connecting to server from command line
  if (cmd_strServer != "") {
    CTString strPort = "";

    if (cmd_iPort > 0) {
      _pShell->SetINDEX("net_iPort", cmd_iPort);
      strPort.PrintF(":%d", cmd_iPort);
    }

    CPrintF(LOCALIZE("Command line connection: '%s%s'\n"), cmd_strServer, strPort);

    // Go to join menu
    GetGameAPI()->JoinAddress() = cmd_strServer;

    if (cmd_bQuickJoin) {
      extern void JoinNetworkGame(void);
      JoinNetworkGame();

    } else {
      StartMenus("join");
    }

  // If starting world from command line
  } else if (cmd_strWorld != "") {
    // [Cecil] Start map from the game directory (passes "..\\Levels\\ExampleWorld.wld" into StartMap() command)
    _pShell->Execute(CTString(0, "StartMap(\"%s\");", "..\\\\" + cmd_strWorld));

  // Hasn't started anything
  } else {
    return FALSE;
  }

  // Started a new game
  return TRUE;
};
