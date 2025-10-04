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

#ifndef SE_INCL_VARLIST_H
#define SE_INCL_VARLIST_H
#ifdef PRAGMA_ONCE
#pragma once
#endif

class CVarSetting {
  public:
    // [Cecil] Setting types
    enum ESettingType {
      E_SEPARATOR, // Equivalent of vanilla 'vs_bSeparator'
      E_TOGGLE,
      E_TEXTBOX,
      E_BUTTON,
      E_KEYBIND,
    };

    // [Cecil] Slider types
    enum ESliderType {
      SLD_NOSLIDER = 0,
      SLD_FILL     = 1,
      SLD_RATIO    = 2,
      SLD_BIGFILL  = 3,
    };

  public:
    CListNode vs_lnNode;
    ESettingType vs_eType; // [Cecil] Setting type
    BOOL vs_bCanChangeInGame;
    ESliderType vs_eSlider;
    CTString vs_strName;
    CTString vs_strTip;
    CTString vs_strVar;
    CTString vs_strFilter;
    CTString vs_strSchedule; // [Cecil] Repurposed as a shell command
    INDEX vs_iValue;
    CTString vs_strValue; // [Cecil] String value
    INDEX vs_ctValues;
    INDEX vs_iOrgValue;
    BOOL vs_bCustom;
    BOOL vs_bHidden; // [Cecil] Hide value string
    BOOL vs_bRealTime; // [Cecil] Apply values in real time while changing them
    CStringStack vs_astrTexts;
    CStringStack vs_astrValues;
    CVarSetting();
    void Clear(void);
    BOOL Validate(void);

    // [Cecil] Copy constructor
    CVarSetting(const CVarSetting &vsOther);

    // [Cecil] Immediately apply new value to this setting (taken out of FlushVarSettings() method)
    BOOL ApplyValue(void);

    // [Cecil] Update original value after applying it with ApplyValue() (taken out of LoadVarSettings() method)
    void UpdateValue(void);
};

// [Cecil] Options tab
class CVarTab {
  public:
    CTString strName; // Tab name
    CListHead lhVars; // Tab variables
};

// [Cecil] Tabs of options
extern CStaticStackArray<CVarTab> _aTabs;

void LoadVarSettings(const CTFileName &fnmCfg);
void FlushVarSettings(BOOL bApply);

#endif /* include-once check. */
