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

#include "StdH.h"

#include "HUD.h"

// Main HUD structure
CHud _HUD;

// Define method hooks
CHud::CGetPropsFunc     CHud::pGetSP = NULL;
CHud::CPowerUpSoundFunc CHud::pPlayPowerUpSound_opt = NULL;
CHud::CIsConnectedFunc  CHud::pIsConnected_opt = NULL;
CHud::CWorldGlaringFunc CHud::pGetWorldGlaring_opt = NULL;
CHud::CParticlesFunc    CHud::pRenderChainsawParticles_opt = NULL;
CHud::CRenderHudFunc    CHud::pRenderHud = NULL;
CHud::CRenderWeaponFunc CHud::pRenderWeaponModel_opt = NULL;
CHud::CRenderCrossFunc  CHud::pRenderCrosshair_opt = NULL;
CHud::CGetAmmoFunc      CHud::pGetAmmo = NULL;
CHud::CGetMaxAmmoFunc   CHud::pGetMaxAmmo = NULL;

// Update weapon and ammo tables with current info
void CHud::UpdateWeaponArsenal(void) {
  // Ammo quantities
  GetAmmo()[0].iAmmo    = _penWeapons->m_iShells;
  GetAmmo()[0].iMaxAmmo = _penWeapons->m_iMaxShells;
  GetAmmo()[1].iAmmo    = _penWeapons->m_iBullets;
  GetAmmo()[1].iMaxAmmo = _penWeapons->m_iMaxBullets;
  GetAmmo()[2].iAmmo    = _penWeapons->m_iRockets;
  GetAmmo()[2].iMaxAmmo = _penWeapons->m_iMaxRockets;
  GetAmmo()[3].iAmmo    = _penWeapons->m_iGrenades;
  GetAmmo()[3].iMaxAmmo = _penWeapons->m_iMaxGrenades;

#if SE1_GAME != SS_TFE
  GetAmmo()[4].iAmmo    = _penWeapons->m_iNapalm;
  GetAmmo()[4].iMaxAmmo = _penWeapons->m_iMaxNapalm;
  GetAmmo()[5].iAmmo    = _penWeapons->m_iSniperBullets;
  GetAmmo()[5].iMaxAmmo = _penWeapons->m_iMaxSniperBullets;
#endif

  GetAmmo()[6].iAmmo    = _penWeapons->m_iElectricity;
  GetAmmo()[6].iMaxAmmo = _penWeapons->m_iMaxElectricity;
  GetAmmo()[7].iAmmo    = _penWeapons->m_iIronBalls;
  GetAmmo()[7].iMaxAmmo = _penWeapons->m_iMaxIronBalls;

  // Prepare ammo table for weapon possesion
  for (INDEX i = 0; i < GetAmmo().Count(); i++) {
    GetAmmo()[i].bHasWeapon = FALSE;
  }

  INDEX iAvailableWeapons = _penWeapons->m_iAvailableWeapons;

  // Weapon possesion
  for (INDEX iCheck = 1; iCheck < GetWeapons().Count(); iCheck++) {
    HudWeapon *pInfo = &GetWeapons()[iCheck];

    // Regular weapons
    pInfo->bHasWeapon = iAvailableWeapons & (1 << (pInfo->iWeapon - 1));

    if (pInfo->paiAmmo != NULL) {
      pInfo->paiAmmo->bHasWeapon |= pInfo->bHasWeapon;
    }
  }
};

