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

#ifndef CECIL_INCL_TEXTURESET_H
#define CECIL_INCL_TEXTURESET_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Available HUD themes
enum EHudTheme {
  E_HUD_TFE,
  E_HUD_WARPED,
  E_HUD_TSE,
  E_HUD_SSR,

  E_HUD_MAX, // Maximum amount of themes
};

#define MAX_POWERUPS 4

// Multi-theme container for icons
struct SIconTexture {
  CTextureObject ato[E_HUD_MAX];

  // Set icon texture for a specific theme
  void SetIcon(INDEX iTheme, const CTString &strTexture) {
    ato[iTheme].SetData_t(strTexture);
    ((CTextureData *)ato[iTheme].GetData())->Force(TEX_CONSTANT);
  };

  // Return texture depending on the theme
  inline CTextureObject &Texture(void) {
    INDEX iTheme = Clamp(_psTheme.GetIndex(), (INDEX)0, INDEX(E_HUD_MAX - 1));
    return ato[iTheme];
  };

  // Implicit conversion
  inline operator CTextureObject &() {
    return Texture();
  };
};

// Set of textures for the theme
struct HudTextureSet {
  // Status bar
  SIconTexture toHealth;
  SIconTexture toOxygen;
  SIconTexture toScore;
  SIconTexture toHiScore;
  SIconTexture toMessage;
  SIconTexture toFrags;
  SIconTexture toDeaths;
  SIconTexture atoArmor[3];

  // Ammo
  SIconTexture toAShells;
  SIconTexture toABullets;
  SIconTexture toARockets;
  SIconTexture toAGrenades;
  SIconTexture toAElectricity;
  SIconTexture toAIronBall;

  SIconTexture toANapalm;
  SIconTexture toASniperBullets;
  SIconTexture toASeriousBomb;

  // Weapons
  SIconTexture toWKnife;
  SIconTexture toWColt;
  SIconTexture toWSingleShotgun;
  SIconTexture toWDoubleShotgun;
  SIconTexture toWTommygun;
  SIconTexture toWMinigun;
  SIconTexture toWRocketLauncher;
  SIconTexture toWGrenadeLauncher;
  SIconTexture toWLaser;
  SIconTexture toWIronCannon;

  SIconTexture toWChainsaw;
  SIconTexture toWFlamer;
  SIconTexture toWSniper;

#if SE1_GAME != SS_TFE
  // Powerups
  SIconTexture atoPowerups[MAX_POWERUPS];

  // Sniper mask
  CTextureObject toSniperMask;
  CTextureObject toSniperWheel;
  CTextureObject toSniperArrow;
  CTextureObject toSniperEye;
  CTextureObject toSniperLed;
#endif

  // Tile texture with one corner, edges and center
  CTextureObject toTile;

  // Lives counter
  CTextureObject toLives;

  // Player marker
  CTextureObject toMarker;

  void LoadTextures(void);
};

// Set of colors for the theme
struct HudColorSet {
  // Base colors
  COLOR colBase;
  COLOR colIcon;
  COLOR colNames;
  COLOR colAmmoSelected;
  COLOR colAmmoDepleted;

  // Value colors
  COLOR colValueOverTop;
  COLOR colValueTop;
  COLOR colValueMid;
  COLOR colValueLow;

  // Sniper scope
  COLOR colScopeMask;
  COLOR colScopeDetails;
  COLOR colScopeLedIdle;
  COLOR colScopeLedFire;

  // Weapon selection list
  COLOR colWeaponBorder;
  COLOR colWeaponIcon;
  COLOR colWeaponNoAmmo;
  COLOR colWeaponWanted;
};

extern const HudColorSet _hcolTFE;
extern const HudColorSet _hcolWarped;
extern const HudColorSet _hcolTSE;
extern const HudColorSet _hcolSSR;

#endif
