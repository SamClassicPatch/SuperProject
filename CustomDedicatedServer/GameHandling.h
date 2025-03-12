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

// Game state properties
extern INDEX _iRound;
extern BOOL _bHadPlayers;
extern BOOL _bRestart;

extern CTimerValue _tvLastLevelEnd;

// Begin round on the current map
void RoundBegin(void);

// End round on the current map
void RoundEnd(BOOL bGameEnd);

// Start new map loading
BOOL StartNewMap(void);

// Main game loop
void DoGame(void);
