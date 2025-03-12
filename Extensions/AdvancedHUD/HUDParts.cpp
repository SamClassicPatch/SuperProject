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

// Armor & health constants (they don't reflect tourist/easy top values by design)
#define TOP_ARMOR  100
#define TOP_HEALTH 100

void CHud::RenderVitals(void) {
  // Prepare and draw health info
  FLOAT fValue = ClampDn(_penPlayer->GetHealth(), 0.0f);
  FLOAT fNormValue = fValue / TOP_HEALTH;

  // Adjust border width based on which value is bigger
  const FLOAT fArmor = _penPlayer->m_fArmor;
  const FLOAT fMaxHealthArmor = Max(fValue, fArmor);
  FLOAT fBorderWidth = Clamp((FLOAT)floor(log10(fMaxHealthArmor) + 1.0f), 3.0f, 5.0f);

  if (_psTheme.GetIndex() == E_HUD_SSR) {
    fBorderWidth = 3.0f;
  }

  CTString strValue;
  strValue.PrintF("%d", (SLONG)ceil(fValue));

  PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, 0.5f, 0.25f, FALSE);

  FLOAT fMoverX, fMoverY;
  COLOR col = AddShaker(5, fValue, _penLast->m_iLastHealth, _penLast->m_tmHealthChanged, fMoverX, fMoverY);

  if (col == NONE) col = GetCurrentColor(fNormValue);

  FLOAT fCol = _vpixTL(1) + units.fHalf;
  FLOAT fRow = _vpixBR(2) - units.fHalf;

  DrawBorder(fCol + fMoverX, fRow + fMoverY, units.fOne, units.fOne, _colBorder);
  DrawIcon(fCol + fMoverX, fRow + fMoverY, tex.toHealth, _colIconStd, fNormValue, TRUE);

  fCol += units.fAdv + units.fChar * (fBorderWidth * 0.5f) - units.fHalf;

  DrawBorder(fCol, fRow, units.fChar * fBorderWidth, units.fOne, _colBorder);
  DrawString(fCol, fRow, strValue, col, fNormValue);

  // Don't display empty armor
  if (fArmor <= 0.0f) return;

  fValue = fArmor;
  fNormValue = fValue / TOP_ARMOR;
  strValue.PrintF("%d", (SLONG)ceil(fValue));

  PrepareColorTransitions(_colMax, _colTop, _colMid, C_lGRAY, 0.5f, 0.25f, FALSE);

  fCol = _vpixTL(1) + units.fHalf;
  fRow = _vpixBR(2) - (units.fNext + units.fHalf);

  AddShaker(3, fValue, _penLast->m_iLastArmor, _penLast->m_tmArmorChanged, fMoverX, fMoverY);

  fCol += fMoverX;
  fRow += fMoverY;

  DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);

  if (fValue <= 50.5f) {
    DrawIcon(fCol, fRow, tex.atoArmor[0], _colIconStd, fNormValue, FALSE);

  } else if (fValue <= 100.5f) {
    DrawIcon(fCol, fRow, tex.atoArmor[1], _colIconStd, fNormValue, FALSE);

  } else {
    DrawIcon(fCol, fRow, tex.atoArmor[2], _colIconStd, fNormValue, FALSE);
  }

  fCol -= fMoverX;
  fRow -= fMoverY;

  fCol += units.fAdv + units.fChar * (fBorderWidth * 0.5f) - units.fHalf;
  DrawBorder(fCol, fRow, units.fChar * fBorderWidth, units.fOne, _colBorder);
  DrawString(fCol, fRow, strValue, GetCurrentColor(fNormValue), fNormValue);
};

