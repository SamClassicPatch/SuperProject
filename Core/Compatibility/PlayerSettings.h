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

#ifndef SE_INCL_PLAYERSETTINGS_H
#define SE_INCL_PLAYERSETTINGS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Class responsible for describing player appearance/settings
class CPlayerSettings {
  public:
    char ps_achModelFile[16]; // Filename of the player model (zero padded, not zero terminated)

    #define PS_WAS_ONLYNEW  0 // Select weapon if it's new
    #define PS_WAS_NONE     1 // Never autoselect
    #define PS_WAS_ALL      2 // Always autoselect
    #define PS_WAS_BETTER   3 // Only if the weapon is better than the one held
    SBYTE ps_iWeaponAutoSelect; // Weapon autoselection mode

    SBYTE ps_iCrossHairType; // Used crosshair

    #define PSF_HIDEWEAPON      (1L << 0) // Don't render weapon viewmodel
    #define PSF_PREFER3RDPERSON (1L << 1) // Automatically switch to the third person view
    #define PSF_NOQUOTES        (1L << 2) // Don't play quotes via VoiceHolder
    #define PSF_AUTOSAVE        (1L << 3) // Autosave at specific locations
    #define PSF_COMPSINGLECLICK (1L << 4) // Invoke computer with a single click, not double click
    #define PSF_SHARPTURNING    (1L << 5) // Use prescanning to eliminate mouse lag
    #define PSF_NOBOBBING       (1L << 6) // Disable view bobbing (for people with motion sickness problems)
    ULONG ps_ulFlags; // various flags

  public:
    // Get full path to the player model
    CTFileName GetModelFilename(void) const
    {
      char achModelFile[MAX_PATH + 1];
      memset(achModelFile, 0, sizeof(achModelFile));
      memcpy(achModelFile, ps_achModelFile, sizeof(ps_achModelFile));

      CTString strModelName = achModelFile;

      // [Cecil] Different implementation for TFE and Revolution
      #if SE1_GAME != SS_TSE
        CTString strModelFile = "Models\\Player\\" + strModelName + ".amc";

      #else
        CTString strModelFile = "ModelsMP\\Player\\" + strModelName + ".amc";

        if (!FileExists(strModelFile)) {
          strModelFile = "Models\\Player\\" + strModelName + ".amc";
        }
      #endif

      return strModelFile;
    };
};

#endif
