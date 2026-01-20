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

#ifndef CECIL_INCL_GAMEINTERFACE_H
#define CECIL_INCL_GAMEINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Core/Base/ObserverCamera.h>

// Compatibility with vanilla Game interface
#ifndef CORE_NO_GAME_HEADER
  #include <Core/Compatibility/Game.h>
#endif

CORE_API extern class CGame *_pGame;

class CHighScoreEntry;
class CControls;

// Engine-compatible replacement for CUniversalSessionProperties
typedef UBYTE CSesPropsContainer[NET_MAXSESSIONPROPERTIES];

// Common controls file
const CTString gam_strCommonControlsFile = "Controls\\System\\Common.ctl";

// API class for the Game library
class CORE_API CGameAPI : public IClassicsGame {
  public:
    // Network provider type for CGame
    enum ENetworkProvider {
      NP_LOCAL,
      NP_SERVER,
      NP_CLIENT,
    };

  public:
    // Session properties
    CStaticArray<INDEX> sp_aiGameModes; // Game mode indices

    bool bGameHooked; // CGame fields have been hooked

    // Field offsets within CLocalPlayer
    struct LocalPlayerOffsets {
      ULONG ctSize; // sizeof(CLocalPlayer)

      SLONG slActive; // lp_bActive
      SLONG slPlayer; // lp_iPlayer
    } lpOffsets;

    // Amounts of available local players and player profiles for iteration
    INDEX ctLocalPlayers;   // 4
    INDEX ctPlayerProfiles; // 8

    // Pointers to CGame fields
    INDEX *piConsoleState;  // gm_csConsoleState
    INDEX *piComputerState; // gm_csComputerState

    CTString *pstrCustomLevel; // gam_strCustomLevel
    CTString *pstrSessionName; // gam_strSessionName
    CTString *pstrJoinAddress; // gam_strJoinAddress

    CTString         *astrAxisNames;      // gm_astrAxisNames[0]
    CHighScoreEntry  *ahseHighScores;     // gm_ahseHighScores[0]
    INDEX            *piLastSetHighScore; // gm_iLastSetHighScore
    CPlayerCharacter *apcPlayers;         // gm_apcPlayers[0]
    CControls        *pctrlControlsExtra; // gm_ctrlControlsExtra
    INDEX            *piSinglePlayer;     // gm_iSinglePlayer

    INDEX *piMenuSplitCfg;    // gm_MenuSplitScreenCfg
    INDEX *piStartSplitCfg;   // gm_StartSplitScreenCfg
    INDEX *piCurrentSplitCfg; // gm_CurrentSplitScreenCfg

    BOOL *pbGameOn;       // gm_bGameOn
    BOOL *pbMenuOn;       // gm_bMenuOn
    BOOL *pbFirstLoading; // gm_bFirstLoading

    CTString *pstrNetProvider; // gm_strNetworkProvider

    INDEX *aiMenuLocalPlayers;  // gm_aiMenuLocalPlayers[0]
    INDEX *aiStartLocalPlayers; // gm_aiStartLocalPlayers[0]
    UBYTE *aLocalPlayers;       // gm_lpLocalPlayers[0]

    // Only usable after the CGame loads the common controls (inside CGame::InitInternal)
    CControls *pctrlCommon; // _ctrlCommonControls from global scope

  public:
    // Constructor
    CGameAPI();

    // Destructor
    ~CGameAPI();

    // Start new game
    BOOL NewGame(const CTString &strSession, const CTFileName &fnmWorld, class CSessionProperties &sp);

    // Get observer camera
    CObserverCamera &GetCamera(void);

  // Shell symbol wrappers
  public:

    // Get name of a specific gamemode
    CTString GetGameTypeNameSS(INDEX iGameMode);

    // Get name of the current gamemode
    CTString GetCurrentGameTypeNameSS(void);

    // Get spawn flags of a specific gamemode
    ULONG GetSpawnFlagsForGameTypeSS(INDEX iGameMode);

    // Check if some menu is enabled
    BOOL IsMenuEnabledSS(const CTString &strMenu);

  // CGame field wrappers
  public:

    const CTString &GetCustomLevel(void) { return *pstrCustomLevel; };
    void SetCustomLevel(const CTString &strLevel) { *pstrCustomLevel = strLevel; };

    CTString &SessionName(void) { return *pstrSessionName; };
    CTString &JoinAddress(void) { return *pstrJoinAddress; };

