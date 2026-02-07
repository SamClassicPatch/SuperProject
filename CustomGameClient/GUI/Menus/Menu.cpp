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

#include "StdH.h"
#include <sys/timeb.h>
#include <time.h>
#include <io.h>
#include "MainWindow.h"
#include <Engine/Templates/Stock_CSoundData.h>
#include "MenuPrinting.h"
#include "LevelInfo.h"
#include "VarList.h"
#include "FileInfo.h"

#include "MenuStuff.h"

// macros for translating radio button text arrays
#define TRANSLATERADIOARRAY(array) TranslateRadioTexts(array, ARRAYCOUNT(array))

extern BOOL bMenuActive;
extern BOOL bMenuRendering;
extern CTextureObject *_ptoLogoCT;
extern CTextureObject *_ptoLogoODI;
extern CTextureObject *_ptoLogoEAX;

INDEX _iLocalPlayer = -1;
extern BOOL _bPlayerMenuFromSinglePlayer = FALSE;

ERunningGameMode _gmMenuGameMode = GM_NONE;
CListHead _lhServers;

void OnPlayerSelect(void);

// last tick done
TIME _tmMenuLastTickDone = -1;
// all possible menu entities
CListHead lhMenuEntities;

extern CTString _strLastPlayerAppearance = "";
extern CTString sam_strNetworkSettings;

// function to activate when level is chosen
void (*_pAfterLevelChosen)(void);

// [Cecil] Rewind visited menus to this one before changing to some menu
CGameMenu *_pgmRewindToAfterLevelChosen = NULL; // Sets _pgmRewindTo before calling _pAfterLevelChosen()
CGameMenu *_pgmRewindTo = NULL;

// functions for init actions

static void FixupBackButton(CGameMenu *pgm);

// mouse cursor position
PIX _pixCursorPosI = 0;
PIX _pixCursorPosJ = 0;

extern BOOL _bMouseUsedLast = FALSE;
extern CMenuGadget *_pmgUnderCursor = NULL;

// thumbnail for showing in menu
CTextureObject _toThumbnail;
BOOL _bThumbnailOn = FALSE;

CFontData _fdBig;
CFontData _fdMedium;
CFontData _fdSmall;
CFontData _fdTitle;

CSoundData *_psdSelect = NULL;
CSoundData *_psdPress = NULL;
CSoundObject *_psoMenuSound = NULL;

static CTextureObject _toLogoMenuA;
static CTextureObject _toLogoMenuB;

// [Cecil] Classics Patch logo
static CTextureObject _toPatchLogo1;
static CTextureObject _toPatchLogo2;

// [Cecil] Scrollbar arrow texture
CTextureObject _toMenuArrow;

// -------------- All possible menu entities
#define BIG_BUTTONS_CT 6

#define CHANGETRIGGERARRAY(ltbmg, astr) \
  ltbmg.mg_astrTexts = astr; \
  ltbmg.mg_ctTexts = sizeof(astr) / sizeof(astr[0]); \
  ltbmg.mg_iSelected = 0; \
  ltbmg.SetText(astr[ltbmg.mg_iSelected]);

#define PLACEMENT(x, y, z) CPlacement3D(FLOAT3D(x, y, z), ANGLE3D(0.0f, 0.0f, 0.0f))

// ptr to current menu
CGameMenu *pgmCurrentMenu = NULL;

// global back button
CMGButton _mgBack;

// -------- console variable adjustment menu
extern BOOL _bVarChanged = FALSE;

// [Cecil] Flag for playing over other sounds
extern void PlayMenuSound(CSoundData *psd, BOOL bOverOtherSounds) {
  if (bOverOtherSounds || (_psoMenuSound != NULL && !_psoMenuSound->IsPlaying())) {
    _psoMenuSound->Play(psd, SOF_NONGAME);
  }

  // [Cecil] Play IFeel effects here
  if (psd == _psdPress) {
    IFeel_PlayEffect("Menu_press");

  } else if (psd == _psdSelect) {
    IFeel_PlayEffect("Menu_select");
  }
}

// translate all texts in array for one radio button
void TranslateRadioTexts(CTString astr[], INDEX ct) {
  for (INDEX i = 0; i < ct; i++) {
    // [Cecil] No "ETRS" chunk to skip anymore, so: 4 -> 0
    astr[i] = TranslateConst(astr[i], 0);
  }
}

// set new thumbnail
void SetThumbnail(CTFileName fn) {
  _bThumbnailOn = TRUE;
  try {
    _toThumbnail.SetData_t(fn.NoExt() + "Tbn.tex");
  } catch (char *strError) {
    (void)strError;
    try {
      _toThumbnail.SetData_t(fn.NoExt() + ".tbn");
    } catch (char *strError) {
      (void)strError;
      _toThumbnail.SetData(NULL);
    }
  }
}

