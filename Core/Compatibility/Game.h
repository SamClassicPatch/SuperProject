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

// [Cecil] Custom include guard
#ifndef CORE_NO_GAME_HEADER
#define CORE_NO_GAME_HEADER

// [Cecil] Relative path
#include "PlayerSettings.h"
#include "GameControls.h"

// [Cecil] Declare the class and its container just to avoid useless inclusion of "SessionProperties.h"
class CSessionProperties;

// One player in split-screen
class CLocalPlayer {
  public:
    BOOL lp_bActive;
    INDEX lp_iPlayer;
    CPlayerSource *lp_pplsPlayerSource;
    UBYTE lp_ubPlayerControlsState[2048]; // Current state of player controls that are local to the player

  public:
    // Constructor
    CLocalPlayer(void) {
      lp_pplsPlayerSource = NULL; 
      lp_bActive = FALSE; 
      memset(lp_ubPlayerControlsState, 0, sizeof(lp_ubPlayerControlsState));
    };
};

#define HIGHSCORE_COUNT 10

// Entry in the table of high scores
class CHighScoreEntry {
  public:
    CTString hse_strPlayer;
    INDEX hse_gdDifficulty; // [Cecil] 'enum CSessionProperties::GameDifficulty' -> 'INDEX'
    TIME hse_tmTime;
    INDEX hse_ctKills;
    INDEX hse_ctScore;

  public:
    // [Cecil] Constructor definition
    CHighScoreEntry(void) {
      hse_strPlayer = "";
      hse_gdDifficulty = -100;
      hse_tmTime = -1.0f;
      hse_ctKills = -1;
      hse_ctScore = 0;
    };
};

// Current console/computer state
enum ConsoleState {
  CS_OFF,
  CS_ON,
  CS_TURNINGON,
  CS_TURNINGOFF,
  CS_ONINBACKGROUND,
  CS_TALK,
};

class CGameTimerHandler : public CTimerHandler {
  public:
    // This is called every TickQuantum seconds
    virtual void HandleTimer(void);
};

// [Cecil] Function declaration for Revolution support
#if SE1_GAME == SS_REV
  #define REV_VIRTUAL_FUNC(_Declaration) virtual _Declaration
#else
  #define REV_VIRTUAL_FUNC(_Declaration)
#endif

// Class responsible for handling the game interface
class CGame {
  public:
    ConsoleState gm_csConsoleState;
    ConsoleState gm_csComputerState;

    CTFileName gm_fnSaveFileName;

    CTString gam_strCustomLevel;
    CTString gam_strSessionName;
    CTString gam_strJoinAddress;

  #if SE1_GAME != SS_REV
    CTString gam_strConsoleInputBuffer;
    CTString gm_astrAxisNames[AXIS_ACTIONS_CT];

    CHighScoreEntry gm_ahseHighScores[HIGHSCORE_COUNT];
    INDEX gm_iLastSetHighScore;

  #else
    CNetworkSession gm_nsField3; // [Cecil] Rev: Unknown field

    CTString gam_strConsoleInputBuffer;
    CTString gm_astrAxisNames[AXIS_ACTIONS_CT];
  #endif

    // Player profiles and their controls
    CPlayerCharacter gm_apcPlayers[8];
    CControls gm_actrlControls[8];
    CControls gm_ctrlControlsExtra;

    INDEX gm_iSinglePlayer;    // Selected profile for singleplayer game
    INDEX gm_iWEDSinglePlayer; // Selected profile for test game in Serious Editor

    enum SplitScreenCfg {
      SSC_DEDICATED = -2,
      SSC_OBSERVER = -1,
      SSC_PLAY1 = 0,
      SSC_PLAY2 = 1,
      SSC_PLAY3 = 2,
      SSC_PLAY4 = 3,
    };

    SplitScreenCfg gm_MenuSplitScreenCfg;
    SplitScreenCfg gm_StartSplitScreenCfg;
    SplitScreenCfg gm_CurrentSplitScreenCfg;

    CGameTimerHandler m_gthGameTimerHandler;
    BOOL gm_bGameOn;
    BOOL gm_bMenuOn;       // Set by game client to notify that game menu is active
    BOOL gm_bFirstLoading; // Set by game client to notify first loading
    BOOL gm_bProfileDemo;  // Demo profiling required

    // Network provider and its description
    CNetworkProvider gm_npNetworkProvider;
    CTString gm_strNetworkProvider;

    // Local controls for the current player
    SLONG gm_slPlayerControlsSize;
    void *gm_pvGlobalPlayerControls;

    // Profile indices of local players (-1 if inactive)
    INDEX gm_aiMenuLocalPlayers[4];
    INDEX gm_aiStartLocalPlayers[4];

    // Players that are currently playing on a local machine (split-screen for more than one)
    CLocalPlayer gm_lpLocalPlayers[4];

    // [Cecil] Rev: Fly-over observing in games & demos using enhanced CAM_*() methods
    REV_VIRTUAL_FUNC(BOOL IsObservingOn(void)); // Calls CAM_IsOn()
    REV_VIRTUAL_FUNC(void StartObserving(void)); // Calls CAM_Start()
    REV_VIRTUAL_FUNC(void StopObserving(void)); // Calls CAM_Stop()

