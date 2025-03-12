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

#include <shlobj.h>

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
  char strPath[MAX_PATH];

  switch (uMsg)
  {
    case BFFM_INITIALIZED: {
      SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pData);
    } break;

    case BFFM_SELCHANGED: {
      // enable or disable ok button if current dir is inside app path
      if (SHGetPathFromIDListA((LPITEMIDLIST)lp, strPath)) {
        CTFileName fnSelectedDir = CTString(strPath) + CTString("\\");

        try {
          fnSelectedDir.RemoveApplicationPath_t();
          SendMessage(hwnd, BFFM_ENABLEOK, 0, TRUE);

        } catch (char *) {
          SendMessage(hwnd, BFFM_ENABLEOK, 0, FALSE);
        }
      }
    } break;
  }

  return 0;
}

// Folder browse
ENGINEGUI_API BOOL CEngineGUI::BrowseForFolder(CTFileName &fnBrowsedFolder, CTString strDefaultDir, char *strWindowTitle)
{
  CTString strFullRootDir = IDir::AppPath() + strDefaultDir;

  BROWSEINFOA biBrowse;
  memset(&biBrowse, 0, sizeof(BROWSEINFO));

  biBrowse.hwndOwner = AfxGetMainWnd()->m_hWnd;
  biBrowse.pidlRoot = NULL;
  biBrowse.pszDisplayName = NULL;
  biBrowse.lpszTitle = strWindowTitle;
  biBrowse.ulFlags = BIF_RETURNFSANCESTORS;
  biBrowse.lpfn = BrowseCallbackProc;
  biBrowse.lParam = (LPARAM)strFullRootDir.str_String;

  // From msdn
  ITEMIDLIST *pidl = SHBrowseForFolderA(&biBrowse);

  if (pidl != NULL) {
    char strPath[MAX_PATH];
    SHGetPathFromIDListA(pidl, strPath);

    IMalloc *pm;
    SHGetMalloc(&pm);

    if (pm != NULL) {
      pm->Free(pidl);
      pm->Release();
    }

    CTFileName fnFullBrowsedDir = CTString(strPath) + CTString("\\");

    try {
      fnFullBrowsedDir.RemoveApplicationPath_t();
      fnBrowsedFolder = fnFullBrowsedDir;
      return TRUE;

    } catch(char *strErr) {
      WarningMessage(strErr);
      return FALSE;
    }
  }

  return FALSE;
}
