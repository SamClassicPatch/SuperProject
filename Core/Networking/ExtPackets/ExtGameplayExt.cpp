/* Copyright (c) 2024-2026 Dreamy Cecil
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

// Find variable index by its name
int CExtGameplayExt::FindVar(const CTString &strVar) {
#if _PATCHCONFIG_GAMEPLAY_EXT

  if (strVar != "") {
    for (int i = 0; i < k_EGameplayExt_Max; i++) {
      if (strVar == IConfig::gex.props[i].strKey) return i + 1;
    }
  }

#endif // _PATCHCONFIG_GAMEPLAY_EXT

  ClassicsPackets_ServerReport(this, TRANS("Couldn't find index for '%s' gameplay extension!\n"), strVar.str_String);
  return 0;
};

// Set string value
void CExtGameplayExt::SetValue(const CTString &strVar, const CTString &str) {
  props["iVar"] = FindVar(strVar);
  props["value"] = str;
};

// Set number value
void CExtGameplayExt::SetValue(const CTString &strVar, DOUBLE f) {
  props["iVar"] = FindVar(strVar);
  props["value"] = f;
};

bool CExtGameplayExt::Write(CNetworkMessage &nm) {
#if _PATCHCONFIG_GAMEPLAY_EXT

  // It's not like there will ever be more than 16383 GEX variables,
  // plus if 'value' is 0.0, it'll all be neatly packed in just 2 bytes!
  UWORD iVar = props["iVar"].GetIndex();
  if (iVar == 0) return false;

  BOOL bString = (props["value"].GetType() == CAnyValue::E_VAL_STRING);
  nm.WriteBits(&iVar, 14);
  nm.WriteBits(&bString, 1);

  if (bString) {
    nm << props["value"].ToString();
  } else {
    INetCompress::Double(nm, props["value"].ToFloat());
  }

  return true;
#else
  return false;
#endif // _PATCHCONFIG_GAMEPLAY_EXT
};

void CExtGameplayExt::Read(CNetworkMessage &nm) {
#if _PATCHCONFIG_GAMEPLAY_EXT

  UWORD iVar = 0;
  BOOL bString = FALSE;

  nm.ReadBits(&iVar, 14);
  nm.ReadBits(&bString, 1);

  props["iVar"] = (int)iVar;

  if (bString) {
    CTString strValue;
    nm >> strValue;
    props["value"] = strValue;

  } else {
    DOUBLE fValue;
    INetDecompress::Double(nm, fValue);
    props["value"] = fValue;
  }

#endif // _PATCHCONFIG_GAMEPLAY_EXT
};

void CExtGameplayExt::Process(void) {
#if _PATCHCONFIG_GAMEPLAY_EXT

  // Invalid offset
  INDEX iGameplayExt = props["iVar"].GetIndex() - 1;
  if (iGameplayExt < 0 || iGameplayExt >= k_EGameplayExt_Max) return;

  IConfig::NamedValue &entry = IConfig::gex.props[iGameplayExt];
  CAnyValue::EType eType = entry.val.GetType();

  bool bString = (props["value"].GetType() == CAnyValue::E_VAL_STRING);

  // Got a number but expected a string
  if (!bString && eType == CAnyValue::E_VAL_STRING) {
    ClassicsPackets_ServerReport(this, TRANS("Expected a string value for '%s' gameplay extension!\n"), entry.strKey);
    return;

  // Got a string but expected a number
  } else if (bString && eType != CAnyValue::E_VAL_STRING) {
    ClassicsPackets_ServerReport(this, TRANS("Expected a number value for '%s' gameplay extension!\n"), entry.strKey);
    return;
  }

  // Set new value depending on type
  switch (eType) {
    case CAnyValue::E_VAL_BOOL:   entry.val.GetIndex()  = props["value"].GetDouble(); break;
    case CAnyValue::E_VAL_INDEX:  entry.val.GetIndex()  = props["value"].GetDouble(); break;
    case CAnyValue::E_VAL_FLOAT:  entry.val.GetFloat()  = props["value"].GetDouble(); break;
    case CAnyValue::E_VAL_STRING: entry.val.GetString() = props["value"].GetString(); break;
  }

#else
  ASSERTALWAYS(GAMEPLAY_EXT_ASSERT_MSG);
  ClassicsPackets_ServerReport(this, GAMEPLAY_EXT_ASSERT_MSG);
#endif // _PATCHCONFIG_GAMEPLAY_EXT
};

#endif // _PATCHCONFIG_EXT_PACKETS
