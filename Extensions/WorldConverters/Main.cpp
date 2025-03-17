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

// Define extension
CLASSICSPATCH_DEFINE_EXTENSION("PATCH_EXT_wldconverters", k_EPluginFlagGame | k_EPluginFlagServer | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "World Converters",
  "Collection of world file converters between different game formats.");

CLASSICSPATCH_EXTENSION_SIGNALS_BEGIN {
  { "SetConverterForFormat",    &IMapConverter::SetConverterForFormat }, // Arg ptr : ELevelFormat
  { "ResetConverter",           &IMapConverter::ResetConverter },
  { "ConvertWorld",             &IMapConverter::ConvertWorld }, // Arg ptr : CWorld
  { "HandleUnknownProperty",    &IMapConverter::HandleUnknownProperty }, // Arg ptr : ExtArgUnknownProp_t
  { "ReplaceMissingClasses",    &ReplaceMissingClasses }, // Arg ptr : ExtArgEclData_t
  { "ReplaceRevolutionClasses", &ReplaceRevolutionClasses }, // Arg ptr : CTFileName
} CLASSICSPATCH_EXTENSION_SIGNALS_END;

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events)
{
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props)
{
};