// remove thumbnail
void ClearThumbnail(void) {
  _bThumbnailOn = FALSE;
  _toThumbnail.SetData(NULL);
  _pShell->Execute("FreeUnusedStock();");
}

void StartMenus(const char *str) {
  _tmMenuLastTickDone = _pTimer->GetRealTimeTick();
  // disable printing of last lines
  CON_DiscardLastLineTimes();

  // stop all IFeel effects
  IFeel_StopEffect(NULL);
  if (pgmCurrentMenu == &_pGUIM->gmMainMenu || pgmCurrentMenu == &_pGUIM->gmInGameMenu) {
    if (_gmRunningGameMode == GM_NONE) {
      pgmCurrentMenu = &_pGUIM->gmMainMenu;
    } else {
      pgmCurrentMenu = &_pGUIM->gmInGameMenu;
    }
  }

  // Restore last active menu
  if (pgmCurrentMenu != NULL) {
    ChangeToMenu(pgmCurrentMenu);

  } else {
    // [Cecil] Reset visited menus
    _pGUIM->aVisitedMenus.PopAll();

    // Start the main menu
    if (_gmRunningGameMode == GM_NONE) {
      CMainMenu::ChangeTo();
    } else {
      CInGameMenu::ChangeTo();
    }
  }

  // [Cecil] Replaced individual if blocks with if-else chain
  const CTString strMenu = str;

  if (strMenu == "load") {
    extern void StartCurrentLoadMenu(void);
    StartCurrentLoadMenu();

  } else if (strMenu == "save") {
    extern void StartCurrentSaveMenu(void);
    StartCurrentSaveMenu();
    FixupBackButton(&_pGUIM->gmLoadSaveMenu);

  } else if (strMenu == "controls") {
    CControlsMenu::ChangeTo();
    FixupBackButton(&_pGUIM->gmControls);

  } else if (strMenu == "join") {
    extern void StartJoinServerMenu(void);
    StartJoinServerMenu();
    FixupBackButton(&_pGUIM->gmSelectPlayersMenu);

  } else if (strMenu == "hiscore") {
    CHighScoreMenu::ChangeTo();
    FixupBackButton(&_pGUIM->gmHighScoreMenu);
  }

  bMenuActive = TRUE;
  bMenuRendering = TRUE;
}

void StopMenus(BOOL bGoToRoot /*=TRUE*/) {
  ClearThumbnail();
  if (pgmCurrentMenu != NULL && bMenuActive) {
    pgmCurrentMenu->EndMenu();
  }
  bMenuActive = FALSE;
  if (bGoToRoot) {
    if (_gmRunningGameMode == GM_NONE) {
      pgmCurrentMenu = &_pGUIM->gmMainMenu;
    } else {
      pgmCurrentMenu = &_pGUIM->gmInGameMenu;
    }
  }
}

// [Cecil] Check if it's a root menu
BOOL IsMenuRoot(class CGameMenu *pgm) {
  return pgm == NULL || pgm == &_pGUIM->gmMainMenu || pgm == &_pGUIM->gmInGameMenu;
};

