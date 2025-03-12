/* Copyright (c) 2022-2024 Dreamy Cecil
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

// Define plugin
CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagGame | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Accessibility", "Various toggleable options for enhanced accessibility.");

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(CIniConfig &props, PluginEvents_t &events)
{
  // Initialize accessibility modules
  InitPlayerInfoSound();
  InitRedScreen();
};