// Prepare interface for rendering
BOOL CHud::PrepareHUD(CPlayer *penCurrent, CDrawPort *pdpCurrent)
{
  static CSymbolPtr pfOpacity("hud_fOpacity");
  static CSymbolPtr pfScaling("hud_fScaling");

  // Clear beforehand in case it can't reach GatherPlayers() call at the end
  // Realistically it's not supposed to be used at all if HUD preparation fails
  _cenPlayers.Clear();

  // No player or no owner for snooping
  if (penCurrent == NULL || penCurrent->GetFlags() & ENF_DELETED) return FALSE;

  // Find last values in case of a predictor
  _penLast = penCurrent;

  if (penCurrent->IsPredictor()) {
    _penLast = (CPlayer *)_penLast->GetPredicted();
  }

  // Make sure there's an entity
  ASSERT(_penLast != NULL);
  if (_penLast == NULL) return FALSE;

  // Player entities
  _penPlayer = penCurrent;
  _penWeapons = (CPlayerWeapons *)&*_penPlayer->m_penWeapons;

  // Get drawport with its dimensions
  _pdp = pdpCurrent;
  _vpixScreen = PIX2D(_pdp->GetWidth(), _pdp->GetHeight());

  // Update time
  _tmLast = _tmNow;
  _tmNow = _pTimer->CurrentTick();

  // Limit scaling
  _fHudScaling = Clamp(pfScaling.GetFloat(), 0.05f, 2.0f);

  // Set wide adjustment dynamically and apply it to scaling
  _fWideAdjustment = ((FLOAT)_vpixScreen(2) / (FLOAT)_vpixScreen(1)) * (4.0f / 3.0f);
  _fHudScaling *= _fWideAdjustment;

  _vScaling(1) = (FLOAT)_vpixScreen(1) / 640.0f;
  _vScaling(2) = (FLOAT)_vpixScreen(2) / (480.0f * _fWideAdjustment);

  // Determine screen edges
  INDEX iScreenEdgeX = ClampDn(_psScreenEdgeX.GetIndex(), (INDEX)0);
  INDEX iScreenEdgeY = ClampDn(_psScreenEdgeY.GetIndex(), (INDEX)0);

  _vpixTL = PIX2D(iScreenEdgeX + 1, iScreenEdgeY + 1);
  _vpixBR = PIX2D(640 - _vpixTL(1), (480 * _fWideAdjustment) - _vpixTL(2));

  _ulAlphaHUD = NormFloatToByte(Clamp(pfOpacity.GetFloat(), 0.0f, 1.0f));

  // Setup HUD theme
  const INDEX iCurrentTheme = Clamp(_psTheme.GetIndex(), (INDEX)0, INDEX(E_HUD_MAX - 1));
  _bTSEColors = (iCurrentTheme > E_HUD_TFE);
  _bTSETheme = (iCurrentTheme >= E_HUD_TSE);

  static const HudColorSet *aColorSets[E_HUD_MAX] = {
    &_hcolTFE, &_hcolWarped, &_hcolTSE, &_hcolSSR,
  };

  pColorSet = aColorSets[iCurrentTheme];

  // Set colors
  _colHUD = COL_Base();
  _colBorder = _colHUD;
  _colIconStd = COL_Icon();

  _colMax = COL_ValueOverTop();
  _colTop = COL_ValueTop();
  _colMid = COL_ValueMid();
  _colLow = COL_ValueLow();

  // Select current fonts
  _pfdCurrentText = &_afdText[iCurrentTheme];
  _pfdCurrentNumbers = &_afdNumbers[iCurrentTheme];

  // Calculate relative scaling for the text font
  _fTextFontScale = (FLOAT)_pfdDisplayFont->GetHeight() / (FLOAT)_pfdCurrentText->GetHeight();

  // Determine current gamemode
  _eGameMode = E_GM_SP;

  if (!pGetSP()->sp_bSinglePlayer) {
    // Gather players for multiplayer
    GatherPlayers();

    if (pGetSP()->sp_bCooperative) {
      _eGameMode = E_GM_COOP;

    } else if (!pGetSP()->sp_bUseFrags) {
      _eGameMode = E_GM_SCORE;

    } else {
      _eGameMode = E_GM_FRAG;
    }
  }

  return TRUE;
};