// ------------------------ Global menu function implementation
void InitializeMenus(void) {
  _pGUIM = new CMenuManager();

  try {
    // initialize and load corresponding fonts
  #if SE1_GAME != SS_REV
    _fdSmall.Load_t( CTFILENAME("Fonts\\Display3-narrow.fnt"));
    _fdMedium.Load_t(CTFILENAME("Fonts\\Display3-normal.fnt"));
    _fdBig.Load_t(   CTFILENAME("Fonts\\Display3-caps.fnt"));
    _fdTitle.Load_t( CTFILENAME("Fonts\\Title2.fnt"));
  #else
    _fdSmall.Load_t( CTFILENAME("Fonts\\Asap.fnt"));
    _fdMedium.Load_t(CTFILENAME("Fonts\\AsapBig.fnt"));
    _fdBig.Load_t(   CTFILENAME("Fonts\\AsapBig.fnt"));
    _fdTitle.Load_t( CTFILENAME("Fonts\\Modern\\Cabin.fnt"));
  #endif

    _fdSmall.SetCharSpacing(-1);
    _fdSmall.SetLineSpacing(0);
    _fdSmall.SetSpaceWidth(0.4f);
    _fdMedium.SetCharSpacing(+1);
    _fdMedium.SetLineSpacing(0);
    _fdMedium.SetSpaceWidth(0.4f);
    _fdBig.SetCharSpacing(+1);
    _fdBig.SetLineSpacing(0);
    _fdTitle.SetCharSpacing(+1);
    _fdTitle.SetLineSpacing(0);

    // load menu sounds
    _psdSelect = _pSoundStock->Obtain_t(CTFILENAME("Sounds\\Menu\\Select.wav"));
    _psdPress = _pSoundStock->Obtain_t(CTFILENAME("Sounds\\Menu\\Press.wav"));
    _psoMenuSound = new CSoundObject;

    // initialize and load menu textures
    _toLogoMenuA.SetData_t(CTFILENAME("Textures\\Logo\\sam_menulogo256a.tex"));
    _toLogoMenuB.SetData_t(CTFILENAME("Textures\\Logo\\sam_menulogo256b.tex"));

    // [Cecil] Classics Patch logo
    _toPatchLogo1.SetData_t(CTFILENAME("TexturesPatch\\General\\PatchLogo1.tex"));
    _toPatchLogo2.SetData_t(CTFILENAME("TexturesPatch\\General\\PatchLogo2.tex"));

    // [Cecil] Scrollbar arrow texture
    _toMenuArrow.SetData_t(CTFILENAME("TexturesPatch\\General\\MenuArrow.tex"));

  } catch (char *strError) {
    FatalError(strError);
  }

  // force logo textures to be of maximal size
  ((CTextureData *)_toLogoMenuA.GetData())->Force(TEX_CONSTANT);
  ((CTextureData *)_toLogoMenuB.GetData())->Force(TEX_CONSTANT);

  // [Cecil] Classics Patch logo
  ((CTextureData *)_toPatchLogo1.GetData())->Force(TEX_CONSTANT);
  ((CTextureData *)_toPatchLogo2.GetData())->Force(TEX_CONSTANT);

  // [Cecil] Scrollbar arrow texture
  ((CTextureData *)_toMenuArrow.GetData())->Force(TEX_CONSTANT);

  try {
    TRANSLATERADIOARRAY(astrNoYes);
    TRANSLATERADIOARRAY(astrComputerInvoke);
    TRANSLATERADIOARRAY(astrDisplayPrefsRadioTexts);
    TRANSLATERADIOARRAY(astrWeapon);
    TRANSLATERADIOARRAY(astrSplitScreenRadioTexts);

    // initialize game type strings table
    InitGameTypes();

    // ------------------- Initialize menus
    _pGUIM->gmConfirmMenu.Initialize_t();
    _pGUIM->gmMainMenu.Initialize_t();
    _pGUIM->gmInGameMenu.Initialize_t();
    _pGUIM->gmSinglePlayerMenu.Initialize_t();
    _pGUIM->gmSinglePlayerNewMenu.Initialize_t();
    _pGUIM->gmPlayerProfile.Initialize_t();
    _pGUIM->gmControls.Initialize_t();
    _pGUIM->gmLoadSaveMenu.Initialize_t();
    _pGUIM->gmHighScoreMenu.Initialize_t();
    _pGUIM->gmCustomizeKeyboardMenu.Initialize_t();
    _pGUIM->gmCustomizeAxisMenu.Initialize_t();
    _pGUIM->gmOptionsMenu.Initialize_t();
    _pGUIM->gmLevelsMenu.Initialize_t();
    _pGUIM->gmVarMenu.Initialize_t();
    _pGUIM->gmServersMenu.Initialize_t();
    _pGUIM->gmNetworkMenu.Initialize_t();
    _pGUIM->gmNetworkStartMenu.Initialize_t();
    _pGUIM->gmNetworkJoinMenu.Initialize_t();
    _pGUIM->gmSelectPlayersMenu.Initialize_t();
    _pGUIM->gmNetworkOpenMenu.Initialize_t();
    _pGUIM->gmSplitScreenMenu.Initialize_t();
    _pGUIM->gmSplitStartMenu.Initialize_t();

    // [Cecil] Initialize new menus
    _pGUIM->gmLevelCategories.Initialize_t();
    _pGUIM->gmExtras.Initialize_t();
    _pGUIM->gmPatchCredits.Initialize_t();

  } catch (char *strError) {
    FatalError("Cannot initialize game menus:\n", strError);
  }
}

void DestroyMenus(void) {
  _pGUIM->gmMainMenu.Destroy();
  pgmCurrentMenu = NULL;
  _pSoundStock->Release(_psdSelect);
  _pSoundStock->Release(_psdPress);
  delete _psoMenuSound;
  _psdSelect = NULL;
  _psdPress = NULL;
  _psoMenuSound = NULL;
}

// go to parent menu if possible
void MenuGoToParent(void) {
  // [Cecil] No more visited menus
  if (_pGUIM->aVisitedMenus.Count() == 0) {
    // if in game
    if (_gmRunningGameMode != GM_NONE) {
      // exit menus
      StopMenus();
      // if no game is running
    } else {
      // go to main menu
      CMainMenu::ChangeTo();
    }

  // [Cecil] No next menu - return to the previous menu
  } else {
    ChangeToMenu(NULL);
  }
}

