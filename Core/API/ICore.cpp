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

#include <Engine/Base/Console_internal.h>
#include "Query/QueryManager.h"

#include <STLIncludesBegin.h>
#include <string>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <STLIncludesEnd.h>

// Pre-1.10 variables removed from Revolution engine
#if SE1_GAME == SS_REV
  CTString _strModName = "";
  CTString _strModURL = "";
  CTString _strModExt = "";
#endif

// Common game variables
#if SE1_GAME == SS_TFE
  CTString sam_strFirstLevel = "Levels\\01_Hatshepsut.wld";
  CTString sam_strIntroLevel = "Levels\\Intro.wld";
  CTString sam_strGameName = "serioussam";
#else
  CTString sam_strFirstLevel = "Levels\\LevelsMP\\1_0_InTheLastEpisode.wld";
  CTString sam_strIntroLevel = "Levels\\LevelsMP\\Intro.wld";
  CTString sam_strGameName = "serioussamse";
#endif

CTString sam_strVersion = _SE_VER_STRING; // Use version string

// Temporary password for connecting to some server
CTString cli_strConnectPassword = "";

// Current values of input axes
FLOAT inp_afAxisValues[MAX_OVERALL_AXES];

// Toggle vanilla query manager
void ICore::DisableGameSpy(void) {
#if _PATCHCONFIG_NEW_QUERY

  // Symbol for accessing GameSpy master server
  static CSymbolPtr pbHeartbeatGameSpy;

  // Update only when vanilla query is toggled
  static INDEX iLastVanillaQuery = -1;
  if (iLastVanillaQuery == ms_bVanillaQuery) return;

  // Find the symbol
  if (!pbHeartbeatGameSpy.Exists()) {
    pbHeartbeatGameSpy.Find("ser_bHeartbeatGameSpy");

    // Better luck next time
    if (!pbHeartbeatGameSpy.Exists()) return;
  }

  iLastVanillaQuery = ms_bVanillaQuery;

  // Remember pointer to the original value
  static INDEX *piValue = &pbHeartbeatGameSpy.GetIndex();

  if (!ms_bVanillaQuery) {
    // Update last value
    static INDEX iDummyValue;
    iDummyValue = *piValue;

    // Forcefully disable it
    *piValue = FALSE;

    // Make it inaccessible
    pbHeartbeatGameSpy._pss->ss_pvValue = &iDummyValue;

  } else {
    // Make it accessible again
    pbHeartbeatGameSpy._pss->ss_pvValue = piValue;
  }

#endif // _PATCHCONFIG_NEW_QUERY
};

// Reinitialize console in the engine
void ICore::ReinitConsole(INDEX ctCharsPerLine, INDEX ctLines) {
  CConsole &con = *_pConsole;

  // Synchronize access to console
  CTSingleLock slConsole(&con.con_csConsole, TRUE);

  // Limit characters per line
  ctCharsPerLine = Clamp(ctCharsPerLine, (INDEX)90, (INDEX)256);

  // Save last console log
  CStringStack aLastLines;
  INDEX ctLastLines = con.con_ctLines;

  while (--ctLastLines >= 0) {
    CTString strLine = CON_GetLastLine(ctLastLines);

    // Remove the rest of the line
    strLine.TrimRight(con.con_ctCharsPerLine);
    strLine.TrimSpacesRight();

    aLastLines.Push() = strLine;
  }

  // Allocate the buffer
  con.con_ctCharsPerLine = ctCharsPerLine;
  con.con_ctLines = ctLines;
  con.con_ctLinesPrinted = 0;

  // Add line break to the line
  ctCharsPerLine += 1;

  // Add a null terminator at the end of string buffers
  ResizeMemory((void **)&con.con_strBuffer, ctCharsPerLine * ctLines + 1);
  ResizeMemory((void **)&con.con_strLineBuffer, ctCharsPerLine + 1);
  ResizeMemory((void **)&con.con_atmLines, (ctLines + 1) * sizeof(TIME));

  // Clear all lines
  for (INDEX iLine = 0; iLine < ctLines; iLine++) {
    // Fill the line with spaced from the start
    char *pchLine = con.con_strBuffer + iLine * (con.con_ctCharsPerLine + 1);
    memset(pchLine, ' ', con.con_ctCharsPerLine);

    // Add line break at the end
    pchLine[con.con_ctCharsPerLine] = '\n';
    con.con_atmLines[iLine] = _pTimer->GetRealTimeTick();
  }

  // Set null terminator
  con.con_strBuffer[ctCharsPerLine * ctLines] = '\0';

  // Start printing from the last line
  con.con_strLastLine = con.con_strBuffer + ctCharsPerLine * (ctLines - 1);
  con.con_strCurrent = con.con_strLastLine;

  // Break the log before restoring previous lines
  CPrintF("\n--- ICore::ReinitConsole(%d, %d) ---\n\n", ctCharsPerLine - 1, ctLines);

  // Restore contents of the last log
  BOOL bSkipEmptyLines = TRUE;

  for (INDEX iRestore = 0; iRestore < aLastLines.Count(); iRestore++) {
    // Skip empty lines in the beginning
    if (bSkipEmptyLines) {
      CTString strLine = aLastLines[iRestore];
      strLine.TrimSpacesLeft();

      if (strLine == "") continue;
    }

    // At least one non-empty line printed
    CPutString(aLastLines[iRestore] + "\n");
    bSkipEmptyLines = FALSE;
  }
};

