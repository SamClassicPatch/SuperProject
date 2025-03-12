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

#include "StdH.h"

// Define the patcher
#include <Extras/XGizmo/Patcher/patcher.h>
#include <Extras/XGizmo/Patcher/patcher.cpp>

ICorePatches *_pCorePatches = NULL;

// Actual data of the function patch that's being physically constructed
struct CoreFunctionPatch_t : public FuncPatch_t
{
  CTString strName; // Unique printable patch identifier
  CPatch *pPatch; // Pointer to the associated function patch

  // Default constructor
  CoreFunctionPatch_t() : pPatch(NULL)
  {
  };

  // Constructor from name and patch
  CoreFunctionPatch_t(const CTString &strSetName, CPatch *pSetPatch) :
    strName(strSetName), pPatch(pSetPatch)
  {
  };

  // Destructor
  ~CoreFunctionPatch_t() {
    Clear();
  };

  // Clear the function patch
  inline void Clear(void) {
    if (pPatch != NULL) delete pPatch;

    strName = "";
    pPatch = NULL;
  };

  // Comparison operator
  inline bool operator==(const CoreFunctionPatch_t &fpOther) const {
    return (pPatch == fpOther.pPatch && strName == fpOther.strName);
  };

  // Calculate hash value from the name
  static inline ULONG GetHash(const CTString &strIdentifier) {
    return strIdentifier.GetHash();
  };

  // Overridden methods
  virtual const char *GetName(void) const { return strName; };
  virtual ULONG GetHash(void) const { return GetHash(strName); };
  virtual bool IsPatched(void) const { return pPatch->IsPatched(); };

  virtual bool Enable(void) {
    pPatch->SetPatch();
    return pPatch->IsValid();
  };

  virtual void Disable(void) {
    pPatch->RemovePatch();
  };
};

// Function patch storage
typedef se1::map<ULONG, HFuncPatch> CFuncPatches;
static CFuncPatches _mapFuncPatches;

void FuncPatch_SetDebug(bool bState) {
  CPatch::SetDebug(bState);
};

void FuncPatch_ForceRewrite(int nBytes) {
  CPatch::_iForceRewriteLen = nBytes;
};

HFuncPatch CreateOpcodePatch(void *pOpcodeToReplace, long iJumpDestinationAddress, const char *strIdentifier) {
  CPrintF("  %s\n", strIdentifier);

  // Create new patch and hook the functions
  CPatch *pPatch = new CPatch(FALSE);

  long *piNewCallAddress = (long *)pOpcodeToReplace;
  pPatch->HookFunction(*piNewCallAddress, iJumpDestinationAddress, piNewCallAddress, true);

  // Couldn't patch
  if (!pPatch->IsValid()) {
    // Don't terminate the game in debug
    #ifdef NDEBUG
      FatalError("Cannot set function patch for %s!", strIdentifier);
    #else
      InfoMessage("Cannot set function patch for %s!", strIdentifier);
    #endif

    delete pPatch;
    return NULL;
  }

  // Add to the patch registry
  HFuncPatch hPatch = new CoreFunctionPatch_t(strIdentifier, pPatch);
  _mapFuncPatches[hPatch->GetHash()] = hPatch;

  // Add patch to the plugin
  extern CPluginModule *_pInitializingPlugin;

  if (_pInitializingPlugin != NULL) {
    _pInitializingPlugin->AddPatch(hPatch);
  }

  return hPatch;
};

// Delete a function patch
void DestroyPatch(HFuncPatch hPatch)
{
  // Remove from the storage first
  CFuncPatches::const_iterator itPatch = _mapFuncPatches.find(hPatch->GetHash());

  if (itPatch != _mapFuncPatches.end()) {
    _mapFuncPatches.remove(*itPatch);
  } else {
    ASSERTALWAYS("Cannot find the function patch in the patch storage!");
  }

  delete (CoreFunctionPatch_t *)hPatch;
};

HFuncPatch FuncPatch_FindByName(const char *strIdentifier)
{
  // Find a patch under some hash
  const ULONG ulHash = CoreFunctionPatch_t::GetHash(strIdentifier);
  CFuncPatches::const_iterator it = _mapFuncPatches.find(ulHash);

  if (it != _mapFuncPatches.end()) {
    return it->second;
  }

  return NULL;
};

// Interface initialization
namespace IInitAPI {

// List available function patches
static void ListFuncPatches(void) {
  if (_mapFuncPatches.empty()) {
    CPutString(TRANS("No function patches available!\n"));
    return;
  }

  CPutString(TRANS("Available function patches:\n"));

  CFuncPatches::const_iterator it = _mapFuncPatches.begin();

  for (INDEX iPatch = 0; it != _mapFuncPatches.end(); it++, iPatch++)
  {
    HFuncPatch hPatch = it->second;
    const bool bPatched = ((CoreFunctionPatch_t *)hPatch)->IsPatched();

    // Mark as enabled or not and indent the index
    const char *strPatched = (bPatched ? " [^c00ff00ON^r]" : "[^cff0000OFF^r]");
    const INDEX ctIndentLog10 = (iPatch == 0) ? 0 : log10((FLOAT)iPatch);
    const INDEX ctIndent = ClampDn(2 - ctIndentLog10, (INDEX)0);

    CPrintF("%s %*s%d - %s\n", strPatched, ctIndent, "", iPatch, hPatch->GetName());
  }
};

// Enable specific function patch
static void EnablePatch(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iPatch = NEXT_ARG(INDEX);

  if (iPatch < 0 || iPatch >= _mapFuncPatches.size()) {
    CPutString(TRANS("Invalid patch index!\n"));
    return;
  }

  CFuncPatches::iterator it = _mapFuncPatches.begin();
  std::advance(it, iPatch);

  HFuncPatch hPatch = it->second;

  if (hPatch->Enable()) {
    CPrintF(TRANS("Successfully set '%s' function patch!\n"), hPatch->GetName());
  } else {
    CPrintF(TRANS("Cannot set '%s' function patch!\n"), hPatch->GetName());
  }
};

// Disable specific function patch
static void DisablePatch(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iPatch = NEXT_ARG(INDEX);

  if (iPatch < 0 || iPatch >= _mapFuncPatches.size()) {
    CPutString(TRANS("Invalid patch index!\n"));
    return;
  }

  CFuncPatches::iterator it = _mapFuncPatches.begin();
  std::advance(it, iPatch);

  HFuncPatch hPatch = it->second;
  hPatch->Disable();

  CPrintF(TRANS("Successfully removed '%s' function patch!\n"), hPatch->GetName());
};

// Get function patch index by its name
static INDEX GetFuncPatch(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strName = *NEXT_ARG(CTString *);

  const ULONG ulHash = CoreFunctionPatch_t::GetHash(strName);
  CFuncPatches::const_iterator it = _mapFuncPatches.begin();

  for (INDEX iPatch = 0; it != _mapFuncPatches.end(); it++, iPatch++)
  {
    // Matching handle
    if (it->first == ulHash) return iPatch;
  }

  // Not found
  return -1;
};

void Patches(void) {
  // Commands for manually toggling function patches
  _pShell->DeclareSymbol("void ListPatches(void);",   &ListFuncPatches);
  _pShell->DeclareSymbol("void EnablePatch(INDEX);",  &EnablePatch);
  _pShell->DeclareSymbol("void DisablePatch(INDEX);", &DisablePatch);
  _pShell->DeclareSymbol("INDEX GetFuncPatch(CTString);", &GetFuncPatch);
};

}; // namespace
