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

// [Cecil] Definitions of unexported CPlayerTarget methods from the engine

#ifndef CECIL_INCL_PLAYERTARGET_DEFS_H
#define CECIL_INCL_PLAYERTARGET_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_FUNC_DEFINITIONS

#include "PlayerActionDefs.inl"

// Constructor
CPlayerTarget::CPlayerTarget(void) {
  plt_bActive = FALSE;
  plt_penPlayerEntity = NULL;
  plt_csAction.cs_iIndex = -1;

  plt_paLastAction.Clear();
  plt_paPreLastAction.Clear();
  plt_abPrediction.Clear();
};

// Destructor
CPlayerTarget::~CPlayerTarget(void) {};

// Read player information from a stream
void CPlayerTarget::Read_t(CTStream *pstr) {
  INDEX iEntity;
  ULONG bActive;

  // Synchronize access to actions
  CTSingleLock slActions(&plt_csAction, TRUE);

  // Read activity flag
  *pstr >> bActive;

  // If client is active
  if (bActive) {
    // Set it up
    Activate();

    // Read data
    *pstr >> iEntity >> plt_paLastAction >> plt_paPreLastAction;

    CPlayerEntity *penPlayer = (CPlayerEntity *)&_pNetwork->ga_World.wo_cenAllEntities[iEntity];

    ASSERT(penPlayer != NULL);
    AttachEntity(penPlayer);
  }

  plt_abPrediction.Clear();
};

// Write client information into a stream
void CPlayerTarget::Write_t(CTStream *pstr) {
  INDEX iEntity;
  ULONG bActive = plt_bActive;

  // Synchronize access to actions
  CTSingleLock slActions(&plt_csAction, TRUE);

  // Write activity flag
  *pstr << bActive;

  // If client is active
  if (bActive) {
    // Prepare its data
    iEntity = _pNetwork->ga_World.wo_cenAllEntities.Index(plt_penPlayerEntity);

    // Write data
    *pstr << iEntity << plt_paLastAction << plt_paPreLastAction;
  }
};

// Activate client data for a new client
void CPlayerTarget::Activate(void) {
  ASSERT(!plt_bActive);

  plt_bActive = TRUE;
  plt_abPrediction.Clear();
  plt_paPreLastAction.Clear();
  plt_paLastAction.Clear();
};

// Deactivate client data for removed client
void CPlayerTarget::Deactivate(void) {
  ASSERT(plt_bActive);

  plt_bActive = FALSE;
  plt_penPlayerEntity = NULL;
  plt_abPrediction.Clear();
  plt_paPreLastAction.Clear();
  plt_paLastAction.Clear();
};

// Attach an entity to this client
void CPlayerTarget::AttachEntity(CPlayerEntity *penClientEntity) {
  ASSERT(plt_bActive);
  plt_penPlayerEntity = penClientEntity;
};

