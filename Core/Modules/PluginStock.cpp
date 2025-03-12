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

#include "StdH.h"

#include "PluginStock.h"

// Define plugin name table
#define TYPE CPluginModule
#define CNameTable_TYPE CPluginTable
#define CNameTableSlot_TYPE CPluginTableSlot

#include <Engine/Templates/NameTable.cpp>

#undef CNameTableSlot_TYPE
#undef CNameTable_TYPE
#undef TYPE

// Constructor
CPluginStock::CPluginStock(void) {
  st_ntObjects.SetAllocationParameters(50, 2, 2);
};

// Destructor
CPluginStock::~CPluginStock(void) {
  // Free unused plugins
  FreeUnused();
};

// Let plugin module load itself from a file
CPluginModule *CPluginStock::Obtain_t(const CTFileName &fnmFileName) {
  // Find stocked plugin with the same name
  CPluginModule *pExisting = st_ntObjects.Find(fnmFileName);

  // Use it once more, if found
  if (pExisting != NULL) {
    pExisting->MarkUsed();
    return pExisting;
  }

  // Add new plugin module
  CPluginModule *pNewPlugin = new CPluginModule;
  pNewPlugin->ser_FileName = fnmFileName;

  st_ctObjects.Add(pNewPlugin);
  st_ntObjects.Add(pNewPlugin);

  try {
    // Try to load the plugin
    pNewPlugin->Load_t(fnmFileName);

  } catch (char *) {
    // Free the plugin if couldn't load it
    Free(pNewPlugin);
    throw;
  }

  // Mark as used for the first time
  pNewPlugin->MarkUsed();

  // Return it
  return pNewPlugin;
};

// Release a plugin when it's not needed anymore
void CPluginStock::Release(CPluginModule *pPlugin) {
  // Use once less
  pPlugin->MarkUnused();

  // Forcefully release it if not used anymore
  if (!pPlugin->IsUsed()) {
    Free(pPlugin);
  }
};

// Free some plugin immediately
void CPluginStock::Free(CPluginModule *pPlugin) {
  // Remove from the lists and delete
  st_ctObjects.Remove(pPlugin);
  st_ntObjects.Remove(pPlugin);
  delete pPlugin;
};

// Free unused plugins
void CPluginStock::FreeUnused(void) {
  BOOL bAnyRemoved;

  do {
    // Gather unused plugins
    CDynamicContainer<CPluginModule> cFree;

    FOREACHINDYNAMICCONTAINER(st_ctObjects, CPluginModule, itCheck) {
      if (!itCheck->IsUsed()) {
        cFree.Add(itCheck);
      }
    }

    // There are plugins to remove
    bAnyRemoved = cFree.Count() > 0;

    // Free unneeded plugins
    FOREACHINDYNAMICCONTAINER(cFree, CPluginModule, itFree) {
      Free(itFree);
    }

  // Keep doing it as long as there's anything to remove
  } while (bAnyRemoved);
};

// Calculate used memory of all plugins
SLONG CPluginStock::CalculateUsedMemory(void) {
  SLONG slUsedTotal = 0;
  
  // Go through all plugins
  FOREACHINDYNAMICCONTAINER(st_ctObjects, CPluginModule, itpm) {
    SLONG slUsedByObject = itpm->GetUsedMemory();

    // Invalid memory
    if (slUsedByObject < 0) {
      return -1;
    }

    slUsedTotal += slUsedByObject;
  }

  return slUsedTotal;
};

// Dump memory usage report into a file
void CPluginStock::DumpMemoryUsage_t(CTStream &strm) {
  CTString strLine;
  SLONG slUsedTotal = 0;

  // Go through all plugins
  FOREACHINDYNAMICCONTAINER(st_ctObjects, CPluginModule, itpm) {
    SLONG slUsedByObject = itpm->GetUsedMemory();

    // Invalid memory
    if (slUsedByObject < 0) {
      strm.PutLine_t("Error!");
      return;
    }

    // Print out usage count and memory
    strLine.PrintF("%7.1fk %s(%d) %s", slUsedByObject/1024.0f, itpm->GetName().str_String,
                   itpm->GetUsedCount(), itpm->GetDescription().str_String);

    strm.PutLine_t(strLine);
  }
};

// Get number of plugins in the stock
INDEX CPluginStock::GetTotalCount(void) {
  return st_ctObjects.Count();
};

// Get number of used plugins in the stock
INDEX CPluginStock::GetUsedCount(void) {
  INDEX ctUsed = 0;
  
  // Count used plugins
  FOREACHINDYNAMICCONTAINER(st_ctObjects, CPluginModule, itpm) {
    if (itpm->IsUsed()) {
      ctUsed++;
    }
  }

  return ctUsed;
};
