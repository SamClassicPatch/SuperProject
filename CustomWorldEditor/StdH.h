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

#define VC_EXTRALEAN    // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxadv.h>
#include <afxole.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>      // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// [Cecil] Compatibility with older MFC
#if _MSC_VER > 1200
  #pragma comment(lib, "htmlhelp.lib")
  #include <htmlhelp.h>

  #define SE1_HTMLHELP 1
  typedef DWORD Task_t;

#else
  #define SE1_HTMLHELP 0
  typedef HTASK Task_t;
#endif

// [Cecil] Newer engine features
#define SE1_NEWFEATURES (SE1_VER >= SE1_107) // 1.07 or newer

#define SE1_TERRAINS      SE1_NEWFEATURES // Terrain system
#define SE1_DSPOLYGONS    SE1_NEWFEATURES // Double-sided brush polygons flag
#define SE1_WRP_FARCLIP   SE1_NEWFEATURES // Far clip adjustment settings for CWorldRenderPrefs
#define SE1_RAYHITBRUSHES SE1_NEWFEATURES // cr_bHitBrushes setting for CCastRay

#define ENGINE_INTERNAL 1
#include <PatchedGui/EngineGUI.h>
#include <Engine/GameShell.h>
#include <Engine/Base/ChangeableRT.h>
#include <Engine/Base/UpdateableRT.h>

#if SE1_TERRAINS
  #include <Engine/Terrain/Terrain.h>
  #include <Engine/Terrain/TerrainMisc.h>
#endif

#include "WorldEditor.h"

// [Cecil] Compatibility with older compiler
#if _MSC_VER > 1200
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
