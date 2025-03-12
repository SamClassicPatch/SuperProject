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

// Dedicated server properties
INDEX ded_iMaxFPS = 100;
CTString ded_strConfig = "";
CTString ded_strLevel = "";
INDEX ded_bRestartWhenEmpty = TRUE;
FLOAT ded_tmTimeout = -1;

// [Cecil] Change level of the current round mid-game
CTString ded_strForceLevelChange = "";

// Break/close handler
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
  if (dwCtrlType == CTRL_C_EVENT
   || dwCtrlType == CTRL_BREAK_EVENT
   || dwCtrlType == CTRL_CLOSE_EVENT
   || dwCtrlType == CTRL_LOGOFF_EVENT
   || dwCtrlType == CTRL_SHUTDOWN_EVENT) {
    _bRunning = FALSE;
  }

  return TRUE;
};

// Execute shell script
void ExecScript(const CTString &str)
{
  CPrintF("Executing: '%s'\n", str);
  CTString strCmd;
  strCmd.PrintF("include \"%s\"", str);
  _pShell->Execute(strCmd);
};

BOOL Init(int argc, char *argv[])
{
  _bDedicatedServer = TRUE;

  if (argc != 1 + 1 && argc != 2 + 1) {
    // NOTE: this cannot be translated - translations are not loaded yet
    printf("Usage: DedicatedServer <configname> [<modname>]\n"
           "This starts a server reading configs from directory 'Scripts\\Dedicated\\<configname>\\'\n");
    getch();
    exit(0);
  }

  SetConsoleTitleA(argv[1]);

  ded_strConfig = CTString("Scripts\\Dedicated\\") + argv[1] + "\\";

  if (argc == 2 + 1) {
    _fnmMod = CTString("Mods\\") + argv[2] + "\\";
  }

  _strLogFile = CTString("Dedicated_") + argv[1];

  // [Cecil] Mark as a server application
  ClassicsPatch_Setup(k_EClassicsPatchAppType_Server);

#if _PATCHCONFIG_ENGINEPATCHES
  // [Cecil] Function patches
  _EnginePatches.FileSystem();
#endif

  // initialize engine
  SE_InitEngine(sam_strGameName);

  // [Cecil] Initialize the core
  ClassicsPatch_Init();

  // load all translation tables
  InitTranslation();
  CTFileName fnmTransTable;

  try {
    fnmTransTable = CTFILENAME("Data\\Translations\\Engine.txt");
    AddTranslationTable_t(fnmTransTable);

    fnmTransTable = CTFILENAME("Data\\Translations\\Game.txt");
    AddTranslationTable_t(fnmTransTable);

    fnmTransTable = CTFILENAME("Data\\Translations\\Entities.txt");
    AddTranslationTable_t(fnmTransTable);

    fnmTransTable = CTFILENAME("Data\\Translations\\SeriousSam.txt");
    AddTranslationTable_t(fnmTransTable);

    fnmTransTable = CTFILENAME("Data\\Translations\\Levels.txt");
    AddTranslationTable_t(fnmTransTable);

    FinishTranslationTable();

  } catch (char *strError) {
    FatalError("Cannot load translation table '%s':\n%s", fnmTransTable.str_String, strError);
  }

#if _PATCHCONFIG_ENGINEPATCHES

  // Function patches
  CPutString("--- Server: Intercepting Engine functions ---\n");
  _EnginePatches.CorePatches();
  CPutString("--- Done! ---\n");

#endif // _PATCHCONFIG_ENGINEPATCHES

  // always disable all warnings when in serious sam
  _pShell->Execute("con_bNoWarnings=1;");

  // declare shell symbols
  _pShell->DeclareSymbol("persistent user INDEX ded_iMaxFPS;", &ded_iMaxFPS);
  _pShell->DeclareSymbol("user void Quit(void);", &QuitGame);
  _pShell->DeclareSymbol("user CTString ded_strLevel;", &ded_strLevel);
  _pShell->DeclareSymbol("user FLOAT ded_tmTimeout;", &ded_tmTimeout);
  _pShell->DeclareSymbol("user INDEX ded_bRestartWhenEmpty;", &ded_bRestartWhenEmpty);
  _pShell->DeclareSymbol("user void Restart(void);", &RestartGame);
  _pShell->DeclareSymbol("user void NextMap(void);", &NextMap);

  // [Cecil] Custom symbols
  _pShell->DeclareSymbol("user CTString ded_strForceLevelChange;", &ded_strForceLevelChange);

  // [Cecil] Load Game library as a module
  GetPluginAPI()->LoadGameLib("Data\\DedicatedServer.gms");

  _pNetwork->md_strGameID = sam_strGameName;

  LoadStringVar(CTString("Data\\Var\\Sam_Version.var"), sam_strVersion);
  CPrintF(LOCALIZE("Serious Sam version: %s\n"), sam_strVersion);

  SetConsoleCtrlHandler(HandlerRoutine, TRUE);

  // if there is a mod
  if (_fnmMod != "") {
    // execute the mod startup script
    _pShell->Execute(CTString("include \"Scripts\\Mod_startup.ini\";"));
  }

  // [Cecil] Load server plugins
  GetPluginAPI()->LoadPlugins(k_EPluginFlagServer);

  return TRUE;
};

