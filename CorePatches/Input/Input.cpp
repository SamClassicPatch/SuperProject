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

#include "StdH.h"

#include "Input.h"

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_EXTEND_INPUT

#pragma comment(lib, "SDL2.lib")

/*

NOTE: Three different types of key codes are used here:
  1) kid - engine internal type - defined in KeyNames.h
  2) scancode - raw PC keyboard scancodes as returned in keydown/keyup messages
  3) virtkey - virtual key codes used by Windows or SDL

*/

// name that is not translated (international)
#define INTNAME(str) str, ""
// name that is translated
#define TRANAME(str) str, str

// basic key conversion table
struct KeyConversion {
  INDEX kc_iKID;
  INDEX kc_iVirtKey;

  // [Cecil] NOTE: Some keys are offsetted by 256 to account for the "extended" flag that's being set
  // near scancodes when receiving key messages on Windows, otherwise it's impossible to distinguish
  // certain keys, e.g. Left Alt/Right Alt or Enter/Num Enter. As a result, the received scancode is
  // determined with a 0x1FF mask instead of just 0xFF. These scancodes are useless with SDL because
  // SDL is capable of distinguishing the keys when converting between its virtual keys and scancodes.
  INDEX kc_iScanCode;

  const char *kc_strName;
  const char *kc_strNameTrans;
};