void MenuOnKeyDown(PressedMenuButton pmb) {
  // check if mouse buttons used
  _bMouseUsedLast = (pmb.iKey == VK_LBUTTON || pmb.iKey == VK_RBUTTON || pmb.iKey == VK_MBUTTON
    || pmb.iKey == VK_XBUTTON1 || pmb.iKey == VK_XBUTTON2 || pmb.iKey == MOUSEWHEEL_DN || pmb.iKey == MOUSEWHEEL_UP);

  // [Cecil] Ignore mouse only when editing strings
  if (_eEditingValue == VED_STRING && _bMouseUsedLast) {
    _bMouseUsedLast = FALSE;
    return;
  }

  // initially the message is not handled
  BOOL bHandled = FALSE;

  // if not a mouse button, or mouse is over some gadget
  if (!_bMouseUsedLast || _pmgUnderCursor != NULL) {
    // ask current menu to handle the key
    bHandled = pgmCurrentMenu->OnKeyDown(pmb);
  }

  // if not handled
  if (!bHandled) {
    // if escape or right mouse pressed
    if (pmb.Back(TRUE)) {
      // go to parent menu if possible
      MenuGoToParent();
    }
  }
}

// [Cecil] Releasing some button
void MenuOnKeyUp(PressedMenuButton pmb) {
  // Let the menu handle released buttons
  if (pgmCurrentMenu != NULL) {
    const BOOL bHandled = pgmCurrentMenu->OnKeyUp(pmb);
    (void)bHandled;
  }

  // Reset last pressed gadget, if there was any
  _pmgLastPressedGadget = NULL;
  _pmbLastPressedButton.SetNone();
};

// [Cecil] Holding any mouse button
void MenuOnMouseHold(PressedMenuButton pmb) {
  // Only accept mouse buttons
  ASSERT(pmb.iKey == VK_LBUTTON || pmb.iKey == VK_RBUTTON || pmb.iKey == VK_MBUTTON);

  if (_pmgUnderCursor != NULL) {
    pgmCurrentMenu->OnMouseHeld(pmb);
  }
};

void MenuOnChar(MSG msg) {
  // check if mouse buttons used
  _bMouseUsedLast = FALSE;

  // ask current menu to handle the key
  pgmCurrentMenu->OnChar(msg);
}

void MenuOnMouseMove(PIX pixI, PIX pixJ) {
  static PIX pixLastI = 0;
  static PIX pixLastJ = 0;
  if (pixLastI == pixI && pixLastJ == pixJ) {
    return;
  }
  pixLastI = pixI;
  pixLastJ = pixJ;
  _bMouseUsedLast = _eEditingValue == VED_NONE && !_pInput->IsInputEnabled();
}

void MenuUpdateMouseFocus(void) {
  // get real cursor position
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(_hwndMain, &pt);

  _pixCursorPosI = pt.x;
  _pixCursorPosJ = pt.y;

  // if mouse not used last
  if (!_bMouseUsedLast || _eEditingValue != VED_NONE) {
    // do nothing
    return;
  }

  CMenuGadget *pmgActive = NULL;
  // for all gadgets in menu
  FOREACHNODE(pgmCurrentMenu, CMenuGadget, itmg) {
    CMenuGadget &mg = *itmg;
    // if focused
    if (itmg->mg_bFocused) {
      // remember it
      pmgActive = &itmg.Current();
    }
  }

  // if there is some under cursor
  if (_pmgUnderCursor != NULL) {
    _pmgUnderCursor->OnMouseOver(_pixCursorPosI, _pixCursorPosJ);
    // if the one under cursor has no neighbours
    if (_pmgUnderCursor->mg_pmgLeft == NULL
     && _pmgUnderCursor->mg_pmgRight == NULL
     && _pmgUnderCursor->mg_pmgUp == NULL
     && _pmgUnderCursor->mg_pmgDown == NULL) {
      // it cannot be focused
      _pmgUnderCursor = NULL;
      return;
    }

    // if the one under cursor is not active and not disappearing
    if (pmgActive != _pmgUnderCursor && _pmgUnderCursor->mg_bVisible) {
      // change focus
      if (pmgActive != NULL) {
        pmgActive->OnKillFocus();
      }
      _pmgUnderCursor->OnSetFocus();
    }
  }
}

static CTimerValue _tvInitialization;
static TIME _tmInitializationTick = -1;
extern TIME _tmMenuLastTickDone;

