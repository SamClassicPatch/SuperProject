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

#include "StdH.h"

#include "VarList.h"

#include "GUI/Menus/MenuStuff.h"

// [Cecil] Tabs of options
CStaticStackArray<CVarTab> _aTabs;

// [Cecil] List of opened configs to prevent recursion
static CDynamicContainer<CTFileStream> _cOpenedConfigs;

CTString _strFile;
INDEX _ctLines;

CTString GetNonEmptyLine_t(CTStream &strm) {
  FOREVER {
    if (strm.AtEOF()) {
      ThrowF_t(LOCALIZE("Unexpected end of file"));
    }

    CTString str;
    _ctLines++;

    strm.GetLine_t(str);
    str.TrimSpacesLeft();

    if (str.RemovePrefix("//")) { // skip comments
      continue;
    }

    if (str != "") {
      str.TrimSpacesRight();
      return str;
    }
  }
}

void TranslateLine(CTString &str) {
  str.TrimSpacesLeft();

  // [Cecil] "TTRS" in the EXE as is gets picked up by the Depend utility
  if (str.RemovePrefix(CTString("TT") + "RS")) {
    str.TrimSpacesLeft();
    str = TRANSV(str);

  // [Cecil] Translate already officially localized string (ignored by Depend utility compared to "TTRS")
  } else if (str.RemovePrefix("PATCHLOCALIZE")) {
    str.TrimSpacesLeft();
    str = TRANSV(str);
  }

  str.TrimSpacesLeft();
}

void FixupFileName_t(CTString &strFnm) {
  strFnm.TrimSpacesLeft();
  strFnm.TrimSpacesRight();

  // [Cecil] "TFNM" in the EXE as is gets picked up by the Depend utility
  if (!strFnm.RemovePrefix(CTString("TF") + "NM ")) {
    ThrowF_t(LOCALIZE("Expected %s%s before filename"), "TF", "NM");
  }
}

void CheckPVS_t(CVarSetting *pvs) {
  if (pvs == NULL) {
    ThrowF_t("Gadget expected");
  }
}

// [Cecil] Add screen resolution as one of the setting values
static void AddScreenResolution(CVarSetting *pvs, CStaticStackArray<PIX2D> &aResolutions, PIX2D vAddRes) {
  PIX2D vAspect = vAddRes;
  const DOUBLE fRatio = DOUBLE(vAspect(1)) / DOUBLE(vAspect(2));

  // Check every ratio until the smallest one with full integers is found
  for (INDEX iCheckY = 1; iCheckY < vAddRes(2); iCheckY++) {
    DOUBLE fRatioX = (DOUBLE)iCheckY * fRatio;

    // Allow a small rounding error (e.g. to allow 854x480 and 1366x768 to be listed as 16:9)
    if (Abs(fRatioX - floor(fRatioX + 0.5)) <= 0.015f) {
      vAspect = PIX2D((PIX)fRatioX, iCheckY);
      break;
    }
  }

  if (vAspect == PIX2D(8, 5)) vAspect = PIX2D(16, 10); // 8:5 == 16:10
  if (vAspect == PIX2D(7, 3)) vAspect = PIX2D(21,  9); // 7:3 == 21:9

  CTString strResolution(0, "%dx%d", vAddRes(1), vAddRes(2));
  CTString strRatio(0, " (%d:%d)", vAspect(1), vAspect(2));

  // Fits within the screen
  if (vAddRes(1) <= _vpixScreenRes(1) && vAddRes(2) <= _vpixScreenRes(2)) {
    // Matching the screen
    if (vAddRes == _vpixScreenRes) {
      strRatio += TRANS(" (Native)");
    }

    pvs->vs_astrTexts.Push() = strResolution + strRatio;
    pvs->vs_astrValues.Push() = strResolution;

    aResolutions.Push() = vAddRes;
  }
};

// [Cecil] For keeping track of added resolutions under one option
static CStaticStackArray<PIX2D> _aScreenResolutions;