// Render entire interface
void CHud::DrawHUD(const CPlayer *penCurrent, BOOL bSnooping, const CPlayer *penOwner)
{
  static CSymbolPtr pfWeapons("hud_tmWeaponsOnScreen");
  static CSymbolPtr pbLatency("hud_bShowLatency");

  const FLOAT tmWeaponsOnScreen = pfWeapons.GetFloat();
  const INDEX bShowLatency = pbLatency.GetIndex();

  // No player or no owner for snooping
  if (penCurrent == NULL || penCurrent->GetFlags() & ENF_DELETED) return;
  if (bSnooping && penOwner == NULL) return;

  // Adjust border color during snooping
  if (bSnooping) {
    _colBorder = COL_SnoopingLight();

    // Darken flash and scale
    if (ULONG(_tmNow * 5) & 1) {
      _colBorder = COL_SnoopingDark();
      _fHudScaling *= 0.933f;
    }
  }

#if SE1_GAME != SS_TFE
  // Render sniper mask (even while snooping)
  CPlayerWeapons &enMyWeapons = (CPlayerWeapons &)*penOwner->m_penWeapons;

  if (enMyWeapons.m_iCurrentWeapon == WEAPON_SNIPER && enMyWeapons.m_bSniping) {
    DrawSniperMask();
  }
#endif

  // Set font and unit sizes
  _pdp->SetFont(_pfdCurrentNumbers);
  ResetScale(_fHudScaling);

  // Render parts of the interface
  SIconTexture *ptoWantedWeapon = NULL;
  SIconTexture *ptoCurrentAmmo = NULL;

  RenderVitals();
  RenderCurrentWeapon(&ptoWantedWeapon, &ptoCurrentAmmo);

  Rescale(0.8f);
  RenderActiveArsenal(ptoCurrentAmmo);
  ResetScale(_fHudScaling);

  // If weapon change is in progress
  if (_tmNow - _penWeapons->m_tmWeaponChangeRequired < tmWeaponsOnScreen) {
    // Determine amount of available weapons
    INDEX ctWeapons = 0;

    for (INDEX iCount = 1; iCount < GetWeapons().Count(); iCount++) {
      if (GetWeapons()[iCount].iWeapon != WEAPON_NONE && GetWeapons()[iCount].iWeapon != WEAPON_DOUBLECOLT
       && GetWeapons()[iCount].bHasWeapon) {
        ctWeapons++;
      }
    }

    FLOAT fCol = 320.0f - (ctWeapons * units.fAdv - units.fOne) * 0.5f;
    FLOAT fRow = _vpixBR(2) - units.fHalf - units.fNext * 3;

    // Display all available weapons
    for (INDEX iWeapon = 0; iWeapon < GetWeapons().Count(); iWeapon++) {
      HudWeapon &wiInfo = GetWeapons()[iWeapon];

      // Skip if no weapon
      if (wiInfo.iWeapon == WEAPON_NONE || wiInfo.iWeapon == WEAPON_DOUBLECOLT || !wiInfo.bHasWeapon) {
        continue;
      }

      // Display weapon icon
      COLOR colBorder = COL_WeaponBorder();
      COLOR colIcon = COL_WeaponIcon();

      // No ammo
      if (wiInfo.paiAmmo != NULL && wiInfo.paiAmmo->iAmmo == 0) {
        colBorder = colIcon = COL_WeaponNoAmmo();

      // Selected weapon
      } else if (ptoWantedWeapon == wiInfo.ptoWeapon) {
        colBorder = colIcon = COL_WeaponWanted();
      }

      DrawBorder(fCol, fRow, units.fOne, units.fOne, colBorder);
      DrawIcon(fCol, fRow, *wiInfo.ptoWeapon, colIcon, 1.0f, FALSE);

      // Advance to the next position
      fCol += units.fAdv;
    }
  }

  Rescale(0.5f / _fWideAdjustment);
  RenderBars();
  ResetScale(_fHudScaling);

  Rescale(0.6f);
  RenderGameModeInfo();
  ResetScale(_fHudScaling);

  // Display local client latency
  if (bShowLatency) {
    const FLOAT fTextScale = (_vScaling(1) + 1) * 0.5f * _fTextFontScale;

    _pfdCurrentText->SetFixedWidth();
    _pdp->SetFont(_pfdCurrentText);
    _pdp->SetTextScaling(fTextScale);
    _pdp->SetTextCharSpacing(-2.0f * fTextScale);

    CTString strLatency;
    strLatency.PrintF("%4.0fms", _penPlayer->m_tmLatency * 1000.0f);

    const PIX pixFontHeight = _pfdCurrentText->GetHeight() * fTextScale + fTextScale + 1;
    _pdp->PutTextR(strLatency, _vpixScreen(1), _vpixScreen(2) - pixFontHeight, C_WHITE | CT_OPAQUE);
  }

  // Restore font defaults
  _pfdCurrentText->SetVariableWidth();

  RenderCheats();

#if SE1_GAME == SS_TFE
  // Display real time
  INDEX iClockMode = _psShowClock.GetIndex();

  if (!ClassicsCore_IsCustomModActive() && iClockMode) {
    // Set font
    _pdp->SetFont(_pfdConsoleFont);
    _pdp->SetTextScaling(1.0f);
    _pdp->SetTextAspect(1.0f);

    // Determine time
    time_t iLongTime;
    time(&iLongTime);
    tm *tmNewTime = localtime(&iLongTime);

    CTString strTime;

    // Show seconds as extra
    if (iClockMode > 1) {
      strTime.PrintF("%2d:%02d:%02d", tmNewTime->tm_hour, tmNewTime->tm_min, tmNewTime->tm_sec);
    } else {
      strTime.PrintF("%2d:%02d", tmNewTime->tm_hour, tmNewTime->tm_min);
    }

    _pdp->PutTextR(strTime, _vpixScreen(1) - 3, 2, C_lYELLOW | CT_OPAQUE);
  }
#endif
};