void End(void)
{
  // end game
  _pGame->End();

  // [Cecil] Clean up the core
  ClassicsPatch_Shutdown();

  // end engine
  SE_EndEngine();
};

int SubMain(int argc, char *argv[])
{
  // initialize
  if (!Init(argc, argv)) {
    End();
    return -1;
  }

  // initialy, application is running
  _bRunning = TRUE;

  // execute dedicated server startup script
  ExecScript(CTFILENAME("Scripts\\Dedicated_startup.ini"));

  // execute startup script for this config
  ExecScript(ded_strConfig + "init.ini");

#if _PATCHCONFIG_NEW_QUERY
  // [Cecil] Update internal master server just in case
  _pShell->Execute("UpdateInternalGameSpyMS(0);");
#endif

  // start first round
  RoundBegin();

  // while it is still running
  while (_bRunning)
  {
    // do the main game loop
    DoGame();

    // if game is finished
    if (_pNetwork->IsGameFinished()) {
      // if not yet remembered end of level
      if (_tvLastLevelEnd.tv_llValue < 0) {
        // remember end of level
        _tvLastLevelEnd = _pTimer->GetHighPrecisionTimer();

        // finish this round
        RoundEnd(FALSE);

      // if already remembered
      } else {
        // if time is out
        if ((_pTimer->GetHighPrecisionTimer() - _tvLastLevelEnd).GetSeconds() > ded_tmTimeout) {
          // start next round
          RoundBegin();
        }
      }
    }

    // [Cecil] Force restart if needs a level change
    const BOOL bMidGameRestart = (_bForceRestart || ded_strForceLevelChange != "");

    if (_bRestart || bMidGameRestart) {
      if (ded_bRestartWhenEmpty || bMidGameRestart) {
        _bForceRestart = FALSE;
        _bRestart = FALSE;

        // [Cecil] Remember current round
        INDEX iLastRound = _iRound;

        RoundEnd(FALSE);

        CPutString(LOCALIZE("\nNOTE: Restarting server!\n\n"));
        RoundBegin();

        // [Cecil] Restore current round after changing levels
        if (ded_strForceLevelChange != "") {
          _iRound = iLastRound;
        }

        ded_strForceLevelChange = "";

      } else {
        _bRestart = FALSE;
        _bHadPlayers = FALSE;
      }
    }

    if (_bForceNextMap) {
      StartNewMap();
      _bForceNextMap = FALSE;
    }

  } // end of main application loop

  // [Cecil] End of the game
  RoundEnd(TRUE);
  End();

  return 0;
};

// Entry point
int main(int argc, char *argv[])
{
  int iResult;
  CTSTREAM_BEGIN {
    iResult = SubMain(argc, argv);
  } CTSTREAM_END;

  return iResult;
};
