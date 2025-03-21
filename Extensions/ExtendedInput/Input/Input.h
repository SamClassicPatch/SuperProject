/* Copyright (c) 2002-2012 Croteam Ltd.
   Copyright (c) 2024-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_INPUTPATCH_H
#define CECIL_INCL_INPUTPATCH_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <SDL3/SDL.h>
#include <Core/Base/InputApiCompatibility.h>

// For mimicking Win32 wheel scrolling
#define MOUSEWHEEL_SCROLL_INTERVAL 120

#define CECIL_MAX_OVERALL_BUTTONS (KID_TOTALCOUNT + MAX_JOYSTICKS * SDL_GAMEPAD_BUTTON_COUNT)
#define CECIL_FIRST_AXIS_ACTION (CECIL_MAX_OVERALL_BUTTONS)

enum EInputAxis {
  EIA_NONE = 0, // Invalid/no axis
  EIA_MOUSE_X,  // Mouse movement
  EIA_MOUSE_Y,
  EIA_MOUSE_Z,  // Mouse wheel
  EIA_MOUSE2_X, // Second mouse movement
  EIA_MOUSE2_Y,

  // Amount of mouse axes / first controller axis
  EIA_MAX_MOUSE,
  EIA_CONTROLLER_OFFSET = EIA_MAX_MOUSE,

  // Amount of axes (mouse axes + all controller axes * all controllers)
  EIA_MAX_ALL = (EIA_MAX_MOUSE + SDL_GAMEPAD_AXIS_COUNT * MAX_JOYSTICKS),
};

// Information about a single input action
struct InputDeviceAction {
  DOUBLE ida_fReading; // Current reading of the action (from -1 to +1)
  BOOL ida_bExists; // Whether this action (controller axis) can be used

  // Set current reading value that's synchronized with vanilla button values
  static void SetReading(INDEX iActionIndex, DOUBLE fSetReading);

  // Whether the action is active (button is held / controller stick is fully to the side)
  bool IsActive(DOUBLE fThreshold = 0.5) const;
};

// [Cecil] Individual game controller
struct GameController_t {
  SDL_Gamepad *handle; // Opened controller
  INDEX iInfoSlot; // Used controller slot for info output

  GameController_t();
  ~GameController_t();

  void Connect(SDL_JoystickID iDevice, INDEX iArraySlot);
  void Disconnect(void);
  BOOL IsConnected(void);
};

// [Cecil] All possible actions that can be used as controls
extern InputDeviceAction inp_aInputActions[MAX_OVERALL_BUTTONS];

// [Cecil] Game controllers
extern CStaticArray<GameController_t> inp_aControllers;

class CInputPatch : public CInput {
  public:
    // [Cecil] Mimicking constructor and destructor
    static void Construct(void);
    static void Destruct(void);

    // [Cecil] Check if input patch has been initialized
    static int IsInitialized(void *);

    // Sets name for every key
    void P_SetKeyNames(void);
    // Initializes all available devices and enumerates available controls
    void P_Initialize(void);
    // Enable input inside one window
    void P_EnableInput(HWND hWnd);
    // Disable input
    void P_DisableInput(void);
    // Scan states of all available input sources
    void P_GetInput(BOOL bPreScan);
    // Clear all input states (keys become not pressed, axes are reset to zero)
    void P_ClearInput(void);

  // [Cecil] Second mouse interface
  public:

    static void Mouse2_Clear(void);
    static void Mouse2_Startup(void);
    static void Mouse2_Shutdown(void);
    static void Mouse2_Update(void);

  // [Cecil] Joystick interface
  public:

    // [Cecil] Display info about current joysticks
    static void PrintJoysticksInfo(void);

    // [Cecil] Open a game controller under some device index
    static void OpenGameController(SDL_JoystickID iDevice);

    // [Cecil] Close a game controller under some device index
    static void CloseGameController(SDL_JoystickID iDevice);

    // [Cecil] Find controller slot from its device index
    static INDEX GetControllerSlotForDevice(SDL_JoystickID iDevice);

    // [Cecil] Setup controller events from button & axis actions
    static BOOL SetupControllerEvent(INDEX iCtrl, MSG &msg);

    // [Cecil] Update SDL joysticks manually (if SDL_PollEvent() isn't being used)
    static int UpdateJoysticks(void *pMsg);

    // [Cecil] Joystick setup on initialization
    static void StartupJoysticks(void);

    // [Cecil] Joystick cleanup on destruction
    static void ShutdownJoysticks(void);

    // Adds axis and buttons for given joystick
    void P_AddJoystickAbbilities(INDEX iSlot);

    // Scans axis and buttons for given joystick
    BOOL P_ScanJoystick(INDEX iSlot, BOOL bPreScan);

    // [Cecil] Get input from joysticks
    void P_PollJoysticks(BOOL bPreScan);

  public:

    // Get translated name of given axis
    inline const CTString &P_GetAxisTransName(INDEX iAxisNo) const {
      return inp_strButtonNamesTra[CECIL_FIRST_AXIS_ACTION + iAxisNo];
    };

    // Get current position of given axis
    inline FLOAT P_GetAxisValue(INDEX iAxisNo) const {
      return inp_aInputActions[CECIL_FIRST_AXIS_ACTION + iAxisNo].ida_fReading;
    };
};

#endif