static const KeyConversion _akcKeys[] = {
  // reserved for 'no-key-pressed'
  { KID_NONE, -1, -1, TRANAME("None")},

  // numbers row
  { KID_1               , SE1K_1,       2, INTNAME("1")},
  { KID_2               , SE1K_2,       3, INTNAME("2")},
  { KID_3               , SE1K_3,       4, INTNAME("3")},
  { KID_4               , SE1K_4,       5, INTNAME("4")},
  { KID_5               , SE1K_5,       6, INTNAME("5")},
  { KID_6               , SE1K_6,       7, INTNAME("6")},
  { KID_7               , SE1K_7,       8, INTNAME("7")},
  { KID_8               , SE1K_8,       9, INTNAME("8")},
  { KID_9               , SE1K_9,      10, INTNAME("9")},
  { KID_0               , SE1K_0,      11, INTNAME("0")},
  { KID_MINUS           , SE1K_MINUS,  12, INTNAME("-")},
  { KID_EQUALS          , SE1K_EQUALS, 13, INTNAME("=")},

  // 1st alpha row
  { KID_Q               , SE1K_q,            16, INTNAME("Q")},
  { KID_W               , SE1K_w,            17, INTNAME("W")},
  { KID_E               , SE1K_e,            18, INTNAME("E")},
  { KID_R               , SE1K_r,            19, INTNAME("R")},
  { KID_T               , SE1K_t,            20, INTNAME("T")},
  { KID_Y               , SE1K_y,            21, INTNAME("Y")},
  { KID_U               , SE1K_u,            22, INTNAME("U")},
  { KID_I               , SE1K_i,            23, INTNAME("I")},
  { KID_O               , SE1K_o,            24, INTNAME("O")},
  { KID_P               , SE1K_p,            25, INTNAME("P")},
  { KID_LBRACKET        , SE1K_LEFTBRACKET,  26, INTNAME("[")},
  { KID_RBRACKET        , SE1K_RIGHTBRACKET, 27, INTNAME("]")},
  { KID_BACKSLASH       , SE1K_BACKSLASH,    43, INTNAME("\\")},

  // 2nd alpha row
  { KID_A               , SE1K_a,         30, INTNAME("A")},
  { KID_S               , SE1K_s,         31, INTNAME("S")},
  { KID_D               , SE1K_d,         32, INTNAME("D")},
  { KID_F               , SE1K_f,         33, INTNAME("F")},
  { KID_G               , SE1K_g,         34, INTNAME("G")},
  { KID_H               , SE1K_h,         35, INTNAME("H")},
  { KID_J               , SE1K_j,         36, INTNAME("J")},
  { KID_K               , SE1K_k,         37, INTNAME("K")},
  { KID_L               , SE1K_l,         38, INTNAME("L")},
  { KID_SEMICOLON       , SE1K_SEMICOLON, 39, INTNAME(";")},
  { KID_APOSTROPHE      , SE1K_QUOTE,     40, INTNAME("'")},

  // 3rd alpha row
  { KID_Z               , SE1K_z,      44, INTNAME("Z")},
  { KID_X               , SE1K_x,      45, INTNAME("X")},
  { KID_C               , SE1K_c,      46, INTNAME("C")},
  { KID_V               , SE1K_v,      47, INTNAME("V")},
  { KID_B               , SE1K_b,      48, INTNAME("B")},
  { KID_N               , SE1K_n,      49, INTNAME("N")},
  { KID_M               , SE1K_m,      50, INTNAME("M")},
  { KID_COMMA           , SE1K_COMMA,  51, INTNAME(",")},
  { KID_PERIOD          , SE1K_PERIOD, 52, INTNAME(".")},
  { KID_SLASH           , SE1K_SLASH,  53, INTNAME("/")},

  // row with F-keys
  { KID_F1              , SE1K_F1,   59, INTNAME("F1")},
  { KID_F2              , SE1K_F2,   60, INTNAME("F2")},
  { KID_F3              , SE1K_F3,   61, INTNAME("F3")},
  { KID_F4              , SE1K_F4,   62, INTNAME("F4")},
  { KID_F5              , SE1K_F5,   63, INTNAME("F5")},
  { KID_F6              , SE1K_F6,   64, INTNAME("F6")},
  { KID_F7              , SE1K_F7,   65, INTNAME("F7")},
  { KID_F8              , SE1K_F8,   66, INTNAME("F8")},
  { KID_F9              , SE1K_F9,   67, INTNAME("F9")},
  { KID_F10             , SE1K_F10,  68, INTNAME("F10")},
  { KID_F11             , SE1K_F11,  87, INTNAME("F11")},
  { KID_F12             , SE1K_F12,  88, INTNAME("F12")},

  // extra keys
  { KID_ESCAPE          , SE1K_ESCAPE,     1, TRANAME("Escape")},
  { KID_TILDE           , -1,             41, TRANAME("Tilde")},
  { KID_BACKSPACE       , SE1K_BACKSPACE, 14, TRANAME("Backspace")},
  { KID_TAB             , SE1K_TAB,       15, TRANAME("Tab")},
  { KID_CAPSLOCK        , SE1K_CAPSLOCK,  58, TRANAME("Caps Lock")},
  { KID_ENTER           , SE1K_RETURN,    28, TRANAME("Enter")},
  { KID_SPACE           , SE1K_SPACE,     57, TRANAME("Space")},

  // modifier keys
  { KID_LSHIFT          , SE1K_LSHIFT, 42,     TRANAME("Left Shift")},
  { KID_RSHIFT          , SE1K_RSHIFT, 54,     TRANAME("Right Shift")},
  { KID_LCONTROL        , SE1K_LCTRL,  29,     TRANAME("Left Control")},
  { KID_RCONTROL        , SE1K_RCTRL,  29+256, TRANAME("Right Control")},
  { KID_LALT            , SE1K_LALT,   56,     TRANAME("Left Alt")},
  { KID_RALT            , SE1K_RALT,   56+256, TRANAME("Right Alt")},

  // navigation keys
  { KID_ARROWUP         , SE1K_UP,           72+256, TRANAME("Arrow Up")},
  { KID_ARROWDOWN       , SE1K_DOWN,         80+256, TRANAME("Arrow Down")},
  { KID_ARROWLEFT       , SE1K_LEFT,         75+256, TRANAME("Arrow Left")},
  { KID_ARROWRIGHT      , SE1K_RIGHT,        77+256, TRANAME("Arrow Right")},
  { KID_INSERT          , SE1K_INSERT,       82+256, TRANAME("Insert")},
  { KID_DELETE          , SE1K_DELETE,       83+256, TRANAME("Delete")},
  { KID_HOME            , SE1K_HOME,         71+256, TRANAME("Home")},
  { KID_END             , SE1K_END,          79+256, TRANAME("End")},
  { KID_PAGEUP          , SE1K_PAGEUP,       73+256, TRANAME("Page Up")},
  { KID_PAGEDOWN        , SE1K_PAGEDOWN,     81+256, TRANAME("Page Down")},
  { KID_PRINTSCR        , SE1K_PRINTSCREEN,  55+256, TRANAME("Print Screen")},
  { KID_SCROLLLOCK      , SE1K_SCROLLLOCK,   70,     TRANAME("Scroll Lock")},
  { KID_PAUSE           , SE1K_PAUSE,        69,     TRANAME("Pause")},

  // numpad numbers
  { KID_NUM0            , SE1K_KP_0,      82, INTNAME("Num 0")},
  { KID_NUM1            , SE1K_KP_1,      79, INTNAME("Num 1")},
  { KID_NUM2            , SE1K_KP_2,      80, INTNAME("Num 2")},
  { KID_NUM3            , SE1K_KP_3,      81, INTNAME("Num 3")},
  { KID_NUM4            , SE1K_KP_4,      75, INTNAME("Num 4")},
  { KID_NUM5            , SE1K_KP_5,      76, INTNAME("Num 5")},
  { KID_NUM6            , SE1K_KP_6,      77, INTNAME("Num 6")},
  { KID_NUM7            , SE1K_KP_7,      71, INTNAME("Num 7")},
  { KID_NUM8            , SE1K_KP_8,      72, INTNAME("Num 8")},
  { KID_NUM9            , SE1K_KP_9,      73, INTNAME("Num 9")},
  { KID_NUMDECIMAL      , SE1K_KP_PERIOD, 83, INTNAME("Num .")},

  // numpad gray keys
  { KID_NUMLOCK         , SE1K_NUMLOCKCLEAR, 69+256, INTNAME("Num Lock")},
  { KID_NUMSLASH        , SE1K_KP_DIVIDE,    53+256, INTNAME("Num /")},
  { KID_NUMMULTIPLY     , SE1K_KP_MULTIPLY,  55,     INTNAME("Num *")},
  { KID_NUMMINUS        , SE1K_KP_MINUS,     74,     INTNAME("Num -")},
  { KID_NUMPLUS         , SE1K_KP_PLUS,      78,     INTNAME("Num +")},
  { KID_NUMENTER        , SE1K_KP_ENTER,     28+256, TRANAME("Num Enter")},

  // mouse buttons
  { KID_MOUSE1          , SE1K_LBUTTON,  -1, TRANAME("Mouse Button 1")},
  { KID_MOUSE2          , SE1K_RBUTTON,  -1, TRANAME("Mouse Button 2")},
  { KID_MOUSE3          , SE1K_MBUTTON,  -1, TRANAME("Mouse Button 3")},
  { KID_MOUSE4          , SE1K_XBUTTON1, -1, TRANAME("Mouse Button 4")},
  { KID_MOUSE5          , SE1K_XBUTTON2, -1, TRANAME("Mouse Button 5")},
  { KID_MOUSEWHEELUP    , -1, -1, TRANAME("Mouse Wheel Up")},
  { KID_MOUSEWHEELDOWN  , -1, -1, TRANAME("Mouse Wheel Down")},

  // 2nd mouse buttons
  { KID_2MOUSE1         , -1, -1, TRANAME("2nd Mouse Button 1")},
  { KID_2MOUSE2         , -1, -1, TRANAME("2nd Mouse Button 2")},
  { KID_2MOUSE3         , -1, -1, TRANAME("2nd Mouse Button 3")},
};

