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

// StdAfx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// [Cecil] Use patched EngineGUI
#define CORE_NO_GAME_HEADER
#include <PatchedGui/EngineGUI.h>

#include <Game/Game.h>
#include <GameGUI/GameGUI.h>
#include <Engine/GameShell.h>

// [Cecil] Custom mod is disabled
#if !_PATCHCONFIG_CUSTOM_MOD
  #error Custom mod has been disabled in this build!
#endif

/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "ConsoleSymbolsCombo.h"
#include "ActionsListControl.h"
#include "AxisListCtrl.h"
#include "EditConsole.h"
#include "LocalPlayersList.h"
#include "PressKeyEditControl.h"
#include "DlgSelectPlayer.h"
#include "DlgRenameControls.h"
#include "DlgAudioQuality.h"
#include "DlgPlayerAppearance.h"
#include "DlgPlayerControls.h"
#include "DlgPlayerSettings.h"
#include "DlgVideoQuality.h"
#include "DlgConsole.h"
#include "DlgEditButtonAction.h"