    const CTString &GetAxisName(INDEX iAxis) { return astrAxisNames[iAxis]; };
    void SetAxisName(INDEX iAxis, const CTString &strAxis) { astrAxisNames[iAxis] = strAxis; };

    // Get one of the high score entries
    CHighScoreEntry *GetHighScore(INDEX iEntry);

    // Get index of the last set high score
    INDEX GetLastSetHighScore(void) {
      #if SE1_GAME != SS_REV
        return *piLastSetHighScore;
      #else
        return -1;
      #endif
    };

    // Set index of the last set high score
    void SetLastSetHighScore(INDEX iHighScore) {
      #if SE1_GAME != SS_REV
        *piLastSetHighScore = iHighScore;
      #endif
    };

    // Get player character for some profile
    CPlayerCharacter *GetPlayerCharacter(INDEX iPlayer) { return &apcPlayers[iPlayer]; };

    // Get controls used for storing changes made through the game menu
    CControls *GetControls(void) { return pctrlControlsExtra; };

    // Get actions for changing controls through the game menu
    CListHead &GetActions(CControls *pctrl);

    // Set network provider
    void SetNetworkProvider(ENetworkProvider eProvider) {
      static const char *astrProviders[3] = {
        "Local", "TCP/IP Server", "TCP/IP Client",
      };

      *pstrNetProvider = astrProviders[eProvider];
    };

    // Overridden API methods
    virtual void HookFields(void);
    virtual bool IsHooked(void) { return bGameHooked; };
    virtual void SetHooked(bool bState) { bGameHooked = bState; };

    virtual int GetGameMode(int i) { return sp_aiGameModes[i]; };

    virtual int GetConState(void) { return *piConsoleState; };
    virtual void SetConState(int iState) {
      // [Cecil] Rev: Force console to become closed (CS_TURNINGOFF -> CS_OFF)
      #if SE1_GAME == SS_REV
        if (iState == 3) iState = 0;
      #endif

      *piConsoleState = iState;
    };

    virtual int GetCompState(void) { return *piComputerState; };
    virtual void SetCompState(int iState) { *piComputerState = iState; };

    virtual bool IsGameOn(void) { return !!*pbGameOn; };
    virtual void SetGameState(bool bState) { *pbGameOn = bState; };

    virtual bool IsMenuOn(void) { return !!*pbMenuOn; };
    virtual void SetMenuState(bool bState) { *pbMenuOn = bState; };

    virtual bool GetFirstLoading(void) { return !!*pbFirstLoading; };
    virtual void SetFirstLoading(bool bState) { *pbFirstLoading = bState; };

    virtual int GetMenuSplitCfg(void) { return *piMenuSplitCfg; };
    virtual void SetMenuSplitCfg(int iConfiguration) { *piMenuSplitCfg = iConfiguration; };

    virtual int GetStartSplitCfg(void) { return *piStartSplitCfg; };
    virtual void SetStartSplitCfg(int iConfiguration) { *piStartSplitCfg = iConfiguration; };

    virtual int GetCurrentSplitCfg(void) { return *piCurrentSplitCfg; };
    virtual void SetCurrentSplitCfg(int iConfiguration) { *piCurrentSplitCfg = iConfiguration; };

    virtual int GetLocalPlayerCount(void) { return ctLocalPlayers; };
    virtual int GetProfileCount(void) { return ctPlayerProfiles; };

    virtual bool IsLocalPlayerActive(int iPlayer);
    virtual int GetProfileFromLocalPlayer(int iPlayer);

    virtual int GetProfileForSP(void) { return *piSinglePlayer; };
    virtual void SetProfileForSP(int iProfile) { *piSinglePlayer = iProfile; };

    virtual int GetProfileForMenu(int iPlayer) { return aiMenuLocalPlayers[iPlayer]; };
    virtual void SetProfileForMenu(int iPlayer, int iProfile) { aiMenuLocalPlayers[iPlayer] = iProfile; };

    virtual int GetProfileForStart(int iPlayer) { return aiStartLocalPlayers[iPlayer]; };
    virtual void SetProfileForStart(int iPlayer, int iProfile) { aiStartLocalPlayers[iPlayer] = iProfile; };

    virtual void ResetStartProfiles(void);
    virtual void SetStartProfilesFromMenuProfiles(void);
};

// Full interface getter
CORE_API CGameAPI *GetGameAPI(void);

#endif
