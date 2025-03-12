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

// [Cecil] Definitions of source-only classes from Engine/Base source code

#ifndef CECIL_INCL_BASE_DEFS_H
#define CECIL_INCL_BASE_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Base/ListIterator.inl>

#if _PATCHCONFIG_CLASS_DEFINITIONS

// One animation of CAnimObject
class COneAnim {
  public:
    NAME oa_Name;
    TIME oa_SecsPerFrame; // Animation speed
    INDEX oa_NumberOfFrames;
    INDEX *oa_FrameIndices; // Array of frame indices

  public:
    // Constructor
    COneAnim() {
      oa_FrameIndices = NULL;
    };

    // Destructor
    ~COneAnim() {
      ASSERT(oa_FrameIndices != NULL);

      FreeMemory(oa_FrameIndices);
      oa_FrameIndices = NULL;
    };

    // Copy constructor
    COneAnim &operator=(const COneAnim &oaAnim) {
      ASSERT(oaAnim.oa_NumberOfFrames > 0);

      // Copy metadata
      strcpy(oa_Name, oaAnim.oa_Name);
      oa_SecsPerFrame = oaAnim.oa_SecsPerFrame;
      oa_NumberOfFrames = oaAnim.oa_NumberOfFrames;

      // Delete previous frame indices
      if (oa_FrameIndices != NULL) {
        FreeMemory(oa_FrameIndices);
      }

      // Copy frame indices
      oa_FrameIndices = (INDEX *)AllocMemory(sizeof(INDEX) * oa_NumberOfFrames);

      for (INDEX iFrame = 0; iFrame < oa_NumberOfFrames; iFrame++) {
        oa_FrameIndices[iFrame] = oaAnim.oa_FrameIndices[iFrame];
      }

      return *this;
    };
};

// Node for linking COneAnim classes together
class COneAnimNode {
  public:
    CListNode coan_Node;
    COneAnim *coan_OneAnim;

  public:
    // Constructor
    COneAnimNode(COneAnim *AnimToInsert, CListHead *LH) {
      coan_OneAnim = AnimToInsert;
      LH->AddTail(coan_Node);
    };

    // Destructor
    ~COneAnimNode() {
      ASSERT(coan_OneAnim != NULL);
      delete coan_OneAnim;
    };
};

// Temporary list head that deletes all COneAnimNode children with itself
class CTmpListHead : public CListHead {
  public:
    // Destructor
    ~CTmpListHead() {
      FORDELETELIST(COneAnimNode, coan_Node, *this, it) {
        delete &it.Current();
      }
    }
};

// CRC entry for one file
class CCRCEntry {
  public:
    CTFileName ce_fnmFile; // The file it belongs to
    ULONG ce_ulCRC; // CRC of the file
    BOOL ce_bActive; // Set if the file is now active for CRC checking

    // Clear the entry
    void Clear(void) {
      ce_fnmFile.Clear(); 
      ce_ulCRC = 0;
      ce_bActive = FALSE;
    };

    // Use filename for the nametable
    inline const CTFileName &GetName(void) {
      return ce_fnmFile;
    };
};

#endif // _PATCHCONFIG_CLASS_DEFINITIONS

#endif
