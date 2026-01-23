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

#ifndef SE_INCL_MENU_H
#define SE_INCL_MENU_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

// [Cecil] One pressed menu button (keyboard, mouse or controller)
struct PressedMenuButton {
  int iKey; // Keyboard key (or mouse button)
  int iCtrl; // Controller button

  PressedMenuButton(int iSetKey, int iSetCtrl) :
    iKey(iSetKey), iCtrl(iSetCtrl) {};

  // Set to "no button" state
  inline void SetNone(void) {
    iKey = iCtrl = -1;
  };

  // Check if there is no button
  inline bool IsNone(void) {
    return (iKey == -1 && iCtrl == -1);
  };

  // Cancel / Go back to the previous menu
  inline bool Back(BOOL bMouse) {
    return iKey == VK_ESCAPE || (bMouse && iKey == VK_RBUTTON)
        || iCtrl == SDL_GAMEPAD_BUTTON_EAST || iCtrl == SDL_GAMEPAD_BUTTON_BACK;
  };

  // Apply / Enter the next menu
  inline bool Apply(BOOL bMouse) {
    return iKey == VK_RETURN || (bMouse && iKey == VK_LBUTTON)
        || iCtrl == SDL_GAMEPAD_BUTTON_SOUTH || iCtrl == SDL_GAMEPAD_BUTTON_START;
  };

  // Decrease value
  inline bool Decrease(void) {
    return iKey == VK_BACK || iKey == VK_LEFT
        || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_LEFT;
  };

  // Increase value
  inline bool Increase(void) {
    return iKey == VK_RETURN || iKey == VK_RIGHT
        || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
  };

  inline INDEX ChangeValue(void) {
    // Weak
    if (Decrease()) return -1;
    if (Increase()) return +1;

    // Strong
    if (iCtrl == SDL_GAMEPAD_BUTTON_WEST) return -5;
    if (iCtrl == SDL_GAMEPAD_BUTTON_SOUTH) return +5;

    // None
    return 0;
  };

  // Select previous value
  inline bool Prev(void) {
    return Decrease() || iKey == VK_RBUTTON;
  };

  // Select next value
  inline bool Next(void) {
    return Increase() || iKey == VK_LBUTTON;
  };

  // Directions
  inline bool Up(void)    { return iKey == VK_UP    || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_UP; };
  inline bool Down(void)  { return iKey == VK_DOWN  || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_DOWN; };
  inline bool Left(void)  { return iKey == VK_LEFT  || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_LEFT; };
  inline bool Right(void) { return iKey == VK_RIGHT || iCtrl == SDL_GAMEPAD_BUTTON_DPAD_RIGHT; };

  inline INDEX ScrollPower(void) {
    // Weak
    if (iKey == VK_PRIOR || iCtrl == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)  return -1;
    if (iKey == VK_NEXT  || iCtrl == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) return +1;

    // Strong
    if (iKey == MOUSEWHEEL_UP) return -2;
    if (iKey == MOUSEWHEEL_DN) return +2;

    // None
    return 0;
  };
};

// set new thumbnail
void SetThumbnail(CTFileName fn);
// remove thumbnail
void ClearThumbnail(void);

void InitializeMenus(void);
void DestroyMenus(void);
void MenuGoToParent(void); // [Cecil] Declared here
void MenuOnKeyDown(PressedMenuButton pmb); // [Cecil] Handle different buttons
void MenuOnKeyUp(PressedMenuButton pmb); // [Cecil]
void MenuOnMouseHold(PressedMenuButton pmb); // [Cecil]
void MenuOnChar(MSG msg);
void MenuOnMouseMove(PIX pixI, PIX pixJ);
void MenuOnLMBDown(void);
BOOL DoMenu(CDrawPort *pdp); // returns TRUE if still active, FALSE if should quit
void StartMenus(const char *str = "");
void StopMenus(BOOL bGoToRoot = TRUE);
BOOL IsMenuRoot(class CGameMenu *pgm); // [Cecil] Check if it's a root menu
void ChangeToMenu(class CGameMenu *pgmNew);

// [Cecil] Flag for playing over other sounds
extern void PlayMenuSound(CSoundData *psd, BOOL bOverOtherSounds = TRUE);

#define KEYS_ON_SCREEN    16 // [Cecil] 14 -> 16
#define LEVELS_ON_SCREEN  17 // [Cecil] 16 -> 17
#define VARS_ON_SCREEN    16 // [Cecil] 14 -> 16

// [Cecil] Amount of supported local players
#define MAX_GAME_LOCAL_PLAYERS Min((INDEX)GetGameAPI()->GetLocalPlayerCount(), (INDEX)4)

extern CListHead _lhServers;
extern INDEX _iLocalPlayer;
extern ERunningGameMode _gmMenuGameMode;
extern CGameMenu *pgmCurrentMenu;

#include "GameMenu.h"

#include "MLoadSave.h"
#include "MPlayerProfile.h"
#include "MSelectPlayers.h"

#endif /* include-once check. */