static const size_t _ctKeyArray = ARRAYCOUNT(_akcKeys);

// autogenerated fast conversion tables
static INDEX _aiScanToKid[SDL_NUM_SCANCODES];

// make fast conversion tables from the general table
static void MakeConversionTables(void) {
  INDEX i;

  // Clear conversion table
  for (i = 0; i < ARRAYCOUNT(_aiScanToKid); i++) {
    _aiScanToKid[i] = -1;
  }

  for (i = 0; i < _ctKeyArray; i++) {
    const KeyConversion &kc = _akcKeys[i];

    const INDEX iKID  = kc.kc_iKID;
    const INDEX iVirt = kc.kc_iVirtKey;

    if (iVirt >= 0) {
      // [Cecil] NOTE: Cannot use MapVirtualKey() here; see comment near kc_iScanCode field
      INDEX iScan = kc.kc_iScanCode;

      _aiScanToKid[iScan] = iKID;
    }
  }

  // In case some keycode couldn't be mapped to a scancode
  _aiScanToKid[SDL_SCANCODE_UNKNOWN] = -1;
};

// variables for message interception
static int _iMouseZ = 0;
static BOOL _bWheelUp = FALSE;
static BOOL _bWheelDn = FALSE;

CTCriticalSection csInput;

// which keys are pressed, as recorded by message interception (by KIDs)
static UBYTE _abKeysPressed[256];

