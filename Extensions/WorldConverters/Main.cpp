/* Copyright (c) 2025 Dreamy Cecil
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

#include "Converters/TFEMaps.h"
#include "Converters/RevMaps.h"

// Define extension
CLASSICSPATCH_DEFINE_EXTENSION("PATCH_EXT_wldconverters", k_EPluginFlagGame | k_EPluginFlagServer | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "World Converters",
  "Collection of world file converters between different game formats.");

CLASSICSPATCH_EXTENSION_SIGNALS_BEGIN {
  { "SetConverterForFormat",    &SetConverterForFormat }, // Arg ptr : ELevelFormat
  { "ResetConverter",           &ResetConverter },
  { "ConvertWorld",             &ConvertWorld }, // Arg ptr : CWorld
  { "HandleUnknownProperty",    &HandleUnknownProperty }, // Arg ptr : ExtArgUnknownProp_t
  { "ReplaceMissingClasses",    &ReplaceMissingClasses }, // Arg ptr : ExtArgEclData_t
  { "ReplaceRevolutionClasses", &ReplaceRevolutionClasses }, // Arg ptr : CTFileName
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Register default converters only once
  static bool _bDefaultAdded = false;
  if (_bDefaultAdded) return;
  _bDefaultAdded = true;

#if CLASSIC_TSE_FUSION_MODE
  IWorldFormatConverter *pTFE = IWorldFormatConverter::Add("tfe");

  if (pTFE != NULL) {
    pTFE->m_pReset          = &IConvertTFE::Reset;
    pTFE->m_pHandleProperty = &IConvertTFE::HandleProperty;
    pTFE->m_pConvertWorld   = &IConvertTFE::ConvertWorld;
  }

  IWorldFormatConverter *pSSR = IWorldFormatConverter::Add("ssr");

  if (pSSR != NULL) {
    pSSR->m_pReset          = &IConvertSSR::Reset;
    pSSR->m_pHandleProperty = &IConvertSSR::HandleProperty;
    pSSR->m_pConvertWorld   = &IConvertSSR::ConvertWorld;
  }
#endif
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
