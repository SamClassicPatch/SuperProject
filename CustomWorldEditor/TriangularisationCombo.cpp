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

// TriangularisationCombo.cpp : implementation file
//

#include "StdH.h"

#ifdef _DEBUG
#undef new
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTriangularisationCombo

CTriangularisationCombo::CTriangularisationCombo()
{
}

CTriangularisationCombo::~CTriangularisationCombo()
{
}


BEGIN_MESSAGE_MAP(CTriangularisationCombo, CComboBox)
  //{{AFX_MSG_MAP(CTriangularisationCombo)
  ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
  ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTriangularisationCombo message handlers

BOOL CTriangularisationCombo::OnIdle(LONG lCount)
{
  // get document ptr
  CWorldEditorDoc* pDoc = theApp.GetActiveDocument();

  if( (pDoc == NULL) ||
      (pDoc->GetEditingMode() != CSG_MODE) ||
      (!pDoc->m_bPrimitiveMode) )
  {
    // we should disable triangularisation combo
    if(GetCount() == 1) return TRUE; // we allready have "not available" in combo
    // remove all combo entries
    ResetContent();
    // set none available message
    AddString( _T("None Available"));
    SetCurSel( 0);
  }
  // we should enable triangularisation combo 
  else
  {
    // if it is allready enabled
    if(GetCount() != 1) return TRUE;
    ResetContent();
    // add all possible triangularisation types
    AddString( _T("None"));
    AddString( _T("Center"));
    AddString( _T("Vertex 1"));
    AddString( _T("Vertex 2"));
    AddString( _T("Vertex 3"));
    AddString( _T("Vertex 4"));
    AddString( _T("Vertex 5"));
    AddString( _T("Vertex 6"));
    AddString( _T("Vertex 7"));
    AddString( _T("Vertex 8"));
    AddString( _T("Vertex 9"));
    AddString( _T("Vertex 10"));
    AddString( _T("Vertex 11"));
    AddString( _T("Vertex 12"));
    AddString( _T("Vertex 13"));
    AddString( _T("Vertex 14"));
    AddString( _T("Vertex 15"));
    AddString( _T("Vertex 16"));
    // select currently selected triangularisation type
    SetCurSel( (int)theApp.m_vfpCurrent.vfp_ttTriangularisationType);
  }
  return TRUE;
}

void CTriangularisationCombo::OnSelchange() 
{
  // get document ptr
  CWorldEditorDoc* pDoc = theApp.GetActiveDocument();
  if( pDoc == NULL) return;

  int iSelected = GetCurSel();
  theApp.m_vfpCurrent.vfp_ttTriangularisationType = (enum TriangularisationType) iSelected;
  pDoc->CreatePrimitive();
  pDoc->UpdateAllViews(NULL);
}

void CTriangularisationCombo::OnDropdown() 
{
  INDEX ctItems = GetCount();
  if( ctItems == CB_ERR) return;
  
  CRect rectCombo;
  GetWindowRect( &rectCombo);
  
  PIX pixScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
  PIX pixMaxHeight = pixScreenHeight - rectCombo.top;

  CWnd *pwndParent = GetParent();
  if( pwndParent == NULL) return;
  pwndParent->ScreenToClient( &rectCombo);
  PIX pixNewHeight = GetItemHeight(0)*(ctItems+2);
  rectCombo.bottom = rectCombo.top + ClampUp( pixNewHeight, pixMaxHeight);
  MoveWindow( rectCombo);
}