static HHOOK _hGetMsgHook = NULL;
static HHOOK _hSendMsgHook = NULL;

// Set key state according to the key message
static void SetKeyFromMsg(MSG *pMsg, BOOL bDown) {
  // Get key ID from scan code
  const INDEX iScan = (pMsg->lParam >> 16) & 0x1FF; // Use extended bit too!
  const INDEX iKID = _aiScanToKid[iScan];

  if (iKID != -1) {
    //CPrintF("%s: %d\n", _pInput->inp_strButtonNames[iKID], bDown);
    _abKeysPressed[iKID] = bDown;
  }
};

static void CheckMessage(MSG *pMsg)
{
  if (pMsg->message == WM_LBUTTONUP) {
    _abKeysPressed[KID_MOUSE1] = FALSE;
  } else if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE1] = TRUE;

  } else if (pMsg->message == WM_RBUTTONUP) {
    _abKeysPressed[KID_MOUSE2] = FALSE;
  } else if (pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_RBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE2] = TRUE;

  } else if (pMsg->message == WM_MBUTTONUP) {
    _abKeysPressed[KID_MOUSE3] = FALSE;
  } else if (pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_MBUTTONDBLCLK) {
    _abKeysPressed[KID_MOUSE3] = TRUE;

  // [Cecil] Proper support for MB4 and MB5
  } else if (pMsg->message == WM_XBUTTONUP) {
    if (GET_XBUTTON_WPARAM(pMsg->wParam) & XBUTTON1) {
      _abKeysPressed[KID_MOUSE4] = FALSE;
    }
    if (GET_XBUTTON_WPARAM(pMsg->wParam) & XBUTTON2) {
      _abKeysPressed[KID_MOUSE5] = FALSE;
    }

  } else if (pMsg->message == WM_XBUTTONDOWN || pMsg->message == WM_XBUTTONDBLCLK) {
    if (GET_XBUTTON_WPARAM(pMsg->wParam) & XBUTTON1) {
      _abKeysPressed[KID_MOUSE4] = TRUE;
    }
    if (GET_XBUTTON_WPARAM(pMsg->wParam) & XBUTTON2) {
      _abKeysPressed[KID_MOUSE5] = TRUE;
    }

  } else if (pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP) {
    SetKeyFromMsg(pMsg, FALSE);

  } else if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) {
    SetKeyFromMsg(pMsg, TRUE);
  }
};

// Procedure called when message is retreived
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
  MSG *pMsg = reinterpret_cast<MSG *>(lParam);
  CheckMessage(pMsg);

  LRESULT r = CallNextHookEx(_hGetMsgHook, nCode, wParam, lParam);

  if (wParam == PM_NOREMOVE) {
    return r;
  }

  if (pMsg->message == WM_MOUSEWHEEL) {
    _iMouseZ += SWORD(UWORD(HIWORD(pMsg->wParam)));
  }

  return r;
};

// Procedure called when message is sent
LRESULT CALLBACK SendMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
  MSG *pMsg = reinterpret_cast<MSG *>(lParam);
  CheckMessage(pMsg);

  return CallNextHookEx(_hSendMsgHook, nCode, wParam, lParam);
};

// [Cecil] All possible actions that can be used as controls
InputDeviceAction inp_aInputActions[MAX_OVERALL_BUTTONS];

// [Cecil] Game controllers
CStaticArray<GameController_t> inp_aControllers;

// [Cecil] Threshold for moving any axis to consider it as being "held down"
FLOAT inp_fAxisPressThreshold = 0.2f;

// [Cecil]
void InputDeviceAction::SetReading(INDEX iActionIndex, DOUBLE fSetReading) {
  // Set internal reading
  InputDeviceAction &ida = inp_aInputActions[iActionIndex];
  ida.ida_fReading = fSetReading;

  // Set vanilla states for compatibility
  BOOL bSetButtonState = FALSE;
  const INDEX iAxisOffset = CECIL_FIRST_AXIS_ACTION;

  // Axis actions
  if (iActionIndex >= iAxisOffset) {
    // Set axis reading
    _pInput->inp_caiAllAxisInfo[iActionIndex - iAxisOffset].cai_fReading = ida.ida_fReading;

    // Set button state only for controller axes
    if (iActionIndex >= iAxisOffset + EIA_CONTROLLER_OFFSET && iActionIndex < iAxisOffset + EIA_MAX_ALL) {
      bSetButtonState = ida.IsActive(inp_fAxisPressThreshold);
    }

  // Buttons
  } else {
    bSetButtonState = ida.IsActive();
  }

  // Set button state
  _pInput->inp_ubButtonsBuffer[iActionIndex] = (bSetButtonState ? 0xFF : 0x00);
};

