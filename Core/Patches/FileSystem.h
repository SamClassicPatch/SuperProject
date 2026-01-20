/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_PATCHES_FILESYSTEM_H
#define CECIL_INCL_PATCHES_FILESYSTEM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Entities/EntityClass.h>

#if SE1_VER >= SE1_107
  #include <Engine/Graphics/Shader.h>
#endif

#if _PATCHCONFIG_ENGINEPATCHES && _PATCHCONFIG_EXTEND_FILESYSTEM

class CEntityClassPatch : public CEntityClass {
  public:
    // Obtain components of the entity class
    void P_ObtainComponents(void);

    // Load entity class from a library
    void P_Read(CTStream *istr);
};

#if SE1_VER >= SE1_107

class CShaderPatch : public CShader {
  public:
    // Load shader from a library
    void P_Read(CTStream *istr);
};

#endif

class CStreamPatch : public CTStream {
  public:
    void P_GetLine(char *strBuffer, SLONG slBufferSize, char cDelimiter) {
      IData::GetLineFromStream_t(*this, strBuffer, slBufferSize, cDelimiter);
    };

    // Read the dictionary from a given offset
    void P_ReadDictionary_intenal(SLONG slOffset);
};

// Initialize various file paths and load game content
void P_InitStreams(void);

// Make a list of all files in a directory
void P_MakeDirList(CFileList &afnmDir, const CTFileName &fnmDir, const CTString &strPattern, ULONG ulFlags);

// Argument list for the ExpandFilePath() function
#if SE1_GAME != SS_REV
  #define EXPAND_PATH_ARGS(_Type, _File, _Expanded, _UseRPH) _Type, _File, _Expanded
#else
  #define EXPAND_PATH_ARGS(_Type, _File, _Expanded, _UseRPH) _Type, _File, _Expanded, _UseRPH
#endif

// Expand a filename to absolute path
INDEX P_ExpandFilePath(EXPAND_PATH_ARGS(ULONG ulType, const CTFileName &fnmFile, CTFileName &fnmExpanded, BOOL bUseRPH = 0));

#endif // _PATCHCONFIG_EXTEND_FILESYSTEM

#endif
