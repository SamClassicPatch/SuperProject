/* Copyright (c) 2024-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_VOTINGSYSTEM_H
#define CECIL_INCL_VOTINGSYSTEM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "VoteTypes.h"

CORE_API extern INDEX ser_bVotingSystem;
CORE_API extern INDEX ser_bPlayersStartVote;
CORE_API extern INDEX ser_bPlayersCanVote;
CORE_API extern INDEX ser_bObserversStartVote;
CORE_API extern INDEX ser_bObserversCanVote;
CORE_API extern FLOAT ser_fVotingTime;
CORE_API extern FLOAT ser_fVotingTimeout;

CORE_API extern INDEX ser_bVoteMap;
CORE_API extern INDEX ser_bVoteKick;
CORE_API extern INDEX ser_bVoteMute;
CORE_API extern INDEX ser_bVoteSkip;
CORE_API extern FLOAT ser_fVoteKickTime;
CORE_API extern FLOAT ser_fVoteMuteTime;

// Interface for voting via chat
namespace IVotingSystem {

// Initialize voting system
void Initialize(void);

inline CTString VoteYesCommand(void) {
  return "^c00ff00" + ser_strCommandPrefix + "y";
};

inline CTString VoteNoCommand(void) {
  return "^cff0000" + ser_strCommandPrefix + "n";
};

// Check if voting is available
CORE_API BOOL IsVotingAvailable(void);

// Terminate current vote
CORE_API void EndVote(BOOL bTimeout);

// Initiate voting for a specific thing by some client
CORE_API BOOL InitiateVoting(INDEX iClient, CGenericVote *pvt);

// Update current vote (needs to be synced with the game loop)
CORE_API void UpdateVote(void);

// Load map pool from a file
CORE_API void LoadMapPool(const CTFileName &fnmMapPool);

// Add world file to the map pool
CORE_API BOOL AddMapToPool(const CTFileName &fnmWorldFile);

// Print current map pool
CORE_API void PrintMapPool(CTString &str);

// Print current clients
CORE_API void PrintClientList(CTString &str);

// Check if can initiate voting
CORE_API BOOL CanInitiateVoting(CTString &strWhyCannot, INDEX iClient);

// Make a vote as a client (used by Chat::VoteYes() and Chat::VoteNo() methods)
CORE_API BOOL CheckVote(CTString &strResult, INDEX iClient, BOOL bVoteYes);

// Chat commands
struct CORE_API Chat {
  // Initiate voting to change a map
  static BOOL VoteMap(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Initiate voting to kick a client
  static BOOL VoteKick(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Initiate voting to mute a client
  static BOOL VoteMute(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Initiate voting to skip current round
  static BOOL VoteSkip(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Vote yes in the current vote
  static BOOL VoteYes(CTString &strResult, INDEX iClient, const CTString &strArguments);

  // Vote no in the current vote
  static BOOL VoteNo(CTString &strResult, INDEX iClient, const CTString &strArguments);
};

}; // namespace

#endif