PatchVer_t ClassicsCore_GetVersion(void) {
  return CORE_PATCH_VERSION;
};

const PatchVerString_t &ClassicsCore_GetVersionName(void) {
  static PatchVerString_t _strVer;
  static bool _bMakeString = true;

  if (_bMakeString) {
    _bMakeString = false;
    MakeVersionString(_strVer, ClassicsCore_GetVersion());

    // Append dev build tag
    #if CORE_PRERELEASE_BUILD > 0
      sprintf(_strVer, "%s-pre%d", _strVer, CORE_PRERELEASE_BUILD);
    #endif
  }

  return _strVer;
};

const char *ClassicsCore_GetVanillaExt(void) {
  static std::string _strVanillaExt = "";

  // Load vanilla extension only once
  static bool _bLoadExt = true;

  if (_bLoadExt) {
    _bLoadExt = false;
    std::ifstream strm((IDir::AppPath() + "ModExt.txt").str_String);

    if (!strm.fail()) {
      std::getline(strm, _strVanillaExt);
    }

    strm.close();
  }

  return _strVanillaExt.c_str();
};

// Retrieve path once on the first call
static const CTString &GetEntitiesPath(void) {
#if _PATCHCONFIG_CUSTOM_MOD && _PATCHCONFIG_CUSTOM_MOD_ENTITIES
  static CTString _strEntitiesLib = IDir::FullLibPath("Entities" + _strModExt);
#else
  static CTString _strEntitiesLib = IDir::FullLibPath(CTString("Entities") + ClassicsCore_GetVanillaExt());
#endif

  return _strEntitiesLib;
};

const char *ClassicsCore_GetEntitiesPath(void) {
  return GetEntitiesPath().str_String;
};

bool ClassicsCore_IsEntitiesModded(void) {
  return !!GetEntitiesPath().HasPrefix(_fnmMod);
};

HMODULE ClassicsCore_GetEngineHandle(void) {
  static HMODULE _hEngine = HMODULE_ENGINE;
  return _hEngine;
};

HMODULE ClassicsCore_GetEntitiesHandle(void) {
  static HMODULE _hEntities = LoadLibraryA((IDir::AppPath() + ClassicsCore_GetEntitiesPath()).str_String);
  return _hEntities;
};

void *ClassicsCore_GetEngineSymbol(const char *strSymbol) {
  return GetProcAddress(ClassicsCore_GetEngineHandle(), strSymbol);
};

void *ClassicsCore_GetEntitiesSymbol(const char *strSymbol) {
  return GetProcAddress(ClassicsCore_GetEntitiesHandle(), strSymbol);
};

