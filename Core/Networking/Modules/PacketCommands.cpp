/* Copyright (c) 2023-2024 Dreamy Cecil
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

#if _PATCHCONFIG_EXT_PACKETS

#include "PacketCommands.h"
#include "Networking/ExtPackets.h"

#define VANILLA_EVENTS_ENTITY_ID
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

namespace IPacketCommands {

// Preconfigured event for packet commands
static EExtEntityEvent _eePacketEvent;
static ULONG _ctPacketEventFields = 0;

// Begin event setup of a specific type
void SetupEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEventType = NEXT_ARG(INDEX);

  // Reset event
  _eePacketEvent.Reset();
  _ctPacketEventFields = 0;

  // Set new type
  _eePacketEvent.ee_slEvent = iEventType;
};

// Set event field to an integer
void EventFieldIndex(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  INDEX iValue = NEXT_ARG(INDEX);

  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (INDEX &)_eePacketEvent.aulFields[iField] = iValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set event field to a float
void EventFieldFloat(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  FLOAT fValue = NEXT_ARG(FLOAT);

  ASSERT(iField >= 0 && iField < 64);
  iField = Clamp(iField, (INDEX)0, (INDEX)63);

  // Use as many fields as the set one
  (FLOAT &)_eePacketEvent.aulFields[iField] = fValue;
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Set three event fields to vector values
void EventFieldVector(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iField = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  ASSERT(iField >= 0 && iField < 62);
  iField = Clamp(iField, (INDEX)0, (INDEX)61);

  // Use as many fields as the set one
  (FLOAT3D &)_eePacketEvent.aulFields[iField] = FLOAT3D(fX, fY, fZ);
  _ctPacketEventFields = Max(_ctPacketEventFields, ULONG(iField + 1));
};

// Create entity from the list
void EntityCreate(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strClass = *NEXT_ARG(CTString *);

  CExtEntityCreate pck;
  pck("fnmClass", "Classes\\" + strClass + ".ecl");
  pck.SendToClients();
};

// Destroy entities
void EntityDelete(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iSameClass = NEXT_ARG(INDEX);

  CExtEntityDelete pck;
  pck("ulEntity", (int)iEntity);
  pck("bSameClass", bool(iSameClass != 0));
  pck.SendToClients();
};

// Copy entity
void EntityCopy(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iCopies = NEXT_ARG(INDEX);

  CExtEntityCopy pck;
  pck("ulEntity", (int)iEntity);
  pck("iCopies", Clamp(iCopies, (INDEX)0, (INDEX)31));
  pck.SendToClients();
};

// Send set up event to an entity
void EntityEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityEvent pck;
  pck("ulEntity", (int)iEntity);
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendToClients();
};

// Receive item by an entity via a set up event
void EntityItem(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityItem pck;
  pck("ulEntity", (int)iEntity);
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendToClients();
};

// Initialize entity
void EntityInit(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityInit pck;
  pck("ulEntity", (int)iEntity);
  pck.SetEvent(EVoid(), sizeof(EVoid));
  pck.SendToClients();
};

// Initialize entity with preset event
void EntityInitEvent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);

  CExtEntityInit pck;
  pck("ulEntity", (int)iEntity);
  pck.Copy(_eePacketEvent, _ctPacketEventFields);
  pck.SendToClients();
};

// Set new entity position
void EntitySetPos(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityPosition pck;
  pck("ulEntity", (int)iEntity);
  pck("vSet", FLOAT3D(fX, fY, fZ));
  pck("bRotation", false);
  pck("bRelative", bool(iRelative != 0));
  pck.SendToClients();
};

// Set new entity rotation
void EntitySetRot(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityPosition pck;
  pck("ulEntity", (int)iEntity);
  pck("vSet", FLOAT3D(fH, fP, fB));
  pck("bRotation", true);
  pck("bRelative", bool(iRelative != 0));
  pck.SendToClients();
};

// Set new entity placement
void EntityTeleport(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);
  INDEX iRelative = NEXT_ARG(INDEX);

  CExtEntityTeleport pck;
  pck("ulEntity", (int)iEntity);
  pck("plSet", CPlacement3D(FLOAT3D(fX, fY, fZ), ANGLE3D(fH, fP, fB)));
  pck("bRelative", bool(iRelative != 0));
  pck.SendToClients();
};

// Parent entity
void EntityParent(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iParent = NEXT_ARG(INDEX);

  CExtEntityParent pck;
  pck("ulEntity", (int)iEntity);
  pck("ulParent", (int)iParent);
  pck.SendToClients();
};

// Change number property by name or ID
void EntityNumberProp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  const CTString &strProp = *NEXT_ARG(CTString *);
  INDEX iPropID = NEXT_ARG(INDEX);
  FLOAT fValue = NEXT_ARG(FLOAT);

  CExtEntityProp pck;
  pck("ulEntity", (int)iEntity);

  if (strProp != "") {
    pck.SetProperty(strProp);
  } else {
    pck.SetProperty(iPropID);
  }

  pck.SetValue(fValue);
  pck.SendToClients();
};

// Change string property by name or ID
void EntityStringProp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  const CTString &strProp = *NEXT_ARG(CTString *);
  INDEX iPropID = NEXT_ARG(INDEX);
  const CTString &strValue = *NEXT_ARG(CTString *);

  CExtEntityProp pck;
  pck("ulEntity", (int)iEntity);

  if (strProp != "") {
    pck.SetProperty(strProp);
  } else {
    pck.SetProperty(iPropID);
  }

  pck.SetValue(strValue);
  pck.SendToClients();
};

// Change entity health
void EntityHealth(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fHealth = NEXT_ARG(FLOAT);

  CExtEntityHealth pck;
  pck("ulEntity", (int)iEntity);
  pck("fHealth", fHealth);
  pck.SendToClients();
};

// Change entity flags
void EntityFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.EntityFlags(iFlags, (iRemove != 0));
  pck.SendToClients();
};

// Change physical flags
void EntityPhysicalFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.PhysicalFlags(iFlags, (iRemove != 0));
  pck.SendToClients();
};

// Change collision flags
void EntityCollisionFlags(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  INDEX iRemove = NEXT_ARG(INDEX);

  CExtEntityFlags pck;
  pck("ulEntity", (int)iEntity);
  pck.CollisionFlags(iFlags, (iRemove != 0));
  pck.SendToClients();
};

// Set movement speed
void EntityMove(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  CExtEntityMove pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", FLOAT3D(fX, fY, fZ));
  pck.SendToClients();
};

// Set rotation speed
void EntityRotate(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fH = NEXT_ARG(FLOAT);
  FLOAT fP = NEXT_ARG(FLOAT);
  FLOAT fB = NEXT_ARG(FLOAT);

  CExtEntityRotate pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", FLOAT3D(fH, fP, fB));
  pck.SendToClients();
};

// Give impulse in an absolute direction
void EntityImpulse(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iEntity = NEXT_ARG(INDEX);
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);

  CExtEntityImpulse pck;
  pck("ulEntity", (int)iEntity);
  pck("vSpeed", FLOAT3D(fX, fY, fZ));
  pck.SendToClients();
};

static INDEX _iDamageSetup = -1;

static ULONG _ulDamageInflictor = -1;
static ULONG _ulDamageTarget = -1;
static ULONG _ulDamageType = DMT_NONE;
static FLOAT _fDamageAmount = 0.0f;

static FLOAT3D _vDamageVec1(0, 0, 0);
static FLOAT3D _vDamageVec2(0, 0, 0);

// Begin damage setup with an inflictor
void SetupDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iInflictor = NEXT_ARG(INDEX);
  INDEX iType = NEXT_ARG(INDEX);
  FLOAT fDamage = NEXT_ARG(FLOAT);

  _iDamageSetup = -1;

  _ulDamageInflictor = iInflictor;
  _ulDamageType = iType;
  _fDamageAmount = fDamage;
};

// Setup direct damage (target, hit point, direction)
void SetDirectDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iTarget = NEXT_ARG(INDEX);
  FLOAT fHitX = NEXT_ARG(FLOAT);
  FLOAT fHitY = NEXT_ARG(FLOAT);
  FLOAT fHitZ = NEXT_ARG(FLOAT);
  FLOAT fDirX = NEXT_ARG(FLOAT);
  FLOAT fDirY = NEXT_ARG(FLOAT);
  FLOAT fDirZ = NEXT_ARG(FLOAT);

  _iDamageSetup = 0;

  _ulDamageTarget = iTarget;
  _vDamageVec1 = FLOAT3D(fHitX, fHitY, fHitZ);
  _vDamageVec2 = FLOAT3D(fDirX, fDirY, fDirZ);
};

// Setup range damage (hit center, fall off, hot spot)
void SetRangeDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  FLOAT fX = NEXT_ARG(FLOAT);
  FLOAT fY = NEXT_ARG(FLOAT);
  FLOAT fZ = NEXT_ARG(FLOAT);
  FLOAT fFallOff = NEXT_ARG(FLOAT);
  FLOAT fHotSpot = NEXT_ARG(FLOAT);

  _iDamageSetup = 1;

  _vDamageVec1 = FLOAT3D(fX, fY, fZ);
  _vDamageVec2 = FLOAT3D(fFallOff, fHotSpot, 0.0f);
};

// Setup box damage (min corner, max corner)
void SetBoxDamage(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  FLOAT fX1 = NEXT_ARG(FLOAT);
  FLOAT fY1 = NEXT_ARG(FLOAT);
  FLOAT fZ1 = NEXT_ARG(FLOAT);
  FLOAT fX2 = NEXT_ARG(FLOAT);
  FLOAT fY2 = NEXT_ARG(FLOAT);
  FLOAT fZ2 = NEXT_ARG(FLOAT);

  _iDamageSetup = 2;

  _vDamageVec1 = FLOAT3D(fX1, fY1, fZ1);
  _vDamageVec2 = FLOAT3D(fX2, fY2, fZ2);
};

// Inflict set up damage
void EntityDamage(void) {
  switch (_iDamageSetup) {
    case 0: {
      CExtEntityDirectDamage pck;
      pck("ulEntity", (int)_ulDamageInflictor);
      pck("eDamageType", (int)_ulDamageType);
      pck("fDamage", _fDamageAmount);

      pck("ulTarget", (int)_ulDamageTarget);
      pck("vHitPoint", _vDamageVec1);
      pck("vDirection", _vDamageVec2);
      pck.SendToClients();
    } break;

    case 1: {
      CExtEntityRangeDamage pck;
      pck("ulEntity", (int)_ulDamageInflictor);
      pck("eDamageType", (int)_ulDamageType);
      pck("fDamage", _fDamageAmount);

      pck("vCenter", _vDamageVec1);
      pck("fFallOff", _vDamageVec2(1));
      pck("fHotSpot", _vDamageVec2(2));
      pck.SendToClients();
    } break;

    case 2: {
      CExtEntityBoxDamage pck;
      pck("ulEntity", (int)_ulDamageInflictor);
      pck("eDamageType", (int)_ulDamageType);
      pck("fDamage", _fDamageAmount);

      pck("boxArea", FLOATaabbox3D(_vDamageVec1, _vDamageVec2));
      pck.SendToClients();
    } break;
  }
};

// Change level using WorldLink
void ChangeLevel(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strWorld = *NEXT_ARG(CTString *);

  CExtChangeLevel pck;
  pck("strWorld", strWorld);
  pck.SendToClients();
};

// Force immediate world change
void ChangeWorld(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strWorld = *NEXT_ARG(CTString *);

  CExtChangeWorld pck;
  pck("strWorld", strWorld);
  pck.SendToClients();
};

// Preconfigured session properties to change
static CExtSessionProps _pckSesProps;

// Begin session properties setup from specific offset
void StartSesProps(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  _pckSesProps.GetOffset() = NEXT_ARG(INDEX);
  _pckSesProps.GetSize() = 0;
};

// Set integer in session properties
void SesPropIndex(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iValue = NEXT_ARG(INDEX);
  _pckSesProps.AddData(&iValue, sizeof(iValue));
};

// Set float number in session properties
void SesPropFloat(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  FLOAT fValue = NEXT_ARG(FLOAT);
  _pckSesProps.AddData(&fValue, sizeof(fValue));
};

// Set string characters in session properties
void SesPropString(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strValue = *NEXT_ARG(CTString *);
  _pckSesProps.AddData(strValue.str_String, strValue.Length());
};

// Skip setting specific bytes
void SeekSesProp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iBytes = NEXT_ARG(INDEX);

  INDEX &iSize = _pckSesProps.GetSize();
  iSize = ClampDn(iSize + iBytes, (INDEX)0);
};

// Send previously set session properties
void SendSesProps(void) {
  _pckSesProps.SendToClients();
};

// Change number value of some gameplay extension
void GameplayExtNumber(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strVar = *NEXT_ARG(CTString *);
  FLOAT fValue = NEXT_ARG(FLOAT);

  CExtGameplayExt pck;
  pck.SetValue(strVar, fValue);
  pck.SendToClients();
};

// Change string value of some gameplay extension
void GameplayExtString(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strVar = *NEXT_ARG(CTString *);
  const CTString &strValue = *NEXT_ARG(CTString *);

  CExtGameplayExt pck;
  pck.SetValue(strVar, strValue);
  pck.SendToClients();
};

// Play global sound with minimal setup
void PlayQuickSound(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strFile = *NEXT_ARG(CTString *);
  INDEX iChannel = NEXT_ARG(INDEX);
  FLOAT fVolume = NEXT_ARG(FLOAT);

  CExtPlaySound pck;
  pck("strFile", strFile);
  pck("iChannel", (int)iChannel);
  pck("fVolumeL", fVolume);
  pck("fVolumeR", fVolume);
  pck.SendToClients();
};

// Play global sound with full setup
void PlayGlobalSound(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strFile = *NEXT_ARG(CTString *);
  INDEX iChannel = NEXT_ARG(INDEX);
  INDEX iFlags = NEXT_ARG(INDEX);
  FLOAT fDelay = NEXT_ARG(FLOAT);
  FLOAT fOffset = NEXT_ARG(FLOAT);

  FLOAT fVolume = NEXT_ARG(FLOAT);
  FLOAT fFilter = NEXT_ARG(FLOAT);
  FLOAT fPitch = NEXT_ARG(FLOAT);

  CExtPlaySound pck;
  pck("strFile", strFile);
  pck("iChannel", (int)iChannel);
  pck("ulFlags", (int)iFlags);
  pck("fDelay", fDelay);
  pck("fOffset", fOffset);

  pck("fVolumeL", fVolume);
  pck("fVolumeR", fVolume);
  pck("fFilterL", fFilter);
  pck("fFilterR", fFilter);
  pck("fPitch", fPitch);
  pck.SendToClients();
};

}; // namespace

#endif // _PATCHCONFIG_EXT_PACKETS
