/* Copyright (c) 2002-2012 Croteam Ltd.
   Copyright (c) 2024-2026 Dreamy Cecil
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

#include "Input.h"

GameController_t::GameController_t() : handle(NULL), iInfoSlot(-1)
{
};

GameController_t::~GameController_t() {
  Disconnect();
};

// Open a game controller under some slot
void GameController_t::Connect(SDL_JoystickID iDevice, INDEX iArraySlot) {
  ASSERT(handle == NULL && iInfoSlot == -1);

  handle = SDL_OpenGamepad(iDevice);
  iInfoSlot = iArraySlot + 1;
};

// Close an open game controller
void GameController_t::Disconnect(void) {
  if (handle != NULL) {
    SDL_Joystick *pJoystick = SDL_GetGamepadJoystick(handle);
    CPrintF(TRANS("Disconnected '%s' from controller slot %d\n"), SDL_GetJoystickName(pJoystick), iInfoSlot);

    SDL_CloseGamepad(handle);
  }

  handle = NULL;
  iInfoSlot = -1;
};

// Check if the controller is connected
BOOL GameController_t::IsConnected(void) {
  return (handle != NULL);
};

// [Cecil] Display info about current joysticks
void CInputPatch::PrintJoysticksInfo(void) {
  if (_pInput == NULL) return;

  const INDEX ct = inp_aControllers.Count();
  CPrintF(TRANS("%d controller slots:\n"), ct);

  for (INDEX i = 0; i < ct; i++) {
    GameController_t &gctrl = inp_aControllers[i];
    CPrintF(" %d. ", i + 1);

    if (!gctrl.IsConnected()) {
      CPutString(TRANS("not connected\n"));
      continue;
    }

    SDL_Joystick *pJoystick = SDL_GetGamepadJoystick(gctrl.handle);
    const char *strName = SDL_GetJoystickName(pJoystick);

    if (strName == NULL) {
      strName = SDL_GetError();
    }

    CPrintF("'%s':", strName);
    CPrintF(TRANS(" %d axes, %d buttons, %d hats\n"), SDL_GetNumJoystickAxes(pJoystick),
      SDL_GetNumJoystickButtons(pJoystick), SDL_GetNumJoystickHats(pJoystick));
  }

  CPutString("-\n");
};

// [Cecil] Open a game controller under some device index
void CInputPatch::OpenGameController(SDL_JoystickID iDevice)
{
  // Not a game controller
  if (!SDL_IsGamepad(iDevice)) return;

  // Check if this controller is already connected
  if (GetControllerSlotForDevice(iDevice) != -1) return;

  // Find an empty slot for opening a new controller
  GameController_t *pToOpen = NULL;
  INDEX iArraySlot = -1;
  const INDEX ct = inp_aControllers.Count();

  for (INDEX i = 0; i < ct; i++) {
    if (!inp_aControllers[i].IsConnected()) {
      // Remember controller and its slot
      pToOpen = &inp_aControllers[i];
      iArraySlot = i;
      break;
    }
  }

  // No slots left
  if (pToOpen == NULL || iArraySlot == -1) {
    CPrintF(TRANS("Cannot open another game controller due to all %d slots being occupied!\n"), ct);
    return;
  }

  pToOpen->Connect(iDevice, iArraySlot);

  if (!pToOpen->IsConnected()) {
    CPrintF(TRANS("Cannot open another game controller! SDL Error: %s\n"), SDL_GetError());
    return;
  }

  // Report controller info
  SDL_Joystick *pJoystick = SDL_GetGamepadJoystick(pToOpen->handle);
  const char *strName = SDL_GetJoystickName(pJoystick);

  if (strName == NULL) {
    strName = SDL_GetError();
  }

  CPrintF(TRANS("Connected '%s' to controller slot %d\n"), strName, pToOpen->iInfoSlot);

  int ctAxes = SDL_GetNumJoystickAxes(pJoystick);
  CPrintF(TRANS(" %d axes, %d buttons, %d hats\n"), ctAxes, SDL_GetNumJoystickButtons(pJoystick), SDL_GetNumJoystickHats(pJoystick));

  // Check whether all axes exist
  const INDEX iFirstAxis = CECIL_FIRST_AXIS_ACTION + EIA_CONTROLLER_OFFSET + iArraySlot * SDL_GAMEPAD_AXIS_COUNT;

  for (INDEX iAxis = 0; iAxis < SDL_GAMEPAD_AXIS_COUNT; iAxis++) {
    InputDeviceAction &ida = inp_aInputActions[iFirstAxis + iAxis];
    ida.ida_bExists = (iAxis < ctAxes);
  }
};

// [Cecil] Close a game controller under some device index
void CInputPatch::CloseGameController(SDL_JoystickID iDevice)
{
  INDEX iSlot = GetControllerSlotForDevice(iDevice);

  if (iSlot != -1) {
    inp_aControllers[iSlot].Disconnect();
  }
};

// [Cecil] Find controller slot from its device index
INDEX CInputPatch::GetControllerSlotForDevice(SDL_JoystickID iDevice) {
  INDEX i = inp_aControllers.Count();

  while (--i >= 0) {
    GameController_t &gctrl = inp_aControllers[i];

    // No open controller
    if (!gctrl.IsConnected()) continue;

    // Get device ID from the controller
    SDL_Joystick *pJoystick = SDL_GetGamepadJoystick(gctrl.handle);
    SDL_JoystickID id = SDL_GetJoystickID(pJoystick);

    // Found matching ID
    if (id == iDevice) {
      return i;
    }
  }

  // Couldn't find
  return -1;
};

// [Cecil] Setup controller events from button & axis actions
BOOL CInputPatch::SetupControllerEvent(INDEX iCtrl, MSG &msg)
{
  GameController_t &ctrl = inp_aControllers[iCtrl];
  if (!ctrl.IsConnected()) return FALSE;

  static BOOL _abButtonStates[MAX_JOYSTICKS * SDL_GAMEPAD_BUTTON_COUNT] = { 0 };
  const INDEX iFirstButton = iCtrl * SDL_GAMEPAD_BUTTON_COUNT;

  for (ULONG eButton = 0; eButton < SDL_GAMEPAD_BUTTON_COUNT; eButton++) {
    const BOOL bHolding = !!SDL_GetGamepadButton(ctrl.handle, (SDL_GamepadButton)eButton);

    const BOOL bJustPressed  = (bHolding && !_abButtonStates[iFirstButton + eButton]);
    const BOOL bJustReleased = (!bHolding && _abButtonStates[iFirstButton + eButton]);

    _abButtonStates[iFirstButton + eButton] = bHolding;

    if (bJustPressed) {
      memset(&msg, 0, sizeof(msg));
      msg.message = WM_CTRLBUTTONDOWN;
      msg.wParam = eButton;
      msg.lParam = TRUE;
      return TRUE;
    }

    if (bJustReleased) {
      memset(&msg, 0, sizeof(msg));
      msg.message = WM_CTRLBUTTONUP;
      msg.wParam = eButton;
      msg.lParam = TRUE;
      return TRUE;
    }
  }

  static BOOL _abAxisStates[MAX_JOYSTICKS * SDL_GAMEPAD_AXIS_COUNT] = { 0 };
  const INDEX iFirstAxis = iCtrl * SDL_GAMEPAD_AXIS_COUNT;

  // [Cecil] NOTE: This code only checks whether some axis has been moved past 50% in either direction
  // in order to determine when it has been significantly moved and reset
  for (ULONG eAxis = 0; eAxis < SDL_GAMEPAD_AXIS_COUNT; eAxis++) {
    const SLONG slMotion = SDL_GetGamepadAxis(ctrl.handle, (SDL_GamepadAxis)eAxis);

    // Holding the axis past the half of the max value in either direction
    const BOOL bHolding = Abs(slMotion) > SDL_JOYSTICK_AXIS_MAX / 2;
    const BOOL bJustPressed = (bHolding && !_abAxisStates[iFirstAxis + eAxis]);

    _abAxisStates[iFirstAxis + eAxis] = bHolding;

    if (bJustPressed) {
      memset(&msg, 0, sizeof(msg));
      msg.message = WM_CTRLAXISMOTION;
      msg.wParam = eAxis;
      msg.lParam = Sgn(slMotion);
      return TRUE;
    }
  }

  return FALSE;
};

// [Cecil] Update SDL joysticks manually (if SDL_PollEvent() isn't being used)
int CInputPatch::UpdateJoysticks(void *pMsg) {
  // Update SDL joysticks
  SDL_UpdateJoysticks();

  // Open all valid controllers
  int ctJoysticks;
  SDL_JoystickID *aJoysticks = SDL_GetJoysticks(&ctJoysticks);

  if (aJoysticks != NULL) {
    for (int iJoy = 0; iJoy < ctJoysticks; iJoy++) {
      OpenGameController(aJoysticks[iJoy]);
    }

    SDL_free(aJoysticks);
  }

  // Go through connected controllers
  FOREACHINSTATICARRAY(inp_aControllers, GameController_t, it) {
    if (!it->IsConnected()) continue;

    // Disconnect this controller if it has been detached
    SDL_Joystick *pJoystick = SDL_GetGamepadJoystick(it->handle);

    if (!SDL_JoystickConnected(pJoystick)) {
      it->Disconnect();
    }
  }

  // Process event for the first controller that sends it
  const INDEX ctControllers = inp_aControllers.Count();
  MSG &msg = *(MSG *)pMsg;

  for (INDEX iCtrl = 0; iCtrl < ctControllers; iCtrl++) {
    if (SetupControllerEvent(iCtrl, msg)) {
      return TRUE;
    }
  }

  return FALSE;
};

// [Cecil] Joystick setup on initialization
void CInputPatch::StartupJoysticks(void) {
  // Create an empty array of controllers
  ASSERT(inp_aControllers.Count() == 0);
  inp_aControllers.New(MAX_JOYSTICKS);
};

// [Cecil] Joystick cleanup on destruction
void CInputPatch::ShutdownJoysticks(void) {
  // Should close all controllers automatically on array destruction
  inp_aControllers.Clear();
};

// Temporary joystick names
static CTString _strJoyInt;
static CTString _strJoyTra;

// [Cecil] NOTE: Button names for the axes are actually set to the unused remaining joystick buttons
// (from SDL_GAMEPAD_BUTTON_COUNT to MAX_BUTTONS_PER_JOYSTICK) to allow vanilla game to read saved
// controls without resetting binds that use "axis" buttons (e.g. "[C1] Axis Z" becomes "Joy 1 Button 23")
static void SetAxisNames(INDEX iFirstAxis, INDEX iAxis, const char *strAxisName, const char *strDisplayName)
{
  CTString strIntButtonName(0, "Button %d", SDL_GAMEPAD_BUTTON_COUNT + iAxis);

  iAxis += iFirstAxis;
  _pInput->inp_strButtonNames   [CECIL_FIRST_AXIS_ACTION + iAxis] = _strJoyInt + strIntButtonName;
  _pInput->inp_strButtonNamesTra[CECIL_FIRST_AXIS_ACTION + iAxis] = _strJoyTra + strDisplayName;

  // Old name for compatibility
  if (iAxis >= 0 && iAxis < EIA_MAX_ALL) {
    _pInput->inp_caiAllAxisInfo[iAxis].cai_strAxisName = _strJoyInt + strAxisName;
  }
};

static void SetButtonNames(INDEX iFirstButton, INDEX iButton, const char *strButtonName, const char *strDisplayName)
{
  CTString strIntButtonName(0, "Button %d", iButton);

  // Use automatic names
  if (strButtonName  == NULL) strButtonName  = strIntButtonName;
  if (strDisplayName == NULL) strDisplayName = strButtonName;

  _pInput->inp_strButtonNames   [iFirstButton + iButton] = _strJoyInt + strButtonName;
  _pInput->inp_strButtonNamesTra[iFirstButton + iButton] = _strJoyTra + strDisplayName;
};

// Adds axis and buttons for given joystick
void CInputPatch::P_AddJoystickAbbilities(INDEX iSlot) {
  // [Cecil] NOTE: Internal names remain intact with the engine in order to read vanilla controls
  // and, for example, remap "Joy 1 Button 0" to "[C1] A", the latter being purely a display name
  _strJoyInt.PrintF("Joy %d ", iSlot + 1);
  _strJoyTra.PrintF("[C%d] ", iSlot + 1);

  // Set proper names for axes
  const INDEX iFirstAxis = EIA_CONTROLLER_OFFSET + iSlot * SDL_GAMEPAD_AXIS_COUNT;

  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_LEFTX,         "Axis X", "L Stick X");
  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_LEFTY,         "Axis Y", "L Stick Y");
  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_RIGHTX,        "Axis Z", "R Stick X");
  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_RIGHTY,        "Axis R", "R Stick Y");
  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  "Axis U", "L2");
  SetAxisNames(iFirstAxis, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, "Axis V", "R2");

  const INDEX iFirstButton = FIRST_JOYBUTTON + iSlot * SDL_GAMEPAD_BUTTON_COUNT;

  // Set default names for all buttons
  for (INDEX iButton = 0; iButton < SDL_GAMEPAD_BUTTON_COUNT; iButton++) {
    SetButtonNames(iFirstButton, iButton, NULL, NULL);
  }

  // Set proper names for buttons
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_SOUTH,          NULL,    "A");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_EAST,           NULL,    "B");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_WEST,           NULL,    "X");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_NORTH,          NULL,    "Y");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_BACK,           NULL,    "Back");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_GUIDE,          NULL,    "Guide");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_START,          NULL,    "Start");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_LEFT_STICK,     NULL,    "L3");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_RIGHT_STICK,    NULL,    "R3");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  NULL,    "L1");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, NULL,    "R1");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_DPAD_UP,        "POV N", "D-pad Up");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_DPAD_DOWN,      "POV S", "D-pad Down");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_DPAD_LEFT,      "POV W", "D-pad Left");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_DPAD_RIGHT,     "POV E", "D-pad Right");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC1,          NULL,    "Misc 1");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,  NULL,    "R4");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,   NULL,    "L4");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,  NULL,    "R5");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,   NULL,    "L5");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC2,          NULL,    "Misc 2");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC3,          NULL,    "Misc 3");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC4,          NULL,    "Misc 4");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC5,          NULL,    "Misc 5");
  SetButtonNames(iFirstButton, SDL_GAMEPAD_BUTTON_MISC6,          NULL,    "Misc 6");
};

// Scans axis and buttons for given joystick
BOOL CInputPatch::P_ScanJoystick(INDEX iSlot, BOOL bPreScan) {
  SDL_Gamepad *pController = inp_aControllers[iSlot].handle;

  const INDEX iFirstAxis = CECIL_FIRST_AXIS_ACTION + EIA_CONTROLLER_OFFSET + iSlot * SDL_GAMEPAD_AXIS_COUNT;

  // For each available axis
  for (INDEX iAxis = 0; iAxis < SDL_GAMEPAD_AXIS_COUNT; iAxis++) {
    const INDEX iCurAxis = iFirstAxis + iAxis;
    InputDeviceAction &ida = inp_aInputActions[iCurAxis];

    // If the axis is not present
    if (!ida.ida_bExists) {
      // Read as zero and skip to the next one
      InputDeviceAction::SetReading(iCurAxis, 0.0);
      continue;
    }

    // [Cecil] FIXME: I cannot put a bigger emphasis on why it's important to reset readings during pre-scanning here.
    // If this isn't done and the sticks are being used for the camera rotation, the rotation speed changes drastically
    // depending on the current FPS, either making it suddenly too fast or too slow.
    // It doesn't affect the speed only when the maximum FPS is limited by using sam_iMaxFPSActive command.
    //
    // But even when you reset it, there's still a noticable choppiness that sometimes happens during view rotation if
    // the maximum FPS is set too high because the reading is only being set once every CTimer::TickQuantum seconds.
    //
    // I have tried multiple methods to try and solve it, even multipling the reading by the time difference between
    // calling this function (per axis), but it always ended up broken. This is the most stable fix I could figure out.
    if (bPreScan) {
      InputDeviceAction::SetReading(iCurAxis, 0.0);
      continue;
    }

    // Read its state
    SLONG slAxisReading = SDL_GetGamepadAxis(pController, (SDL_GamepadAxis)iAxis);

    // Set current axis value from -1 to +1
    const DOUBLE fCurrentValue = DOUBLE(slAxisReading - SDL_JOYSTICK_AXIS_MIN);
    const DOUBLE fMaxValue = DOUBLE(SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN);

    InputDeviceAction::SetReading(iCurAxis, fCurrentValue / fMaxValue * 2.0 - 1.0);
  }

  if (!bPreScan) {
    const INDEX iFirstButton = FIRST_JOYBUTTON + iSlot * SDL_GAMEPAD_BUTTON_COUNT;

    // For each available button
    for (INDEX iButton = 0; iButton < SDL_GAMEPAD_BUTTON_COUNT; iButton++) {
      // Test if the button is pressed
      const BOOL bJoyButtonPressed = SDL_GetGamepadButton(pController, (SDL_GamepadButton)iButton);

      if (bJoyButtonPressed) {
        InputDeviceAction::SetReading(iFirstButton + iButton, 1);
      } else {
        InputDeviceAction::SetReading(iFirstButton + iButton, 0);
      }
    }
  }

  // [Cecil] NOTE: Return value doesn't matter, this is purely for compatibility
  return TRUE;
};

// [Cecil] Get input from joysticks
void CInputPatch::P_PollJoysticks(BOOL bPreScan) {
  static CSymbolPtr pbForcePolling("inp_bForceJoystickPolling");
  static CSymbolPtr piJoysticksAllowed("inp_ctJoysticksAllowed");

  // Only if joystick polling is enabled or forced
  if (!inp_bPollJoysticks && !pbForcePolling.GetIndex()) return;

  // Scan states of all available joysticks
  const INDEX ct = inp_aControllers.Count();

  for (INDEX i = 0; i < ct; i++) {
    if (!inp_aControllers[i].IsConnected() || i >= piJoysticksAllowed.GetIndex()) continue;

    P_ScanJoystick(i, bPreScan);
  }
};
