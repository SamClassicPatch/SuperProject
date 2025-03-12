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

#include "DlgRecursiveFileBrowser.h"

static void (*_pFileFoundCallback)(CTFileName fnFound) = NULL;
static HWND _hDlgWndFileBrowser = NULL;
static CDlgRecursiveFileBrowser *_pdlgFileBrowser = NULL;
static CTimerValue _tvConvertStartAt;
extern BOOL _bCanceled = FALSE;
static BOOL _bUpdateGUI = FALSE;
static INDEX _ctFilesTotal = 0;
static INDEX _ctFilesFound = 0;
static CTString _strFileMask = "*";
static CStaticStackArray<CTString> _strFileExt;

static void UpdateGui(const CTFileName &fnFileFound)
{
  ASSERT(fnFileFound.Length() > 0);
  ASSERT(_hDlgWndFileBrowser != NULL);

  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
  FLOAT fElapsed = (tvNow - _tvConvertStartAt).GetSeconds();
  FLOAT fEstimated = ((fElapsed / _ctFilesFound) * (_ctFilesTotal - _ctFilesFound));

  _pdlgFileBrowser->SetMessage(IDC_FILE_BEING_PROCESSED, fnFileFound.str_String);

  CTString strElapsedTime;
  CTString strEstimatedTime;
  
  if (fElapsed >= 60.0f) {
    strElapsedTime.PrintF("%g min", fElapsed / 60.0f);
  } else {
    strElapsedTime.PrintF("%g sec", fElapsed);
  }

  if (fEstimated >= 60.0f) {
    strEstimatedTime.PrintF("%g min", fEstimated / 60.0f);
  } else {
    strEstimatedTime.PrintF("%g sec", fEstimated);
  }

  _pdlgFileBrowser->SetMessage(IDC_ELAPSED_TIME, "Elapsed: %s", strElapsedTime.str_String);
  _pdlgFileBrowser->SetMessage(IDC_ESTIMATED_TIME, "Estimated: %s", strEstimatedTime.str_String);
  _pdlgFileBrowser->SetMessage(IDC_PROGRESS_COUNT, "%d / %d", _ctFilesFound, _ctFilesTotal);

  MSG msg;

  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if (msg.hwnd == _hDlgWndFileBrowser) {
      if (!TranslateMessage(&msg)) {
        DispatchMessage(&msg);
      }

    } else {
      // if it is not a mouse message
      if (!(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)) {
        // if not system key messages
        if (!(msg.message == WM_KEYDOWN && msg.wParam == VK_F10 || msg.message == WM_SYSKEYDOWN)) {
          // dispatch it
          TranslateMessage(&msg);
        }

        // if paint message
        if (msg.message == WM_PAINT) {
          // dispatch it
          DispatchMessage(&msg);
        }
      }
    }
  }
}