// Display tags above players
void CHud::RenderPlayerTags(CPlayer *penThis, CPerspectiveProjection3D &prProjection) {
  // Tags are disabled or it's a singleplayer game
  const INDEX iPlayerTags = _psPlayerTags.GetIndex();
  if (iPlayerTags <= 0 || pGetSP()->sp_bSinglePlayer) return;

  // Set font
  const FLOAT fScaling = HEIGHT_SCALING(_pdp);
  const FLOAT fTextScale = fScaling * _fTextFontScale;
  _pdp->SetFont(_pfdCurrentText);
  _pdp->SetTextScaling(fTextScale);

  // Render tags for each player
  FOREACHINDYNAMICCONTAINER(_cenPlayers, CPlayer, iten) {
    CPlayer *pen = iten;

    // Skip this player (or a prediction of it)
    if (pen == penThis->GetPredictionTail()) continue;

    const BOOL bAlive = (pen->GetFlags() & ENF_ALIVE);
    const FLOAT3D vPlayer = pen->GetLerpedPlacement().pl_PositionVector;

    FLOATmatrix3D mThis;
    MakeRotationMatrixFast(mThis, penThis->GetLerpedPlacement().pl_OrientationAngle);

    // Calculate tag position on screen
    FLOAT3D vBoxCenter(0, 0, 0);
    FLOAT fBoxTop = 0.5f;

    if (bAlive) {
      FLOATaabbox3D box;
      pen->GetBoundingBox(box);
      box -= pen->GetPlacement().pl_PositionVector;

      vBoxCenter = box.Center();

      // Get half of the longest axis
      const FLOAT3D vSize = box.Size();
      fBoxTop = Max(Max(vSize(1), vSize(2)), vSize(3)) * 0.5f;
    }

    const FLOAT3D vPlayerCenter = vPlayer + vBoxCenter; // Center of the target player
    const FLOAT3D vTagOffset = FLOAT3D(0, fBoxTop, 0) * mThis; // Offset relative to the local rotation

    FLOAT3D vTag(0, 0, 0);
    prProjection.ProjectCoordinate(vPlayerCenter + vTagOffset, vTag);

    if (vTag(3) >= 0.0f) continue;

    vTag(2) = _vpixScreen(2) - vTag(2);

    // Marker color based on health level (0..100 health = 0..2 ratio)
    const FLOAT fHealthRatio = Clamp(pen->GetHealth() * 0.02f, 0.0f, 2.0f);
    COLOR colTag;

    if (fHealthRatio < 1.0f) {
      colTag = LerpColor(C_RED, C_YELLOW, fHealthRatio);
    } else {
      colTag = LerpColor(C_YELLOW, C_GREEN, fHealthRatio - 1.0f);
    }

    // Alpha level based on relative distance (0..32 meters = 95..191 alpha)
    const FLOAT fDist = (vPlayer - penThis->GetLerpedPlacement().pl_PositionVector).Length();
    const FLOAT fDistRatio = Clamp(fDist * 0.03125f, 0.0f, 1.0f);
    UBYTE ubAlpha = 0xBF - UBYTE(fDistRatio * 96.0f);

    // Marker size based on relative distance
    const FLOAT fMarkerSize = (6.0f - fDistRatio * 3.0f) * fScaling;

    _pdp->InitTexture(&tex.toMarker);
    _pdp->AddTexture(vTag(1) - fMarkerSize, vTag(2) - fMarkerSize * 2,
                     vTag(1) + fMarkerSize, vTag(2), colTag | ubAlpha);
    _pdp->FlushRenderingQueue();

    // Only marker
    if (iPlayerTags < 2) continue;

    // Player name
    CTString strPlayerName = pen->GetPlayerName();

    INDEX iMaxChars = 30;
    INDEX iMaxAllow = 32; // Two extra characters after the limit

    // Names with decorations
    if (_psDecoratedNames.GetIndex()) {
      iMaxChars = IData::GetDecoratedChar(strPlayerName, iMaxChars);
      iMaxAllow = IData::GetDecoratedChar(strPlayerName, iMaxAllow);

    } else {
      strPlayerName = strPlayerName.Undecorated();
    }

    // Limit length
    if (strPlayerName.Length() > iMaxAllow) {
      strPlayerName.TrimRight(iMaxChars);
      strPlayerName += "^r...";
    }

    // Add distance
    if (iPlayerTags > 2) {
      strPlayerName.PrintF("%s^r (%dm)", strPlayerName, (INDEX)fDist);
    }

    // Alpha level based on relative distance (0..32 meters = 95..255 alpha)
    ubAlpha = 0xFF - UBYTE(fDistRatio * 160.0f);
    const COLOR colName = (bAlive ? COL_PlayerNames() : COL_ValueLow());

    const PIX pixCharH = (_pfdCurrentText->GetHeight() - 2) * fTextScale;
    _pdp->PutTextC(strPlayerName, vTag(1), vTag(2) - pixCharH - fMarkerSize * 2, colName | ubAlpha);
  }
};

