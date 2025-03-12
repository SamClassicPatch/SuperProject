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

// [Cecil] Definitions of source-only classes from Engine/Entities source code

#ifndef CECIL_INCL_ENTITIES_DEFS_H
#define CECIL_INCL_ENTITIES_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_CLASS_DEFINITIONS

class CSentEvent {
  public:
    CEntityPointer se_penEntity;
    CEntityEvent *se_peeEvent;

    inline void Clear(void) {
      se_penEntity = NULL;
    };
};

// The copy of this class is also used in world source code
class CActiveSector {
  public:
    CBrushSector *as_pbsc;

    // Dummy method for CStaticStackArray
    void Clear(void) {};
};

#endif // _PATCHCONFIG_CLASS_DEFINITIONS

#endif