// [Cecil]
bool InputDeviceAction::IsActive(DOUBLE fThreshold) const {
  return Abs(ida_fReading) >= Clamp(fThreshold, 0.01, 1.0);
};

// [Cecil] State switch
static BOOL _bPatchedInput = FALSE;

// deafult constructor
void CInputPatch::Construct(void)
{
  if (IsInitialized()) return;
  _bPatchedInput = TRUE;

  // [Cecil] Compatibility checks
  ASSERT(EIA_NONE == AXIS_NONE);
  ASSERT(EIA_MOUSE_X == MOUSE_X_AXIS);
  ASSERT(EIA_MOUSE_Y == MOUSE_Y_AXIS);

  // There should be the exact same amount of mouse & controller axes
  ASSERT(SDL_CONTROLLER_AXIS_MAX == MAX_AXES_PER_JOYSTICK);
  ASSERT(EIA_CONTROLLER_OFFSET == FIRST_JOYAXIS);
  ASSERT(EIA_MAX_ALL == MAX_OVERALL_AXES);

  ASSERT(SDL_CONTROLLER_BUTTON_MAX <= MAX_BUTTONS_PER_JOYSTICK); // Less controller buttons than joystick ones
  ASSERT(CECIL_MAX_OVERALL_BUTTONS <= MAX_OVERALL_BUTTONS); // Controller buttons shouldn't exceed the array

  // All controller buttons with axes should fit within the button array
  ASSERT(CECIL_MAX_OVERALL_BUTTONS + EIA_MAX_ALL <= MAX_OVERALL_BUTTONS);

  // [Cecil] Initialize SDL
  if (SDL_Init(SDL_INIT_GAMECONTROLLER) == -1) {
    CPrintF("SDL_Init(SDL_INIT_GAMECONTROLLER) failed:\n%s\n", SDL_GetError());
  }

  // disable control scaning
  _pInput->inp_bInputEnabled = FALSE;
  _pInput->inp_bPollJoysticks = FALSE;
  _pInput->inp_bLastPrescan = FALSE;

  // [Cecil] Clear all actions
  for (INDEX iAction = 0; iAction < MAX_OVERALL_BUTTONS; iAction++) {
    InputDeviceAction::SetReading(iAction, 0);
    inp_aInputActions[iAction].ida_bExists = (iAction < CECIL_FIRST_AXIS_ACTION); // Axes are disabled by default
  }

  MakeConversionTables();
};

// Destructor
void CInputPatch::Destruct()
{
  if (!IsInitialized()) return;
  _bPatchedInput = FALSE;

  // [Cecil] Various cleanups
  ShutdownJoysticks();
  Mouse2_Clear();

  // [Cecil] End SDL
  SDL_Quit();
};

// [Cecil] Check if input patch has been initialized
BOOL CInputPatch::IsInitialized(void) {
  return _bPatchedInput;
};

/*
 * Sets names of keys on keyboard
 */
