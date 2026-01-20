/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_HOOKSINTERFACE_H
#define CECIL_INCL_HOOKSINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface with global hooks for implementing core logic
class CORE_API IHooks {
  // Processing hooks
  public:

    // Called every simulation tick
    static void OnTick(void);

    // Called every render frame
    static void OnFrame(CDrawPort *pdp);

  // Rendering hooks
  public:

    // Called before redrawing game view
    static void OnPreDraw(CDrawPort *pdp);

    // Called after redrawing game view
    static void OnPostDraw(CDrawPort *pdp);

    // Called after rendering the world
    static void OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp);

  // Game hooks
  public:

    // Called after starting world simulation
    static void OnGameStart(void);

    // Called after changing the level
    static void OnChangeLevel(void);

    // Called before stopping world simulation
    static void OnGameStop(void);

    // Called after saving the game
    static void OnGameSave(const CTFileName &fnmSave);

    // Called after loading a saved game
    static void OnGameLoad(const CTFileName &fnmSave);

    // Called every time a new player is added
    static void OnAddPlayer(CPlayerTarget &plt, BOOL bLocal);

    // Called every time a player is removed
    static void OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal);

  // Demo hooks
  public:

    // Called after starting demo playback
    static void OnDemoPlay(const CTFileName &fnmDemo);

    // Called after starting demo recording
    static void OnDemoStart(const CTFileName &fnmDemo);

    // Called after stopping demo recording
    static void OnDemoStop(void);

  // World hooks
  public:

    // Called after finishing reading the world file
    static void OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld);
};

#endif