// Interface initialization
namespace IInitAPI {

// Display information about the Classics patch
static void PatchInfo(void) {
  static CTString strInfo(0,
    "\nSerious Sam Classics Patch"
    "\n" CLASSICSPATCH_URL_SHORT
    "\n"
    "\n- Compiler version: %d"
    "\n- Engine version: %s"
    "\n- Patch version: %s"
    "\n- API version: %s"
    "\n\n(c) Dreamy Cecil, 2022-2025\n",
  _MSC_FULL_VER, _SE_VER_STRING, ClassicsCore_GetVersionName(),
  CLASSICSPATCH_STRINGIFY(CLASSICSPATCH_INTERFACE_VERSION));

  CPutString(strInfo);
};

// Helper method for loading scripts via string variables
static void IncludeScript(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strScript = *NEXT_ARG(CTString *);

  // Include command doesn't support variables, so the string needs to be inserted
  CTString strLoad(0, "include \"%s\";", strScript.str_String);
  _pShell->Execute(strLoad);
};

// Clear contents of the entire console log in-game
static void ClearConsole(void) {
  // Synchronize access to console
  CConsole &con = *_pConsole;
  CTSingleLock slConsole(&con.con_csConsole, TRUE);

  // Clear buffer contents
  memset(con.con_strBuffer, '\0', con.GetBufferSize());
};

void Core(void) {
  // Add API to symbols
  CShellSymbol *aNew = _pShell->sh_assSymbols.New(2);
  aNew[0].ss_strName = "CoreAPI"; // Old symbol for compatibility
  aNew[1].ss_strName = "ClassicsPatchAPI"; // New symbol signifying new API

  for (INDEX i = 0; i < 2; i++) {
    aNew[i].ss_pvValue = ClassicsPatchAPI(); // Pointer to the interface
    aNew[i].ss_istType = 0; // Should be '_shell_istUndeclared'
    aNew[i].ss_ulFlags = SSF_CONSTANT; // Unchangable
    aNew[i].ss_pPreFunc = NULL; // Unused
    aNew[i].ss_pPostFunc = NULL; // Unused
  }

  // Information about the patch
  _pShell->DeclareSymbol("user void PatchInfo(void);", &PatchInfo);

  // Game symbols
  if (ClassicsCore_IsGameApp() || ClassicsCore_IsServerApp()) {
    _pShell->DeclareSymbol("           user CTString sam_strFirstLevel;", &sam_strFirstLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strIntroLevel;", &sam_strIntroLevel);
    _pShell->DeclareSymbol("persistent user CTString sam_strGameName;",   &sam_strGameName);
    _pShell->DeclareSymbol("           user CTString sam_strVersion;",    &sam_strVersion);

    _pShell->DeclareSymbol("persistent user CTString cli_strConnectPassword;", &cli_strConnectPassword);
  }

  _pShell->DeclareSymbol("user void IncludeScript(CTString);", &IncludeScript);
  _pShell->DeclareSymbol("user void ClearConsole(void);", &ClearConsole);

  // Current values of input axes
  static const CTString strAxisValues(0, "user const FLOAT inp_afAxisValues[%d];", MAX_OVERALL_AXES);
  _pShell->DeclareSymbol(strAxisValues, &inp_afAxisValues);

  // Input axes constants
  static const INDEX iAxisNone = AXIS_NONE;
  static const INDEX iAxisMouseX = MOUSE_X_AXIS;
  static const INDEX iAxisMouseY = MOUSE_Y_AXIS;
  static const INDEX iAxisMouseZ = 3;
  static const INDEX iAxisMouse2X = 4;
  static const INDEX iAxisMouse2Y = 5;
  static const INDEX iMaxJoysticks = MAX_JOYSTICKS;
  static const INDEX iAxesPerJoystick = MAX_AXES_PER_JOYSTICK;
  static const INDEX iFirstJoystickAxis = FIRST_JOYAXIS;
  static const INDEX iMaxInputAxes = MAX_OVERALL_AXES;

  _pShell->DeclareSymbol("const INDEX AXIS_NONE;",    (void *)&iAxisNone);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_X;",    (void *)&iAxisMouseX);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_Y;",    (void *)&iAxisMouseY);
  _pShell->DeclareSymbol("const INDEX AXIS_M1_Z;",    (void *)&iAxisMouseZ);
  _pShell->DeclareSymbol("const INDEX AXIS_M2_X;",    (void *)&iAxisMouse2X);
  _pShell->DeclareSymbol("const INDEX AXIS_M2_Y;",    (void *)&iAxisMouse2Y);
  _pShell->DeclareSymbol("const INDEX AXIS_JOY_CT;",  (void *)&iMaxJoysticks);
  _pShell->DeclareSymbol("const INDEX AXIS_PER_JOY;", (void *)&iAxesPerJoystick);
  _pShell->DeclareSymbol("const INDEX AXIS_JOY_1;",   (void *)&iFirstJoystickAxis);
  _pShell->DeclareSymbol("const INDEX AXIS_CT;",      (void *)&iMaxInputAxes);
};

}; // namespace