void CHud::RenderCurrentWeapon(SIconTexture **pptoWantedWeapon, SIconTexture **pptoCurrentAmmo) {
  // Prepare and draw ammo and weapon info
  SIconTexture *ptoAmmo = NULL;
  SIconTexture *ptoCurrent = NULL;
  SIconTexture *ptoWanted = NULL;
  INDEX iCurrentWeapon = _penWeapons->m_iCurrentWeapon;
  INDEX iWantedWeapon  = _penWeapons->m_iWantedWeapon;

  // Determine corresponding ammo and weapon texture component
  HudWeapon *pWeaponInfo = NULL;

  if (GetWeaponInfo(&pWeaponInfo, iCurrentWeapon)) {
    ptoCurrent = pWeaponInfo->ptoWeapon;

    if (pWeaponInfo->paiAmmo != NULL) {
      ptoAmmo = pWeaponInfo->paiAmmo->ptoAmmo;
    }
  }

  if (GetWeaponInfo(&pWeaponInfo, iWantedWeapon)) {
    ptoWanted = pWeaponInfo->ptoWeapon;
  }

  // Borrow icons
  if (pptoCurrentAmmo != NULL) {
    *pptoCurrentAmmo = ptoAmmo;
  }

  if (pptoWantedWeapon != NULL) {
    *pptoWantedWeapon = ptoWanted;
  }

  FLOAT fCol = 320;
  FLOAT fRow = _vpixBR(2) - units.fHalf;

  // Draw weapons with ammo
  if (ptoAmmo != NULL && !pGetSP()->sp_bInfiniteAmmo) {
    // Get amount of ammo
    INDEX iMaxValue = (_penWeapons->*pGetMaxAmmo)();
    INDEX iValue = (_penWeapons->*pGetAmmo)();
    FLOAT fNormValue = (FLOAT)iValue / (FLOAT)iMaxValue;

    CTString strValue;
    strValue.PrintF("%d", iValue);

    PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, (_bTSEColors ? 0.30f : 0.5f), (_bTSEColors ? 0.15f : 0.25f), FALSE);
    BOOL bDrawAmmoIcon = _fCustomScaling <= 1.0f;

    // Draw weapon and its ammo
    FLOAT fMoverX, fMoverY;
    COLOR col = AddShaker(4, iValue, _penLast->m_iLastAmmo, _penLast->m_tmAmmoChanged, fMoverX, fMoverY);

    // Override with the current color
    if (_bTSEColors || col == NONE) {
      col = GetCurrentColor(fNormValue);
    }

    fCol -= units.fAdv + units.fChar * 1.5f - units.fHalf;

    DrawBorder(fCol + fMoverX, fRow + fMoverY, units.fOne, units.fOne, _colBorder);
    DrawIcon(fCol + fMoverX, fRow + fMoverY, *ptoCurrent, _colIconStd, fNormValue, !bDrawAmmoIcon);

    fCol += units.fAdv + units.fChar * 1.5f - units.fHalf;
    DrawBorder(fCol, fRow, units.fChar * 3, units.fOne, _colBorder);
    DrawString(fCol, fRow, strValue, col, fNormValue);

    if (bDrawAmmoIcon) {
      fCol += units.fAdv + units.fChar * 1.5f - units.fHalf;
      DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
      DrawIcon(fCol, fRow, *ptoAmmo, _colIconStd, fNormValue, TRUE);
    }

  // Draw weapons without ammo
  } else if (ptoCurrent != NULL) {
    DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
    DrawIcon(fCol, fRow, *ptoCurrent, _colIconStd, 1.0f, FALSE);
  }
};

void CHud::RenderActiveArsenal(SIconTexture *ptoAmmo) {
  PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, 0.5f, 0.25f, FALSE);

  // Prepare position and the weapon arsenal
  FLOAT fCol = _vpixBR(1) - units.fHalf;
  FLOAT fRow = _vpixBR(2) - units.fHalf;
  const FLOAT fBarPos = units.fHalf * 0.7f;

  UpdateWeaponArsenal();

#if SE1_GAME != SS_TFE
  // Display stored bombs
  #define BOMB_FIRE_TIME 1.5f

  INDEX iBombCount = _penPlayer->m_iSeriousBombCount;
  BOOL bBombFiring = FALSE;

  // Active Serious Bomb
  if (_penPlayer->m_tmSeriousBombFired + BOMB_FIRE_TIME > _pTimer->GetLerpedCurrentTick()) {
    iBombCount = ClampUp(INDEX(iBombCount + 1), (INDEX)3);
    bBombFiring = TRUE;
  }

  if (iBombCount > 0) {
    FLOAT fNormValue = (FLOAT)iBombCount / 3.0f;

    COLOR colBombBorder = _colBorder;
    COLOR colBombIcon = _colIconStd;
    COLOR colBombBar = (iBombCount == 1) ? _colLow : _colTop;

    if (bBombFiring) {
      FLOAT fFactor = (_pTimer->GetLerpedCurrentTick() - _penPlayer->m_tmSeriousBombFired) / BOMB_FIRE_TIME;
      colBombBorder = LerpColor(colBombBorder, _colLow, fFactor);
      colBombIcon = LerpColor(colBombIcon, _colLow, fFactor);
      colBombBar = LerpColor(colBombBar, _colLow, fFactor);
    }

    DrawBorder(fCol, fRow, units.fOne, units.fOne, colBombBorder);
    DrawIcon(fCol, fRow, tex.toASeriousBomb, colBombIcon, fNormValue, FALSE);
    DrawBar(fCol + fBarPos, fRow, units.fOne * 0.2f, units.fOne - 2, E_BD_DOWN, colBombBar, fNormValue);

    // Make space for bombs
    fCol -= units.fAdv;
  }
