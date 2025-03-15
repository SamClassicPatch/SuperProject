/* Copyright (c) 2022-2025 Dreamy Cecil
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

#include "Input/Input.h"

// Define extension
CLASSICSPATCH_DEFINE_EXTENSION("PATCH_EXT_input", k_EPluginFlagGame | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Extended Input",
  "Extended input functionality that includes proper support of game controllers.");

// Threshold for moving any axis to consider it as being "held down"
CPluginSymbol _psAxisPressThreshold(SSF_PERSISTENT | SSF_USER, FLOAT(0.2f));

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Don't patch input
  if (!IConfig::global[k_EConfigProps_ExtendedInput]) return;

  // Custom symbols
  _psAxisPressThreshold.Register("inp_fAxisPressThreshold");
  GetPluginAPI()->RegisterMethod(TRUE, "void", "inp_JoysticksInfo", "void", &CInputPatch::PrintJoysticksInfo);

  // Initialization
  void (CInput::*pInitialize)(void) = &CInput::Initialize;
  CreatePatch(pInitialize, &CInputPatch::P_Initialize, "CInput::Initialize()");

  // Input
  void (CInput::*pEnableInput)(HWND) = &CInput::EnableInput;
  CreatePatch(pEnableInput, &CInputPatch::P_EnableInput, "CInput::EnableInput(HWND)");

  void (CInput::*pDisableInput)(void) = &CInput::DisableInput;
  CreatePatch(pDisableInput, &CInputPatch::P_DisableInput, "CInput::DisableInput()");

  void (CInput::*pGetInput)(BOOL) = &CInput::GetInput;
  CreatePatch(pGetInput, &CInputPatch::P_GetInput, "CInput::GetInput(...)");

  void (CInput::*pClearInput)(void) = &CInput::ClearInput;
  CreatePatch(pClearInput, &CInputPatch::P_ClearInput, "CInput::ClearInput()");

  // Joysticks
  void (CInput::*pSetKeyNames)(void) = &CInput::SetKeyNames;
  CreatePatch(pSetKeyNames, &CInputPatch::P_SetKeyNames, "CInput::SetKeyNames()");

  void (CInput::*pAddJoystickAbbilities)(INDEX) = &CInput::AddJoystickAbbilities;
  CreatePatch(pAddJoystickAbbilities, &CInputPatch::P_AddJoystickAbbilities, "CInput::AddJoystickAbbilities()");

  BOOL (CInput::*pScanJoystick)(INDEX, BOOL) = &CInput::ScanJoystick;
  CreatePatch(pScanJoystick, &CInputPatch::P_ScanJoystick, "CInput::ScanJoystick()");

  // Getters
  const CTString &(CInput::*pGetAxisTransName)(INDEX) const = &CInput::GetAxisTransName;
  CreatePatch(pGetAxisTransName, &CInputPatch::P_GetAxisTransName, "CInput::GetAxisTransName()");

  FLOAT (CInput::*pGetAxisValue)(INDEX) const = &CInput::GetAxisValue;
  CreatePatch(pGetAxisValue, &CInputPatch::P_GetAxisValue, "CInput::GetAxisValue()");

  // Initialize the new input system after patching the old one
  CInputPatch::Construct();
  _pInput->Initialize();
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
  CInputPatch::Destruct();
};