// [Cecil] Parse an autovalue
static void ParseAutoValue(CVarSetting *pvs, CTString &strLine) {
  // Scan special variable
  char strScanVar[256] = { 0 };
  CTString strVar = "";

  // Simply add the same text and value, if couldn't parse any variable
  if (strLine.ScanF("#%[^#]#", strScanVar) == 0) {
    // Translate text but not value
    CTString &strAddedText = pvs->vs_astrTexts.Push();
    strAddedText = strLine;
    TranslateLine(strAddedText);

    pvs->vs_astrValues.Push() = strLine;
    return;
  }

  strVar = strScanVar;

  // Remove variable from the rest of the string
  strLine.RemovePrefix("#" + strVar + "#");
  strLine.TrimSpacesLeft();

  // Generate a list of values based on some numeric range
  if (strVar == "RANGE") {
    DOUBLE fMin, fMax, fStep;
    INDEX iScan = strLine.ScanF("%lf %lf %lf %255s", &fMin, &fMax, &fStep, strScanVar);

    if (iScan < 3) {
      ThrowF_t(TRANS("Range property requires three numbers: min, max and step"));
    }

    // 0 - numbers as is; 1 - min..max ratio
    INDEX iDisplayType = 0;

    // Determine display type
    if (iScan >= 4) {
      CTString strDisplay = strScanVar;

      if (strDisplay == "ratio") {
        iDisplayType = 1;
      }
    }

    // Safety checks
    if (fMin  < -1e6) ThrowF_t(TRANS("Range: min value is too low"));
    if (fMax  > +1e6) ThrowF_t(TRANS("Range: max value is too high"));
    if (fStep < 1e-3) ThrowF_t(TRANS("Range: step value is too low"));

    // Add each step from min to max
    for (DOUBLE fAdd = fMin; fAdd < fMax; fAdd += fStep) {
      const DOUBLE fRatio = (fAdd - fMin) / (fMax - fMin);

      // E.g. "0.25" value and "25%" text
      CTString &strValue = pvs->vs_astrValues.Push();
      strValue.PrintF("%g", fAdd);

      CTString &strText = pvs->vs_astrTexts.Push();

      if (iDisplayType == 1) {
        strText.PrintF("%d%%", INDEX(fRatio * 100));
      } else {
        strText = strValue;
      }
    }

    // Add maximum value
    CTString &strMaxValue = pvs->vs_astrValues.Push();
    strMaxValue.PrintF("%g", fMax);

    CTString &strMaxText = pvs->vs_astrTexts.Push();

    if (iDisplayType == 1) {
      strMaxText = "100%";
    } else {
      strMaxText = strMaxValue;
    }

  // Add screen resolutions from the list file
  } else if (strVar == "RESOLUTION_LIST") {
    CFileList aResList;

    if (!IFiles::LoadStringList(aResList, CTString("Data\\ClassicsPatch\\Resolutions.lst"))) {
      ThrowF_t(TRANS("AutoValue: Cannot load resolution list"));
    }

    const INDEX ctRes = aResList.Count();
    PIX2D vRes;

    for (INDEX iAddRes = 0; iAddRes < ctRes; iAddRes++) {
      CTFileName &strRes = aResList[iAddRes];

      // Skip invalid strings
      if (strRes.ScanF("%dx%d", &vRes(1), &vRes(2)) != 2) continue;

      AddScreenResolution(pvs, _aScreenResolutions, vRes);
    }

  // Add native screen resolution if it's not already in the list
  } else if (strVar == "NATIVE_RESOLUTION") {
    BOOL bAdded = FALSE;
    const INDEX ctRes = _aScreenResolutions.Count();

    for (INDEX iRes = 0; iRes < ctRes; iRes++) {
      if (_aScreenResolutions[iRes] == _vpixScreenRes) {
        bAdded = TRUE;
        break;
      }
    }

    if (!bAdded) {
      AddScreenResolution(pvs, _aScreenResolutions, _vpixScreenRes);
    }

  // Add a display adapter under a specific number
  } else if (strVar == "DISPLAY_ADAPTER") {
    INDEX iAdapter;

    if (strLine.ScanF("%d", &iAdapter) != 1) {
      ThrowF_t(TRANS("AutoValue: Expected display adapter index"));
    }

    INDEX iAPI = NormalizeGfxAPI(sam_iGfxAPI);
    const INDEX ctAdapters = _pGfx->gl_gaAPI[iAPI].ga_ctAdapters;

    if (iAdapter >= 0 && iAdapter < ctAdapters) {
      CTString strAdapter = _pGfx->gl_gaAPI[iAPI].ga_adaAdapter[iAdapter].da_strRenderer;

      // Make sure it's not a duplicate
      BOOL bDuplicate = FALSE;
      const INDEX ctAdaptersInList = pvs->vs_astrTexts.Count();

      for (INDEX iCheckAdapter = 0; iCheckAdapter < ctAdaptersInList; iCheckAdapter++) {
        if (pvs->vs_astrTexts[iCheckAdapter] == strAdapter) {
          bDuplicate = TRUE;
          break;
        }
      }

      if (!bDuplicate) {
        pvs->vs_astrTexts.Push() = strAdapter;
        pvs->vs_astrValues.Push() = strLine;
      }

      // Filter out the option if there's only one adapter
      if (pvs->vs_astrTexts.Count() <= 1) {
        pvs->vs_strFilter = "FALSE";
      } else {
        pvs->vs_strFilter.Clear();
      }
    }

  } else {
    ThrowF_t(TRANS("AutoValue: Unknown variable '%s'"), strVar.str_String);
  }
};