void CInputPatch::P_SetKeyNames( void)
{
  // [Cecil] Reset names of all actions
  for (INDEX iResetAction = 0; iResetAction < MAX_OVERALL_BUTTONS; iResetAction++) {
    CTString &strInt = inp_strButtonNames[iResetAction];
    CTString &strTra = inp_strButtonNamesTra[iResetAction];

    // [Cecil] FIXME: For some weird freaking reason every 5th string starting from 1 (index 1, 6, 11, 16 etc.)
    // is set to NULL instead of "" (or "None" at the very least), which was supposed to be set by default.
    // It spams the assertions on each such string in Debug, and it can be verified with this check in Release.
    if (strInt.str_String == NULL) strInt.str_String = StringDuplicate("");

    strInt = "None";
    strTra = LOCALIZE("None");
  }

  // [Cecil] Reset old axis names for compatibility
  for (INDEX iResetOldAxis = 0; iResetOldAxis < MAX_OVERALL_AXES; iResetOldAxis++) {
    inp_caiAllAxisInfo[iResetOldAxis].cai_strAxisName = "None";
  }

  // Set names of key IDs
  for (INDEX iKey = 0; iKey < _ctKeyArray; iKey++) {
    const KeyConversion &kc = _akcKeys[iKey];
    if (kc.kc_strName == NULL) continue;

    inp_strButtonNames[kc.kc_iKID] = kc.kc_strName;
    CTString &strTranslated = inp_strButtonNamesTra[kc.kc_iKID];

    if (strlen(kc.kc_strNameTrans) == 0) {
      strTranslated = kc.kc_strName;
    } else {
      strTranslated = LOCALIZE(kc.kc_strNameTrans);
    }
  }

  // Set names of mouse axes
  #define SET_AXIS_NAME(_Axis, _Name) \
    inp_strButtonNames   [CECIL_FIRST_AXIS_ACTION + _Axis] = _Name; \
    inp_strButtonNamesTra[CECIL_FIRST_AXIS_ACTION + _Axis] = LOCALIZE(_Name); \
    /* [Cecil] Old name for compatibility */ \
    inp_caiAllAxisInfo[_Axis].cai_strAxisName = _Name;

  SET_AXIS_NAME(EIA_MOUSE_X,  "mouse X");
  SET_AXIS_NAME(EIA_MOUSE_Y,  "mouse Y");
  SET_AXIS_NAME(EIA_MOUSE_Z,  "mouse Z");
  SET_AXIS_NAME(EIA_MOUSE2_X, "2nd mouse X");
  SET_AXIS_NAME(EIA_MOUSE2_Y, "2nd mouse Y");

  #undef SET_AXIS_NAME

  // [Cecil] Set joystick names
  const INDEX ct = inp_aControllers.Count();

  for (INDEX i = 0; i < ct; i++) {
    P_AddJoystickAbbilities(i);
  }
}

/*
 * Initializes all available devices and enumerates available controls
 */
void CInputPatch::P_Initialize( void )
{
  // [Cecil] Various initializations
  StartupJoysticks();
  Mouse2_Clear();

  P_SetKeyNames();
}

// Enable direct input
void CInputPatch::P_EnableInput(HWND hwnd)
{
  // skip if already enabled
  if( inp_bInputEnabled) return;

  HWND hwndCurrent = hwnd;

  // get window rectangle
  RECT rectClient;
  GetClientRect(hwndCurrent, &rectClient);
  POINT pt;
  pt.x = pt.y = 0;
  ClientToScreen(hwndCurrent, &pt);
  OffsetRect(&rectClient, pt.x, pt.y);

  // remember mouse pos
  GetCursorPos(&inp_ptOldMousePos);

  // set mouse clip region
  ClipCursor(&rectClient);
  // determine screen center position
  inp_slScreenCenterX = (rectClient.left + rectClient.right) / 2;
  inp_slScreenCenterY = (rectClient.top + rectClient.bottom) / 2;

  // clear mouse from screen
  while (ShowCursor(FALSE) >= 0);
  // save system mouse settings
  SystemParametersInfo(SPI_GETMOUSE, 0, &inp_mscMouseSettings, 0);

  // set new mouse speed
  static CSymbolPtr pbAllowAcceleration("inp_bAllowMouseAcceleration");

  if (!pbAllowAcceleration.GetIndex()) {
    MouseSpeedControl mscNewSetting = { 0, 0, 0 };
    SystemParametersInfo(SPI_SETMOUSE, 0, &mscNewSetting, 0);
  }

  // set cursor position to screen center
  SetCursorPos(inp_slScreenCenterX, inp_slScreenCenterY);

  _hGetMsgHook  = SetWindowsHookEx(WH_GETMESSAGE, &GetMsgProc, NULL, GetCurrentThreadId());
  _hSendMsgHook = SetWindowsHookEx(WH_CALLWNDPROC, &SendMsgProc, NULL, GetCurrentThreadId());

  // if required, try to enable 2nd mouse
  Mouse2_Startup(); // [Cecil]

  // clear button's buffer
  memset( _abKeysPressed, 0, sizeof( _abKeysPressed));

  // remember current status
  inp_bInputEnabled = TRUE;
  inp_bPollJoysticks = FALSE;
}


