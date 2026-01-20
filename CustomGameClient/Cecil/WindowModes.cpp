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

#include "StdH.h"

#include "WindowModes.h"
#include "MainWindow.h"

// Aspect ratio of the current resolution
FLOAT _fAspectRatio = (4.0f / 3.0f);

// Game application is DPI-aware
static BOOL _bDPIAware = FALSE;

// Make game application be aware of the DPI scaling on Windows Vista and later
void SetDPIAwareness(void) {
  // Ignore DPI awareness if it's disabled
  if (!IConfig::global[k_EConfigProps_DPIAware]) return;

  // Load the library
  HMODULE hUser = LoadLibraryA("User32.dll");

  if (hUser == NULL) return;

  // Try to find the DPI awareness method
  typedef BOOL (*CSetAwarenessFunc)(void);
  CSetAwarenessFunc pFunc = (CSetAwarenessFunc)GetProcAddress(hUser, "SetProcessDPIAware");

  if (pFunc == NULL) return;

  // Mark game application as DPI-aware
  _bDPIAware = pFunc();
};

// Open the main application window in borderless mode
void OpenMainWindowBorderless(PIX pixSizeI, PIX pixSizeJ) {
  ASSERT(_hwndMain == NULL);

  // Create an invisible window
  _hwndMain = CreateWindowExA(WS_EX_APPWINDOW, APPLICATION_NAME, "",
    WS_POPUP, 0, 0, pixSizeI, pixSizeJ, NULL, NULL, _hInstance, NULL);

  // Can't create the window
  if (_hwndMain == NULL) {
    // Append windows error message
    FatalError("OpenMainWindowBorderless(): %s\n%s", LOCALIZE("Cannot open main window!"), GetWindowsError(GetLastError()));
  }

  SE_UpdateWindowHandle(_hwndMain);

  // Set window title and show it
  sprintf(_achWindowTitle, TRANS("Serious Sam (Borderless %dx%d)"), pixSizeI, pixSizeJ);
  SetWindowTextA(_hwndMain, _achWindowTitle);

  _pixLastSizeI = pixSizeI;
  _pixLastSizeJ = pixSizeJ;

  ResetMainWindowNormal();
};
