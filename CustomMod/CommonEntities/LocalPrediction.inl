/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_ENTITIES_LOCALPREDICTION_INL
#define CECIL_INCL_ENTITIES_LOCALPREDICTION_INL

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "LocalPrediction.h"

static INDEX gam_bDisableLocalPrediction = -1; // Auto

// Check if local prediction should be disabled (determine latency from player)
BOOL DisableLocalPrediction(const CEntity *penPlayer)
{
  if (gam_bDisableLocalPrediction == -1 && penPlayer != NULL) {
    ASSERT(IsDerivedFromID(const_cast<CEntity *>(penPlayer), CPlayerEntity_ClassID));

    TIME tmLatency = ((const CPlayerEntity *)penPlayer)->en_tmPing * 1000.0;
    return tmLatency <= 50;
  }

  return gam_bDisableLocalPrediction > 0;
};

// Get lerped placement for predicted entities (and not adjusted for prediction)
CPlacement3D GetLocalLerpedPlacement(const CMovableEntity *pen, const CEntity *penPlayer)
{
  // [Cecil] NOTE: There's been a NULL pointer exception near the end at some point, so we're doing this now
  if (pen == NULL) return CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));

  // It's the original entity or local prediction is kept on
  if (pen->IsPredicted() || !DisableLocalPrediction(penPlayer)) {
    return pen->CMovableEntity::GetLerpedPlacement();
  }

  // Set the same lerp factor for predicted entities as for normal rendering
  FLOAT fLerpFactor = _pTimer->GetLerpFactor2();
  _pTimer->SetLerp2(_pTimer->GetLerpFactor());

  CPlacement3D plLerped = pen->CMovableEntity::GetLerpedPlacement();

  // Restore lerp factor
  _pTimer->SetLerp2(fLerpFactor);
  return plLerped;
};

#endif
