/* Copyright (c) 2025 Dreamy Cecil
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

namespace sq {

namespace Input {

static SQInteger GetAxisCount(HSQUIRRELVM v) {
  sq_pushinteger(v, _pInput->GetAvailableAxisCount());
  return 1;
};

static SQInteger GetButtonsCount(HSQUIRRELVM v) {
  sq_pushinteger(v, _pInput->GetAvailableButtonsCount());
  return 1;
};

static SQInteger GetAxisName(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushstring(v, _pInput->GetAxisName(i).str_String, -1);
  return 1;
};

static SQInteger GetAxisTransName(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushstring(v, _pInput->GetAxisTransName(i).str_String, -1);
  return 1;
};

static SQInteger GetAxisValue(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushfloat(v, _pInput->GetAxisValue(i));
  return 1;
};

static SQInteger GetButtonName(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushstring(v, _pInput->GetButtonName(i).str_String, -1);
  return 1;
};

static SQInteger GetButtonTransName(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushstring(v, _pInput->GetButtonTransName(i).str_String, -1);
  return 1;
};

static SQInteger GetButtonState(HSQUIRRELVM v) {
  SQInteger i;
  sq_getinteger(v, 2, &i);
  sq_pushbool(v, _pInput->GetButtonState(i) ? SQTrue : SQFalse);
  return 1;
};

static SQInteger GetAxisByName(HSQUIRRELVM v) {
  const SQChar *str = "None";
  sq_getstring(v, 2, &str);

  SQInteger iAxis = AXIS_NONE;
  const INDEX ct = _pInput->GetAvailableAxisCount();

  for (INDEX i = 0; i < ct; i++) {
    if (_pInput->GetAxisName(i) == str) {
      iAxis = i;
      break;
    }
  }

  sq_pushinteger(v, iAxis);
  return 1;
};

static SQInteger GetButtonByName(HSQUIRRELVM v) {
  const SQChar *str = "None";
  sq_getstring(v, 2, &str);

  SQInteger iKey = KID_NONE;
  const INDEX ct = _pInput->GetAvailableButtonsCount();

  for (INDEX i = 0; i < ct; i++) {
    if (_pInput->GetButtonName(i) == str) {
      iKey = i;
      break;
    }
  }

  sq_pushinteger(v, iKey);
  return 1;
};

}; // namespace

// "Input" namespace functions
static SQRegFunction _aInputFuncs[] = {
  { "GetAxisCount",       &Input::GetAxisCount, 1, "." },
  { "GetButtonsCount",    &Input::GetButtonsCount, 1, "." },
  { "GetAxisName",        &Input::GetAxisName, 2, ".n" },
  { "GetAxisTransName",   &Input::GetAxisTransName, 2, ".n" },
  { "GetAxisValue",       &Input::GetAxisValue, 2, ".n" },
  { "GetButtonName",      &Input::GetButtonName, 2, ".n" },
  { "GetButtonTransName", &Input::GetButtonTransName, 2, ".n" },
  { "GetButtonState",     &Input::GetButtonState, 2, ".n" },
  { "GetAxisByName",      &Input::GetAxisByName, 2, ".s" },
  { "GetButtonByName",    &Input::GetButtonByName, 2, ".s" },
};

void VM::RegisterInput(void) {
  Table sqtInput = Root().AddTable("Input");

  // Register functions
  for (INDEX i = 0; i < ARRAYCOUNT(_aInputFuncs); i++) {
    sqtInput.RegisterFunc(_aInputFuncs[i]);
  }

  // Input buttons
  Enumeration enButtons(GetVM());

#define ADD_KEY(_Key) enButtons.SetValue(#_Key, (SQInteger)_Key)
  ADD_KEY(KID_NONE          );
  ADD_KEY(KID_1             );
  ADD_KEY(KID_2             );
  ADD_KEY(KID_3             );
  ADD_KEY(KID_4             );
  ADD_KEY(KID_5             );
  ADD_KEY(KID_6             );
  ADD_KEY(KID_7             );
  ADD_KEY(KID_8             );
  ADD_KEY(KID_9             );
  ADD_KEY(KID_0             );
  ADD_KEY(KID_MINUS         );
  ADD_KEY(KID_EQUALS        );
  ADD_KEY(KID_Q             );
  ADD_KEY(KID_W             );
  ADD_KEY(KID_E             );
  ADD_KEY(KID_R             );
  ADD_KEY(KID_T             );
  ADD_KEY(KID_Y             );
  ADD_KEY(KID_U             );
  ADD_KEY(KID_I             );
  ADD_KEY(KID_O             );
  ADD_KEY(KID_P             );
  ADD_KEY(KID_LBRACKET      );
  ADD_KEY(KID_RBRACKET      );
  ADD_KEY(KID_BACKSLASH     );
  ADD_KEY(KID_A             );
  ADD_KEY(KID_S             );
  ADD_KEY(KID_D             );
  ADD_KEY(KID_F             );
  ADD_KEY(KID_G             );
  ADD_KEY(KID_H             );
  ADD_KEY(KID_J             );
  ADD_KEY(KID_K             );
  ADD_KEY(KID_L             );
  ADD_KEY(KID_SEMICOLON     );
  ADD_KEY(KID_APOSTROPHE    );
  ADD_KEY(KID_Z             );
  ADD_KEY(KID_X             );
  ADD_KEY(KID_C             );
  ADD_KEY(KID_V             );
  ADD_KEY(KID_B             );
  ADD_KEY(KID_N             );
  ADD_KEY(KID_M             );
  ADD_KEY(KID_COMMA         );
  ADD_KEY(KID_PERIOD        );
  ADD_KEY(KID_SLASH         );
  ADD_KEY(KID_F1            );
  ADD_KEY(KID_F2            );
  ADD_KEY(KID_F3            );
  ADD_KEY(KID_F4            );
  ADD_KEY(KID_F5            );
  ADD_KEY(KID_F6            );
  ADD_KEY(KID_F7            );
  ADD_KEY(KID_F8            );
  ADD_KEY(KID_F9            );
  ADD_KEY(KID_F10           );
  ADD_KEY(KID_F11           );
  ADD_KEY(KID_F12           );
  ADD_KEY(KID_ESCAPE        );
  ADD_KEY(KID_TILDE         );
  ADD_KEY(KID_BACKSPACE     );
  ADD_KEY(KID_TAB           );
  ADD_KEY(KID_CAPSLOCK      );
  ADD_KEY(KID_ENTER         );
  ADD_KEY(KID_SPACE         );
  ADD_KEY(KID_LSHIFT        );
  ADD_KEY(KID_RSHIFT        );
  ADD_KEY(KID_LCONTROL      );
  ADD_KEY(KID_RCONTROL      );
  ADD_KEY(KID_LALT          );
  ADD_KEY(KID_RALT          );
  ADD_KEY(KID_ARROWUP       );
  ADD_KEY(KID_ARROWDOWN     );
  ADD_KEY(KID_ARROWLEFT     );
  ADD_KEY(KID_ARROWRIGHT    );
  ADD_KEY(KID_INSERT        );
  ADD_KEY(KID_DELETE        );
  ADD_KEY(KID_HOME          );
  ADD_KEY(KID_END           );
  ADD_KEY(KID_PAGEUP        );
  ADD_KEY(KID_PAGEDOWN      );
  ADD_KEY(KID_PRINTSCR      );
  ADD_KEY(KID_SCROLLLOCK    );
  ADD_KEY(KID_PAUSE         );
  ADD_KEY(KID_NUM0          );
  ADD_KEY(KID_NUM1          );
  ADD_KEY(KID_NUM2          );
  ADD_KEY(KID_NUM3          );
  ADD_KEY(KID_NUM4          );
  ADD_KEY(KID_NUM5          );
  ADD_KEY(KID_NUM6          );
  ADD_KEY(KID_NUM7          );
  ADD_KEY(KID_NUM8          );
  ADD_KEY(KID_NUM9          );
  ADD_KEY(KID_NUMDECIMAL    );
  ADD_KEY(KID_NUMLOCK       );
  ADD_KEY(KID_NUMSLASH      );
  ADD_KEY(KID_NUMMULTIPLY   );
  ADD_KEY(KID_NUMMINUS      );
  ADD_KEY(KID_NUMPLUS       );
  ADD_KEY(KID_NUMENTER      );
  ADD_KEY(KID_MOUSE1        );
  ADD_KEY(KID_MOUSE2        );
  ADD_KEY(KID_MOUSE3        );
  ADD_KEY(KID_MOUSE4        );
  ADD_KEY(KID_MOUSE5        );
  ADD_KEY(KID_MOUSEWHEELUP  );
  ADD_KEY(KID_MOUSEWHEELDOWN);
  ADD_KEY(KID_2MOUSE1       );
  ADD_KEY(KID_2MOUSE2       );
  ADD_KEY(KID_2MOUSE3       );
#undef ADD_KEY

  Const().AddEnum("Button", enButtons);

  // Input axes
  Enumeration enAxes(GetVM());

#define ADD_AXIS(_AxisName, _AxisValue) enAxes.SetValue(_AxisName, (SQInteger)_AxisValue)
  ADD_AXIS("None",          AXIS_NONE);
  ADD_AXIS("X1",            MOUSE_X_AXIS);
  ADD_AXIS("Y1",            MOUSE_Y_AXIS);
  ADD_AXIS("Z1",            3);
  ADD_AXIS("X2",            4);
  ADD_AXIS("Y2",            5);

  ADD_AXIS("Joysticks",     MAX_JOYSTICKS);
  ADD_AXIS("PerJoystick",   MAX_AXES_PER_JOYSTICK);
  ADD_AXIS("FirstJoystick", FIRST_JOYAXIS);
  ADD_AXIS("Count",         MAX_OVERALL_AXES);
#undef ADD_AXIS

  Const().AddEnum("Axis", enAxes);
};

}; // namespace
