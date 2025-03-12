/* Copyright (c) 2022-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_GAMESPECIFIC_H
#define CECIL_INCL_GAMESPECIFIC_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Choose value based on engine
#if SE1_VER <= SE1_107
  #define CHOOSE_FOR_ENGINE(_107, _150, _110) _107
#elif SE1_VER == SE1_150
  #define CHOOSE_FOR_ENGINE(_107, _150, _110) _150
#elif SE1_VER == SE1_110
  #define CHOOSE_FOR_ENGINE(_107, _150, _110) _110
#else
  #error Unsupported engine version!
#endif

// Choose value based on game
#if SE1_VER == SE1_105 && SE1_GAME == SS_TFE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TFE105
#elif SE1_VER == SE1_105 && SE1_GAME == SS_TSE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE105
#elif SE1_VER >= SE1_107
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE107

  // [Cecil] TEMP: 1.10 support
  #if SE1_VER == SE1_110
    #define GetGameSpyPlayerInfo GetGameAgentPlayerInfo
  #endif

  // [Cecil] TEMP: Revolution support
  #if SE1_GAME == SS_REV
    #define LoadAnyGfxFormat_t Load_t
  #endif

#else
  #error Unsupported game!
#endif

// Engine module address
#ifdef NDEBUG
  #define HMODULE_ENGINE (GetModuleHandleA("Engine.dll"))
#else
  #define HMODULE_ENGINE (GetModuleHandleA("EngineD.dll"))
#endif

// Engine module address by single byte offsetting
#define ADDR_ENGINE ((UBYTE *)HMODULE_ENGINE)

// Relative addresses of specific elements in the engine
#if SE1_GAME == SS_REV // Revolution addresses
  // InitStreams()
  #define ADDR_INITSTREAMS (ADDR_ENGINE + 0x80ED0)

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  (ADDR_ENGINE + 0x29B938) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  (ADDR_ENGINE + 0x270D58) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  (ADDR_ENGINE + 0x270D48) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES (ADDR_ENGINE + 0x270D68) // &_afnmArchives

  // Offsets within virtual tables of class functions
  #define VFOFFSET_ENTITY_GETFORCE (31) // CEntity::GetForce()
  #define VFOFFSET_CONTROLS_LOAD (11) // CControls::Load_t()

#elif SE1_VER == SE1_110 // Open-source 1.10 addresses
  // [Cecil] NOTE: These functions and structures need to be exported from the engine module
  ENGINE_API void InitStreams(void);                                   // Defined in Engine/Base/Stream.cpp
  ENGINE_API extern CTCriticalSection zip_csLock;                      // Defined in Engine/Engine.cpp
  ENGINE_API extern CStaticStackArray<class CZipHandle> _azhHandles;   // Defined in Engine/Base/Unzip.cpp
  ENGINE_API extern CStaticStackArray<class CZipEntry>  _azeFiles;     // Defined in Engine/Base/Unzip.cpp
  ENGINE_API extern CStaticStackArray<class CTFileName> _afnmArchives; // Defined in Engine/Base/Unzip.cpp

  // InitStreams()
  #define ADDR_INITSTREAMS (&InitStreams)

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  (&zip_csLock)
  #define ADDR_UNZIP_HANDLES  (&_azhHandles)
  #define ADDR_UNZIP_ENTRIES  (&_azeFiles)
  #define ADDR_UNZIP_ARCHIVES (&_afnmArchives)

  // Offsets within virtual tables of class functions
  #define VFOFFSET_ENTITY_GETFORCE (31) // CEntity::GetForce()
  #define VFOFFSET_CONTROLS_LOAD (11) // CControls::Load_t()

#elif defined(NDEBUG) // Release 1.05 & 1.07 addresses
  // InitStreams()
  #define ADDR_INITSTREAMS (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1A2C0, 0x1A2C0, 0x265F0))

  // Strings with the master server address
  #define ADDR_GAMESPY_MS_1 (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1BD1CC, 0x1BD20C, 0x1ED9FC))
  #define ADDR_GAMESPY_MS_2 (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1BD300, 0x1BD340, 0x1EDB30))
  #define ADDR_GAMESPY_MS_3 (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1BD314, 0x1BD354, 0x1EDB44))

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1E0388, 0x1E03C8, 0x2131C8)) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1C5030, 0x1C5070, 0x1F6298)) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1C5040, 0x1C5080, 0x1F62A8)) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES (ADDR_ENGINE + CHOOSE_FOR_GAME(0x1C5020, 0x1C5060, 0x1F6288)) // &_afnmArchives

  // UNZIP* methods
  #define ADDR_UNZIP_OPEN      (ADDR_ENGINE + CHOOSE_FOR_GAME(0x219E0, 0x21A70, 0x2DD50)) // UNZIPOpen_t()
  #define ADDR_UNZIP_GETSIZE   (ADDR_ENGINE + CHOOSE_FOR_GAME(0x21D90, 0x21E20, 0x2E100)) // UNZIPGetSize()
  #define ADDR_UNZIP_READBLOCK (ADDR_ENGINE + CHOOSE_FOR_GAME(0x21DF0, 0x21E80, 0x2E160)) // UNZIPReadBlock_t()
  #define ADDR_UNZIP_CLOSE     (ADDR_ENGINE + CHOOSE_FOR_GAME(0x220F0, 0x22180, 0x2E460)) // UNZIPClose()
  #define ADDR_UNZIP_GETFILECOUNT     (ADDR_ENGINE + CHOOSE_FOR_GAME(0x218C0, 0x21950, 0x2DC30)) // UNZIPGetFileCount()
  #define ADDR_UNZIP_GETFILEATINDEX   (ADDR_ENGINE + CHOOSE_FOR_GAME(0x218D0, 0x21960, 0x2DC40)) // UNZIPGetFileAtIndex()
  #define ADDR_UNZIP_ISFILEATINDEXMOD (ADDR_ENGINE + CHOOSE_FOR_GAME(0x218F0, 0x21980, 0x2DC60)) // UNZIPIsFileAtIndexMod()

  // Offsets within virtual tables of class functions
  #define VFOFFSET_ENTITY_GETFORCE CHOOSE_FOR_GAME(27, 27, 31) // CEntity::GetForce()
  #define VFOFFSET_CONTROLS_LOAD (11) // CControls::Load_t()

#elif SE1_VER == SE1_107 // Debug addresses
  // InitStreams()
  #define ADDR_INITSTREAMS (ADDR_ENGINE + 0x7B49)

  // Strings with the master server address
  #define ADDR_GAMESPY_MS_1 (ADDR_ENGINE + 0x413D14)
  #define ADDR_GAMESPY_MS_2 (ADDR_ENGINE + 0x4140FC)
  #define ADDR_GAMESPY_MS_3 (ADDR_ENGINE + 0x414110)

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  (ADDR_ENGINE + 0x446BC8) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  (ADDR_ENGINE + 0x424158) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  (ADDR_ENGINE + 0x424168) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES (ADDR_ENGINE + 0x424148) // &_afnmArchives

  // UNZIP* methods
  #define ADDR_UNZIP_OPEN      (ADDR_ENGINE + 0x2C20) // UNZIPOpen_t()
  #define ADDR_UNZIP_GETSIZE   (ADDR_ENGINE + 0x720C) // UNZIPGetSize()
  #define ADDR_UNZIP_READBLOCK (ADDR_ENGINE + 0x8693) // UNZIPReadBlock_t()
  #define ADDR_UNZIP_CLOSE     (ADDR_ENGINE + 0x88FA) // UNZIPClose()
  #define ADDR_UNZIP_GETFILECOUNT     (ADDR_ENGINE + 0x1456) // UNZIPGetFileCount()
  #define ADDR_UNZIP_GETFILEATINDEX   (ADDR_ENGINE + 0x5268) // UNZIPGetFileAtIndex()
  #define ADDR_UNZIP_ISFILEATINDEXMOD (ADDR_ENGINE + 0xA45C) // UNZIPIsFileAtIndexMod()

  // Offsets within virtual tables of class functions
  #define VFOFFSET_ENTITY_GETFORCE (31) // CEntity::GetForce()
  #define VFOFFSET_CONTROLS_LOAD (11) // CControls::Load_t()
#endif

#endif