void SetMenuLerping(void) {
  CTimerValue tvPatch(ClassicsCore_GetInitTime()); // [Cecil]
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer() - tvPatch;

  // if lerping was never set before
  if (_tmInitializationTick < 0) {
    // initialize it
    _tvInitialization = tvNow;
    _tmInitializationTick = _tmMenuLastTickDone;
  }

  // get passed time from session state starting in precise time and in ticks
  FLOAT tmRealDelta = FLOAT((tvNow - _tvInitialization).GetSeconds());
  FLOAT tmTickDelta = _tmMenuLastTickDone - _tmInitializationTick;
  // calculate factor
  FLOAT fFactor = 1.0f - (tmTickDelta - tmRealDelta) / _pTimer->TickQuantum;

  // if the factor starts getting below zero
  if (fFactor < 0) {
    // clamp it
    fFactor = 0.0f;
    // readjust timers so that it gets better
    _tvInitialization = tvNow;
    _tmInitializationTick = _tmMenuLastTickDone - _pTimer->TickQuantum;
  }
  if (fFactor > 1) {
    // clamp it
    fFactor = 1.0f;
    // readjust timers so that it gets better
    _tvInitialization = tvNow;
    _tmInitializationTick = _tmMenuLastTickDone;
  }
  // set lerping factor and timer
  _pTimer->SetCurrentTick(_tmMenuLastTickDone);
  _pTimer->SetLerp(fFactor);
}

// render mouse cursor if needed
void RenderMouseCursor(CDrawPort *pdp) {
  // if mouse not used last
  if ((!_bMouseUsedLast && _eEditingValue != VED_LIST) || _eEditingValue == VED_KEYBIND) {
    // don't render cursor
    return;
  }
  SetDrawportForGame(pdp);
  _pGame->LCDDrawPointer(_pixCursorPosI, _pixCursorPosJ);
}

