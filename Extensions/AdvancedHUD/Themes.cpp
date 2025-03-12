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

// Define color sets for themes
const HudColorSet _hcolTFE = {
  C_GREEN, C_GREEN, C_GREEN, C_WHITE, C_GRAY, // Base
  C_GREEN, C_GREEN, C_GREEN, C_RED, // Values
  C_mlGREEN, C_lGRAY, 0x44FF22BB, 0xFF4422DD, // Sniper scope
  C_GREEN, C_GREEN, C_dGRAY, C_WHITE, // Weapon selection
};

const HudColorSet _hcolWarped = {
  0x4C80BB00, 0x4C80BB00, 0x4C80BB00, C_WHITE, C_mdGRAY, // Base
  0x6CFF6C00, 0xFFD70000, 0xFF6B0000, C_RED, // Values
  0x64B4FF00, 0xFFCC3300, 0x44FF22BB, 0xFF4422DD, // Sniper scope
  0x4C80BB00, 0x4C80BB00, C_dGRAY, C_WHITE, // Weapon selection
};

const HudColorSet _hcolTSE = {
  0x4C80BB00, C_WHITE, 0x4C80BB00, C_WHITE, C_mdGRAY, // Base
  0x6CFF6C00, 0xFFD70000, 0xFF6B0000, C_RED, // Values
  0x64B4FF00, 0xFFCC3300, 0x44FF22BB, 0xFF4422DD, // Sniper scope
  0x4C80BB00, 0xCCDDFF00, 0x22334400, 0xFFCC0000, // Weapon selection
};

const HudColorSet _hcolSSR = {
  0x56596700, C_WHITE, C_lGRAY, C_WHITE, C_mdGRAY, // Base
  0x6CFF6C00, 0xFFD70000, 0xFF6B0000, C_RED, // Values
  0x64B4FF00, 0xFFCC3300, 0x44FF22BB, 0xFF4422DD, // Sniper scope
  0x56596700, 0xCCDDFF00, 0x22334400, 0xFFBF5B00, // Weapon selection
};

