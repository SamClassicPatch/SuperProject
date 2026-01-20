/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_PATCHES_ENTITIES_H
#define CECIL_INCL_PATCHES_ENTITIES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#if _PATCHCONFIG_EXTEND_ENTITIES && _PATCHCONFIG_ENGINEPATCHES

class CEntityPatch : public CEntity {
  public:
    // ReceiveItem() pointer
    typedef BOOL (CEntity::*CReceiveItem)(const CEntityEvent &);

    // RenderGameView() pointer
    typedef void (CEntity::*CRenderGameView)(CDrawPort *, void *);

    // GetForce() pointer
    typedef void (CEntity::*CGetForce)(INDEX, const FLOAT3D &, CForceStrength &, CForceStrength &);

  public:
    // Read entity property values
    void P_ReadProperties(CTStream &istrm);

    // Send an event to this entity
    void P_SendEvent(const CEntityEvent &ee);

    // Receive item by a player entity
    BOOL P_ReceiveItem(const CEntityEvent &ee);

    // Render game view from the player's perspective
    void P_RenderGameView(CDrawPort *pdp, void *pvUserData);

    // Multiply gravity acceleration of specific mod-independent brush entities
    void P_WorldBase_GetForce(INDEX iForce, const FLOAT3D &vPoint, CForceStrength &fsGravity, CForceStrength &fsField);
    void P_MovingBrush_GetForce(INDEX iForce, const FLOAT3D &vPoint, CForceStrength &fsGravity, CForceStrength &fsField);
};

class CRationalEntityPatch : public CRationalEntity {
  public:
    // Call a subautomaton
    void P_Call(SLONG slThisState, SLONG slTargetState, BOOL bOverride, const CEntityEvent &eeInput);
};

#endif // _PATCHCONFIG_EXTEND_ENTITIES

#endif