#endif

  // Display available ammo
  if (!pGetSP()->sp_bInfiniteAmmo && _psShowAmmoRow.GetIndex()) {
    for (INDEX iAmmo = GetAmmo().Count() - 1; iAmmo >= 0; iAmmo--) {
      HudAmmo &ai = GetAmmo()[iAmmo];
      ASSERT(ai.iAmmo >= 0);

      // No ammo and no weapon that uses it
      BOOL bShowDepletedAmmo = (ai.bHasWeapon && _psShowDepletedAmmo.GetIndex());

      if (ai.iAmmo == 0 && !bShowDepletedAmmo) continue;

      // Display ammo info
      COLOR colIcon = _colIconStd;

      if (ptoAmmo == ai.ptoAmmo) {
        colIcon = COL_AmmoSelected();

      } else if (ai.iAmmo == 0) {
        colIcon = COL_AmmoDepleted();
      }

      FLOAT fNormValue = (FLOAT)ai.iAmmo / (FLOAT)ai.iMaxAmmo;

      FLOAT fMoverX, fMoverY;
      COLOR col = AddShaker(4, ai.iAmmo, ai.iLastAmmo, ai.tmAmmoChanged, fMoverX, fMoverY);

      if (col == NONE) col = GetCurrentColor(fNormValue);

      DrawBorder(fCol, fRow + fMoverY, units.fOne, units.fOne, _colBorder);
      DrawIcon(fCol, fRow + fMoverY, *ai.ptoAmmo, colIcon, fNormValue, FALSE);
      DrawBar(fCol + fBarPos, fRow + fMoverY, units.fOne * 0.2f, units.fOne - 2, E_BD_DOWN, col, fNormValue);

      // Advance to the next position
      fCol -= units.fAdv;
    }

    // Set new starting position for powerups
    fCol = _vpixBR(1) - units.fHalf;
    fRow -= units.fAdv + units.fHalf;
  }

#if SE1_GAME != SS_TFE
  // Display active powerups
  PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, 0.66f, 0.33f, FALSE);

  // Arrange into arrays
  TIME ptmPowerups[] = {
    _penPlayer->m_tmInvisibility,
    _penPlayer->m_tmInvulnerability,
    _penPlayer->m_tmSeriousDamage,
    _penPlayer->m_tmSeriousSpeed,
  };

  TIME ptmPowerupsMax[] = {
    _penPlayer->m_tmInvisibilityMax,
    _penPlayer->m_tmInvulnerabilityMax,
    _penPlayer->m_tmSeriousDamageMax,
    _penPlayer->m_tmSeriousSpeedMax,
  };

  // Count spawn invulnerability as normal invulnerability
  static CSymbolPtr piSpawnInvul("plr_iSpawnInvulIndicator");

  const BOOL bSpawnInvul = (piSpawnInvul.Exists() && piSpawnInvul.GetIndex() > 0);
  const TIME tmSpawnInvul = pGetSP()->sp_tmSpawnInvulnerability;

  if (bSpawnInvul && tmSpawnInvul > 0.0f) {
    const TIME tmRemaining = _penPlayer->m_tmSpawned + tmSpawnInvul;
    ptmPowerups[1] = Max(ptmPowerups[1], tmRemaining);
  }

  for (INDEX iPowerUp = 0; iPowerUp < MAX_POWERUPS; iPowerUp++) {
    const TIME tmDelta = ptmPowerups[iPowerUp] - _tmNow;

    // Skip inactive
    if (tmDelta <= 0) continue;

    // Don't go over 100%
    FLOAT fNormValue = ClampUp(tmDelta / ptmPowerupsMax[iPowerUp], (TIME)1.0);

    // Draw icon and a little bar
    DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
    DrawIcon(fCol, fRow, tex.atoPowerups[iPowerUp], _colIconStd, fNormValue, TRUE);
    DrawBar(fCol + fBarPos, fRow, units.fOne * 0.2f, units.fOne - 2, E_BD_DOWN, GetCurrentColor(fNormValue), fNormValue);

    // Play sound if icon is flashing
    if (pPlayPowerUpSound_opt != NULL && fNormValue <= _cttHUD.ctt_fLowMedium * 0.5f)
    {
      INDEX iLastTime = INDEX(_tmLast * 4);
      INDEX iCurrentTime = INDEX(_tmNow * 4);

      if (iCurrentTime & 1 && !(iLastTime & 1)) {
        (_penPlayer->*pPlayPowerUpSound_opt)();
      }
    }

    // Advance to the next position
    fCol -= units.fAdv;
  }
#endif
};

