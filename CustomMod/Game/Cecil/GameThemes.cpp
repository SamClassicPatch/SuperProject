/* Copyright (c) 2023-2025 Dreamy Cecil
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

#include "StdAfx.h"

#include "GameThemes.h"

// Main theme structure
CGameTheme _gmtTheme;

// Path to the theme config to set
#if SE1_GAME == SS_TFE
  CTString CGameTheme::strTheme = "Scripts\\ClassicsPatch\\Themes\\01_TFE.ini";
#else
  CTString CGameTheme::strTheme = "Scripts\\ClassicsPatch\\Themes\\03_TSE.ini";
#endif

// Loaded theme config
static CIniConfig _iniTheme;

// Currently loading theme
static CIniConfig *_pini = NULL;

// Set texture from a file or show a potential error message
static inline void SetTexture(CTextureObject &to, const CTString &strFile, BOOL bShowError) {
  try {
    to.SetData_t(strFile);

    if (to.GetData() != NULL) {
      ((CTextureData *)to.GetData())->Force(TEX_CONSTANT);
    }

  } catch (char *strError) {
    if (bShowError) {
      FatalError("%s", strError);
    }
  }
};

// Set color from a config to a variable
static inline void SetColor(const CTString &strKey, COLOR &colVariable) {
  // Shift to ignore alpha channel in configs
  colVariable = _pini->GetIntValue("Colors", strKey, colVariable >> 8);
  colVariable <<= 8;
};

// Set flag from a config to a variable
static inline void SetFlag(const CTString &strKey, BOOL &bVariable, BOOL bDefValue) {
  bVariable = _pini->GetBoolValue("Flags", strKey, !!bDefValue);
};

// Reload textures ("*" for default TSE)
void CGameTheme::ReloadTextures(CTString strBackdrop, CTString strClouds, CTString strGrid, CTString strPointer) {
  // Reset textures and load new ones
  toBackdrop.SetData(NULL);
  toClouds.SetData(NULL);
  toGrid.SetData(NULL);
  toPointer.SetData(NULL);

  if (strBackdrop == "*") strBackdrop = "TexturesMP\\General\\MenuBack.tex";
  if (strClouds == "*") strClouds = "Textures\\General\\Background6.tex";
  if (strGrid == "*") strGrid = "TexturesMP\\General\\Grid.tex";

  SetTexture(toBackdrop, strBackdrop, FALSE);
  SetTexture(toClouds, strClouds, FALSE);
  SetTexture(toGrid, strGrid, FALSE);

  // Make sure the pointer texture is always loaded
  if (strPointer == "*" || !FileExists(strPointer)) {
    strPointer = "TexturesMP\\General\\Pointer.tex";
  }

  SetTexture(toPointer, strPointer, TRUE);
};

// Load game theme from a config file
void CGameTheme::Load(const CTString &strFile, BOOL bNewTheme) {
  // Already loaded
  if (strConfig == strFile) return;

  // Clear last theme
  if (bNewTheme) {
    _iniTheme.Clear();
  }

  // Try loading a new theme
  CIniConfig iniConfig;

  try {
    iniConfig.Load_t(strFile, TRUE);

    // Set new theme
    if (bNewTheme) {
      _iniTheme = iniConfig;

    // Disallow recursive synchronization
    } else if (iniConfig.SectionExists("Sync")) {
      ThrowF_t(TRANS("Trying to load a sync theme during an ongoing synchronization!"));
    }

  } catch (char *strError) {
    CPrintF(TRANS("Cannot load game theme config '%s':\n%s\n"), strFile.str_String, strError);
    return;
  }

  // Loading new config
  _pini = &iniConfig;
  strConfig = strFile;

  // Textures
  ReloadTextures(_pini->GetValue("Textures", "BackTex", "*"),
                 _pini->GetValue("Textures", "CloudsTex", "*"),
                 _pini->GetValue("Textures", "GridTex", "*"),
                 _pini->GetValue("Textures", "PointerTex", "*"));

  // Reset the game theme
  Reset();

  // Background elements
  SetFlag("Column", bColumn, TRUE);
  SetFlag("Sam", bSam, TRUE);

  // Stylized rendering
  SetFlag("TFEClouds1", bTFEClouds1, FALSE);
  SetFlag("TFEClouds2", bTFEClouds2, FALSE);
  SetFlag("TFEGrid", bTFEGrid, FALSE);
  SetFlag("ConClouds", bConClouds, TRUE);
  SetFlag("CompRawColor", bCompRawColor, FALSE);

  // Get color values and scan a hexadecimal integer
  #define SET_COLOR(_Identifier) SetColor(#_Identifier, col##_Identifier)

  SET_COLOR(OrangeLight);
  SET_COLOR(OrangeNeutral);
  SET_COLOR(OrangeDark);
  SET_COLOR(OrangeDarkLT);
  SET_COLOR(BlueDarkHV);
  SET_COLOR(BlueDark);
  SET_COLOR(BlueDarkLT);
  SET_COLOR(BlueNeutral);
  SET_COLOR(BlueLight);
  SET_COLOR(BlueGreenLT);

  SET_COLOR(Light);
  SET_COLOR(Neutral);
  SET_COLOR(Border);
  SET_COLOR(ConBorder);
  SET_COLOR(ConText);
  SET_COLOR(CompText);
  SET_COLOR(CompClouds);
  SET_COLOR(CompFloor);
};

// Load new game themes when needed
void CGameTheme::Update(void) {
  // Try loading a new theme
  static CTString strLastTheme = "";

  if (strLastTheme != strTheme) {
    strLastTheme = strTheme;
    Load(strTheme, TRUE);
  }

  // Try synchonizing loaded theme with the HUD theme
  static CSymbolPtr pHudTheme("ahud_iTheme");

  if (pHudTheme.Exists()) {
    // Check if there is a config under a specific sync index
    CTString strSync = _iniTheme.GetValue("Sync", CTString(0, "%d", pHudTheme.GetIndex()), "");

    // Load it
    if (FileExists(strSync)) {
      Load(strSync, FALSE);
    }
  }
};
