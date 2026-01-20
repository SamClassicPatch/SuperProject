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

#include "StdH.h"

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

bool CExtEntityDamage::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  ULONG ulDamageType = props["eDamageType"].GetIndex();
  INetCompress::Integer(nm, ulDamageType);

  // Write damage amount up to 2 decimal places
  ULONG ulDamagePoints = ULONG(props["fDamage"].GetFloat()) * 100;
  INetCompress::Integer(nm, ulDamagePoints);
  return true;
};

void CExtEntityDamage::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  ULONG ulDamageType;
  INetDecompress::Integer(nm, ulDamageType);

  // Read damage amount
  ULONG ulDamagePoints;
  INetDecompress::Integer(nm, ulDamagePoints);

  props["eDamageType"].GetIndex() = ulDamageType;
  props["fDamage"].GetFloat() = FLOAT(ulDamagePoints) * 0.01f;
};

bool CExtEntityDirectDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  INetCompress::Integer(nm, props["ulTarget"].GetIndex());
  INetCompress::Float3D(nm, props["vHitPoint"].GetVector());
  INetCompress::Float3D(nm, props["vDirection"].GetVector());
  return true;
};

void CExtEntityDirectDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  ULONG ulTarget;
  INetDecompress::Integer(nm, ulTarget);
  props["ulTarget"].GetIndex() = ulTarget;

  INetDecompress::Float3D(nm, props["vHitPoint"].GetVector());
  INetDecompress::Float3D(nm, props["vDirection"].GetVector());
};

void CExtEntityDirectDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  const ULONG ulTarget = props["ulTarget"].GetIndex();
  CEntity *penTarget = FindExtEntity(ulTarget);

  if (penTarget == NULL) return;

  const DamageType eDamageType = (DamageType)props["eDamageType"].GetIndex();
  const FLOAT fDamage = props["fDamage"].GetFloat();
  const FLOAT3D vHitPoint = props["vHitPoint"].GetVector();
  const FLOAT3D vDirection = props["vDirection"].GetVector();

  pen->InflictDirectDamage(penTarget, pen, eDamageType, fDamage, vHitPoint, vDirection);

  ClassicsPackets_ServerReport(this, TRANS("Entity %u inflicted %.2f damage to entity %u\n"), pen->en_ulID, fDamage, penTarget->en_ulID);
};

bool CExtEntityRangeDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  INetCompress::Float3D(nm, props["vCenter"].GetVector());

  ULONG ulRange = ULONG(props["fFallOff"].GetFloat()) * 10;
  INetCompress::Integer(nm, ulRange);

  ulRange = ULONG(props["fHotSpot"].GetFloat()) * 10;
  INetCompress::Integer(nm, ulRange);
  return true;
};

void CExtEntityRangeDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  INetDecompress::Float3D(nm, props["vCenter"].GetVector());

  ULONG ulRange;
  INetDecompress::Integer(nm, ulRange);
  props["fFallOff"].GetFloat() = FLOAT(ulRange) / 10.0f;

  INetDecompress::Integer(nm, ulRange);
  props["fHotSpot"].GetFloat() = FLOAT(ulRange) / 10.0f;
};

void CExtEntityRangeDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  const DamageType eDamageType = (DamageType)props["eDamageType"].GetIndex();
  const FLOAT fDamage = props["fDamage"].GetFloat();
  const FLOAT3D vCenter = props["vCenter"].GetVector();
  const FLOAT fFallOff = props["fFallOff"].GetFloat();
  const FLOAT fHotSpot = props["fHotSpot"].GetFloat();

  pen->InflictRangeDamage(pen, eDamageType, fDamage, vCenter, fHotSpot, fFallOff);

  ClassicsPackets_ServerReport(this, TRANS("Entity %u inflicted %.2f damage in a %.1f range\n"), pen->en_ulID, fDamage, fFallOff);
};

bool CExtEntityBoxDamage::Write(CNetworkMessage &nm) {
  CExtEntityDamage::Write(nm);

  const FLOATaabbox3D &boxArea = props["boxArea"].GetBox();
  INetCompress::Float3D(nm, boxArea.minvect);
  INetCompress::Float3D(nm, boxArea.maxvect);
  return true;
};

void CExtEntityBoxDamage::Read(CNetworkMessage &nm) {
  CExtEntityDamage::Read(nm);

  FLOATaabbox3D &boxArea = props["boxArea"].GetBox();
  INetDecompress::Float3D(nm, boxArea.minvect);
  INetDecompress::Float3D(nm, boxArea.maxvect);
};

void CExtEntityBoxDamage::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  const DamageType eDamageType = (DamageType)props["eDamageType"].GetIndex();
  const FLOAT fDamage = props["fDamage"].GetFloat();
  const FLOATaabbox3D &boxArea = props["boxArea"].GetBox();

  pen->InflictBoxDamage(pen, eDamageType, fDamage, boxArea);

  ClassicsPackets_ServerReport(this, TRANS("Entity %u inflicted %.2f damage in a [%.2f, %.2f, %.2f] sized area\n"),
    pen->en_ulID, fDamage, boxArea.Size()(1), boxArea.Size()(2), boxArea.Size()(3));
};

#endif // _PATCHCONFIG_EXT_PACKETS
