/* Copyright (c) 2023-2026 Dreamy Cecil
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

// Define plugin
CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagGame | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Advanced HUD", "Patches for the heads-up display with various improvements and expanded customization.");

// Check if playing with modified entities
BOOL _bModdedEntities = FALSE;

CPluginSymbol _psEnable(SSF_PERSISTENT | SSF_USER, INDEX(1));

// Select default theme
#if SE1_GAME == SS_TFE
  CPluginSymbol _psTheme(0, (INDEX)E_HUD_TFE); // [Cecil] NOTE: Deprecated
  CPluginSymbol _psThemeScript(SSF_PERSISTENT | SSF_USER, "Scripts\\ClassicsPatch\\HUDThemes\\01_TFE.nut");
#else
  CPluginSymbol _psTheme(0, (INDEX)E_HUD_TSE); // [Cecil] NOTE: Deprecated
  CPluginSymbol _psThemeScript(SSF_PERSISTENT | SSF_USER, "Scripts\\ClassicsPatch\\HUDThemes\\03_TSE.nut");
#endif

CPluginSymbol _psScreenEdgeX(SSF_PERSISTENT | SSF_USER, INDEX(5));
CPluginSymbol _psScreenEdgeY(SSF_PERSISTENT | SSF_USER, INDEX(5));
CPluginSymbol _psTextScaling(SSF_PERSISTENT | SSF_USER, FLOAT(1.0f));
CPluginSymbol _psIconShake(SSF_PERSISTENT | SSF_USER, INDEX(1));

#if SE1_GAME == SS_TFE
  // TFE specific
  CPluginSymbol _psShowClock(SSF_PERSISTENT | SSF_USER, INDEX(0));
  CPluginSymbol _psShowMatchInfo(SSF_PERSISTENT | SSF_USER, INDEX(1));

#else
  // TSE specific
  CPluginSymbol _psScopeAlpha(SSF_PERSISTENT | SSF_USER, FLOAT(1.0f));
  CPluginSymbol _psScopeColoring(SSF_PERSISTENT | SSF_USER, INDEX(0));
#endif

CPluginSymbol _psShowPlayerPing(SSF_PERSISTENT | SSF_USER, INDEX(0));
CPluginSymbol _psDecoratedNames(SSF_PERSISTENT | SSF_USER, INDEX(1));
CPluginSymbol _psShowAmmoRow(SSF_PERSISTENT | SSF_USER, INDEX(1));
CPluginSymbol _psShowDepletedAmmo(SSF_PERSISTENT | SSF_USER, INDEX(1));
CPluginSymbol _psShowHighScore(SSF_PERSISTENT | SSF_USER, INDEX(1));
CPluginSymbol _psShowLives(SSF_PERSISTENT | SSF_USER, INDEX(1));

CPluginSymbol _psPlayerTags(SSF_PERSISTENT | SSF_USER, INDEX(2));
CPluginSymbol _psTagsInDemos(SSF_PERSISTENT | SSF_USER, INDEX(1));
CPluginSymbol _psTagsForObservers(SSF_PERSISTENT | SSF_USER, INDEX(1));

// HUD colorization (no alpha channel)
CPluginSymbol _psColorize(SSF_PERSISTENT | SSF_USER, INDEX(0));
static CPluginSymbol _psColorPreset(SSF_PERSISTENT | SSF_USER, "");

CPluginSymbol _psColorBase(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorIcon(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorNames(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorWeapon(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorSelect(SSF_PERSISTENT | SSF_USER, INDEX(0xFFFFFF));
CPluginSymbol _psColorMax(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorTop(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorMid(SSF_PERSISTENT | SSF_USER, INDEX(0x00FF00));
CPluginSymbol _psColorLow(SSF_PERSISTENT | SSF_USER, INDEX(0xFF0000));

static void UpdateTheme(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  CTString strLoad(0, "include \"%s\";", _psThemeScript.GetString().str_String);
  _pShell->Execute(strLoad);
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Check if standard entities are modified
  if (ClassicsCore_IsEntitiesModded())
  {
    // Enabling "SameHook" means that it's safe to replace mod's HUD, so it counts as non-modified entities
    _bModdedEntities = !props->GetBoolValue("", "SameHook", false);
  }

  // Custom symbols
  _psEnable.Register("ahud_bEnable");
  _psTheme.Register("ahud_iTheme");

  GetPluginAPI()->RegisterMethod(FALSE, "void", "ahud_UpdateTheme", "INDEX", &UpdateTheme);
  _psThemeScript.Register("ahud_strThemeScript", "", "ahud_UpdateTheme");

  _psScreenEdgeX.Register("ahud_iScreenEdgeX");
  _psScreenEdgeY.Register("ahud_iScreenEdgeY");
  _psTextScaling.Register("ahud_fTextScaling");
  _psIconShake.Register("ahud_bIconShake");

  #if SE1_GAME == SS_TFE
    // TFE specific
    _psShowClock.Register("hud_bShowClock");
    _psShowMatchInfo.Register("hud_bShowMatchInfo");

  #else
    // TSE specific
    _psScopeAlpha.Register("ahud_fScopeAlpha");
    _psScopeColoring.Register("ahud_bScopeColoring");
  #endif

  _psShowPlayerPing.Register("ahud_iShowPlayerPing");
  _psDecoratedNames.Register("ahud_bDecoratedNames");
  _psShowAmmoRow.Register("ahud_bShowAmmoRow");
  _psShowDepletedAmmo.Register("ahud_bShowDepletedAmmo");
  _psShowHighScore.Register("ahud_bShowHighScore");
  _psShowLives.Register("ahud_bShowLives");

  _psPlayerTags.Register("ahud_iPlayerTags");
  _psTagsInDemos.Register("ahud_bTagsInDemos");
  _psTagsForObservers.Register("ahud_bTagsForObservers");

  _psColorize.Register("ahud_bColorize");
  _psColorPreset.Register("ahud_strColorPreset");

  _psColorBase.Register("ahud_iColorBase");
  _psColorIcon.Register("ahud_iColorIcon");
  _psColorNames.Register("ahud_iColorNames");
  _psColorWeapon.Register("ahud_iColorWeapon");
  _psColorSelect.Register("ahud_iColorSelect");
  _psColorMax.Register("ahud_iColorMax");
  _psColorTop.Register("ahud_iColorTop");
  _psColorMid.Register("ahud_iColorMid");
  _psColorLow.Register("ahud_iColorLow");

  // Automatically apply selected theme on startup
  UpdateTheme(NULL);

  // Initialize the HUD itself
  _HUD.Initialize();
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
  // Clean up the HUD
  _HUD.End();
};
