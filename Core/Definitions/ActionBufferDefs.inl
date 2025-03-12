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

// [Cecil] Definitions of unexported CActionBuffer methods from the engine

#ifndef CECIL_INCL_ACTIONBUFFER_DEFS_H
#define CECIL_INCL_ACTIONBUFFER_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_FUNC_DEFINITIONS

class CActionEntry {
  public:
    CListNode ae_ln;
    CPlayerAction ae_pa;
};

// Constructor
CActionBuffer::CActionBuffer(void) {};

// Destructor
CActionBuffer::~CActionBuffer(void) {
  Clear();
};

// Clear the buffer
void CActionBuffer::Clear(void) {
  FORDELETELIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    delete &*itae;
  }
};

static int qsort_CompareActions(const void *elem1, const void *elem2) {
  const CActionEntry &ae1 = **(CActionEntry **)elem1;
  const CActionEntry &ae2 = **(CActionEntry **)elem2;
  return ae1.ae_pa.pa_llCreated-ae2.ae_pa.pa_llCreated;
};

// Add a new action to the buffer
void CActionBuffer::AddAction(const CPlayerAction &pa) {
  // Search all buffered actions
  FOREACHINLIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    CActionEntry &ae = *itae;

    // If this is the one
    if (ae.ae_pa.pa_llCreated == pa.pa_llCreated) {
      // Skip adding it again
      return;
    }
  }

  // Add to the tail
  CActionEntry *pae = new CActionEntry;
  pae->ae_pa = pa;

  ab_lhActions.AddTail(pae->ae_ln);

  // Sort the list
  ab_lhActions.Sort(&qsort_CompareActions, offsetof(CActionEntry, ae_ln));
}

// Flush all actions up to given time tag
void CActionBuffer::FlushUntilTime(__int64 llNewest) {
  // For each buffered action
  FORDELETELIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    CActionEntry &ae = *itae;

    // Delete if it's up to that time
    if (ae.ae_pa.pa_llCreated <= llNewest) {
      delete &*itae;
    }
  }
}

// Remove the oldest buffered action
void CActionBuffer::RemoveOldest(void) {
  // For each buffered action
  FORDELETELIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    CActionEntry &ae = *itae;

    // Delete only first one
    delete &*itae;
    break;
  }
};

// Get number of buffered actions
INDEX CActionBuffer::GetCount(void) {
  return ab_lhActions.Count();
};

// Get an action by its index (0 = oldest)
void CActionBuffer::GetActionByIndex(INDEX i, CPlayerAction &pa) {
  // For each buffered action
  INDEX iInList = 0;

  FOREACHINLIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    if (iInList == i) {
      pa = itae->ae_pa;
      return;
    }

    iInList++;
  }

  // If not found, use empty
  pa.Clear();
};

// Get last action older than given time
CPlayerAction *CActionBuffer::GetLastOlderThan(__int64 llTime) {
  CPlayerAction *ppa = NULL;

  FOREACHINLIST(CActionEntry, ae_ln, ab_lhActions, itae) {
    CActionEntry &ae = *itae;

    if (ae.ae_pa.pa_llCreated >= llTime) {
      return ppa;
    }

    ppa = &ae.ae_pa;
  }

  return ppa;
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