void CHud::RenderBars(void) {
  // Draw oxygen info
  BOOL bOxygenOnScreen = FALSE;
  FLOAT fValue = _penPlayer->en_tmMaxHoldBreath - (_pTimer->CurrentTick() - _penPlayer->en_tmLastBreathed);

  const BOOL bConnected = (pIsConnected_opt != NULL) ? (_penPlayer->*pIsConnected_opt)() : TRUE;

  if (bConnected && _penPlayer->GetFlags() & ENF_ALIVE && fValue < 30.0f) { 
    FLOAT fCol = 320.0f + units.fHalf;
    FLOAT fRow = _vpixTL(2) + units.fOne + units.fNext;

    FLOAT fBarSize = units.fOne * 4.0f;
    FLOAT fColIcon = fCol - (fBarSize * 0.5f) - units.fAdv + units.fHalf;

    PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, 0.5f, 0.25f, FALSE);

    FLOAT fNormValue = ClampDn(fValue / 30.0f, 0.0f);

    DrawBorder(fCol, fRow, fBarSize, units.fOne, _colBorder);
    DrawBar(fCol, fRow, fBarSize * 0.975f, units.fOne * 0.9375f, E_BD_LEFT, GetCurrentColor(fNormValue), fNormValue);

    DrawBorder(fColIcon, fRow, units.fOne, units.fOne, _colBorder);
    DrawIcon(fColIcon, fRow, tex.toOxygen, _colIconStd, fNormValue, TRUE);

    bOxygenOnScreen = TRUE;
  }

  // Draw boss energy
  if (_penPlayer->m_penMainMusicHolder != NULL) {
    CMusicHolder &mh = (CMusicHolder &)*_penPlayer->m_penMainMusicHolder;
    FLOAT fNormValue = 0;

    if (mh.m_penBoss != NULL && mh.m_penBoss->GetFlags() & ENF_ALIVE) {
      CEnemyBase &eb = (CEnemyBase &)*mh.m_penBoss;
      ASSERT(eb.m_fMaxHealth > 0);

      fValue = eb.GetHealth();
      fNormValue = fValue / eb.m_fMaxHealth;
    }

    if (mh.m_penCounter != NULL) {
      CEntity *penCounter = mh.m_penCounter;

      // Find CEnemyCounter::m_iCount and CEnemyCounter::m_iCountFrom
      static CPropertyPtr pptrCountCurr(penCounter);
      static CPropertyPtr pptrCountFrom(penCounter);

      BOOL bProps = pptrCountCurr.ByVariable("CEnemyCounter", "m_iCount")
                 && pptrCountFrom.ByVariable("CEnemyCounter", "m_iCountFrom");

      if (bProps) {
        INDEX iCountCurr = ENTITYPROPERTY(penCounter, pptrCountCurr.Offset(), INDEX);
        INDEX iCountFrom = ENTITYPROPERTY(penCounter, pptrCountFrom.Offset(), INDEX);

        if (iCountCurr > 0) {
          fValue = iCountCurr;
          fNormValue = fValue / iCountFrom;
        }
      }
    }

    if (fNormValue > 0) {
      if (_bTSETheme) {
        PrepareColorTransitions(_colMax, _colMax, _colTop, _colLow, 0.5f, 0.25f, FALSE);
      } else {
        PrepareColorTransitions(_colMax, _colTop, _colMid, _colLow, 0.5f, 0.25f, FALSE);
      }

      FLOAT fCol = 320.0f + units.fHalf;
      FLOAT fRow = _vpixTL(2) + units.fOne + units.fNext;

      FLOAT fBarSize = units.fOne * 16.0f;
      FLOAT fColIcon = fCol - (fBarSize * 0.5f) - units.fAdv + units.fHalf;

      if (bOxygenOnScreen) {
        fRow += units.fNext;
      }

      DrawBorder(fCol, fRow, fBarSize, units.fOne, _colBorder);
      DrawBar(fCol, fRow, fBarSize * 0.995f, units.fOne * 0.9375f, E_BD_LEFT, GetCurrentColor(fNormValue), fNormValue);

      DrawBorder(fColIcon, fRow, units.fOne, units.fOne, _colBorder);
      DrawIcon(fColIcon, fRow, tex.toHealth, _colIconStd, fNormValue, FALSE);
    }
  }
};