// Player function patch
class CPlayerPatch : public CPlayer {
  public:
    void P_RenderHUD(RENDER_ARGS(prProjection, pdp, vLightDir, colLight, colAmbient, bRenderWeapon, iEye));
};

// Initialize everything for drawing the HUD
void CHud::Initialize(void) {
  StructPtr pFuncPtr;

  // Abort HUD initialization if some method can't be hooked
  #define GET_SYMBOL_OPT(_Symbol) pFuncPtr = StructPtr(ClassicsCore_GetEntitiesSymbol(_Symbol))

  // Display an assertion only if entities aren't supposed to be modded
  #define GET_SYMBOL(_Symbol) GET_SYMBOL_OPT(_Symbol); \
    if (pFuncPtr.iAddress == NULL) { \
      if (!_bModdedEntities) { ASSERTALWAYS("Cannot hook '" #_Symbol "' for Advanced HUD!"); } \
      CPrintF(TRANS("Cannot hook '%s'!\nAborting HUD initialization...\n"), _Symbol); \
      return; \
    }

  // Hook library methods
  GET_SYMBOL("?GetSP@@YAPBVCSessionProperties@@XZ");
  pGetSP = pFuncPtr(CGetPropsFunc());

#if SE1_GAME != SS_TFE
  GET_SYMBOL_OPT("?PlayPowerUpSound@CPlayer@@QAEXXZ");
  pPlayPowerUpSound_opt = pFuncPtr(CPowerUpSoundFunc());

  GET_SYMBOL_OPT("?RenderChainsawParticles@CPlayer@@QAEXH@Z");
  pRenderChainsawParticles_opt = pFuncPtr(CParticlesFunc());
#endif

  GET_SYMBOL_OPT("?IsConnected@CPlayer@@QBEHXZ");
  pIsConnected_opt = pFuncPtr(CIsConnectedFunc());

  GET_SYMBOL_OPT("?GetWorldGlaring@CPlayer@@QAEKXZ");
  pGetWorldGlaring_opt = pFuncPtr(CWorldGlaringFunc());

#if SE1_VER < SE1_107
  GET_SYMBOL("?RenderHUD@CPlayer@@QAEXAAVCPerspectiveProjection3D@@PAVCDrawPort@@V?$Vector@M$02@@KKH@Z");
  pRenderHud = pFuncPtr(CRenderHudFunc());

  GET_SYMBOL_OPT("?RenderWeaponModel@CPlayerWeapons@@QAEXAAVCPerspectiveProjection3D@@PAVCDrawPort@@V?$Vector@M$02@@KKH@Z");
  pRenderWeaponModel_opt = pFuncPtr(CRenderWeaponFunc());

#else
  GET_SYMBOL("?RenderHUD@CPlayer@@QAEXAAVCPerspectiveProjection3D@@PAVCDrawPort@@V?$Vector@M$02@@KKHJ@Z");
  pRenderHud = pFuncPtr(CRenderHudFunc());

  GET_SYMBOL_OPT("?RenderWeaponModel@CPlayerWeapons@@QAEXAAVCPerspectiveProjection3D@@PAVCDrawPort@@V?$Vector@M$02@@KKHJ@Z");
  pRenderWeaponModel_opt = pFuncPtr(CRenderWeaponFunc());
#endif

  GET_SYMBOL_OPT("?RenderCrosshair@CPlayerWeapons@@QAEXAAVCProjection3D@@PAVCDrawPort@@AAVCPlacement3D@@@Z");
  pRenderCrosshair_opt = pFuncPtr(CRenderCrossFunc());

  GET_SYMBOL("?GetAmmo@CPlayerWeapons@@QAEJXZ");
  pGetAmmo = pFuncPtr(CGetAmmoFunc());

  GET_SYMBOL("?GetMaxAmmo@CPlayerWeapons@@QAEJXZ");
  pGetMaxAmmo = pFuncPtr(CGetMaxAmmoFunc());

  // Patch HUD rendering function
  CreatePatch(pRenderHud, &CPlayerPatch::P_RenderHUD, "CPlayer::RenderHUD(...)");

  try {
    // Load fonts for each theme
    for (INDEX iTheme = 0; iTheme < E_HUD_MAX; iTheme++) {
      if (iTheme <= E_HUD_TSE) {
        _afdText[iTheme].Load_t(CTFILENAME("Fonts\\Display3-narrow.fnt"));
        _afdNumbers[iTheme].Load_t(CTFILENAME("Fonts\\Numbers3.fnt"));

      } else {
        _afdText[iTheme].Load_t(CTFILENAME("Fonts\\Rev_HUD\\Cabin.fnt"));
        _afdNumbers[iTheme].Load_t(CTFILENAME("Fonts\\Rev_HUD\\Numbers.fnt"));
      }

      _afdText[iTheme].SetCharSpacing(0);
      _afdText[iTheme].SetLineSpacing(1);
    }

    tex.LoadTextures();

  } catch (char *strError) {
    FatalError(strError);
  }

  // Prepare weapon arsenal
  HudArsenal::AmmoStack &aAmmo = arWeapons.aAmmo;
  #define NEW_AMMO aAmmo.Push().Setup
  #define NEW_WEAPON arWeapons.aWeapons.Push().Setup

  // Added in order of appearance instead of using 'aiAmmoRemap'
  NEW_AMMO(&tex.toAShells);
  NEW_AMMO(&tex.toABullets);
  NEW_AMMO(&tex.toARockets);
  NEW_AMMO(&tex.toAGrenades);
  NEW_AMMO(&tex.toANapalm);
  NEW_AMMO(&tex.toASniperBullets);
  NEW_AMMO(&tex.toAElectricity);
  NEW_AMMO(&tex.toAIronBall);

  // Added in order of appearance instead of using 'aiWeaponRemap'
  NEW_WEAPON(WEAPON_NONE,  NULL);
  NEW_WEAPON(WEAPON_KNIFE, &tex.toWKnife);

  #if SE1_GAME != SS_TFE
    NEW_WEAPON(WEAPON_CHAINSAW, &tex.toWChainsaw, NULL);
  #endif

  NEW_WEAPON(WEAPON_COLT,            &tex.toWColt);
  NEW_WEAPON(WEAPON_DOUBLECOLT,      &tex.toWColt);
  NEW_WEAPON(WEAPON_SINGLESHOTGUN,   &tex.toWSingleShotgun,   &aAmmo[0]);
  NEW_WEAPON(WEAPON_DOUBLESHOTGUN,   &tex.toWDoubleShotgun,   &aAmmo[0]);
  NEW_WEAPON(WEAPON_TOMMYGUN,        &tex.toWTommygun,        &aAmmo[1]);
  NEW_WEAPON(WEAPON_MINIGUN,         &tex.toWMinigun,         &aAmmo[1]);
  NEW_WEAPON(WEAPON_ROCKETLAUNCHER,  &tex.toWRocketLauncher,  &aAmmo[2]);
  NEW_WEAPON(WEAPON_GRENADELAUNCHER, &tex.toWGrenadeLauncher, &aAmmo[3]);

  #if SE1_GAME != SS_TFE
    NEW_WEAPON(WEAPON_FLAMER, &tex.toWFlamer, &aAmmo[4]);
    NEW_WEAPON(WEAPON_SNIPER, &tex.toWSniper, &aAmmo[5]);
  #endif

  NEW_WEAPON(WEAPON_LASER,      &tex.toWLaser,      &aAmmo[6]);
  NEW_WEAPON(WEAPON_IRONCANNON, &tex.toWIronCannon, &aAmmo[7]);
};

// Clean everything up before disabling the plugin
void CHud::End(void) {
  _tmNow = -1.0f;
  _tmLast = -1.0f;

  GetAmmo().PopAll();
  GetWeapons().PopAll();
};

void CPlayerPatch::P_RenderHUD(RENDER_ARGS(prProjection, pdp, vLightDir, colLight, colAmbient, bRenderWeapon, iEye))
{
  // Replace HUD everywhere or only in vanilla if entities haven't been modified
  const BOOL bReplaceHUD = _psEnable.GetIndex() > 1 || (_psEnable.GetIndex() == 1 && !_bModdedEntities);

  // Proceed to the original function instead
  if (!bReplaceHUD) {
    (this->*CHud::pRenderHud)(RENDER_ARGS_RAW(prProjection, pdp, vLightDir, colLight, colAmbient, bRenderWeapon, iEye));
    return;
  }

  static CSymbolPtr pbRenderModels("gfx_bRenderModels");
  static CSymbolPtr pbShowWeapon("hud_bShowWeapon");
  static CSymbolPtr pbShowInterface("hud_bShowInfo");
  static CSymbolPtr ptmSnoopingTime("plr_tmSnoopingTime");

  // Save intended view placement before rendering the weapon
  const CPlacement3D plViewOld = prProjection.ViewerPlacementR();

  CPlayerWeapons &enWeapons = (CPlayerWeapons &)*m_penWeapons;

#if SE1_GAME == SS_TFE
  const BOOL bSniping = FALSE;
#else
  const BOOL bSniping = enWeapons.m_bSniping;
#endif

  // Don't render the weapon while sniping
  if (CHud::pRenderWeaponModel_opt != NULL && pbShowWeapon.GetIndex() && pbRenderModels.GetIndex() && !bSniping)
  {
    #if SE1_VER < SE1_107
      (enWeapons.*CHud::pRenderWeaponModel_opt)(prProjection, pdp, vLightDir, colLight, colAmbient, bRenderWeapon);
    #else
      (enWeapons.*CHud::pRenderWeaponModel_opt)(prProjection, pdp, vLightDir, colLight, colAmbient, bRenderWeapon, iEye);
    #endif
  }

#if SE1_GAME != SS_TFE
  if (CHud::pRenderChainsawParticles_opt != NULL && m_iViewState == PVT_PLAYEREYES)
  {
    prProjection.ViewerPlacementL() = plViewOld;
    prProjection.Prepare();

    CAnyProjection3D apr;
    apr = prProjection;
    Stereo_AdjustProjection(*apr, iEye, 1);

    Particle_PrepareSystem(pdp, apr);
    Particle_PrepareEntity(2.0f, FALSE, FALSE, this);
    (this->*CHud::pRenderChainsawParticles_opt)(FALSE);
    Particle_EndSystem();
  }
#endif

  // Prepare new HUD
  BOOL bSnooping = FALSE;
  CPlayer *penHUDPlayer = this;

  if (penHUDPlayer->IsPredicted()) {
    penHUDPlayer = (CPlayer *)penHUDPlayer->GetPredictor();
  }

  // Check if snooping is needed
  const TIME tmDelta = _pTimer->CurrentTick() - enWeapons.m_tmSnoopingStarted;
  const FLOAT tmSnooping = ptmSnoopingTime.Exists() ? ptmSnoopingTime.GetFloat() : 1.0f;

  if (tmDelta < tmSnooping) {
    ASSERT(enWeapons.m_penTargeting != NULL);
    penHUDPlayer = (CPlayer *)&*enWeapons.m_penTargeting;
    bSnooping = TRUE;
  }

  // Can't use the HUD if it can't be prepared
  const BOOL bPrepared = _HUD.PrepareHUD(penHUDPlayer, pdp);

  if (bPrepared) {
    // Display tags above players in coop, in demos or while observing
    const BOOL bDemo = (_psTagsInDemos.GetIndex() && _pNetwork->IsPlayingDemo());
    const BOOL bObserving = (_psTagsForObservers.GetIndex() && _pNetwork->IsNetworkEnabled() && !IWorld::AnyLocalPlayers());

    if (CHud::pGetSP()->sp_bCooperative || bDemo || bObserving) {
      prProjection.ViewerPlacementL() = plViewOld;
      prProjection.Prepare();
      _HUD.RenderPlayerTags(this, prProjection);
    }
  }

  CPlacement3D plView;

  // Player view
  if (m_iViewState == PVT_PLAYEREYES) {
    plView = en_plViewpoint;
    plView.RelativeToAbsolute(GetPlacement());

  // Camera view
  } else if (m_iViewState == PVT_3RDPERSONVIEW) {
    plView = m_pen3rdPersonView->GetPlacement();
  }

  // Render crosshair while not sniping
  if (CHud::pRenderCrosshair_opt != NULL && !bSniping)
  {
    (enWeapons.*CHud::pRenderCrosshair_opt)(prProjection, pdp, plView);
  }

  // Toggleable red screen on damage
  static CSymbolPtr pbRedScreen("axs_bRedScreenOnDamage");

  if (!pbRedScreen.Exists() || pbRedScreen.GetIndex()) {
    // Do screen blending for wounding
    CPlayer *pen = (CPlayer *)GetPredictionTail();
  #if SE1_GAME != SS_REV
    ULONG ulA = pen->m_fDamageAmmount * 5.0f;
  #else
    ULONG ulA = pen->m_fDamageAmount * 5.0f;
  #endif

    FLOAT tmSinceWounding = _pTimer->CurrentTick() - pen->m_tmWoundedTime;

    if (tmSinceWounding < 0.001f) {
      ulA = (ulA + 64) / 2;
    }

    // Add rest of the blend amount
    ulA = ClampUp(ulA, (ULONG)224);

    if (m_iViewState == PVT_PLAYEREYES) {
      pdp->dp_ulBlendingRA += 255 * ulA;
      pdp->dp_ulBlendingA += ulA;
    }
  }

  // Add world glaring
  if (CHud::pGetWorldGlaring_opt != NULL)
  {
    COLOR colGlare = (this->*CHud::pGetWorldGlaring_opt)();
    UBYTE ubR, ubG, ubB, ubA;
    ColorToRGBA(colGlare, ubR, ubG, ubB, ubA);

    if (ubA != 0) {
      pdp->dp_ulBlendingRA += ULONG(ubR) * ULONG(ubA);
      pdp->dp_ulBlendingGA += ULONG(ubG) * ULONG(ubA);
      pdp->dp_ulBlendingBA += ULONG(ubB) * ULONG(ubA);
      pdp->dp_ulBlendingA  += ULONG(ubA);
    }
  }

  // Do all queued screen blendings
  pdp->BlendScreen();

  // Draw new HUD
  if (bPrepared && pbShowInterface.GetIndex()) {
    _HUD.DrawHUD(penHUDPlayer, bSnooping, this);
  }
};
