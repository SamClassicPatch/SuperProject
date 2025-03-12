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

#define APPLICATION_NAME "SeriousSam"

// [Cecil] Moved from SeriousSam.cpp
#ifndef WM_MOUSEWHEEL
  #define WM_MOUSEWHEEL 0x020A
#endif

// [Cecil] Renamed and declared here
extern CDrawPort *_pdpMenu;
extern CDrawPort *_pdpNormal;
extern CViewPort *_pvpViewPort;

extern HINSTANCE _hInstance;
extern BOOL _bRunning, _bQuitScreen;

extern INDEX sam_iGfxAPI;
// [Cecil] 0 - speed; 1 - normal; 2 - quality; 3 - extreme; >4 - custom
extern INDEX sam_iVideoSetup;
extern BOOL sam_bAutoAdjustAudio;

// [Cecil] Computer screen resolution
extern PIX2D _vpixScreenRes;

// [Cecil] Directory with addon scripts
#if SE1_GAME != SS_REV
  #define SCRIPTS_ADDONS_DIR "Scripts\\Addons\\"
#else
  #define SCRIPTS_ADDONS_DIR "Scripts\\AddonsSE\\"
#endif

// [Cecil] Mouse wheel directions as buttons
#define MOUSEWHEEL_DN 10
#define MOUSEWHEEL_UP 11

// [Cecil] Declared here to be widely accessible
void RunBrowser(const char *strUrl, BOOL bUseSteamOverlay = FALSE);

// [Cecil] Different window modes
void StartNewMode(GfxAPIType eAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ, DisplayDepth eColorDepth, INDEX iWindowMode);

struct KeyData {
  char *kd_strASCII;
};

// [Cecil] Set new LCD drawport for Game
void SetDrawportForGame(CDrawPort *pdpSet);
