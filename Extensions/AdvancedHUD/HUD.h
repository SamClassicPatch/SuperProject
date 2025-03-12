/* Copyright (c) 2023-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_ADVANCEDHUD_H
#define CECIL_INCL_ADVANCEDHUD_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "Themes.h"
#include "WeaponArsenal.h"

#include <EntitiesV/StdH/StdH.h>
#include <EntitiesV/PlayerWeapons.h>

// Argument list for the RenderHUD() function
#if SE1_VER < SE1_107
  #define RENDER_ARGS_RAW(_prProjection, _pdp, _vLightDir, _colLight, _colAmbient, _bRenderWeapon, _iEye) \
    _prProjection, _pdp, _vLightDir, _colLight, _colAmbient, _bRenderWeapon

#else
  #define RENDER_ARGS_RAW(_prProjection, _pdp, _vLightDir, _colLight, _colAmbient, _bRenderWeapon, _iEye) \
    _prProjection, _pdp, _vLightDir, _colLight, _colAmbient, _bRenderWeapon, _iEye
#endif

#define RENDER_ARGS(_prProjection, _pdp, _vLightDir, _colLight, _colAmbient, _bRenderWeapon, _iEye) \
  RENDER_ARGS_RAW(CPerspectiveProjection3D &_prProjection, CDrawPort *_pdp, FLOAT3D _vLightDir, \
                  COLOR _colLight, COLOR _colAmbient, BOOL _bRenderWeapon, INDEX _iEye)

class CHud {
  public:
    // Keys for sorting player statistics
    enum ESortKeys {
      E_SK_NAME   = 0,
      E_SK_HEALTH = 1,
      E_SK_SCORE  = 2,
      E_SK_MANA   = 3, 
      E_SK_FRAGS  = 4,
      E_SK_DEATHS = 5,
    };

    // Progress bar direction
    enum EBarDir {
      E_BD_LEFT  = 1,
      E_BD_RIGHT = 2, 
      E_BD_UP    = 3,
      E_BD_DOWN  = 4,
    };

  public:
    // Method hooks
    typedef const CSessionProperties *(*CGetPropsFunc)(void);
    typedef void  (CPlayer       ::*CPowerUpSoundFunc)(void);
    typedef BOOL  (CPlayer       ::*CIsConnectedFunc)(void) const;
    typedef COLOR (CPlayer       ::*CWorldGlaringFunc)(void);
    typedef void  (CPlayer       ::*CParticlesFunc)(BOOL);
    typedef void  (CPlayer       ::*CRenderHudFunc)(RENDER_ARGS(pr, pdp, v, colL, colA, b, i));
    typedef void  (CPlayerWeapons::*CRenderWeaponFunc)(RENDER_ARGS(pr, pdp, v, colL, colA, b, i));
    typedef void  (CPlayerWeapons::*CRenderCrossFunc)(CProjection3D &, CDrawPort *, CPlacement3D &);
    typedef INDEX (CPlayerWeapons::*CGetAmmoFunc)(void);
    typedef INDEX (CPlayerWeapons::*CGetMaxAmmoFunc)(void);

    static CGetPropsFunc     pGetSP;
    static CPowerUpSoundFunc pPlayPowerUpSound_opt;
    static CIsConnectedFunc  pIsConnected_opt;
    static CWorldGlaringFunc pGetWorldGlaring_opt;
    static CParticlesFunc    pRenderChainsawParticles_opt;
    static CRenderHudFunc    pRenderHud;
    static CRenderWeaponFunc pRenderWeaponModel_opt;
    static CRenderCrossFunc  pRenderCrosshair_opt;
    static CGetAmmoFunc      pGetAmmo;
    static CGetMaxAmmoFunc   pGetMaxAmmo;

  public:
    // Player entities
    CPlayer *_penPlayer;
    CPlayer *_penLast;
    CPlayerWeapons *_penWeapons;

    // Drawing variables
    CDrawPort *_pdp;
    PIX2D _vpixScreen;
    FLOAT2D _vScaling; // Scaling factors taking aspect ratio in consideration
    FLOAT _fHudScaling; // Current HUD scaling
    FLOAT _fWideAdjustment;
    FLOAT _fCustomScaling;

    ULONG _ulAlphaHUD;
    BOOL _bTSEColors;
    BOOL _bTSETheme;

    // HUD boundaries
    PIX2D _vpixTL;
    PIX2D _vpixBR;

    // Colors
    COLOR _colHUD;
    COLOR _colBorder;
    COLOR _colIconStd;

    COLOR _colMax;
    COLOR _colTop;
    COLOR _colMid;
    COLOR _colLow;

    // Fonts
    CFontData _afdText[E_HUD_MAX];
    CFontData _afdNumbers[E_HUD_MAX];
    CFontData *_pfdCurrentText;
    CFontData *_pfdCurrentNumbers;
    FLOAT _fTextFontScale;

    // Other
    TIME _tmNow;
    TIME _tmLast;

    struct Units {
      FLOAT fChar;
      FLOAT fOne;
      FLOAT fAdv;
      FLOAT fNext;
      FLOAT fHalf;
    } units;

    // Array of pointers to all players
    CDynamicContainer<CPlayer> _cenPlayers;

    // Information about color transitions
    struct ColorTransitionTable {
      COLOR ctt_colFine;     // Color for values over 1.0
      COLOR ctt_colHigh;     // Color for values from 1.0 to 'fMedium'
      COLOR ctt_colMedium;   // Color for values from 'fMedium' to 'fLow'
      COLOR ctt_colLow;      // Color for values under fLow
      FLOAT ctt_fMediumHigh; // When to switch to high color   (normalized float!)
      FLOAT ctt_fLowMedium;  // When to switch to medium color (normalized float!)
      BOOL  ctt_bSmooth;     // Should colors have smooth transition
    } _cttHUD;

    HudTextureSet tex;
    const HudColorSet *pColorSet;
    HudArsenal arWeapons;

  public:
    CHud() {
      _tmNow = -1.0f;
      _tmLast = -1.0f;
    };

    // Get ammo from the arsenal
    inline HudArsenal::AmmoStack &GetAmmo(void) {
      return arWeapons.aAmmo;
    };

    // Get weapons from the arsenal
    inline HudArsenal::WeaponStack &GetWeapons(void) {
      return arWeapons.aWeapons;
    };

    // Find weapon info from weapon type
    BOOL GetWeaponInfo(HudWeapon **ppInfo, INDEX iWeapon) {
      for (INDEX i = 0; i < GetWeapons().Count(); i++) {
        if (GetWeapons()[i].iWeapon == iWeapon) {
          *ppInfo = &GetWeapons()[i];
          return TRUE;
        }
      }

      return FALSE;
    };

    // Reset scaling and unit sizes
    inline void ResetScale(FLOAT fScale) {
      _fCustomScaling = fScale;

      const PIX pixChar = _pfdCurrentNumbers->GetWidth() + _pfdCurrentNumbers->GetCharSpacing() + 1;
      units.fChar = pixChar * fScale;

      units.fOne  = 32 * fScale;
      units.fAdv  = 36 * fScale;
      units.fNext = 40 * fScale;
      units.fHalf = units.fOne * 0.5f;
    };

    // Scale unit sizes by a factor
    inline FLOAT Rescale(FLOAT fFactor) {
      const FLOAT fUpperSize = (_fCustomScaling * fFactor) / _fCustomScaling;
      _fCustomScaling *= fUpperSize;

      units.fChar *= fUpperSize;
      units.fOne  *= fUpperSize;
      units.fAdv  *= fUpperSize;
      units.fNext *= fUpperSize;
      units.fHalf *= fUpperSize;

      // Return factor for undoing current factor
      return 1.0f / fUpperSize;
    };

  // Main methods
  public:

    // Prepare color transitions
    void PrepareColorTransitions(COLOR colFine, COLOR colHigh, COLOR colMedium, COLOR colLow,
      FLOAT fMediumHigh, FLOAT fLowMedium, BOOL bSmooth);

    // Calculate shake amount and color value depending on value change
    COLOR AddShaker(const PIX pixAmount, const INDEX iCurrentValue, INDEX &iLastValue,
                    FLOAT &tmChanged, FLOAT &fMoverX, FLOAT &fMoverY);

    // Get current color from local color transitions table
    COLOR GetCurrentColor(FLOAT fNormValue);

    // Gather all players in the array
    void GatherPlayers(void);

    // Fill array with players sorted by a specific statistic
    void SetAllPlayersStats(CDynamicContainer<CPlayer> &cen, INDEX iSortKey);

    // Update weapon and ammo tables with current info
    void UpdateWeaponArsenal(void);

    // Prepare interface for rendering
    BOOL PrepareHUD(CPlayer *penCurrent, CDrawPort *pdpCurrent);

    // Render entire interface
    void DrawHUD(const CPlayer *penCurrent, BOOL bSnooping, const CPlayer *penOwner);

    // Display tags above players
    void RenderPlayerTags(CPlayer *penThis, CPerspectiveProjection3D &prProjection);

    // Initialize everything for drawing the HUD
    void Initialize(void);

    // Clean everything up before disabling the plugin
    void End(void);

  // Drawing functions
  public:

    // Draw border using a tile texture
    void DrawBorder(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, COLOR colTiles);

    // Draw icon texture
    void DrawIcon(FLOAT fX, FLOAT fY, CTextureObject &toIcon, COLOR colDefault, FLOAT fNormValue, BOOL bBlink);

    // Draw text
    void DrawString(FLOAT fX, FLOAT fY, const CTString &strText, COLOR colDefault, FLOAT fNormValue);

    // Draw percentage bar
    void DrawBar(FLOAT fX, FLOAT fY, PIX pixW, PIX pixH, EBarDir eBarDir, COLOR colDefault, FLOAT fNormValue);

    // Draw texture rotated at a certain angle
    void DrawRotatedQuad(CTextureObject *pto, FLOAT fX, FLOAT fY, FLOAT fSize, ANGLE aAngle, COLOR col);

    // Draw texture with a correct aspect ratio
    void DrawCorrectTexture(CTextureObject *pto, FLOAT fX, FLOAT fY, FLOAT fWidth, COLOR col);

  #if SE1_GAME != SS_TFE
    // Draw sniper mask
    void DrawSniperMask(void);
  #endif

  // Parts of the HUD
  public:

    enum EGameMode {
      E_GM_SP,
      E_GM_COOP,
      E_GM_SCORE,
      E_GM_FRAG,
    } _eGameMode;

    void RenderVitals(void);
    void RenderCurrentWeapon(SIconTexture **pptoWantedWeapon, SIconTexture **pptoCurrentAmmo);
    void RenderActiveArsenal(SIconTexture *ptoAmmo);
    void RenderBars(void);
    void RenderGameModeInfo(void);
    void RenderCheats(void);

  // HUD colors
  public:

    // Base colors
    inline COLOR COL_Base(void);
    inline COLOR COL_Icon(void);
    inline COLOR COL_PlayerNames(void);
    inline COLOR COL_SnoopingLight(void);
    inline COLOR COL_SnoopingDark(void);
    inline COLOR COL_AmmoSelected(void);
    inline COLOR COL_AmmoDepleted(void);

    // Value colors
    inline COLOR COL_ValueOverTop(void);
    inline COLOR COL_ValueTop(void);
    inline COLOR COL_ValueMid(void);
    inline COLOR COL_ValueLow(void);

    // Sniper scope
    inline COLOR COL_ScopeMask(void);
    inline COLOR COL_ScopeDetails(void);
    inline COLOR COL_ScopeLedIdle(void);
    inline COLOR COL_ScopeLedFire(void);

    // Weapon selection list
    inline COLOR COL_WeaponBorder(void);
    inline COLOR COL_WeaponIcon(void);
    inline COLOR COL_WeaponNoAmmo(void);
    inline COLOR COL_WeaponWanted(void);
};

// Main HUD structure
extern CHud _HUD;

// Define color getting methods
#include "Colors.inl"

#endif
