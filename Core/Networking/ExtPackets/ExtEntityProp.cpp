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

#include "Networking/ExtPackets.h"

#if _PATCHCONFIG_EXT_PACKETS

bool CExtEntityProp::Write(CNetworkMessage &nm) {
  WriteEntity(nm);

  BOOL bName = props["bName"].IsTrue();
  nm.WriteBits(&bName, 1);
  nm << (ULONG)props["ulProp"].GetIndex();

  BOOL bString = (props["value"].GetType() == CAnyValue::E_VAL_STRING);
  nm.WriteBits(&bString, 1);

  if (bString) {
    nm << props["value"].ToString();
  } else {
    INetCompress::Double(nm, props["value"].ToFloat());
  }

  return true;
};

void CExtEntityProp::Read(CNetworkMessage &nm) {
  ReadEntity(nm);

  BOOL bName = FALSE;
  nm.ReadBits(&bName, 1);
  props["bName"].GetIndex() = bName;

  ULONG ulProp;
  nm >> ulProp;
  props["ulProp"].GetIndex() = ulProp;

  BOOL bString = FALSE;
  nm.ReadBits(&bString, 1);

  if (bString) {
    CTString strValue;
    nm >> strValue;
    props["value"] = strValue;

  } else {
    DOUBLE fValue;
    INetDecompress::Double(nm, fValue);
    props["value"] = fValue;
  }
};

void CExtEntityProp::Process(void) {
  CEntity *pen = GetEntity();

  if (!EntityExists(pen)) return;

  CEntityProperty *pep = NULL;
  ULONG ulProp = props["ulProp"].GetIndex();

  if (props["bName"].IsTrue()) {
    pep = IWorld::PropertyForHash(pen, ulProp);
  } else {
    pep = IWorld::PropertyForId(pen, ulProp);
  }

  if (pep == NULL) return;

  INDEX iType = IProperties::ConvertType(pep->ep_eptType);

  bool bString = (props["value"].GetType() == CAnyValue::E_VAL_STRING);

  if (bString) {
    if (iType == CEntityProperty::EPT_STRING) {
      CTString &strValue = props["value"].GetString();
      IProperties::SetPropValue(pen, pep, &strValue);
    } else {
      ClassicsPackets_ServerReport(this, TRANS("Expected string property type but got %d\n"), iType);
    }

  } else if (iType == CEntityProperty::EPT_FLOAT) {
    FLOAT fFloatProp = props["value"].ToFloat();
    IProperties::SetPropValue(pen, pep, &fFloatProp);

  } else if (iType == CEntityProperty::EPT_INDEX) {
    INDEX iIntProp = props["value"].ToIndex();
    IProperties::SetPropValue(pen, pep, &iIntProp);

  } else {
    ClassicsPackets_ServerReport(this, TRANS("Expected number property type but got %d\n"), iType);
  }
};

#endif // _PATCHCONFIG_EXT_PACKETS