BOOL DoMenu(CDrawPort *pdp) {
  // [Cecil] Keep trying to disable GameSpy until it falls through
  ICore::DisableGameSpy();

  pdp->Unlock();
  CDrawPort dpMenu(pdp, TRUE);
  dpMenu.Lock();

  MenuUpdateMouseFocus();

  // if in fullscreen
  CDisplayMode dmCurrent;
  _pGfx->GetCurrentDisplayMode(dmCurrent);

  if (dmCurrent.IsFullScreen()) {
    // clamp mouse pointer
    _pixCursorPosI = Clamp(_pixCursorPosI, 0L, dpMenu.GetWidth());
    _pixCursorPosJ = Clamp(_pixCursorPosJ, 0L, dpMenu.GetHeight());
  }

  pgmCurrentMenu->Think();

  TIME tmTickNow = _pTimer->GetRealTimeTick();

  while (_tmMenuLastTickDone < tmTickNow) {
    _pTimer->SetCurrentTick(_tmMenuLastTickDone);
    // call think for all gadgets in menu
    FOREACHNODE(pgmCurrentMenu, CMenuGadget, itmg) {
      itmg->Think();
    }
    _tmMenuLastTickDone += _pTimer->TickQuantum;
  }

  SetMenuLerping();

  PIX pixW = dpMenu.GetWidth();
  PIX pixH = dpMenu.GetHeight();

  // blend background if menu is on
  if (bMenuActive) {
    // clear screen with background texture
    _pGame->LCDPrepare(1.0f);
    SetDrawportForGame(&dpMenu);

    // [Cecil] Show the game a bit in the background
    if (sam_bBackgroundGameRender && _gmRunningGameMode != GM_NONE) {
      dpMenu.Fill(C_BLACK | 127);

    } else {
      // do not allow game to show through
      dpMenu.Fill(C_BLACK | 255);
      _pGame->LCDRenderClouds1();
      _pGame->LCDRenderGrid();
      _pGame->LCDRenderClouds2();
    }

    // [Cecil] Use the same scale for width and height
    const FLOAT fScale = (FLOAT)pixH / 480.0f;

    // [Cecil] Proper right side position
    const PIX pixR = 480 * ((FLOAT)pixW / (FLOAT)pixH);

    PIX pixI0, pixJ0, pixI1, pixJ1;
    // put logo(s) to main menu (if logos exist)
    if (pgmCurrentMenu == &_pGUIM->gmMainMenu) {
      if (_ptoLogoODI != NULL) {
        CTextureData &td = (CTextureData &)*_ptoLogoODI->GetData();
        const INDEX iSize = 50;
        const PIX pixLogoWidth = iSize;
        const PIX pixLogoHeight = iSize * td.GetHeight() / td.GetWidth();
        pixI0 = (pixR - pixLogoWidth - 16) * fScale;
        pixJ0 = (480 - pixLogoHeight - 16) * fScale;
        pixI1 = pixI0 + pixLogoWidth * fScale;
        pixJ1 = pixJ0 + pixLogoHeight * fScale;
        dpMenu.PutTexture(_ptoLogoODI, PIXaabbox2D(PIX2D(pixI0, pixJ0), PIX2D(pixI1, pixJ1)));
      }
      if (_ptoLogoCT != NULL) {
        CTextureData &td = (CTextureData &)*_ptoLogoCT->GetData();
        const INDEX iSize = 50;
        const PIX pixLogoWidth = iSize;
        const PIX pixLogoHeight = iSize * td.GetHeight() / td.GetWidth();
        pixI0 = 12 * fScale;
        pixJ0 = (480 - pixLogoHeight - 16) * fScale;
        pixI1 = pixI0 + pixLogoWidth * fScale;
        pixJ1 = pixJ0 + pixLogoHeight * fScale;
        dpMenu.PutTexture(_ptoLogoCT, PIXaabbox2D(PIX2D(pixI0, pixJ0), PIX2D(pixI1, pixJ1)));
      }

      {
        FLOAT fResize = Min(dpMenu.GetWidth() / 640.0f, dpMenu.GetHeight() / 480.0f);
        PIX pixSizeI = 256 * fResize;
        PIX pixSizeJ = 64 * fResize;
        PIX pixCenterI = dpMenu.GetWidth() / 2;
        PIX pixHeightJ = 10 * fResize;
        dpMenu.PutTexture(&_toLogoMenuA, PIXaabbox2D(
          PIX2D(pixCenterI - pixSizeI, pixHeightJ), PIX2D(pixCenterI, pixHeightJ + pixSizeJ)));
        dpMenu.PutTexture(&_toLogoMenuB, PIXaabbox2D(
          PIX2D(pixCenterI, pixHeightJ), PIX2D(pixCenterI + pixSizeI, pixHeightJ + pixSizeJ)));
      }

      // [Cecil] Display patch information
      if (sam_bPatchVersionLabel) {
        dpMenu.SetFont(_pfdConsoleFont);
        dpMenu.SetTextScaling(1.0f);
        dpMenu.SetTextCharSpacing(-1);

        const PIX pixPatchX = dpMenu.GetWidth() - 16;
        const PIX pixPatchY = dpMenu.GetHeight() - _pfdConsoleFont->fd_pixCharHeight;
        const CTString strPatch(0, "Serious Sam Classics Patch v%s", ClassicsCore_GetVersionName());

        dpMenu.PutTextR(strPatch, pixPatchX, pixPatchY, 0xFFFFFFFF);
      }

    } else if (pgmCurrentMenu->gm_strName == "AudioOptions") {
      if (_ptoLogoEAX != NULL) {
        CTextureData &td = (CTextureData &)*_ptoLogoEAX->GetData();
        const INDEX iSize = 95;
        const PIX pixLogoWidth = iSize;
        const PIX pixLogoHeight = iSize * td.GetHeight() / td.GetWidth();
        pixI0 = (pixR - pixLogoWidth - 45) * fScale;
        pixJ0 = (480 - pixLogoHeight - 35) * fScale;
        pixI1 = pixI0 + pixLogoWidth * fScale;
        pixJ1 = pixJ0 + pixLogoHeight * fScale;
        dpMenu.PutTexture(_ptoLogoEAX, PIXaabbox2D(PIX2D(pixI0, pixJ0), PIX2D(pixI1, pixJ1)));
      }

    // [Cecil] Classics Patch credits
    } else if (pgmCurrentMenu == &_pGUIM->gmPatchCredits) {
      // Display patch logo at the top
      FLOAT fResize = Min(dpMenu.GetWidth() / 640.0f, dpMenu.GetHeight() / 480.0f);
      PIX pixSizeI = 192 * fResize;
      PIX pixSizeJ = 96 * fResize;
      PIX pixCenterI = dpMenu.GetWidth() / 2;
      PIX pixHeightJ = 10 * fResize;
      dpMenu.PutTexture(&_toPatchLogo1, PIXaabbox2D(
        PIX2D(pixCenterI - pixSizeI, pixHeightJ), PIX2D(pixCenterI, pixHeightJ + pixSizeJ)));
      dpMenu.PutTexture(&_toPatchLogo2, PIXaabbox2D(
        PIX2D(pixCenterI, pixHeightJ), PIX2D(pixCenterI + pixSizeI, pixHeightJ + pixSizeJ)));
    }

#define THUMBW 96
#define THUMBH 96
    // if there is a thumbnail
    if (_bThumbnailOn) {
      const FLOAT fThumbScaleW = fScale;
      PIX pixOfs = 8 * fScale;
      pixI0 = 8 * fScale;
      pixJ0 = (240 - THUMBW / 2) * fScale;
      pixI1 = pixI0 + THUMBW * fThumbScaleW;
      pixJ1 = pixJ0 + THUMBH * fScale;
      if (_toThumbnail.GetData() != NULL) { // show thumbnail with shadow and border
        dpMenu.Fill(pixI0 + pixOfs, pixJ0 + pixOfs, THUMBW * fThumbScaleW, THUMBH * fScale, C_BLACK | 128);
        dpMenu.PutTexture(&_toThumbnail, PIXaabbox2D(PIX2D(pixI0, pixJ0), PIX2D(pixI1, pixJ1)), C_WHITE | 255);
        dpMenu.DrawBorder(pixI0, pixJ0, THUMBW * fThumbScaleW, THUMBH * fScale, _pGame->LCDGetColor(C_mdGREEN | 255, "thumbnail border"));
      } else {
        dpMenu.SetFont(_pfdDisplayFont);
        dpMenu.SetTextScaling(fScale);
        dpMenu.SetTextAspect(1.0f);
        dpMenu.PutTextCXY(LOCALIZE("no thumbnail"), (pixI0 + pixI1) / 2, (pixJ0 + pixJ1) / 2, _pGame->LCDGetColor(C_GREEN | 255, "no thumbnail"));
      }
    }

    // assure we can listen to non-3d sounds
    _pSound->UpdateSounds();
  }

  // if this is popup menu
  if (pgmCurrentMenu->gm_fPopupSize > 0.0f) { // [Cecil]
    // [Cecil] Render last visited proper menu
    CGameMenu *pgmLast = NULL;

    // Go from the end
    INDEX iMenu = _pGUIM->aVisitedMenus.Count();

    while (--iMenu >= 0) {
      CGameMenu *pgmVisited = _pGUIM->aVisitedMenus[iMenu];

      // Not a popup menu
      if (pgmVisited->gm_fPopupSize <= 0.0f) {
        pgmLast = pgmVisited;
        break;
      }
    }

    if (pgmLast != NULL) {
      _pGame->MenuPreRenderMenu(pgmLast->gm_strName);

      FOREACHNODE(pgmLast, CMenuGadget, itmg) {
        if (itmg->mg_bVisible) {
          itmg->Render(&dpMenu);
        }
      }

      _pGame->MenuPostRenderMenu(pgmLast->gm_strName);
    }

    // [Cecil] Render popup box in a separate method
    CGameMenu::RenderPopup(&dpMenu, pgmCurrentMenu->gm_fPopupSize);
  }

  BOOL bEditingValue = (_eEditingValue != VED_NONE); // [Cecil]

  // [Cecil] If not editing anything
  if (!bEditingValue) {
    // Reset gadget under the cursor
    _pmgUnderCursor = _pmgLastPressedGadget; // [Cecil] To the last pressed one
  }

  // [Cecil] Determine active gadget to prevent double rendering of the "currently edited" gadget

  // no currently active gadget initially
  CMenuGadget *pmgActive = NULL;
  // if mouse was not active last
  if (!_bMouseUsedLast) {
    // find focused gadget
    FOREACHNODE(pgmCurrentMenu, CMenuGadget, itmg) {
      CMenuGadget &mg = *itmg;
      // if focused
      if (itmg->mg_bFocused) {
        // it is active
        pmgActive = &itmg.Current();
        break;
      }
    }
  }

  // [Cecil] Render/process menu extras below everything
  pgmCurrentMenu->PreRender(&dpMenu);

  BOOL bStilInMenus = FALSE;
  _pGame->MenuPreRenderMenu(pgmCurrentMenu->gm_strName);
  // for each menu gadget
  FOREACHNODE(pgmCurrentMenu, CMenuGadget, itmg) {
    // if gadget is visible
    if (itmg->mg_bVisible) {
      bStilInMenus = TRUE;

      // [Cecil] Skip active gadget that's being edited as it's drawn again later
      if (!bEditingValue || pmgActive != itmg) {
        itmg->Render(&dpMenu);
      }

      // [Cecil] Don't update the gadget under the cursor during editing or while holding a button on another gadget
      if (!bEditingValue && _pmgLastPressedGadget == NULL
       && FloatBoxToPixBox(&dpMenu, itmg->mg_boxOnScreen) >= PIX2D(_pixCursorPosI, _pixCursorPosJ))
      {
        _pmgUnderCursor = itmg;
      }
    }
  }
  _pGame->MenuPostRenderMenu(pgmCurrentMenu->gm_strName);

  // if mouse was active last
  if (_bMouseUsedLast) {
    // gadget under cursor is active
    pmgActive = _pmgUnderCursor;
  }

  // if editing
  if (bEditingValue && pmgActive != NULL) {
    // dim the menu  bit
    dpMenu.Fill(C_BLACK | 0x40);
    // render the edit gadget again
    pmgActive->Render(&dpMenu);
  }

  // [Cecil] Render/process menu extras on top of everything
  pgmCurrentMenu->PostRender(&dpMenu);

  // [Cecil] Don't override the tooltip text during key bind editing
  bEditingValue &= (_eEditingValue != VED_KEYBIND);

  // if there is some active gadget and it has tips
  if (pmgActive != NULL && (pmgActive->mg_strTip != "" || bEditingValue)) {
    CTString strTip = pmgActive->mg_strTip;
    if (bEditingValue) {
      strTip = LOCALIZE("Enter - OK, Escape - Cancel");
    }
    // print the tip
    SetFontMedium(&dpMenu, 1.0f);
    dpMenu.PutTextC(strTip, pixW * 0.5f, pixH * 0.92f, _pGame->LCDGetColor(C_WHITE | 255, "tool tip"));
  }

  _pGame->ConsolePrintLastLines(&dpMenu);

  RenderMouseCursor(&dpMenu);

  dpMenu.Unlock();
  pdp->Lock();

  return bStilInMenus;
}

