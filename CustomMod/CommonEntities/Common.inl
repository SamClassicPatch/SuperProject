/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_ENTITIES_COMMON_INL
#define CECIL_INCL_ENTITIES_COMMON_INL

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Include all inline source code
#include "BloodThemes.inl"
#include "LocalPrediction.inl"
#include "WeaponCustomization.inl"

// Gradual crosshair coloring
INDEX hud_bCrosshairGradualColoring = FALSE;

// ModelHolder2 mip factor adjustment
INDEX gfx_bAdjustModelHolderMipFactor = TRUE;

// Declare common symbols
void DeclareCommonEntitiesSymbols(void)
{
  _pShell->DeclareSymbol("persistent user INDEX gam_bDisableLocalPrediction;", &gam_bDisableLocalPrediction);
  _pShell->DeclareSymbol("persistent user INDEX hud_bCrosshairGradualColoring;", &hud_bCrosshairGradualColoring);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bAdjustModelHolderMipFactor;", &gfx_bAdjustModelHolderMipFactor);

  DeclareCustomBloodSymbols();
  DeclareWeaponCustomizationSymbols();
};

#endif
