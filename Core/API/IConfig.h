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

#ifndef CECIL_INCL_CONFIGINTERFACE_H
#define CECIL_INCL_CONFIGINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

namespace IConfig {

// Configuration files
const CTString strGlobalConfigFile = "Data\\ClassicsPatch\\Config.ini";
const CTString strModConfigFile    = "Data\\ClassicsPatch\\ModData.ini";

// Default content directories
const CTString strDefaultTFE = "..\\Serious Sam Classic The First Encounter\\";
const CTString strDefaultSSR = "..\\Serious Sam Revolution\\";
const CTString strDefaultWorkshop = "..\\..\\workshop\\content\\227780\\";

// Value entry in some config
struct NamedValue {
  CTString strSection;
  CTString strKey;
  CAnyValue val;

  NamedValue() {};

  __forceinline NamedValue(const char *strSetSection, const char *strSetKey, CAnyValue valSet) {
    operator()(strSetSection, strSetKey, valSet);
  };

  __forceinline void operator=(const NamedValue &other) {
    operator()(other.strSection, other.strKey, other.val);
  };

  __forceinline void operator()(const char *strSetSection, const char *strSetKey, CAnyValue valSet) {
    strSection = strSetSection;
    strKey = strSetKey;
    val = valSet;
  };
};

// A fixed-size array of properties
typedef CStaticArray<NamedValue> CProperties;

struct GlobalProps {
  // Constructor
  GlobalProps();

  // Declare symbols for each property
  void DeclareSymbols(void);

  // Load properties from the config
  void Load(void);

  // Save properties into the config
  void Save(void);

  // Convenient value getter
  CORE_API CAnyValue &operator[](int iProperty);

private:
  CIniConfig iniConfig; // Config to read and write
  CProperties props; // Actual storage of config properties

  // No copying
  GlobalProps(const GlobalProps &);
  GlobalProps &operator=(const GlobalProps &);
};

// Global config properties
CORE_API extern GlobalProps global;

#if _PATCHCONFIG_GAMEPLAY_EXT

struct GameplayExt {
  CProperties props;

  // Constructor
  GameplayExt(BOOL bVanilla = TRUE);

  // Reset settings
  void Reset(BOOL bVanilla);

  // Declare symbols for each property
  void DeclareSymbols(void);

  // Assignment operator
  inline GameplayExt &operator=(const GameplayExt &other) {
    // Copy all data
    props.CopyArray(other.props);
    return *this;
  };

  // Convenient value getter
  CORE_API CAnyValue &operator[](int iProperty);
};

// Currently active gameplay extensions
CORE_API extern GameplayExt gex;

#else

#define GAMEPLAY_EXT_ASSERT_MSG "_PATCHCONFIG_GAMEPLAY_EXT is turned off in this build!"

#endif // _PATCHCONFIG_GAMEPLAY_EXT

// Maximum amount of difficulties the game can have
const INDEX ctMaxGameDiffs = 16;

struct ModData {
  CProperties props;
  ModDifficulty_t aDiffs[ctMaxGameDiffs];

  // Constructor
  ModData();

  // Load user configs for customization
  void LoadConfigs(void);

  // Reset every difficulty in the array starting from a specific one
  void ClearDiffArray(int iFromDifficulty = 0);

  // Convenient value getter
  CORE_API CAnyValue &operator[](int iProperty);

private:
  // No copying
  ModData(const ModData &);
  ModData &operator=(const ModData &);
};

// Custom read-only data for a mod
CORE_API extern ModData mod;

}; // namespace

#endif