void MenuBack(void) {
  MenuGoToParent();
}

void FixupBackButton(CGameMenu *pgm) {
  BOOL bResume = FALSE;
  BOOL bHasBack = TRUE;

  if (pgm->gm_fPopupSize > 0.0f) { // [Cecil]
    bHasBack = FALSE;
  }

  if (_pGUIM->aVisitedMenus.Count() == 0) {
    if (_gmRunningGameMode != GM_NONE) {
      bResume = TRUE;

    // [Cecil] Only remove the back button in root menus
    } else if (IsMenuRoot(pgm)) {
      bHasBack = FALSE;
    }
  }

  if (!bHasBack) {
    _mgBack.Disappear();
    return;
  }

  if (bResume) {
    _mgBack.SetText(LOCALIZE("RESUME"));
    _mgBack.mg_strTip = LOCALIZE("return to game");
  } else {
    if (_bVarChanged) {
      _mgBack.SetText(LOCALIZE("CANCEL"));
      _mgBack.mg_strTip = LOCALIZE("cancel changes");
    } else {
      _mgBack.SetText(LOCALIZE("BACK"));
      _mgBack.mg_strTip = LOCALIZE("return to previous menu");
    }
  }

  _mgBack.mg_iCenterI = -1;
  _mgBack.mg_bfsFontSize = BFS_LARGE;
  _mgBack.mg_boxOnScreen = BoxBack();
  _mgBack.mg_boxOnScreen = BoxLeftColumn(16.5f);
  pgm->AddChild(&_mgBack);

  _mgBack.mg_pmgLeft =
  _mgBack.mg_pmgRight =
  _mgBack.mg_pmgUp =
  _mgBack.mg_pmgDown = pgm->gm_pmgSelectedByDefault;

  _mgBack.mg_pActivatedFunction = &MenuBack;

  _mgBack.Appear();
}