static void RecursiveListFiles(const CTFileName &fnBrowseDir)
{
  // bail out if canceled
  if (_bCanceled) {
    return;
  }

  // Browse all directories in current directory
  CTString strDirMask = IDir::AppPath() + fnBrowseDir + "*";

  // find first file
  WIN32_FIND_DATAA wfdDirData;
  HANDLE hDirHandle = FindFirstFileA(strDirMask, &wfdDirData);

  // if at least one file exists
  if (hDirHandle != INVALID_HANDLE_VALUE)
  {
    while (TRUE) {
      // bail out if canceled
      if (_bCanceled) {
        return;
      }

      // if next file found is directory
      if (wfdDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        CTString strDirName = wfdDirData.cFileName;

        // if this is valid directory
        if (strDirName != "." && strDirName != "..") {
          // Update gui if required
          if(_bUpdateGUI) {
            UpdateGui(fnBrowseDir + strDirName);
          }

          // browse it
          RecursiveListFiles(fnBrowseDir + strDirName + "\\");
        }
      }

      // if next file isn't found 
      if (!FindNextFileA(hDirHandle, &wfdDirData)) {
        // exit loop
        break;
      }
    }

    // Close handle
    FindClose(hDirHandle);
  }

  // Browse all files in current directory
  CTString strFileMask = IDir::AppPath() + fnBrowseDir + "*";

  WIN32_FIND_DATAA wfdFileData;
  HANDLE hFileHandle = FindFirstFileA(strFileMask, &wfdFileData);

  // if at least one file exists
  if (hFileHandle != INVALID_HANDLE_VALUE)
  {
    while (TRUE) {
      // bail out if canceled
      if (_bCanceled) {
        return;
      }

      // if next file found is not directory
      if (!(wfdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        CTFileName fnFileFound = CTString(wfdFileData.cFileName);
        const INDEX ctExtensions = _strFileExt.Count();

        for (INDEX iExt = 0; iExt < ctExtensions; iExt++) {
          CTString strFileExt = _strFileExt[iExt];

          if (strFileExt == ".*" || strFileExt == fnFileFound.FileExt()) {
            // Update gui if required
            if (_bUpdateGUI) {
              ((CProgressCtrl *)_pdlgFileBrowser->GetDlgItem(IDC_PROGRESS_BAR))->StepIt();
              UpdateGui(fnBrowseDir + fnFileFound);
            }

            // if callback function is valid
            if (_pFileFoundCallback != NULL) {
              // use it
              CTString fnFullFileName = fnBrowseDir + fnFileFound;
              _pFileFoundCallback(fnFullFileName);
            }

            // increment file count
            _ctFilesFound++;
          }
        }
      }

      // if next file isn't found 
      if (!FindNextFileA(hFileHandle, &wfdFileData)) {
        // exit loop
        break;
      }
    }

    // Close handle
    FindClose(hFileHandle);
  }
}

// Recursivly browse all files and returns them through callback function
ENGINEGUI_API FLOAT CEngineGUI::RecursiveFileBrowser(CTFileName fnRootDirectory, char *strWindowTitle,
                                                     void (*pFileFoundCallback)(CTFileName fnFound),
                                                     CTString strFileMask, BOOL bUseGUI)
{
  ASSERT(_pFileFoundCallback == NULL); // must not be used
  ASSERT(_hDlgWndFileBrowser == NULL);
  ASSERT(_pdlgFileBrowser == NULL);
  ASSERT(strFileMask.Length() > 0);
  ASSERT(pFileFoundCallback != NULL);
  ASSERT(strWindowTitle != NULL);

  // last char must be backslash
  if (fnRootDirectory.Length() > 0 && fnRootDirectory[fnRootDirectory.Length() - 1] != '\\') {
    fnRootDirectory += "\\";
  }

  _strFileExt.PopAll(); // Clear extesion masks

  // extract file extensions from extension list
  char strTmpExt[256] = "";
  char *strExtList = strFileMask.str_String;

  while (TRUE) {
    char *strExt = strchr(strExtList, ';');

    // if terminator was found 
    if (strExt != NULL) {
      INDEX iExtLength = strExt - strExtList;

      strncpy(strTmpExt, strExtList + 1, iExtLength - 1); // skip *
      strTmpExt[iExtLength - 1] = 0; // terminate extension string

      CTString &strNewExt = _strFileExt.Push();
      strNewExt = strTmpExt;
      strExtList = ++strExt;

    } else {
      CTString &strNewExt = _strFileExt.Push();
      strNewExt = ++strExtList; // add reast of list as new extension (skip *)
      break; // done
    }
  }

  _ctFilesFound = 0;
  _bCanceled = FALSE;
  _bUpdateGUI = FALSE;
  _strFileMask = strFileMask;

  // Cound files in directories
  RecursiveListFiles(fnRootDirectory);
  _ctFilesTotal = _ctFilesFound;
  _ctFilesFound = 0;

  CDlgRecursiveFileBrowser dlgFileBrowser;

  if (bUseGUI) {
    // stupid way to change resources, but it must be done
    HANDLE hOldResource = AfxGetResourceHandle();
    AfxSetResourceHandle(GetGuiHandle());

    dlgFileBrowser.Create(IDD_RECURSIVE_FILE_BROWSER, NULL);
    dlgFileBrowser.SetWindowText(CString(strWindowTitle));
    dlgFileBrowser.ShowWindow(SW_SHOW);
    dlgFileBrowser.CenterWindow();
    dlgFileBrowser.Update();

    ((CProgressCtrl *)dlgFileBrowser.GetDlgItem(IDC_PROGRESS_BAR))->SetStep(1);
    ((CProgressCtrl *)dlgFileBrowser.GetDlgItem(IDC_PROGRESS_BAR))->SetPos(0);
    ((CProgressCtrl *)dlgFileBrowser.GetDlgItem(IDC_PROGRESS_BAR))->SetRange32(0, _ctFilesTotal);

    _pdlgFileBrowser = &dlgFileBrowser;
    _hDlgWndFileBrowser = dlgFileBrowser.m_hWnd;
    _bUpdateGUI = TRUE;

    // Restore resources
    AfxSetResourceHandle((HINSTANCE)hOldResource);
  }

  _tvConvertStartAt = _pTimer->GetHighPrecisionTimer();

  // Do search one again but use gui and calback function
  _pFileFoundCallback = pFileFoundCallback;
  RecursiveListFiles(fnRootDirectory);

  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
  FLOAT fElapsed = (tvNow - _tvConvertStartAt).GetSeconds();

  if (_bCanceled) {
    fElapsed = -1.0f;
  }

  // Reset stuff for next call
  _strFileMask = "*";
  _pFileFoundCallback = NULL;
  _hDlgWndFileBrowser = NULL;
  _pdlgFileBrowser = NULL;

  return fElapsed;
}