// Apply action packet to current actions
void CPlayerTarget::ApplyActionPacket(const CPlayerAction &paDelta) {
  ASSERT(plt_bActive);
  ASSERT(plt_penPlayerEntity != NULL);

  // Synchronize access to actions
  CTSingleLock slActions(&plt_csAction, TRUE);

  // Create a new action packet from last received packet and given delta
  plt_paPreLastAction = plt_paLastAction;

  __int64 llTag = plt_paLastAction.pa_llCreated += paDelta.pa_llCreated;

  for (INDEX i = 0; i < sizeof(CPlayerAction); i++) {
    ((UBYTE *)&plt_paLastAction)[i] ^= ((UBYTE *)&paDelta)[i];
  }

  plt_paLastAction.pa_llCreated = llTag;

  FLOAT fLatency = 0.0f;

  // If the player is local
  if (_pNetwork->IsPlayerLocal(plt_penPlayerEntity)) {
    // Calculate latency
    __int64 llmsNow = _pTimer->GetHighPrecisionTimer().GetMilliseconds();
    __int64 llmsCreated = plt_paLastAction.pa_llCreated;

    fLatency = DOUBLE(llmsNow - llmsCreated) / 1000.0f;

    if (plt_paLastAction.pa_llCreated == plt_paPreLastAction.pa_llCreated) {
      _pNetwork->AddNetGraphValue(NGET_REPLICATEDACTION, fLatency);

    } else {
      CPlayerAction *ppaOlder = plt_abPrediction.GetLastOlderThan(plt_paLastAction.pa_llCreated);

      if (ppaOlder != NULL && ppaOlder->pa_llCreated != plt_paPreLastAction.pa_llCreated) {
        _pNetwork->AddNetGraphValue(NGET_SKIPPEDACTION, 1.0f);
      }

      _pShell->SetFLOAT("net_tmLatency", fLatency);
      _pNetwork->AddNetGraphValue(NGET_ACTION, fLatency);
    }
  }

  // If the entity is not deleted
  if (!(plt_penPlayerEntity->en_ulFlags & ENF_DELETED)) {
    // Call the player DLL class to apply the new action to the entity
    plt_penPlayerEntity->ApplyAction(plt_paLastAction, fLatency);
  }

  static CSymbolPtr symptr("cli_iPredictionFlushing");
  INDEX iPredictionFlushing = symptr.GetIndex();

  if (iPredictionFlushing == 2 || iPredictionFlushing == 3) {
    plt_abPrediction.RemoveOldest();
  }
};

// Remember prediction action
void CPlayerTarget::PrebufferActionPacket(const CPlayerAction &paPrediction) {
  ASSERT(plt_bActive);

  // Synchronize access to actions
  CTSingleLock slActions(&plt_csAction, TRUE);

  // Buffer the action
  plt_abPrediction.AddAction(paPrediction);
};

// Flush prediction actions that were already processed
void CPlayerTarget::FlushProcessedPredictions(void) {
  CTSingleLock slActions(&plt_csAction, TRUE);

  static CSymbolPtr symptr("cli_iPredictionFlushing");
  INDEX iPredictionFlushing = symptr.GetIndex();

  // Flush all actions that were already processed
  if (iPredictionFlushing == 1) {
    plt_abPrediction.FlushUntilTime(plt_paLastAction.pa_llCreated);
    
  // Flush older actions that were already processed
  } else if (iPredictionFlushing == 3) {
    plt_abPrediction.FlushUntilTime(plt_paPreLastAction.pa_llCreated);
  }
};

// Get maximum number of actions that can be predicted
INDEX CPlayerTarget::GetNumberOfPredictions(void) {
  CTSingleLock slActions(&plt_csAction, TRUE);

  // Return current count
  return plt_abPrediction.GetCount();
};
  
// Apply predicted action with given index
void CPlayerTarget::ApplyPredictedAction(INDEX iAction, FLOAT fFactor) {
  // Synchronize access to actions
  CTSingleLock slActions(&plt_csAction, TRUE);

  CPlayerAction pa;

  // If the player is local
  if (_pNetwork->IsPlayerLocal(plt_penPlayerEntity)) {
    // Get the action from buffer
    plt_abPrediction.GetActionByIndex(iAction, pa);

  // If the player is not local
  } else {
    // Reuse last action
    static CSymbolPtr symptr("cli_bLerpActions");

    if (symptr.GetIndex()) {
      pa.Lerp(plt_paPreLastAction, plt_paLastAction, fFactor);
    } else {
      pa = plt_paLastAction;
    }
  }

  // Get the player's predictor
  if (!plt_penPlayerEntity->IsPredicted()) {
    return;
  }

  CEntity *penPredictor = plt_penPlayerEntity->GetPredictor();

  if (penPredictor == NULL || penPredictor == plt_penPlayerEntity) {
    return;
  }

  // Apply a prediction action packet to the entity's predictor
  ((CPlayerEntity *)penPredictor)->ApplyAction(pa, 0.0f);
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
