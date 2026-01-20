/* Copyright (c) 2022-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_GUI_EXTENSIONS_H
#define CECIL_INCL_GUI_EXTENSIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Custom initialization for other tools
void ClassicsPatch_InitTools(void);

// Custom initialization for Serious Editor
void ClassicsPatch_InitEditor(void);

// Convert MFC's CString to CTString
inline CTString MfcStringToCT(const CString &str);

// Define string conversion method
#include "MfcStringConversion.inl"

#endif
