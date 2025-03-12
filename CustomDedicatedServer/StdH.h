/* Copyright (c) 2002-2012 Croteam Ltd.
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

// [Cecil] Include the core library and patches
#include <Core/Core.h>
#include <CorePatches/Patches.h>

#include "AppState.h"
#include "GameHandling.h"

// Dedicated server properties
extern INDEX ded_iMaxFPS;
extern CTString ded_strConfig;
extern CTString ded_strLevel;
extern INDEX ded_bRestartWhenEmpty;
extern FLOAT ded_tmTimeout;
extern CTString ded_strForceLevelChange; // [Cecil]

// Execute shell script
void ExecScript(const CTString &str);