// [Cecil] All settings list argument
static void ParseCFG_t(CTStream &strm, CListHead &lhAll) {
  CVarSetting *pvs = NULL;

  // [Cecil] Skip everything until the next gadget
  BOOL bSkipUntilNext = FALSE;

  // [Cecil] Clear last resolutions
  _aScreenResolutions.PopAll();

  // repeat
  FOREVER {
    // read one line
    CTString strLine = GetNonEmptyLine_t(strm);

    // Stop parsing
    if (strLine.RemovePrefix("MenuEnd")) break;

    // [Cecil] Keep skipping until a new gadget
    if (bSkipUntilNext && !strLine.HasPrefix("Gadget:")) {
      continue;
    }

    if (strLine.RemovePrefix("Gadget:")) {
      pvs = new CVarSetting;
      lhAll.AddTail(pvs->vs_lnNode);

      TranslateLine(strLine);
      strLine.TrimSpacesLeft();
      pvs->vs_strName = strLine;

      // [Cecil] Parse this gadget
      bSkipUntilNext = FALSE;

      _aScreenResolutions.PopAll();

    } else if (strLine.RemovePrefix("Type:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // [Cecil] Match type name with a type number
      if (strLine == "Toggle") {
        pvs->vs_eType = CVarSetting::E_TOGGLE;

      } else if (strLine == "Separator") {
        pvs->vs_eType = CVarSetting::E_SEPARATOR;

      } else if (strLine == "Textbox") {
        pvs->vs_eType = CVarSetting::E_TEXTBOX;

      } else if (strLine == "Button") {
        pvs->vs_eType = CVarSetting::E_BUTTON;
      }

    } else if (strLine.RemovePrefix("Schedule:")) {
      CheckPVS_t(pvs);
      FixupFileName_t(strLine);

      // [Cecil] This variable is now used as a shell command, so schedule inclusion of a script
      pvs->vs_strSchedule.PrintF("include \"%s\";", strLine);

    // [Cecil] Schedule an inline command instead of a script
    } else if (strLine.RemovePrefix("Command:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();
      pvs->vs_strSchedule = strLine;

    } else if (strLine.RemovePrefix("Tip:")) {
      CheckPVS_t(pvs);
      TranslateLine(strLine);

      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();
      pvs->vs_strTip = strLine;

    } else if (strLine.RemovePrefix("Var:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // [Cecil] Replace old wide screen command
      if (strLine == "sam_bWideScreen") {
        pvs->vs_strVar = "sam_bAdjustForAspectRatio";
      } else {
        pvs->vs_strVar = strLine;
      }

    } else if (strLine.RemovePrefix("Filter:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();
      pvs->vs_strFilter = strLine;

    } else if (strLine.RemovePrefix("Slider:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      if (strLine == "Fill") {
        pvs->vs_eSlider = CVarSetting::SLD_FILL;

      } else if (strLine == "Ratio") {
        pvs->vs_eSlider = CVarSetting::SLD_RATIO;

      // [Cecil] Big fill bar
      } else if (strLine == "BigFill") {
        pvs->vs_eSlider = CVarSetting::SLD_BIGFILL;

      } else {
        pvs->vs_eSlider = CVarSetting::SLD_NOSLIDER;
      }

    } else if (strLine.RemovePrefix("InGame:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      if (strLine == "No") {
        pvs->vs_bCanChangeInGame = FALSE;
      } else {
        ASSERT(strLine == "Yes");
        pvs->vs_bCanChangeInGame = TRUE;
      }

    // [Cecil] Hide value string
    } else if (strLine.RemovePrefix("Hidden:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      if (strLine == "No") {
        pvs->vs_bHidden = FALSE;
      } else {
        ASSERT(strLine == "Yes");
        pvs->vs_bHidden = TRUE;
      }

    // [Cecil] Apply values in real time
    } else if (strLine.RemovePrefix("RealTime:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      if (strLine == "No") {
        pvs->vs_bRealTime = FALSE;
      } else {
        ASSERT(strLine == "Yes");
        pvs->vs_bRealTime = TRUE;
      }

    // [Cecil] Skip adding this option if it doesn't belong to the current game
    } else if (strLine.RemovePrefix("Games:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // Discard the setting if the string doesn't contain the current game anywhere
      if (strLine.FindSubstr(CHOOSE_FOR_GAME("TFE105", "TSE105", "TSE107")) == -1) {
        bSkipUntilNext = TRUE;
        pvs->vs_lnNode.Remove();

        delete pvs;
        pvs = NULL;
      }

    // [Cecil] Skip adding this option if it doesn't belong to the current mod
    } else if (strLine.RemovePrefix("Mods:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // Invert the condition at the end (belongs to any mod that isn't listed)
      const BOOL bInvert = strLine.RemovePrefix("!");

      // Separate the line into a list of mods
      CStaticStackArray<CTString> astrMods;
      IData::GetStrings(astrMods, strLine, ' ');

      // Check current mod out of available mods for the setting
      const INDEX ctMods = astrMods.Count();
      BOOL bSkip = TRUE;

      // Get mod directory name by removing last slash and "Mods\\"
      CTString strCurrentMod = "";

      if (_fnmMod != "") {
        strCurrentMod = _fnmMod;
        strCurrentMod.str_String[strCurrentMod.Length() - 1] = '\0';
        strCurrentMod.RemovePrefix("Mods\\");
      }

      for (INDEX i = 0; i < ctMods; i++) {
        const CTString &strMod = astrMods[i];

        // Special condition for vanilla game
        if (strMod == "/NoMod/") {
          // Not even a custom mod
          if (_fnmMod == "" && !ClassicsCore_IsCustomModActive()) {
            bSkip = FALSE;
            break;
          }

      #if _PATCHCONFIG_CUSTOM_MOD
        // Special condition for the custom mod
        } else if (strMod == "ClassicsPatchMod") {
          // All is fine
          if (ClassicsCore_IsCustomModActive()) {
            bSkip = FALSE;
            break;
          }
      #endif

        // Current mod matches one of the listed mods
        } else if (strMod == strCurrentMod) {
          bSkip = FALSE;
          break;
        }
      }

      if (bInvert) bSkip = !bSkip;

      // Discard the setting
      if (bSkip) {
        bSkipUntilNext = TRUE;
        pvs->vs_lnNode.Remove();

        delete pvs;
        pvs = NULL;
      }

    // [Cecil] List files under a specific directory as value options
    } else if (strLine.RemovePrefix("List:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      // Get directory and the filename pattern from the path
      CTFileName fnmListPath = strLine;
      CTString strPattern = fnmListPath.FileName() + fnmListPath.FileExt();

      CFileList afnmDir;
      ListGameFiles(afnmDir, fnmListPath.FileDir(), strPattern, FLF_SEARCHMOD);

      // Go through listed files
      const INDEX ct = afnmDir.Count();

      for (INDEX i = 0; i < ct; i++) {
        const CTFileName &fnm = afnmDir[i];

        // Set text to the filename by default
        CTString &strText = pvs->vs_astrTexts.Push();

        try {
          // Try loading text from the description file nearby
          strText.Load_t(fnm.NoExt() + ".des");

        } catch (char *strError) {
          // Just set text to the filename
          (void)strError;
          strText = fnm.FileName();
        }

        // Set value to the filename
        pvs->vs_astrValues.Push() = fnm;
      }

    } else if (strLine.RemovePrefix("String:")) {
      CheckPVS_t(pvs);
      TranslateLine(strLine);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();
      pvs->vs_astrTexts.Push() = strLine;

    } else if (strLine.RemovePrefix("Value:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();
      pvs->vs_astrValues.Push() = strLine;

    // [Cecil] Special property that automatically adds whichever names and values it needs
    } else if (strLine.RemovePrefix("AutoValue:")) {
      CheckPVS_t(pvs);
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      ParseAutoValue(pvs, strLine);

    // [Cecil] Include another config in the middle of this one
    } else if (strLine.RemovePrefix("Include:")) {
      FixupFileName_t(strLine);

      // Save it for the button gadget
      if (pvs->vs_eType == CVarSetting::E_BUTTON) {
        pvs->vs_strSchedule = strLine;

      // Include contents of it directly
      } else {
        CTFileStream strmInclude;
        strmInclude.Open_t(strLine);

        FOREACHINDYNAMICCONTAINER(_cOpenedConfigs, CTFileStream, itstrm) {
          // This config is already opened
          if (itstrm->GetDescription() == strmInclude.GetDescription()) {
            ThrowF_t(TRANS("Config '%s' is included recursively"), strLine.str_String);
          }
        }

        // Start new lines and file
        const INDEX ctRestore = _ctLines;
        const CTString strRestore = _strFile;
        _ctLines = 0;
        _strFile = strLine;

        // Parse included config
        ParseCFG_t(strmInclude, lhAll);

        // Restore lines and file
        _ctLines = ctRestore;
        _strFile = strRestore;
      }

    } else {
      ThrowF_t(LOCALIZE("unknown keyword"));
    }
  }
}

// [Cecil] Start parsing the option config
static void StartConfigParsing(CTStream &strm) {
  // Create tab with all options
  CVarTab &tabAll = _aTabs.Push();
  tabAll.strName = TRANS("All options");

  CListHead &lhAll = tabAll.lhVars;

  // Parse the config
  ParseCFG_t(strm, lhAll);

  // Current tab
  CVarTab *pCur = NULL;
  BOOL bOnlySeparators = FALSE;

  // Go through each setting
  {FOREACHINLIST(CVarSetting, vs_lnNode, lhAll, itvs) {
    CVarSetting &vs = *itvs;

    // It's a separator
    if (vs.vs_eType == CVarSetting::E_SEPARATOR)
    {
      // It has a name and there are different settings under the current tab
      if (!bOnlySeparators && vs.vs_strName != "") {
        // Start a new tab
        pCur = &_aTabs.Push();
        pCur->strName = vs.vs_strName;

        // Reset separators state
        bOnlySeparators = TRUE;
      }

    // Not a separator
    } else {
      bOnlySeparators = FALSE;
    }

    // Copy setting into the new tab
    if (pCur != NULL) {
      CVarSetting *pvsCopy = new CVarSetting(vs);
      pCur->lhVars.AddTail(pvsCopy->vs_lnNode);
    }
  }}

  // Remove the last tab if it only consists of separators
  if (bOnlySeparators) {
    CListHead &lhLast = _aTabs[_aTabs.Count() - 1].lhVars;

    {FORDELETELIST(CVarSetting, vs_lnNode, lhLast, itvs) {
      delete &*itvs;
    }}

    _aTabs.Pop();
  }
};

void LoadVarSettings(const CTFileName &fnmCfg) {
  FlushVarSettings(FALSE);

  CTFileStream strm;

  try {
    strm.Open_t(fnmCfg);
  } catch (char *strError) {
    FatalError("%s (%d) : %s", _strFile.str_String, _ctLines, strError);
  }

  _ctLines = 0;
  _strFile = fnmCfg;

  // [Cecil] Start parsing the option config
  _cOpenedConfigs.Clear();
  _cOpenedConfigs.Add(&strm);

  try {
    StartConfigParsing(strm);
  } catch (char *strError) {
    CPrintF("%s (%d) : %s\n", _strFile.str_String, _ctLines, strError);
  }

  // [Cecil] For each tab
  for (INDEX iTab = 0; iTab < _aTabs.Count(); iTab++)
  {
    FOREACHINLIST(CVarSetting, vs_lnNode, _aTabs[iTab].lhVars, itvs) {
      CVarSetting &vs = *itvs;

      // Skip if couldn't validate or it's a separator or a button
      if (!vs.Validate() || vs.vs_eType == CVarSetting::E_SEPARATOR || vs.vs_eType == CVarSetting::E_BUTTON) {
        continue;
      }

      vs.UpdateValue(); // [Cecil]
    }
  }
}

void FlushVarSettings(BOOL bApply) {
  CStringStack astrScheduled;

  // [Cecil] For each tab
  for (INDEX iTab = 0; iTab < _aTabs.Count(); iTab++)
  {
    if (bApply) {
      FOREACHINLIST(CVarSetting, vs_lnNode, _aTabs[iTab].lhVars, itvs) {
        CVarSetting &vs = *itvs;
        const CTString &strSchedule = vs.vs_strSchedule;

        // [Cecil] Apply the value and see if it can schedule a command
        BOOL bScheduled = vs.ApplyValue();
        if (!bScheduled || strSchedule == "") continue;

        // Check if it's not scheduled yet
        bScheduled = FALSE;

        for (INDEX i = 0; i < astrScheduled.Count(); i++) {
          if (astrScheduled[i] == strSchedule) {
            bScheduled = TRUE;
            break;
          }
        }

        // Schedule the command to execute afterwards
        if (!bScheduled) astrScheduled.Add(strSchedule);
      }
    }

    {FORDELETELIST(CVarSetting, vs_lnNode, _aTabs[iTab].lhVars, itvs) {
      delete &*itvs;
    }}
  }

  // [Cecil] Clear tabs
  _aTabs.PopAll();

  for (INDEX i = 0; i < astrScheduled.Count(); i++) {
    _pShell->Execute(astrScheduled[i]);
  }
}

CVarSetting::CVarSetting() {
  Clear();
}

void CVarSetting::Clear() {
  vs_iOrgValue = 0;
  vs_iValue = 0;
  vs_strValue.Clear(); // [Cecil]
  vs_ctValues = 0;
  vs_eType = E_TOGGLE; // [Cecil] Toggleable type by default
  vs_bCanChangeInGame = TRUE;
  vs_eSlider = SLD_NOSLIDER;
  vs_strName.Clear();
  vs_strTip.Clear();
  vs_strVar.Clear();
  vs_strFilter.Clear();
  vs_strSchedule.Clear();
  vs_bCustom = FALSE;
  vs_bHidden = FALSE; // [Cecil]
  vs_bRealTime = FALSE; // [Cecil]
}

BOOL CVarSetting::Validate(void) {
  // [Cecil] Specific types don't need validation
  switch (vs_eType) {
    case E_SEPARATOR:
    case E_TEXTBOX:
    case E_BUTTON:
      return TRUE;
  }

  vs_ctValues = Min(vs_astrValues.Count(), vs_astrTexts.Count());

  if (vs_ctValues <= 0) {
    ASSERT(FALSE);
    return FALSE;
  }

  if (!vs_bCustom) {
    vs_iValue = Clamp(vs_iValue, 0L, vs_ctValues - 1L);
  }

  return TRUE;
}

// [Cecil] Copy constructor
CVarSetting::CVarSetting(const CVarSetting &vsOther) {
  if (&vsOther == this) return;

  vs_eType            = vsOther.vs_eType;
  vs_bCanChangeInGame = vsOther.vs_bCanChangeInGame;
  vs_eSlider          = vsOther.vs_eSlider;
  vs_strName          = vsOther.vs_strName;
  vs_strTip           = vsOther.vs_strTip;
  vs_strVar           = vsOther.vs_strVar;
  vs_strFilter        = vsOther.vs_strFilter;
  vs_strSchedule      = vsOther.vs_strSchedule;
  vs_iValue           = vsOther.vs_iValue;
  vs_strValue         = vsOther.vs_strValue;
  vs_ctValues         = vsOther.vs_ctValues;
  vs_iOrgValue        = vsOther.vs_iOrgValue;
  vs_bCustom          = vsOther.vs_bCustom;
  vs_bHidden          = vsOther.vs_bHidden;
  vs_bRealTime        = vsOther.vs_bRealTime;

  INDEX ct;
  CTString *pstr;

  // Copy texts
  ct = vsOther.vs_astrTexts.Count();

  if (ct > 0) {
    pstr = vs_astrTexts.Push(ct);

    while (--ct >= 0) {
      pstr[ct] = vsOther.vs_astrTexts[ct];
    }
  }

  // Copy values
  ct = vsOther.vs_astrValues.Count();

  if (ct > 0) {
    pstr = vs_astrValues.Push(ct);

    while (--ct >= 0) {
      pstr[ct] = vsOther.vs_astrValues[ct];
    }
  }
};

// [Cecil] Immediately apply new value to this setting (taken out of FlushVarSettings() method)
BOOL CVarSetting::ApplyValue(void) {
  // New value to set
  CTString strNewValue = "";
  BOOL bNewValue = FALSE;

  switch (vs_eType)
  {
    case CVarSetting::E_TOGGLE: {
      // If selected some other value
      if (vs_iValue != vs_iOrgValue) {
        // Set to a new value from the list
        strNewValue = vs_astrValues[vs_iValue];
        bNewValue = TRUE;
      }
    } break;

    case CVarSetting::E_TEXTBOX: {
      // If typed in a different string
      const ULONG ulOldHash = static_cast<ULONG>(vs_iOrgValue);

      if (vs_strValue.GetHash() != ulOldHash) {
        // Set to a new string
        strNewValue = vs_strValue;
        bNewValue = TRUE;
      }
    } break;
  }

  // No new value has been set
  if (!bNewValue) return FALSE;

  // Retrieve shell symbol
  CShellSymbol *pss = _pShell->GetSymbol(vs_strVar, TRUE);

  // Execute pre-function to see if allowed to set new value
  if (pss != NULL && pss->ss_pPreFunc != NULL) {
    if (!pss->ss_pPreFunc(pss->ss_pvValue)) {
      // Not allowed
      return FALSE;
    }
  }

  // Set shell symbol to it
  _pShell->SetValue(vs_strVar, strNewValue);

  // Execute post-function
  if (pss != NULL && pss->ss_pPostFunc != NULL) {
    pss->ss_pPostFunc(pss->ss_pvValue);
  }

  // Schedule a command for execution afterwards
  return TRUE;
};

// [Cecil] Update original value after applying it with ApplyValue() (taken out of LoadVarSettings() method)
void CVarSetting::UpdateValue(void) {
  // Get value from the shell variable
  CTString strValue = _pShell->GetValue(vs_strVar);

  // [Cecil] Different types
  switch (vs_eType)
  {
    case CVarSetting::E_TOGGLE: {
      const INDEX ctValues = vs_ctValues;

      // Custom value by default
      vs_bCustom = TRUE;
      vs_iOrgValue = -1;
      vs_iValue = -1;

      // Search for the same value in the value list
      for (INDEX iValue = 0; iValue < ctValues; iValue++) {
        // If it matches this value
        if (strValue == vs_astrValues[iValue]) {
          // Set index to the value in the list
          vs_iOrgValue = vs_iValue = iValue;
          vs_bCustom = FALSE;
          break;
        }
      }
    } break;

    case CVarSetting::E_TEXTBOX: {
      // Set new string
      vs_strValue = strValue;

      // Save hash value of the string
      vs_iOrgValue = static_cast<ULONG>(strValue.GetHash());
    } break;
  }
};