void CHud::RenderGameModeInfo(void) {
  static CSymbolPtr pbMessages("hud_bShowMessages");
  static CSymbolPtr piPlayers("hud_iShowPlayers");
  static CSymbolPtr piSort("hud_iSortPlayers");

  const INDEX bShowMessages = pbMessages.GetIndex();
  const INDEX iShowPlayers = piPlayers.GetIndex();

  // Display lives counter
  const BOOL bShowLives = _psShowLives.GetIndex() && pGetSP()->sp_ctCredits > 0;

#if SE1_GAME == SS_TFE
  const INDEX bShowMatchInfo = _psShowMatchInfo.GetIndex();
#else
  static CSymbolPtr pbMatchInfo("hud_bShowMatchInfo");
  const INDEX bShowMatchInfo = pbMatchInfo.GetIndex();
#endif

  // Display details for PvE games
  const EGameMode eMode = _eGameMode;
  const BOOL bCoopDetails = (eMode == E_GM_SP || eMode == E_GM_COOP);
  const BOOL bRev = (_psTheme.GetIndex() == E_HUD_SSR);
  const COLOR colDefault = COL_PlayerNames();

  COLOR colMana, colFrags, colDeaths, colHealth, colArmor;
  COLOR colScore = colDefault;
  INDEX iScoreSum = 0;

  // Normal text scaling
  const FLOAT fTextScale = (_vScaling(2) + 1) * 0.5f * _fTextFontScale;

  // Display player list if not in singleplayer
  if (eMode != E_GM_SP) {
    // Set font
    _pfdCurrentText->SetVariableWidth();
    _pdp->SetFont(_pfdCurrentText);
    _pdp->SetTextScaling(fTextScale);

    // Sort player list
    BOOL bMaxScore = TRUE;
    BOOL bMaxMana = TRUE;
    BOOL bMaxFrags = TRUE;
    BOOL bMaxDeaths = TRUE;

    INDEX iSortPlayers = Clamp(piSort.GetIndex(), -1L, 6L);
    ESortKeys eKey = (ESortKeys)iSortPlayers;

    if (iSortPlayers == -1) {
      switch (eMode) {
        case E_GM_COOP:  eKey = E_SK_HEALTH; break;
        case E_GM_SCORE: eKey = E_SK_SCORE; break;
        case E_GM_FRAG:  eKey = E_SK_FRAGS; break;

        default: {
          ASSERT(FALSE);
          eKey = E_SK_NAME;
        }
      }
    }

    if (eMode == E_GM_COOP) {
      eKey = (ESortKeys)Clamp((INDEX)eKey, 0L, 3L);

    // Don't sort by health in deathmatch
    } else if (eKey == E_SK_HEALTH) {
      eKey = E_SK_NAME;
    }

    CDynamicContainer<CPlayer> cenSorted;
    SetAllPlayersStats(cenSorted, eKey);

    // Show ping next to player names
    const INDEX iShowPing = _psShowPlayerPing.GetIndex();

    // Go through all players
    INDEX iPlayer = 0;

    FOREACHINDYNAMICCONTAINER(cenSorted, CPlayer, iten) {
      CPlayer *penPlayer = iten;

      // Get player stats as strings
      const INDEX iScore = penPlayer->m_psGameStats.ps_iScore;
      const INDEX iMana = penPlayer->m_iMana;
      const INDEX iFrags = penPlayer->m_psGameStats.ps_iKills;
      const INDEX iDeaths = penPlayer->m_psGameStats.ps_iDeaths;
      const INDEX iHealth = ClampDn((INDEX)ceil(penPlayer->GetHealth()), 0L);
      const INDEX iArmor = ClampDn((INDEX)ceil(penPlayer->m_fArmor), 0L);

      CTString strScore, strMana, strFrags, strDeaths, strHealth, strArmor, strPing;
      strScore.PrintF("%d", iScore);
      strMana.PrintF("%d", iMana);
      strFrags.PrintF("%d", iFrags);
      strDeaths.PrintF("%d", iDeaths);
      strHealth.PrintF("%d", iHealth);
      strArmor.PrintF("%d", iArmor);

      // Display ping
      if (iShowPing > 0) {
        const INDEX iPing = ClampDn(INDEX(penPlayer->en_tmPing * 1000), (INDEX)0);

        // Ping colors by level
        static const CTString astrPingColors[] = {
          "^c00FF00", "^cFFFF00", "^cCC7711", "^cAA3333",
        };

        // Pick color depending on current ping
        INDEX iPingColor = 3; // Bad

        if (iPing <= 75) {
          iPingColor = 0; // Good
        } else if (iPing <= 150) {
          iPingColor = 1; // Okay
        } else if (iPing <= 250) {
          iPingColor = 2; // Mid
        }

        // Display signal strength
        if (iShowPing > 1) {
          // Pick signal strength and insert empty color in-between
          static const char *astrPingSignals[] = {
            "^b%s////", "^b%s///%s/", "^b%s//%s//", "^b%s/%s///",
          };

          strPing.PrintF(astrPingSignals[iPingColor], astrPingColors[iPingColor], "^caaaaaa");

        // Display milliseconds
        } else if (iPing > 999) {
          strPing.PrintF("%s>999ms", astrPingColors[iPingColor]);

        } else {
          strPing.PrintF("%s%dms", astrPingColors[iPingColor], iPing);
        }
      }

      // Detemine corresponding colors
      colHealth = C_mlRED;
      colMana = colScore = colFrags = colDeaths = colArmor = C_lGRAY;

      if (iMana > _penPlayer->m_iMana) {
        bMaxMana = FALSE;
        colMana = C_WHITE;
      }

      if (iScore > _penPlayer->m_psGameStats.ps_iScore) {
        bMaxScore = FALSE;
        colScore = C_WHITE;
      }

      if (iFrags > _penPlayer->m_psGameStats.ps_iKills) {
        bMaxFrags = FALSE;
        colFrags = C_WHITE;
      }

      if (iDeaths > _penPlayer->m_psGameStats.ps_iDeaths) {
        bMaxDeaths = FALSE;
        colDeaths = C_WHITE;
      }

      // Current player
      if (penPlayer == _penPlayer) {
        colScore = colMana = colFrags = colDeaths = colDefault;
      }

      // Enough health and armor
      if (iHealth > 25) colHealth = colDefault;
      if (iArmor > 25) colArmor = colDefault;

      // Put player in the list
      if ((iShowPlayers == 1 || iShowPlayers == -1) && eMode != E_GM_SP) {
        const PIX pixCharW = (_pfdCurrentText->GetWidth() - 2) * fTextScale;
        const PIX pixCharH = (_pfdCurrentText->GetHeight() - 2) * fTextScale;

        // Vertical offset
        PIX pixOffsetY = _vpixTL(2);
        BOOL bNoDetailsShift = TRUE;

        // Shift for coop details
        if (bCoopDetails) {
          if (bShowLives) {
            // Rescale temporarily
            const FLOAT fUndoScale = Rescale(1.75f);

            pixOffsetY += units.fNext;

            Rescale(fUndoScale);
            bNoDetailsShift = FALSE;
          }

          if (bShowMessages) {
            pixOffsetY += units.fNext;
            bNoDetailsShift = FALSE;
          }
        }

        // Offset for no coop details
        if (bNoDetailsShift) pixOffsetY += 5;

        const PIX pixInfoY = pixOffsetY * _vScaling(2) + pixCharH * iPlayer;

        // Horizontal offset
        PIX pixOffsetX = _vpixBR(1);

        // Display player ping and make space for it
        if (iShowPing > 0) {
          _pdp->PutTextR(strPing, pixOffsetX * _vScaling(1), pixInfoY, C_WHITE | _ulAlphaHUD);

          pixOffsetX -= (iShowPing > 1) ? 12 : 28;
        }

        #define PLAYER_INFO_X(Offset) (pixOffsetX * _vScaling(1) - Offset * pixCharW)

        // Optionally undecorated name
        CTString strName;

        if (_psDecoratedNames.GetIndex()) {
          strName = penPlayer->GetPlayerName();
        } else {
          strName = penPlayer->GetPlayerName().Undecorated();
        }

        // Display player stats
        if (eMode == E_GM_COOP) {
          _pdp->PutTextR(strName,   PLAYER_INFO_X(8), pixInfoY, colScore   | _ulAlphaHUD);
          _pdp->PutTextC(strHealth, PLAYER_INFO_X(6), pixInfoY, colHealth  | _ulAlphaHUD);
          _pdp->PutText("/",        PLAYER_INFO_X(4), pixInfoY, colDefault | _ulAlphaHUD);
          _pdp->PutTextC(strArmor,  PLAYER_INFO_X(2), pixInfoY, colArmor   | _ulAlphaHUD);

        } else if (eMode == E_GM_SCORE) {
          _pdp->PutTextR(strName,  PLAYER_INFO_X(12), pixInfoY, colDefault | _ulAlphaHUD);
          _pdp->PutTextC(strScore, PLAYER_INFO_X(8),  pixInfoY, colScore   | _ulAlphaHUD);
          _pdp->PutText("/",       PLAYER_INFO_X(5),  pixInfoY, colDefault | _ulAlphaHUD);
          _pdp->PutTextC(strMana,  PLAYER_INFO_X(2),  pixInfoY, colMana    | _ulAlphaHUD);

        } else {
          _pdp->PutTextR(strName,   PLAYER_INFO_X(8), pixInfoY, colDefault | _ulAlphaHUD);
          _pdp->PutTextC(strFrags,  PLAYER_INFO_X(6), pixInfoY, colFrags   | _ulAlphaHUD);
          _pdp->PutText("/",        PLAYER_INFO_X(4), pixInfoY, colDefault | _ulAlphaHUD);
          _pdp->PutTextC(strDeaths, PLAYER_INFO_X(2), pixInfoY, colDeaths  | _ulAlphaHUD);
        }
      }

      // Summarize score for coop
      iScoreSum += iScore;

      // Next player in the list
      iPlayer++;
    }

    if ((eMode == E_GM_SCORE || eMode == E_GM_FRAG) && bShowMatchInfo) {
      CTString strLimitsInfo = "";

      // Draw remaining time
      if (pGetSP()->sp_iTimeLimit > 0) {
        FLOAT fTimeLeft = ClampDn(pGetSP()->sp_iTimeLimit * 60.0f - _pNetwork->GetGameTime(), (TIME)0.0);
        strLimitsInfo.PrintF("%s^cFFFFFF%s: %s\n", strLimitsInfo, LOCALIZE("TIME LEFT"), TimeToString(fTimeLeft));
      }

      // Find maximum frags and score from players
      INDEX iMaxFrags = LowerLimit(INDEX(0));
      INDEX iMaxScore = LowerLimit(INDEX(0));

      {FOREACHINDYNAMICCONTAINER(_cenPlayers, CPlayer, itenStats) {
        iMaxFrags = Max(iMaxFrags, itenStats->m_psLevelStats.ps_iKills);
        iMaxScore = Max(iMaxScore, itenStats->m_psLevelStats.ps_iScore);
      }}

      if (pGetSP()->sp_iFragLimit > 0) {
        INDEX iFragsLeft = ClampDn(pGetSP()->sp_iFragLimit-iMaxFrags, INDEX(0));
        strLimitsInfo.PrintF("%s^cFFFFFF%s: %d\n", strLimitsInfo, LOCALIZE("FRAGS LEFT"), iFragsLeft);
      }

      if (pGetSP()->sp_iScoreLimit > 0) {
        INDEX iScoreLeft = ClampDn(pGetSP()->sp_iScoreLimit-iMaxScore, INDEX(0));
        strLimitsInfo.PrintF("%s^cFFFFFF%s: %d\n", strLimitsInfo, LOCALIZE("SCORE LEFT"), iScoreLeft);
      }

      _pfdCurrentText->SetFixedWidth();
      _pdp->SetFont(_pfdCurrentText);
      _pdp->SetTextScaling(fTextScale * 0.8f);
      _pdp->SetTextCharSpacing(-2.0f * fTextScale);

      // [Cecil] Screen edge offset
      const PIX pixInfoX = _vpixTL(1);
      const PIX pixInfoY = _vpixTL(2) + units.fNext * 2;
      _pdp->PutText(strLimitsInfo, pixInfoX * _vScaling(1), pixInfoY * _vScaling(1), C_WHITE | CT_OPAQUE);
    }

    // Prepare colors for local player printouts
    colScore  = (bMaxScore  ? C_WHITE : C_lGRAY);
    colMana   = (bMaxMana   ? C_WHITE : C_lGRAY);
    colFrags  = (bMaxFrags  ? C_WHITE : C_lGRAY);
    colDeaths = (bMaxDeaths ? C_WHITE : C_lGRAY);
  }

  // Restore font defaults
  _pfdCurrentText->SetVariableWidth();

  _pdp->SetFont(_pfdCurrentNumbers);
  _pdp->SetTextCharSpacing(1);

  // Prepare outputs depending on gamemode
  FLOAT fWidthAdj = 8;
  INDEX iScore = _penPlayer->m_psGameStats.ps_iScore;
  INDEX iMana = _penPlayer->m_iMana;

  if (eMode == E_GM_FRAG) {
    if (!bShowMatchInfo) {
      fWidthAdj = 4;
    }

    iScore = _penPlayer->m_psGameStats.ps_iKills;
    iMana = _penPlayer->m_psGameStats.ps_iDeaths;

  // Show score in coop
  } else if (eMode == E_GM_COOP) {
    iScore = iScoreSum;
  }

  // Draw score or frags
  CTString strValue;
  strValue.PrintF("%d", iScore);

  FLOAT fCol = _vpixTL(1) + units.fHalf;
  FLOAT fRow = _vpixTL(2) + units.fHalf;
  FLOAT fAdv = units.fAdv + units.fChar * fWidthAdj * 0.5f - units.fHalf;

  DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
  DrawBorder(fCol + fAdv, fRow, units.fChar * fWidthAdj, units.fOne, _colBorder);
  DrawString(fCol + fAdv, fRow, strValue, (bRev ? _colTop : colScore), 1.0f);
  DrawIcon(fCol, fRow, tex.toFrags, (_bTSETheme ? C_WHITE : colScore), 1.0f, FALSE);

  // Deathmatch
  if (eMode == E_GM_SCORE || eMode == E_GM_FRAG) {
    strValue.PrintF("%d", iMana);
    fCol = _vpixTL(1) + units.fHalf;
    fRow = _vpixTL(2) + units.fNext + units.fHalf;
    fAdv = units.fAdv + units.fChar * fWidthAdj * 0.5f - units.fHalf;

    DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
    DrawBorder(fCol + fAdv, fRow, units.fChar * fWidthAdj, units.fOne, _colBorder);
    DrawString(fCol + fAdv, fRow, strValue, (bRev ? _colTop : colMana), 1.0f);
    DrawIcon(fCol, fRow, tex.toDeaths, (_bTSETheme ? C_WHITE : colMana), 1.0f, FALSE);

  // Singleplayer or cooperative
  } else if (bCoopDetails) {
    // Draw high score
    if (_psShowHighScore.GetIndex())
    {
    #if SE1_GAME != SS_REV
      const INDEX iHighScore = _penPlayer->m_iHighScore;
    #else
      // [Cecil] TODO: Find out which stat from Revolution could be used as a high score
      // Maybe it can read those "world stats" and retrieve the highest score per level?
      const INDEX iHighScore = UpperLimit(INDEX(0));
    #endif

      strValue.PrintF("%d", Max(iHighScore, _penPlayer->m_psGameStats.ps_iScore));
      BOOL bBeating = _penPlayer->m_psGameStats.ps_iScore > iHighScore;

      fCol = 320.0f + units.fHalf;
      fRow = _vpixTL(2) + units.fHalf;
      fAdv = units.fChar * 4 + units.fAdv - units.fHalf;

      DrawBorder(fCol, fRow, units.fChar * 8, units.fOne, _colBorder);
      DrawString(fCol, fRow, strValue, GetCurrentColor(!bBeating), !bBeating);

      DrawBorder(fCol - fAdv, fRow, units.fOne, units.fOne, _colBorder);
      DrawIcon(fCol - fAdv, fRow, tex.toHiScore, _colIconStd, 1.0f, FALSE);
    }

    // Adjustable start of messages
    fRow = _vpixTL(2) + units.fHalf;

    // Draw lives counter
    if (bShowLives) {
      // Rescale
      const FLOAT fUndoScale = Rescale(1.75f);

      const INDEX iValue = ClampDn(pGetSP()->sp_ctCreditsLeft, (INDEX)0);
      const FLOAT fNormValue = (FLOAT)iValue / (FLOAT)pGetSP()->sp_ctCredits;

      strValue.PrintF("%d", iValue);

      fCol = _vpixBR(1) - units.fHalf - units.fChar * 3;
      fRow = _vpixTL(2) + units.fHalf;
      fAdv = units.fAdv + units.fChar * 1.5f - units.fHalf;

      PrepareColorTransitions(_colTop, _colTop, _colMid, _colLow, 0.4f, 0.2f, FALSE);

      DrawBorder(fCol, fRow, units.fOne, units.fOne, _colBorder);
      DrawBorder(fCol + fAdv, fRow, units.fChar * 3, units.fOne, _colBorder);
      DrawString(fCol + fAdv, fRow, strValue, GetCurrentColor(fNormValue), 1.0f);
      DrawIcon(fCol, fRow, tex.toLives, _colIconStd, 0.0f, FALSE);

      // Shift messages and unscale
      fRow += units.fOne;
      Rescale(fUndoScale);
    }

    // Draw unread messages
    if (bShowMessages && _penPlayer->m_ctUnreadMessages > 0) {
      strValue.PrintF("%d", _penPlayer->m_ctUnreadMessages);

      fCol = _vpixBR(1) - units.fHalf - units.fChar * 4;
      fAdv = units.fAdv + units.fChar * 2 - units.fHalf;

      const FLOAT tmIn = 0.5f;
      const FLOAT tmOut = 0.5f;
      const FLOAT tmStay = 2.0f;
      FLOAT tmDelta = _pTimer->GetLerpedCurrentTick() - _penPlayer->m_tmAnimateInbox;
      COLOR colMessageBorder = _colBorder;
      COLOR colMessageIcon = _colHUD;

      if (tmDelta > 0 && tmDelta < (tmIn + tmStay + tmOut) && eMode == E_GM_SP) {
        FLOAT fRatio = 1.0f;

        if (tmDelta < tmIn) {
          fRatio = tmDelta / tmIn;

        } else if (tmDelta > tmIn + tmStay) {
          fRatio = (tmIn + tmStay + tmOut - tmDelta) / tmOut;
        }

        fCol -= units.fAdv * 15 * fRatio;
        fRow += units.fAdv * 5 * fRatio;
        colMessageBorder = LerpColor(colMessageBorder, C_WHITE | CT_OPAQUE, fRatio);
        colMessageIcon = LerpColor(colMessageIcon, C_WHITE | CT_OPAQUE, fRatio);
      }

      DrawBorder(fCol, fRow, units.fOne, units.fOne, colMessageBorder);
      DrawBorder(fCol + fAdv, fRow, units.fChar * 4, units.fOne, colMessageBorder);
      DrawString(fCol + fAdv, fRow, strValue, colMessageIcon, 1.0f);
      DrawIcon(fCol, fRow, tex.toMessage, (_bTSETheme ? C_WHITE : colMessageIcon), 0.0f, TRUE);
    }
  }
};

