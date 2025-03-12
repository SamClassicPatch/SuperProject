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

#include "GUI/Menus/MenuStuff.h"

// List of settings data
static CListHead _lhSettings;
extern INDEX sam_iVideoSetup;

class CSettingsEntry {
  public:
    CListNode se_lnNode;
    CTString se_strRenderer;
    CTString se_strDescription;
    CTFileName se_fnmScript;

    // Check if this entry matches given info
    BOOL Matches(const CTString &strRenderer) const;
};

// Last valid settings info
static CTString _strLastRenderer;
extern CTString _strPreferencesDescription = "";
extern INDEX _iLastPreferences = 1;

// Check if this entry matches given info
BOOL CSettingsEntry::Matches(const CTString &strRenderer) const {
  return strRenderer.Matches(se_strRenderer);
}

const char *RenderingPreferencesDescription(int iMode) {
  ASSERT(iMode >= 0 && iMode <= _iDisplayPrefsLastOpt);

  if (iMode < 0 || iMode > _iDisplayPrefsLastOpt) {
    iMode = 0;
  }

  // [Cecil] Reuse radio array
  return astrDisplayPrefsRadioTexts[iMode];
}

void InitGLSettings(void) {
  ASSERT(_lhSettings.IsEmpty());

  char achrLine[1024];
  char achrRenderer[1024];
  char achrDesc[1024];
  char achrScript[1024];

  CTFileStream strmFile;

  try {
    strmFile.Open_t(CTString("Scripts\\GLSettings\\GLSettings.lst"), CTStream::OM_READ);
    INDEX iIndex = 0;

    do {
      achrLine[0] = 0;
      achrRenderer[0] = 0;
      achrDesc[0] = 0;
      achrScript[0] = 0;

      strmFile.GetLine_t(achrLine, 1024);
      sscanf(achrLine, "\"%1024[^\"]\"%*[^\"]\"%1024[^\"]\"%*[^\"]\"%1024[^\"]\"", achrRenderer, achrDesc, achrScript);

      if (achrRenderer[0] == 0) {
        continue;
      }

      CSettingsEntry &se = *new CSettingsEntry;
      se.se_strRenderer = achrRenderer;
      se.se_strDescription = achrDesc;
      se.se_fnmScript = CTString(achrScript);

      _lhSettings.AddTail(se.se_lnNode);

    } while (!strmFile.AtEOF());

  // ignore errors
  } catch (char *strError) {
    WarningMessage(LOCALIZE("unable to setup OpenGL settings list: %s"), strError);
  }

  _strLastRenderer = "none";
  _iLastPreferences = 1;

  _pShell->DeclareSymbol("persistent CTString sam_strLastRenderer;", &_strLastRenderer);
  _pShell->DeclareSymbol("persistent INDEX    sam_iLastSetup;", &_iLastPreferences);
}

CSettingsEntry *GetGLSettings(const CTString &strRenderer) {
  // for each setting
  FOREACHINLIST(CSettingsEntry, se_lnNode, _lhSettings, itse) {
    // return the one that matches
    CSettingsEntry &se = *itse;

    if (se.Matches(strRenderer)) {
      return &se;
    }
  }

  // none found
  return NULL;
}

extern void ApplyGLSettings(BOOL bForce) {
  CPrintF(LOCALIZE("\nAutomatic 3D-board preferences adjustment...\n"));

  CDisplayAdapter &da = _pGfx->gl_gaAPI[_pGfx->gl_eCurrentAPI].ga_adaAdapter[_pGfx->gl_iCurrentAdapter];
  CPrintF(LOCALIZE("Detected: %s - %s - %s\n"), da.da_strVendor, da.da_strRenderer, da.da_strVersion);

  // get new settings
  CSettingsEntry *pse = GetGLSettings(da.da_strRenderer);

  // if none found
  if (pse == NULL) {
    // error
    CPrintF(LOCALIZE("No matching preferences found! Automatic adjustment disabled!\n"));
    return;
  }

  // report
  CPrintF(LOCALIZE("Matching: %s (%s)\n"), pse->se_strRenderer, pse->se_strDescription);
  _strPreferencesDescription = pse->se_strDescription;

  if (!bForce) {
    // if same as last
    if (pse->se_strDescription == _strLastRenderer && sam_iVideoSetup == _iLastPreferences) {
      // do nothing
      CPrintF(LOCALIZE("Similar to last, keeping same preferences.\n"));
      return;
    }

    CPrintF(LOCALIZE("Different than last, applying new preferences.\n"));

  } else {
    CPrintF(LOCALIZE("Applying new preferences.\n"));
  }

  // clamp rendering preferences (just to be on the safe side)
  sam_iVideoSetup = Clamp(sam_iVideoSetup, 0L, _iDisplayPrefsLastOpt);
  CPrintF(LOCALIZE("Mode: %s\n"), RenderingPreferencesDescription(sam_iVideoSetup));

  // if not in custom mode
  if (sam_iVideoSetup < 3) {
    // execute the script
    CTString strCmd;
    strCmd.PrintF("include \"Scripts\\GLSettings\\%s\"", pse->se_fnmScript.str_String);
    _pShell->Execute(strCmd);

    // refresh textures
    _pShell->Execute("RefreshTextures();");

  // [Cecil] Extreme quality
  } else if (sam_iVideoSetup == 3) {
    _pShell->Execute("include \"Scripts\\Addons\\Extreme.ini\"");
    _pShell->Execute("mdl_iLODDisappear = 1;"); // Fix for non-disappearing items
  }

  // done
  CPrintF(LOCALIZE("Done.\n\n"));

  // remember settings
  _strLastRenderer = pse->se_strDescription;
  _iLastPreferences = sam_iVideoSetup;
}
