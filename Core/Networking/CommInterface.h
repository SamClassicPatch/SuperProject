/* Copyright (c) 2022-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_COMMINTERFACE_H
#define CECIL_INCL_COMMINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Compatibility with SE1.05
#if SE1_VER <= SE1_105
  #include <Engine/Network/Comm.h>
  typedef Communication CCommunicationInterface;
#else
  #include <Engine/Network/CommunicationInterface.h>
#endif

// Retrieve communication interface
__forceinline CCommunicationInterface &GetComm(void) {
  #if SE1_VER <= SE1_105
    return comm;
  #else
    return _cmiComm;
  #endif
};

// Check if communication interface has been initialized
__forceinline BOOL IsCommInitialized(void) {
  #if SE1_VER <= SE1_105
    return TRUE;
  #else
    return GetComm().cci_bClientInitialized;
  #endif
};

#endif
