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

#include "Resource.h"
#include "DlgRecursiveFileBrowser.h"

#ifdef _DEBUG
  #undef new
  #define new DEBUG_NEW

  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

// CDlgRecursiveFileBrowser dialog

CDlgRecursiveFileBrowser::CDlgRecursiveFileBrowser(CWnd *pParent)
  : CDialog(CDlgRecursiveFileBrowser::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDlgRecursiveFileBrowser)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void CDlgRecursiveFileBrowser::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDlgRecursiveFileBrowser)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

void CDlgRecursiveFileBrowser::Update(void)
{
  UpdateData(FALSE);
}

void CDlgRecursiveFileBrowser::SetMessage(INDEX iControlID, const char *strFormat, ...)
{
  CWnd *pCtrl = GetDlgItem(iControlID);
  ASSERT(pCtrl != NULL);

  va_list arg;
  va_start(arg, strFormat);

  CTString strMessage;
  strMessage.VPrintF(strFormat, arg);

  CString strText = strMessage.str_String;
  pCtrl->SetWindowText(strText);
}

BEGIN_MESSAGE_MAP(CDlgRecursiveFileBrowser, CDialog)
  //{{AFX_MSG_MAP(CDlgRecursiveFileBrowser)
  ON_WM_CLOSE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRecursiveFileBrowser message handlers

void CDlgRecursiveFileBrowser::OnClose() 
{
  // Cancel conversion
  extern BOOL _bCanceled;
  _bCanceled = TRUE;
  CDialog::OnClose();
}
