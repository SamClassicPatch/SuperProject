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

#ifndef CECIL_INCL_PLUGINSTOCK_H
#define CECIL_INCL_PLUGINSTOCK_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "PluginModule.h"
#include <Engine/Templates/DynamicContainer.h>

// Declare plugin name table
#define TYPE CPluginModule
#define CNameTable_TYPE CPluginTable
#define CNameTableSlot_TYPE CPluginTableSlot

#include <Engine/Templates/NameTable.h>

#undef CNameTableSlot_TYPE
#undef CNameTable_TYPE
#undef TYPE

// Stock of plugin modules that stores loaded plugins
class CPluginStock {
  public:
    CDynamicContainer<CPluginModule> st_ctObjects; // Plugins in the stock
    CPluginTable st_ntObjects;  // Name table for looking up plugins

  public:
    // Constructor
    CPluginStock(void);

    // Destructor
    ~CPluginStock(void);

    // Let plugin module load itself from a file
    CPluginModule *Obtain_t(const CTFileName &fnmFileName);

    // Forcefully release a plugin
    void Release(CPluginModule *pPlugin);

    // Free some plugin immediately
    void Free(CPluginModule *pPlugin);

    // Free unused plugins
    void FreeUnused(void);

    // Calculate used memory of all plugins
    SLONG CalculateUsedMemory(void);

    // Dump memory usage report into a file
    void DumpMemoryUsage_t(CTStream &strm);

    // Get number of plugins in the stock
    INDEX GetTotalCount(void);

    // Get number of used plugins in the stock
    INDEX GetUsedCount(void);
};

#endif