/*
 * Disable direct input
 */
void CInputPatch::P_DisableInput( void)
{
  // skip if allready disabled
  if( !inp_bInputEnabled) return;

  UnhookWindowsHookEx(_hGetMsgHook);
  UnhookWindowsHookEx(_hSendMsgHook);

  // set mouse clip region to entire screen
  ClipCursor(NULL);
  // restore mouse pos
  SetCursorPos(inp_ptOldMousePos.x, inp_ptOldMousePos.y);

  // show mouse on screen
  while (ShowCursor(TRUE) < 0);
  // set system mouse settings
  SystemParametersInfo(SPI_SETMOUSE, 0, &inp_mscMouseSettings, 0);

  // eventually disable 2nd mouse
  Mouse2_Shutdown(); // [Cecil]

  // remember current status
  inp_bInputEnabled = FALSE;
  inp_bPollJoysticks = FALSE;
}

/*
 * Scan states of all available input sources
 */
void CInputPatch::P_GetInput(BOOL bPreScan)
{
  static CSymbolPtr pbAllowPrescan("inp_bAllowPrescan");
  static CSymbolPtr piReadingMethod("inp_iKeyboardReadingMethod");
  static CSymbolPtr pfMouseSensitivity("inp_fMouseSensitivity");
  static CSymbolPtr pbAllowAcceleration("inp_bAllowMouseAcceleration");

  static CSymbolPtr pbPrecision("inp_bMousePrecision");
  static CSymbolPtr pfPrecisionThreshold("inp_fMousePrecisionThreshold");
  static CSymbolPtr pfPrecisionTimeout("inp_fMousePrecisionTimeout");
  static CSymbolPtr pfPrecisionFactor("inp_fMousePrecisionFactor");

  static CSymbolPtr pbFilterMouse("inp_bFilterMouse");
  static CSymbolPtr pbInvertMouse("inp_bInvertMouse");

  if (!inp_bInputEnabled) {
    return;
  }

  if (bPreScan && !pbAllowPrescan.GetIndex()) {
    return;
  }

  // if not pre-scanning
  if (!bPreScan) {
    // [Cecil] Reset key readings
    for (INDEX iResetAction = 0; iResetAction < CECIL_MAX_OVERALL_BUTTONS; iResetAction++) {
      InputDeviceAction::SetReading(iResetAction, 0);
    }

    // for each Key
    for (INDEX iKey = 0; iKey < _ctKeyArray; iKey++) {
      const KeyConversion &kc = _akcKeys[iKey];
      // get codes
      INDEX iKID  = kc.kc_iKID;
      INDEX iVirt = kc.kc_iVirtKey;

      // if reading async keystate
      if (piReadingMethod.GetIndex() == 0) {
        // if there is a valid virtkey
        if (iVirt >= 0) {
          BOOL bKeyPressed = FALSE;

          // transcribe if modifier
          if (iVirt == VK_LSHIFT)   iVirt = VK_SHIFT;
          if (iVirt == VK_LCONTROL) iVirt = VK_CONTROL;
          if (iVirt == VK_LMENU)    iVirt = VK_MENU;

          bKeyPressed = !!(::GetAsyncKeyState(iVirt) & 0x8000);

          // is state is pressed
          if (bKeyPressed) {
            // mark it as pressed
            InputDeviceAction::SetReading(iKID, 1);
          }
        }

      // if snooping messages
      } else if (_abKeysPressed[iKID]) {
        // mark it as pressed
        InputDeviceAction::SetReading(iKID, 1);
      }
    }
  }

  // read mouse position
  POINT pt;
  if (GetCursorPos(&pt))
  {
    FLOAT fDX = FLOAT(pt.x - inp_slScreenCenterX);
    FLOAT fDY = FLOAT(pt.y - inp_slScreenCenterY);

    FLOAT fSensitivity = pfMouseSensitivity.GetFloat();
    if (pbAllowAcceleration.GetIndex()) fSensitivity *= 0.25f;

    FLOAT fD = Sqrt(fDX*fDX+fDY*fDY);
    if (pbPrecision.GetIndex()) {
      static FLOAT _tmTime = 0.0f;
      if (fD < pfPrecisionThreshold.GetFloat()) _tmTime += 0.05f;
      else _tmTime = 0.0f;
      if (_tmTime > pfPrecisionTimeout.GetFloat()) fSensitivity /= pfPrecisionFactor.GetFloat();
    }

    static FLOAT fDXOld;
    static FLOAT fDYOld;
    static TIME tmOldDelta;
    static CTimerValue tvBefore;
    CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
    TIME tmNowDelta = (tvNow-tvBefore).GetSeconds();
    if (tmNowDelta<0.001f) {
      tmNowDelta = 0.001f;
    }
    tvBefore = tvNow;

    FLOAT fDXSmooth = (fDXOld*tmOldDelta+fDX*tmNowDelta)/(tmOldDelta+tmNowDelta);
    FLOAT fDYSmooth = (fDYOld*tmOldDelta+fDY*tmNowDelta)/(tmOldDelta+tmNowDelta);
    fDXOld = fDX;
    fDYOld = fDY;
    tmOldDelta = tmNowDelta;
    if (pbFilterMouse.GetIndex()) {
      fDX = fDXSmooth;
      fDY = fDYSmooth;
    }

    // get final mouse values
    FLOAT fMouseRelX = +fDX*fSensitivity;
    FLOAT fMouseRelY = -fDY*fSensitivity;
    if (pbInvertMouse.GetIndex()) {
      fMouseRelY = -fMouseRelY;
    }
    FLOAT fMouseRelZ = _iMouseZ;

    // just interpret values as normal
    InputDeviceAction::SetReading(CECIL_FIRST_AXIS_ACTION + EIA_MOUSE_X, fMouseRelX);
    InputDeviceAction::SetReading(CECIL_FIRST_AXIS_ACTION + EIA_MOUSE_Y, fMouseRelY);
    InputDeviceAction::SetReading(CECIL_FIRST_AXIS_ACTION + EIA_MOUSE_Z, fMouseRelZ);
  }

  // if not pre-scanning
  if (!bPreScan) {
    // [Cecil] FIXME: This whole block should be executed at all times and not just when input is needed,
    // otherwise it produces bugs where if you scroll the wheel in the menu a bunch of times, it will:
    // 1. Rapidly switch weapons as soon as you return to the game or...
    // 2. Keep auto-forcing "Mouse Wheel Up" or "Mouse Wheel Dn" to controls on any active key bind
    // until the accumulated _iMouseZ reaches zero
    _bWheelUp = _bWheelDn = FALSE; // [Cecil] TEMP: Reset states to prevent it from toggling each step

    // Detect wheel up/down movement
    if (_iMouseZ > 0) {
      if (_bWheelUp) {
        InputDeviceAction::SetReading(KID_MOUSEWHEELUP, 0);
      } else {
        InputDeviceAction::SetReading(KID_MOUSEWHEELUP, 1);
        _iMouseZ = ClampDn(_iMouseZ - MOUSEWHEEL_SCROLL_INTERVAL, 0);
      }
    }

    _bWheelUp = inp_aInputActions[KID_MOUSEWHEELUP].IsActive();

    if (_iMouseZ < 0) {
      if (_bWheelDn) {
        InputDeviceAction::SetReading(KID_MOUSEWHEELDOWN, 0);
      } else {
        InputDeviceAction::SetReading(KID_MOUSEWHEELDOWN, 1);
        _iMouseZ = ClampUp(_iMouseZ + MOUSEWHEEL_SCROLL_INTERVAL, 0);
      }
    }

    _bWheelDn = inp_aInputActions[KID_MOUSEWHEELDOWN].IsActive();
  }

  inp_bLastPrescan = bPreScan;

  // set cursor position to screen center
  if (pt.x != inp_slScreenCenterX || pt.y != inp_slScreenCenterY) {
    SetCursorPos(inp_slScreenCenterX, inp_slScreenCenterY);
  }

  // readout 2nd mouse if enabled
  Mouse2_Update(); // [Cecil]

  P_PollJoysticks(bPreScan); // [Cecil]
}

// Clear all input states (keys become not pressed, axes are reset to zero)
void CInputPatch::P_ClearInput(void)
{
  for (INDEX i = 0; i < MAX_OVERALL_BUTTONS; i++) {
    InputDeviceAction::SetReading(i, 0);
  }
};

#endif // _PATCHCONFIG_EXTEND_INPUT
