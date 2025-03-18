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
  { "CreateConverter",          &IWorldConverter::CreateConverter },         // Arg ptr : const char (string)
  { "SetMethodDestructor",      &IWorldConverter::SetMethodDestructor },     // Arg ptr : ExtArgWorldConverter_t (pData = FWorldConverterDestructor)
  { "SetMethodReset",           &IWorldConverter::SetMethodReset },          // Arg ptr : ExtArgWorldConverter_t (pData = FWorldConverterReset)
  { "SetMethodReplaceClass",    &IWorldConverter::SetMethodReplaceClass },   // Arg ptr : ExtArgWorldConverter_t (pData = FWorldConverterReplaceClass)
  { "SetMethodHandleProperty",  &IWorldConverter::SetMethodHandleProperty }, // Arg ptr : ExtArgWorldConverter_t (pData = FWorldConverterHandleProperty)
  { "SetMethodConvertWorld",    &IWorldConverter::SetMethodConvertWorld },   // Arg ptr : ExtArgWorldConverter_t (pData = FWorldConverterConvert)

  { "GetConverterForFormat",    &IWorldConverter::GetConverterForFormat }, // Arg ptr : ELevelFormat
  { "GetConverterByName",       &IWorldConverter::GetConverterByName },    // Arg ptr : const char (string)
  { "ResetConverter",           &IWorldConverter::ResetConverter },        // Arg ptr : ExtArgWorldConverter_t (pData = NULL)
  { "ConvertWorld",             &IWorldConverter::ConvertWorld },          // Arg ptr : ExtArgWorldConverter_t (pData = CWorld *)
  { "HandleUnknownProperty",    &IWorldConverter::HandleUnknownProperty }, // Arg ptr : ExtArgUnknownProp_t
  { "ReplaceClass",             &IWorldConverter::ReplaceClass },          // Arg ptr : ExtArgEclData_t
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
  // Register default converters only once
  static bool _bDefaultAdded = false;
  if (_bDefaultAdded) return;
  _bDefaultAdded = true;

#if CLASSIC_TSE_FUSION_MODE
  IWorldConverter *pTFE = IWorldConverter::Add("tfe");

  if (pTFE != NULL) {
    pTFE->m_pReset          = &IConvertTFE::Reset;
    pTFE->m_pHandleProperty = &IConvertTFE::HandleProperty;
    pTFE->m_pConvertWorld   = &IConvertTFE::ConvertWorld;
  }

  IWorldConverter *pSSR = IWorldConverter::Add("ssr");

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