    virtual void LoadPlayersAndControls(void);
    virtual void SavePlayersAndControls(void);
    virtual void Load_t(void);
    virtual void Save_t(void);

    // Set properties for a test game session
    virtual void SetQuickStartSession(CSessionProperties &sp);

    // Set properties for a singleplayer session
    virtual void SetSinglePlayerSession(CSessionProperties &sp);

    // Set properties for a multiplayer session
    virtual void SetMultiPlayerSession(CSessionProperties &sp);

    // [Cecil] Rev: Survival session setup
    REV_VIRTUAL_FUNC(void SetSurvivalSession(CSessionProperties &sp, INDEX ctMaxPlayers));

    // [Cecil] Rev: Set properties for survival gamemode (called by 'SetSinglePlayerSession' and 'SetMultiPlayerSession')
    REV_VIRTUAL_FUNC(void SetSurvivalProperties(CSessionProperties &sp));

    // Game loop functions
    #define GRV_SHOWEXTRAS (1L << 0) // Add extra stuff like console, weapon, pause
    virtual void GameRedrawView(CDrawPort *pdpDrawport, ULONG ulFlags);
    virtual void GameMainLoop(void);

    // Console functions
    virtual void ConsoleKeyDown(MSG msg);
    virtual void ConsoleChar(MSG msg);

    // [Cecil] Rev: Paste clipboard contents
    REV_VIRTUAL_FUNC(void ConsolePasteClipboard(void));

    virtual void ConsoleRender(CDrawPort *pdpDrawport);
    virtual void ConsolePrintLastLines(CDrawPort *pdpDrawport);

    // [Cecil] Rev: Remember mouse pointer coordinates
    REV_VIRTUAL_FUNC(void MenuMouseMove(PIX pixPointX, PIX pixPointY));

    // Computer functions
    virtual void ComputerMouseMove(PIX pixX, PIX pixY);
    virtual void ComputerKeyDown(MSG msg);

    // [Cecil] Rev: Handle controller input in the computer; returns pointer to CInputXController
    REV_VIRTUAL_FUNC(void *ComputerControllerInput(void));

    virtual void ComputerRender(CDrawPort *pdpDrawport);
    virtual void ComputerForceOff();

    // Loading hook functions
    virtual void EnableLoadingHook(CDrawPort *pdpDrawport);
    virtual void DisableLoadingHook(void);

    // Get default description for a game (for save games/demos)
    virtual CTString GetDefaultGameDescription(BOOL bWithInfo);

    // Game start/end functions
    virtual BOOL NewGame(const CTString &strSessionName, const CTFileName &fnWorld, CSessionProperties &sp);

    // [Cecil] Rev: New method for presumably setting up the world after it loads (GameInfo creation, item removal etc.)
    REV_VIRTUAL_FUNC(BOOL WorldStart(void));

    virtual BOOL JoinGame(CNetworkSession &session);
    virtual BOOL LoadGame(const CTFileName &fnGame);
    virtual BOOL SaveGame(const CTFileName &fnGame);
    virtual void StopGame(void);

    // Demo functions
    virtual BOOL StartDemoPlay(const CTFileName &fnDemo);
    virtual BOOL StartDemoRec(const CTFileName &fnDemo);
    virtual void StopDemoRec(void);

    virtual INDEX GetPlayersCount(void);
    virtual INDEX GetLivePlayersCount(void);

    // Printout and extensive demo profile report
    virtual CTString DemoReportFragmentsProfile(INDEX iRate);
    virtual CTString DemoReportAnalyzedProfile(void);

    // Functions called from Serious Editor
    virtual void Initialize(const CTFileName &fnGameSettings);
    virtual void End(void);
    virtual void QuickTest(const CTFileName &fnMapName, CDrawPort *pdpDrawport, CViewPort *pvpViewport);

    // Menu rendering functions
    virtual void LCDInit(void);
    virtual void LCDEnd(void);
    virtual void LCDPrepare(FLOAT fFade);
    virtual void LCDSetDrawport(CDrawPort *pdp);
    virtual void LCDDrawBox(PIX pixUL, PIX pixDR, PIXaabbox2D &box, COLOR col);
    virtual void LCDScreenBox(COLOR col);
    virtual void LCDScreenBoxOpenLeft(COLOR col);
    virtual void LCDScreenBoxOpenRight(COLOR col);
    virtual void LCDRenderClouds1(void);
    virtual void LCDRenderClouds2(void);
    virtual void LCDRenderGrid(void);
    virtual void LCDDrawPointer(PIX pixI, PIX pixJ);
    virtual COLOR LCDGetColor(COLOR colDefault, const char *strName);
    virtual COLOR LCDFadedColor(COLOR col);
    virtual COLOR LCDBlinkingColor(COLOR col0, COLOR col1);

    // Menu interface functions
    virtual void MenuPreRenderMenu(const char *strMenuName);
    virtual void MenuPostRenderMenu(const char *strMenuName);

    // [Cecil] Rev: Get session properties
    REV_VIRTUAL_FUNC(const CSessionProperties *GetSP(void));
};

#endif // [Cecil] Include guard
