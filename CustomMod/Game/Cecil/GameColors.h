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

#ifndef CECIL_INCL_GAMECOLORS_H
#define CECIL_INCL_GAMECOLORS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

/* Usage reference table
 * SE_COL_ORANGE_LIGHT   - GameExecutable: LCDGetColor(<several>)
 * SE_COL_ORANGE_NEUTRAL - GameExecutable: LCDGetColor(<several>)
 * SE_COL_ORANGE_DARK    - GameExecutable: LCDGetColor("disabled unselected")
 * SE_COL_ORANGE_DARK_LT - GameExecutable: LCDGetColor("disabled selected")
 * SE_COL_BLUE_DARK_HV   - Game: Net graph background
 * SE_COL_BLUE_DARK      - Game: Several uses
 * SE_COL_BLUE_DARK_LT   - GameExecutable: LCDGetColor("edit fill")
 * SE_COL_BLUE_NEUTRAL   - Game: Main menu elements
 * SE_COL_BLUE_LIGHT     - Game: Loading bar with several levels of tint
 * SE_COL_BLUEGREEN_LT   - Game: Center message (e.g. "Paused")
 *
 * CECIL_COL_LIGHT      - GameExecutable: LCDGetColor(<several>); equal to C_WHITE in TFE
 * CECIL_COL_NEUTRAL    - GameExecutable: LCDGetColor(<several>); equal to C_WHITE in TFE
 * CECIL_COL_BORDER     - Game: Border boxes (player model, computer)
 * CECIL_COL_CONBORDER  - Game: Bottom line of the opened console
 * CECIL_COL_CONTEXT    - Game: Console text
 * CECIL_COL_COMPTEXT   - Game: Computer text
 * CECIL_COL_COMPCLOUDS - Game: Background clouds of the computer
 * CECIL_COL_COMPFLOOR  - Game: Computer's floor model
 */

// Vanilla colors of the current theme
#define SE_COL_ORANGE_LIGHT   _gmtTheme.colOrangeLight
#define SE_COL_ORANGE_NEUTRAL _gmtTheme.colOrangeNeutral
#define SE_COL_ORANGE_DARK    _gmtTheme.colOrangeDark
#define SE_COL_ORANGE_DARK_LT _gmtTheme.colOrangeDarkLT
#define SE_COL_BLUE_DARK_HV   _gmtTheme.colBlueDarkHV
#define SE_COL_BLUE_DARK      _gmtTheme.colBlueDark
#define SE_COL_BLUE_DARK_LT   _gmtTheme.colBlueDarkLT
#define SE_COL_BLUE_NEUTRAL   _gmtTheme.colBlueNeutral
#define SE_COL_BLUE_LIGHT     _gmtTheme.colBlueLight
#define SE_COL_BLUEGREEN_LT   _gmtTheme.colBlueGreenLT

// New colors of the current theme
#define CECIL_COL_LIGHT      _gmtTheme.colLight      // SE_COL_ORANGE_LIGHT in TSE
#define CECIL_COL_NEUTRAL    _gmtTheme.colNeutral    // SE_COL_ORANGE_NEUTRAL in TSE
#define CECIL_COL_BORDER     _gmtTheme.colBorder     // SE_COL_BLUE_NEUTRAL in TSE
#define CECIL_COL_CONBORDER  _gmtTheme.colConBorder  // SE_COL_BLUE_NEUTRAL in TSE
#define CECIL_COL_CONTEXT    _gmtTheme.colConText    // SE_COL_BLUE_LIGHT in TSE
#define CECIL_COL_COMPTEXT   _gmtTheme.colCompText   // SE_COL_BLUE_LIGHT in TSE
#define CECIL_COL_COMPCLOUDS _gmtTheme.colCompClouds // SE_COL_BLUE_NEUTRAL in TSE
#define CECIL_COL_COMPFLOOR  _gmtTheme.colCompFloor  // 0x5A8EC200 in TSE

#endif