void ChangeToMenu(CGameMenu *pgmNewMenu) {
  // auto-clear old thumbnail when going out of menu
  ClearThumbnail();

  // [Cecil] Reset held mouse buttons
  extern void ReleaseHeldMouseButtons(void);
  ReleaseHeldMouseButtons();

  // [Cecil] Reset gadget under the cursor
  _pmgUnderCursor = NULL;

  // [Cecil] If no new menu specified
  if (pgmNewMenu == NULL) {
    // Simply return to the previous one
    ASSERT(_pGUIM->aVisitedMenus.Count() != 0);
    pgmNewMenu = _pGUIM->aVisitedMenus.Pop();

  // Otherwise remember this menu before changing from it
  } else if (pgmCurrentMenu != NULL && pgmCurrentMenu != pgmNewMenu) {
    _pGUIM->aVisitedMenus.Add(pgmCurrentMenu);

    // [Cecil] Rewind back a few menus until the specified one, if needed
    if (_pgmRewindTo != NULL) {
      while (_pGUIM->aVisitedMenus.Count() != 0) {
        CGameMenu *pgmPrev = _pGUIM->aVisitedMenus.Pop();

        // Popped the menu to rewind to
        if (pgmPrev == _pgmRewindTo) break;
      }

      _pgmRewindTo = NULL;
    }
  }

  ASSERT(pgmNewMenu != NULL);

  // [Cecil] Reset visited menus if returning to any root menu, just in case
  if (IsMenuRoot(pgmNewMenu)) {
    _pGUIM->aVisitedMenus.PopAll();
  }

  if (pgmCurrentMenu != NULL) {
    if (pgmNewMenu->gm_fPopupSize <= 0.0f) { // [Cecil]
      pgmCurrentMenu->EndMenu();
    } else {
      FOREACHNODE(pgmCurrentMenu, CMenuGadget, itmg) {
        itmg->OnKillFocus();
      }
    }
  }

  pgmNewMenu->StartMenu();

  if (pgmNewMenu->gm_pmgSelectedByDefault) {
    if (_mgBack.mg_bFocused) {
      _mgBack.OnKillFocus();
    }
    pgmNewMenu->gm_pmgSelectedByDefault->OnSetFocus();
  }

  FixupBackButton(pgmNewMenu);
  pgmCurrentMenu = pgmNewMenu;
}