void CHud::RenderCheats(void) {
  // Render active cheats while in singleplayer
  if (pGetSP()->sp_ctMaxPlayers != 1) return;

  ULONG ulAlpha = sin(_tmNow * 16) * 96 + 128;
  PIX pixFontHeight = _pfdConsoleFont->fd_pixCharHeight;
  const COLOR colCheat = _colTop | ulAlpha;

  _pdp->SetFont(_pfdConsoleFont);
  _pdp->SetTextScaling(1.0f);

  static CSymbolPtr pbGod("cht_bGod");
  static CSymbolPtr pbFly("cht_bFly");
  static CSymbolPtr pbGhost("cht_bGhost");
  static CSymbolPtr pbInvisible("cht_bInvisible");
  static CSymbolPtr pfTrans("cht_fTranslationMultiplier");

  INDEX iLine = 1;

  #define CHEAT_LINE_Y (_vpixScreen(2) - pixFontHeight * (iLine++))

  if (pfTrans.GetFloat() > 1.0f) {
    _pdp->PutTextR("turbo", _vpixScreen(1) - 1, CHEAT_LINE_Y, colCheat);
  }

  if (pbInvisible.GetIndex()) {
    _pdp->PutTextR("invisible", _vpixScreen(1) - 1, CHEAT_LINE_Y, colCheat);
  }

  if (pbGhost.GetIndex()) {
    _pdp->PutTextR("ghost", _vpixScreen(1) - 1, CHEAT_LINE_Y, colCheat);
  }

  if (pbFly.GetIndex()) {
    _pdp->PutTextR("fly", _vpixScreen(1) - 1, CHEAT_LINE_Y, colCheat);
  }

  if (pbGod.GetIndex()) {
    _pdp->PutTextR("god", _vpixScreen(1) - 1, CHEAT_LINE_Y, colCheat);
  }
};
