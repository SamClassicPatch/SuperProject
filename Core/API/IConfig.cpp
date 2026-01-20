/* Copyright (c) 2024-2026 Dreamy Cecil
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

HIniConfig ClassicsINI_Create(void) {
  return new CIniConfig;
};

void ClassicsINI_Destroy(HIniConfig hINI) {
  delete hINI;
};

void ClassicsINI_Clear(HIniConfig hINI) {
  hINI->Clear();
};

bool ClassicsINI_IsEmpty(HIniConfig hINI) {
  return hINI->IsEmpty();
};

bool ClassicsINI_SectionExists(HIniConfig hINI, const char *strSection) {
  return hINI->SectionExists(strSection);
};

bool ClassicsINI_KeyExists(HIniConfig hINI, const char *strSection, const char *strKey) {
  return hINI->KeyExists(strSection, strKey);
};

bool ClassicsINI_Delete(HIniConfig hINI, const char *strSection, const char *strKey) {
  return hINI->Delete(strSection, strKey);
};

void ClassicsINI_SetValue(HIniConfig hINI, const char *strSection, const char *strKey, const char *strValue) {
  hINI->SetValue(strSection, strKey, strValue);
};

void ClassicsINI_SetBoolValue(HIniConfig hINI, const char *strSection, const char *strKey, bool bValue) {
  hINI->SetBoolValue(strSection, strKey, bValue);
};

void ClassicsINI_SetIntValue(HIniConfig hINI, const char *strSection, const char *strKey, int iValue) {
  hINI->SetIntValue(strSection, strKey, iValue);
};

void ClassicsINI_SetDoubleValue(HIniConfig hINI, const char *strSection, const char *strKey, double dValue) {
  hINI->SetDoubleValue(strSection, strKey, dValue);
};

const char *ClassicsINI_GetValue(HIniConfig hINI, const char *strSection, const char *strKey, const char *strDefValue) {
  static CTString strTemp;
  strTemp = hINI->GetValue(strSection, strKey, strDefValue);
  return strTemp.str_String;
};

bool ClassicsINI_GetBoolValue(HIniConfig hINI, const char *strSection, const char *strKey, bool bDefValue) {
  return hINI->GetBoolValue(strSection, strKey, bDefValue);
};

int ClassicsINI_GetIntValue(HIniConfig hINI, const char *strSection, const char *strKey, int iDefValue) {
  return hINI->GetIntValue(strSection, strKey, iDefValue);
};

double ClassicsINI_GetDoubleValue(HIniConfig hINI, const char *strSection, const char *strKey, double dDefValue) {
  return hINI->GetDoubleValue(strSection, strKey, dDefValue);
};

void ClassicsINI_Read(HIniConfig hINI, const char *str) {
  hINI->Read(str);
};

const char *ClassicsINI_Write(HIniConfig hINI) {
  static IniStr strData;
  hINI->Write(strData);
  return strData.c_str();
};

namespace IConfig {

// Define global configs
GlobalProps global;
ModData mod;

// Constructor
GlobalProps::GlobalProps()
{
  // Set default properties
  props.New(k_EConfigProps_Max);

  // Global options
  props[k_EConfigProps_TFEMount        ] ("", "MountTFE",         true);
  props[k_EConfigProps_TFEDir          ] ("", "TFEDir",           strDefaultTFE);
  props[k_EConfigProps_SSRMount        ] ("", "MountSSR",         false);
  props[k_EConfigProps_SSRDir          ] ("", "SSRDir",           strDefaultSSR);
  props[k_EConfigProps_SSRWorkshopMount] ("", "MountSSRWorkshop", false);
  props[k_EConfigProps_SSRWorkshopDir  ] ("", "SSRWorkshop",      strDefaultWorkshop);

  props[k_EConfigProps_CustomMod         ] ("", "CustomMod",          true );
  props[k_EConfigProps_DebugPatcher      ] ("", "DebugPatcher",       false);
  props[k_EConfigProps_DPIAware          ] ("", "DPIAware",           true );
  props[k_EConfigProps_ExtendedFileSystem] ("", "ExtendedFileSystem", true );
  props[k_EConfigProps_ExtendedInput     ] ("", "ExtendedInput",      true );
  props[k_EConfigProps_FullAppIntegration] ("", "FullAppIntegration", false);
  props[k_EConfigProps_NotifyAboutUpdates] ("", "NotifyAboutUpdates", true );
  props[k_EConfigProps_ForceSeasonalEvent] ("", "ForceSeasonalEvent", -1   );

  // Steam API
  props[k_EConfigProps_SteamEnable    ] ("Steam", "Enable",     true ); // Initialize and use Steamworks API
  props[k_EConfigProps_SteamForServers] ("Steam", "ForServers", false); // Initialize for dedicated servers
  props[k_EConfigProps_SteamForTools  ] ("Steam", "ForTools",   false); // Initialize for tool applications
};

// Declare symbols for each property
void GlobalProps::DeclareSymbols(void)
{
  FOREACHINSTATICARRAY(props, NamedValue, it) {
    if (it->strKey == "") continue;

    CTString strDecl = it->strSection + it->strKey + ";";
    CAnyValue &val = it->val;

    switch (val.GetType()) {
      case CAnyValue::E_VAL_BOOL:   _pShell->DeclareSymbol("INDEX    cfg_b"   + strDecl, &val.GetIndex()); break;
      case CAnyValue::E_VAL_INDEX:  _pShell->DeclareSymbol("INDEX    cfg_i"   + strDecl, &val.GetIndex()); break;
      case CAnyValue::E_VAL_FLOAT:  _pShell->DeclareSymbol("FLOAT    cfg_f"   + strDecl, &val.GetFloat()); break;
      case CAnyValue::E_VAL_STRING: _pShell->DeclareSymbol("CTString cfg_str" + strDecl, &val.GetString()); break;
    }
  }
};

// Load properties from the config
void GlobalProps::Load(void) {
  // Try loading only once
  static BOOL bLoaded = FALSE;

  if (bLoaded) return;
  bLoaded = TRUE;

  // Try to load from a file
  try {
    iniConfig.Load_t(strGlobalConfigFile, FALSE);

  // Abort
  } catch (char *strError) {
    (void)strError;

    iniConfig.Clear();
    return;
  }

  // Get values
  FOREACHINSTATICARRAY(props, NamedValue, it) {
    if (it->strKey == "") continue;

    CAnyValue &val = it->val;

    switch (val.GetType()) {
      case CAnyValue::E_VAL_BOOL: val = iniConfig.GetBoolValue(it->strSection, it->strKey, !!val.GetIndex()); break;
      case CAnyValue::E_VAL_INDEX: val = (int)iniConfig.GetIntValue(it->strSection, it->strKey, val.GetIndex()); break;
      case CAnyValue::E_VAL_FLOAT: val = (float)iniConfig.GetDoubleValue(it->strSection, it->strKey, val.GetFloat()); break;
      case CAnyValue::E_VAL_STRING: val = iniConfig.GetValue(it->strSection, it->strKey, val.GetString()); break;
    }
  }
};

// Save properties into the config
void GlobalProps::Save(void) {
  // Set new values
  FOREACHINSTATICARRAY(props, NamedValue, it) {
    if (it->strKey == "") continue;

    iniConfig.SetValue(it->strSection, it->strKey, it->val.ToString());
  }

  // Save into a file
  IDir::CreateDir(strGlobalConfigFile);

  try {
    iniConfig.Save_t(strGlobalConfigFile);

  } catch (char *strError) {
    CPrintF(TRANS("Cannot save patch configuration file: %s\n"), strError);
  }
};

CAnyValue &GlobalProps::operator[](int iProperty)
{
  // Invalid property
  if (iProperty < 0 || iProperty >= k_EConfigProps_Max) {
    static CAnyValue _valInvalid;
    return _valInvalid;
  }

  return props[iProperty].val;
};

#if _PATCHCONFIG_GAMEPLAY_EXT

GameplayExt gex(TRUE);

// Constructor
GameplayExt::GameplayExt(BOOL bVanilla)
{
  // Set default properties
  props.New(k_EGameplayExt_Max);

  props[k_EGameplayExt_Enable             ] ("", "Enable",              true );
  props[k_EGameplayExt_FixTimers          ] ("", "FixTimers",           true );
  props[k_EGameplayExt_GravityAcc         ] ("", "GravityAcc",          1.0f );
  props[k_EGameplayExt_UnlimitedAirControl] ("", "UnlimitedAirControl", false);
  props[k_EGameplayExt_MoveSpeed          ] ("", "MoveSpeed",           1.0f );
  props[k_EGameplayExt_JumpHeight         ] ("", "JumpHeight",          1.0f );
  props[k_EGameplayExt_FastKnife          ] ("", "FastKnife",           false);

  Reset(bVanilla);
};

// Reset settings
void GameplayExt::Reset(BOOL bVanilla) {
  // Vanilla-compatible settings
  if (bVanilla) {
    (*this)[k_EGameplayExt_Enable].GetIndex() = FALSE;
    (*this)[k_EGameplayExt_FixTimers].GetIndex() = FALSE;

  // Recommended settings
  } else {
    (*this)[k_EGameplayExt_Enable].GetIndex() = TRUE;
    (*this)[k_EGameplayExt_FixTimers].GetIndex() = TRUE;
  }

  // Default settings
  (*this)[k_EGameplayExt_GravityAcc].GetFloat() = 1.0f;
  (*this)[k_EGameplayExt_UnlimitedAirControl].GetIndex() = FALSE;
  (*this)[k_EGameplayExt_MoveSpeed].GetFloat() = 1.0f;
  (*this)[k_EGameplayExt_JumpHeight].GetFloat() = 1.0f;
  (*this)[k_EGameplayExt_FastKnife].GetIndex() = FALSE;
};

void GameplayExt::DeclareSymbols(void) {
  FOREACHINSTATICARRAY(props, NamedValue, it) {
    if (it->strKey == "") continue;

    CTString strDecl = it->strKey + " pre:UpdateServerSymbolValue;";
    CAnyValue &val = it->val;

    switch (val.GetType()) {
      case CAnyValue::E_VAL_BOOL:   _pShell->DeclareSymbol("persistent user INDEX    gex_b"   + strDecl, &val.GetIndex()); break;
      case CAnyValue::E_VAL_INDEX:  _pShell->DeclareSymbol("persistent user INDEX    gex_i"   + strDecl, &val.GetIndex()); break;
      case CAnyValue::E_VAL_FLOAT:  _pShell->DeclareSymbol("persistent user FLOAT    gex_f"   + strDecl, &val.GetFloat()); break;
      case CAnyValue::E_VAL_STRING: _pShell->DeclareSymbol("persistent user CTString gex_str" + strDecl, &val.GetString()); break;
    }
  }
};

CAnyValue &GameplayExt::operator[](int iProperty)
{
  // Invalid property
  if (iProperty < 0 || iProperty >= k_EGameplayExt_Max) {
    static CAnyValue _valInvalid;
    return _valInvalid;
  }

  return props[iProperty].val;
};

#endif // _PATCHCONFIG_GAMEPLAY_EXT

// Constructor
ModData::ModData()
{
  // Set default properties
  props.New(k_EModDataProps_Max);

  props[k_EModDataProps_AdjustFOV        ] ("", "AdjustFOV",         true);
  props[k_EModDataProps_AdjustAR         ] ("", "AdjustAR",          true);
  props[k_EModDataProps_ProperTextScaling] ("", "ProperTextScaling", true);
  props[k_EModDataProps_MenuTextScale    ] ("", "MenuTextScale",     1.0f);

  // Set default game difficulties
  aDiffs[0] = ModDifficulty_t(-1, "Tourist", "for non-FPS players");
  aDiffs[1] = ModDifficulty_t( 0, "Easy",    "for unexperienced FPS players");
  aDiffs[2] = ModDifficulty_t( 1, "Normal",  "for experienced FPS players");
  aDiffs[3] = ModDifficulty_t( 2, "Hard",    "for experienced Serious Sam players");
  aDiffs[4] = ModDifficulty_t( 3, "Serious", "are you serious?");

  aDiffs[5] = ModDifficulty_t( 4, "Mental",  "you are not serious!", "sam_bMentalActivated");
  aDiffs[5].m_bFlash = TRUE;

  // Clear the rest
  ClearDiffArray(6);
};

// Load user configs for customization
void ModData::LoadConfigs(void) {
  // Get values
  try {
    CIniConfig iniModData;
    iniModData.Load_t(strModConfigFile, TRUE);

    FOREACHINSTATICARRAY(props, NamedValue, it) {
      if (it->strKey == "") continue;

      CAnyValue &val = it->val;

      switch (val.GetType()) {
        case CAnyValue::E_VAL_BOOL: val = iniModData.GetBoolValue(it->strSection, it->strKey, !!val.GetIndex()); break;
        case CAnyValue::E_VAL_INDEX: val = (int)iniModData.GetIntValue(it->strSection, it->strKey, val.GetIndex()); break;
        case CAnyValue::E_VAL_FLOAT: val = (float)iniModData.GetDoubleValue(it->strSection, it->strKey, val.GetFloat()); break;
        case CAnyValue::E_VAL_STRING: val = iniModData.GetValue(it->strSection, it->strKey, val.GetString()); break;
      }
    }

  } catch (char *strError) {
    (void)strError;
  }

  // Load difficulties
  CFileList aConfigs;
  BOOL bLoadFromGame = TRUE;

  // Load from the mod
  if (_fnmMod != "") {
    ListGameFiles(aConfigs, "Data\\ClassicsPatch\\Difficulties\\", "*.ini", FLF_ONLYMOD);

    // Don't load from the game if there are any mod difficulties
    if (aConfigs.Count() != 0) bLoadFromGame = FALSE;
  }

  // Load from the game
  if (bLoadFromGame) ListGameFiles(aConfigs, "Data\\ClassicsPatch\\Difficulties\\", "*.ini", 0);

  // Amount of user difficulties
  const INDEX ctDiffs = ClampUp(aConfigs.Count(), ctMaxGameDiffs);
  if (ctDiffs == 0) return;

  // Create new difficulties
  ClearDiffArray();

  for (INDEX i = 0; i < ctDiffs; i++) {
    const CTFileName &fnm = aConfigs[i];
    ModDifficulty_t &diff = aDiffs[i];

    // Load difficulty properties
    try {
      CIniConfig iniDiff;
      iniDiff.Load_t(fnm, TRUE);

      diff.m_iLevel = iniDiff.GetIntValue("", "Level", i);
      diff.m_bFlash = !!iniDiff.GetBoolValue("", "Flash", FALSE);
      diff.SetCommand(iniDiff.GetValue("", "UnlockCommand", ""));

    } catch (char *strError) {
      CPrintF(TRANS("Cannot load user difficulty config '%s':\n%s"), fnm.str_String, strError);
    }

    // Get difficulty name and description
    CTString strName = "???";
    CTString strDesc = "";

    try {
      // Try loading text from the description file nearby
      strName.Load_t(fnm.NoExt() + ".des");

      // Separate text into name and description
      ULONG ulLineBreak = IData::FindChar(strName, '\n');

      if (ulLineBreak != -1) {
        strName.Split(ulLineBreak + 1, strName, strDesc);
      }

    } catch (char *strError) {
      // Just set text to the filename
      (void)strError;
      strName = fnm.FileName();
    }

    diff.SetName(strName);
    diff.SetTip(strDesc);
  }
};

// Reset every difficulty in the array starting from a specific one
void ModData::ClearDiffArray(int iFromDifficulty)
{
  for (INDEX i = iFromDifficulty; i < ctMaxGameDiffs; i++) {
    aDiffs[i] = ModDifficulty_t();
  }
};

CAnyValue &ModData::operator[](int iProperty)
{
  // Invalid property
  if (iProperty < 0 || iProperty >= k_EModDataProps_Max) {
    static CAnyValue _valInvalid;
    return _valInvalid;
  }

  return props[iProperty].val;
};

}; // namespace

bool ClassicsConfig_IsTrue(EConfigProps eProperty) { return IConfig::global[eProperty].IsTrue(); };
int ClassicsConfig_GetInt(EConfigProps eProperty) { return IConfig::global[eProperty].GetIndex(); };
float ClassicsConfig_GetFloat(EConfigProps eProperty) { return IConfig::global[eProperty].GetFloat(); };
const char *ClassicsConfig_GetString(EConfigProps eProperty) { return IConfig::global[eProperty].GetString(); };

#if _PATCHCONFIG_GAMEPLAY_EXT
  bool ClassicsGameplayExt_IsTrue(EGameplayExt eExtension) { return IConfig::gex[eExtension].IsTrue(); };
  int ClassicsGameplayExt_GetInt(EGameplayExt eExtension) { return IConfig::gex[eExtension].GetIndex(); };
  float ClassicsGameplayExt_GetFloat(EGameplayExt eExtension) { return IConfig::gex[eExtension].GetFloat(); };
  const char *ClassicsGameplayExt_GetString(EGameplayExt eExtension) { return IConfig::gex[eExtension].GetString(); };
#else
  bool ClassicsGameplayExt_IsTrue(EGameplayExt eExtension) { ASSERTALWAYS(GAMEPLAY_EXT_ASSERT_MSG); return false; };
  int ClassicsGameplayExt_GetInt(EGameplayExt eExtension) { ASSERTALWAYS(GAMEPLAY_EXT_ASSERT_MSG); return 0; };
  float ClassicsGameplayExt_GetFloat(EGameplayExt eExtension) { ASSERTALWAYS(GAMEPLAY_EXT_ASSERT_MSG); return 0.0f; };
  const char *ClassicsGameplayExt_GetString(EGameplayExt eExtension) { ASSERTALWAYS(GAMEPLAY_EXT_ASSERT_MSG); return ""; };
#endif // _PATCHCONFIG_GAMEPLAY_EXT

bool ClassicsModData_IsTrue(EModDataProps eProperty) { return IConfig::mod[eProperty].IsTrue(); };
int ClassicsModData_GetInt(EModDataProps eProperty) { return IConfig::mod[eProperty].GetIndex(); };
float ClassicsModData_GetFloat(EModDataProps eProperty) { return IConfig::mod[eProperty].GetFloat(); };
const char *ClassicsModData_GetString(EModDataProps eProperty) { return IConfig::mod[eProperty].GetString(); };

ModDifficulty_t *ClassicsModData_GetDiff(int iDifficulty)
{
  if (iDifficulty < 0 || iDifficulty >= IConfig::ctMaxGameDiffs) return NULL;

  return &IConfig::mod.aDiffs[iDifficulty];
};

int ClassicsModData_DiffArrayLength(void)
{
  return IConfig::ctMaxGameDiffs;
};

void ClassicsModData_ClearDiffArray(int iFromDifficulty)
{
  IConfig::mod.ClearDiffArray(iFromDifficulty);
};

bool ClassicsModData_IsDiffActive(int iDifficulty)
{
  ModDifficulty_t *pDiff = ClassicsModData_GetDiff(iDifficulty);

  // Can't be active if no difficulty
  if (pDiff == NULL || strcmp(pDiff->m_strName, "") == 0) return FALSE;

  // Active if no command
  if (strcmp(pDiff->m_strCommand, "") == 0) return TRUE;

  CShellSymbol *pss = _pShell->GetSymbol(pDiff->m_strCommand, TRUE);

  // Active if no symbol
  if (pss == NULL) return TRUE;

  return (*(INDEX *)pss->ss_pvValue) != 0;
};

int ClassicsModData_CountNamedDiffs(void)
{
  INDEX i = 0;

  for (; i < IConfig::ctMaxGameDiffs; i++) {
    // No more difficulties
    if (strcmp(IConfig::mod.aDiffs[i].m_strName, "") == 0) break;
  }

  return i;
};

int ClassicsModData_FindDiffByLevel(int iLevel)
{
  for (INDEX i = 0; i < IConfig::ctMaxGameDiffs; i++) {
    // Matching level
    if (IConfig::mod.aDiffs[i].m_iLevel == iLevel) return i;
  }

  return -1;
};

// Interface initialization
namespace IInitAPI {

// Resave config properties into the file after setting them
static void ResaveConfigProperties(void) {
  IConfig::global.Save();

  CPrintF(TRANS("Config properties have been resaved into '%s'!\n"), IConfig::strGlobalConfigFile);
  CPutString(TRANS("Restart the game for the new settings to take effect!\n"));
};

void Config(void) {
  // Config property symbols
  _pShell->DeclareSymbol("void ResaveConfigProperties(void);", &ResaveConfigProperties);
  IConfig::global.DeclareSymbols();

  // Prepare variable data
  IConfig::mod.LoadConfigs();
};

}; // namespace