void HudTextureSet::LoadTextures(void) {
  // Directories with themed icons
  static const CTString astrPaths[E_HUD_MAX] = {
    "Textures\\Interface\\",
    "Textures\\Interface\\",
    "TexturesMP\\Interface\\",
    "TexturesPatch\\Interface\\Revolution\\",
  };

  for (INDEX iTheme = 0; iTheme < E_HUD_MAX; iTheme++) {
    const CTString &strPath = astrPaths[iTheme];
    const BOOL bTFE = (iTheme <= E_HUD_WARPED);

    // Status bar textures
    toHealth   .SetIcon(iTheme, strPath + "HSuper.tex");
    toOxygen   .SetIcon(iTheme, strPath + "Oxygen-2.tex");
    toFrags    .SetIcon(iTheme, strPath + "IBead.tex");
    toDeaths   .SetIcon(iTheme, strPath + "ISkull.tex");
    toScore    .SetIcon(iTheme, strPath + "IScore.tex");
    toHiScore  .SetIcon(iTheme, strPath + "IHiScore.tex");
    toMessage  .SetIcon(iTheme, strPath + "IMessage.tex");
    atoArmor[0].SetIcon(iTheme, strPath + (bTFE ? "ArStrong.tex" : "ArSmall.tex"));
    atoArmor[1].SetIcon(iTheme, strPath + (bTFE ? "ArStrong.tex" : "ArMedium.tex"));
    atoArmor[2].SetIcon(iTheme, strPath + "ArStrong.tex");

    // Ammo textures
    toAShells     .SetIcon(iTheme, strPath + "AmShells.tex");
    toABullets    .SetIcon(iTheme, strPath + "AmBullets.tex");
    toARockets    .SetIcon(iTheme, strPath + "AmRockets.tex");
    toAGrenades   .SetIcon(iTheme, strPath + "AmGrenades.tex");
    toAElectricity.SetIcon(iTheme, strPath + "AmElectricity.tex");
    toAIronBall   .SetIcon(iTheme, strPath + (bTFE ? "AmCannon.tex" : "AmCannonBall.tex"));

    // Weapon textures
    toWKnife          .SetIcon(iTheme, strPath + "WKnife.tex");
    toWColt           .SetIcon(iTheme, strPath + "WColt.tex");
    toWSingleShotgun  .SetIcon(iTheme, strPath + "WSingleShotgun.tex");
    toWDoubleShotgun  .SetIcon(iTheme, strPath + "WDoubleShotgun.tex");
    toWTommygun       .SetIcon(iTheme, strPath + "WTommygun.tex");
    toWMinigun        .SetIcon(iTheme, strPath + "WMinigun.tex");
    toWRocketLauncher .SetIcon(iTheme, strPath + "WRocketLauncher.tex");
    toWGrenadeLauncher.SetIcon(iTheme, strPath + "WGrenadeLauncher.tex");
    toWLaser          .SetIcon(iTheme, strPath + "WLaser.tex");
    toWIronCannon     .SetIcon(iTheme, strPath + "WCannon.tex");

  #if SE1_GAME != SS_TFE
    // Ammo textures
    toANapalm       .SetIcon(iTheme, strPath + "AmFuelReservoir.tex");
    toASniperBullets.SetIcon(iTheme, strPath + "AmSniperBullets.tex");

    // Weapon textures
    toWChainsaw.SetIcon(iTheme, strPath + "WChainsaw.tex");
    toWSniper  .SetIcon(iTheme, strPath + "WSniper.tex");
    toWFlamer  .SetIcon(iTheme, strPath + "WFlamer.tex");

    // Power up textures
    const CTString &strPowerUp = (bTFE ? astrPaths[E_HUD_TSE] : strPath);

    atoPowerups[0].SetIcon(iTheme, strPowerUp + "PInvisibility.tex");
    atoPowerups[1].SetIcon(iTheme, strPowerUp + "PInvulnerability.tex");
    atoPowerups[2].SetIcon(iTheme, strPowerUp + "PSeriousDamage.tex");
    atoPowerups[3].SetIcon(iTheme, strPowerUp + "PSeriousSpeed.tex");
    toASeriousBomb.SetIcon(iTheme, strPowerUp + "AmSeriousBomb.tex");
  #endif
  }

  // Sniper mask textures for TSE
  #if SE1_GAME != SS_TFE
    toSniperMask.SetData_t(CTFILENAME("TexturesMP\\Interface\\SniperMask.tex"));
    toSniperWheel.SetData_t(CTFILENAME("TexturesMP\\Interface\\SniperWheel.tex"));
    toSniperArrow.SetData_t(CTFILENAME("TexturesMP\\Interface\\SniperArrow.tex"));
    toSniperEye.SetData_t(CTFILENAME("TexturesMP\\Interface\\SniperEye.tex"));
    toSniperLed.SetData_t(CTFILENAME("TexturesMP\\Interface\\SniperLed.tex"));

    ((CTextureData *)toSniperMask.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)toSniperWheel.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)toSniperArrow.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)toSniperEye.GetData())->Force(TEX_CONSTANT);
    ((CTextureData *)toSniperLed.GetData())->Force(TEX_CONSTANT);
  #endif

  // Tile texture
  toTile.SetData_t(CTFILENAME("Textures\\Interface\\Tile.tex"));
  ((CTextureData *)toTile.GetData())->Force(TEX_CONSTANT);

  // Lives counter
  toLives.SetData_t(CTFILENAME("TexturesPatch\\Interface\\ILives.tex"));
  ((CTextureData *)toLives.GetData())->Force(TEX_CONSTANT);

  // Player marker
  toMarker.SetData_t(CTFILENAME("TexturesPatch\\Interface\\IPlayerMarker.tex"));
  ((CTextureData *)toMarker.GetData())->Force(TEX_CONSTANT);
};
