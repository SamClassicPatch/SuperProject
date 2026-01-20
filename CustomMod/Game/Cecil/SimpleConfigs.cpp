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

#include "StdAfx.h"

#include "SimpleConfigs.h"

static __forceinline void FixupFileName_t(CTString &strPath)
{
  if (!strPath.RemovePrefix(CTString("TF") + "NM ")) {
    ThrowF_t(LOCALIZE("Expected %s%s before filename"), "TF", "NM");
  }
};

void CSimpleConfig::AddProcessor(const char *strProp, CSimpleConfig::ProcessorFunc pFunc) {
  CSimpleConfig::PropertyProcessor &proc = _aProcessors.Push();
  proc.strProp = strProp;
  proc.pFunc = pFunc;
};

CSimpleConfig::ProcessorFunc CSimpleConfig::FindProcessor(const char *strProp) {
  INDEX ct = _aProcessors.Count();

  while (--ct >= 0) {
    if (_aProcessors[ct].strProp == strProp) return _aProcessors[ct].pFunc;
  }

  return NULL;
};

void CSimpleConfig::Open_t(const CTString &strConfigFile) {
  // Close last opened file
  _strmFile.Close();

  _strConfig = strConfigFile;
  _strmFile.Open_t(strConfigFile);
  _ctLine = 0;
};

void CSimpleConfig::Close(void) {
  _strConfig = "";
  _strmFile.Close();
  _ctLine = 0;
};

CTString CSimpleConfig::GetNonEmptyLine_t(void) {
  CTString str;

  while (!_strmFile.AtEOF()) {
    _strmFile.GetLine_t(str);
    _ctLine++;

    str.TrimSpacesLeft();
    
    if (str.HasPrefix("//")) continue;
    
    if (str != "") {
      str.TrimSpacesRight();
      return str;
    }
  }

  ThrowF_t(LOCALIZE("Unexpected end of file"));
  return "";
};

void CSimpleConfig::SkipBlock_t(void)
{
  CTString strLine = GetNonEmptyLine_t();

  if (strLine != "{") ThrowF_t(LOCALIZE("Expected '{'"));

  // Start with one open block
  INDEX ctLevel = 1;

  // Count curly braces until all blocks are closed
  do {
    strLine = GetNonEmptyLine_t();

    if (strLine == "{") {
      ctLevel++;

    } else if (strLine == "}") {
      ctLevel--;
    }

  } while (ctLevel > 0);
};

BOOL CSimpleConfig::ProcessProperty(const CTString &strProp, CTString &strValue)
{
  ProcessorFunc pCustom = FindProcessor(strProp);
  if (pCustom == NULL) return FALSE;

  pCustom(*this, strValue);
  return TRUE;
};

void CSimpleConfig::Parse_t(void) {
  CTString strLine = GetNonEmptyLine_t();

  if (strLine != "{") ThrowF_t(LOCALIZE("Expected '{'"));

  while (TRUE) {
    strLine = GetNonEmptyLine_t();

    if (strLine == "}") return;

    // Extract keyword for the property name
    CTString strKeyword = strLine;
    char *pchLine = strKeyword.str_String;

    while (*pchLine != '\0') {
      if (*pchLine == ':' || *pchLine == ' ') {
        *pchLine = '\0';
        break;
      }

      pchLine++;
    }

    // Remove property from the line (with the separator)
    strLine.TrimLeft(strLine.Length() - (pchLine - strKeyword.str_String) - 1);
    strLine.TrimSpacesLeft();

    ProcessProperty(strKeyword, strLine);
  }
};

BOOL CModelConfig::ProcessProperty(const CTString &strProp, CTString &strValue)
{
  if (strProp == "PreviewOnly") {
    if (_bPreview) {
      Parse_t();

    } else {
      SkipBlock_t();
    }

  } else if (strProp == "Include") {
    FixupFileName_t(strValue);

    CModelConfig cfgInclude(*this);
    cfgInclude.Open_t(strValue);
    cfgInclude.Parse_t();

  } else if (strProp == "Model") {
    FixupFileName_t(strValue);
    _pmo->SetData_t(strValue);

  } else if (strProp == "Texture") {
    FixupFileName_t(strValue);
    _pmo->mo_toTexture.SetData_t(strValue);

  } else if (strProp == "Specular") {
    FixupFileName_t(strValue);
    _pmo->mo_toSpecular.SetData_t(strValue);

  } else if (strProp == "Reflection") {
    FixupFileName_t(strValue);
    _pmo->mo_toReflection.SetData_t(strValue);

  } else if (strProp == "Bump") {
    FixupFileName_t(strValue);
    _pmo->mo_toBump.SetData_t(strValue);

  } else if (strProp == "Animation") {
    INDEX iAnim = -1;
    strValue.ScanF("%d", &iAnim);

    if (iAnim < 0) {
      ThrowF_t(LOCALIZE("Invalid animation number"));
    }

    if (iAnim >= _pmo->GetAnimsCt()) {
      ThrowF_t(LOCALIZE("Animation %d does not exist in that model"), iAnim);
    }

    _pmo->PlayAnim(iAnim, AOF_LOOPING);

  } else if (strProp == "Stretch") {
    FLOAT3D vSize(1, 1, 1);
    strValue.ScanF("%g;%g;%g", &vSize(1), &vSize(2), &vSize(3));
    _pmo->StretchModelRelative(vSize);

  } else if (strProp == "Offset") {
    // Ignore offset if not an attachment
    if (_pamo != NULL) {
      FLOAT3D vOffset(0, 0, 0);
      strValue.ScanF("%g;%g;%g", &vOffset(1), &vOffset(2), &vOffset(3));
      _pamo->amo_plRelative.pl_PositionVector = vOffset;
    }

  } else if (strProp == "Rotate") {
    // Ignore offset if not an attachment
    if (_pamo != NULL) {
      FLOAT3D vOffset(0, 0, 0);
      strValue.ScanF("%g;%g;%g", &vOffset(1), &vOffset(2), &vOffset(3));
      _pamo->amo_plRelative.pl_OrientationAngle = vOffset;
    }

  } else if (strProp == "Blend") {
    COLOR col;
    strValue.ScanF("%i", &col);
    _pmo->mo_colBlendColor = col;

  } else if (strProp == "Attachment") {
    INDEX iIndex = -1;
    strValue.ScanF("%d", &iIndex);

    if (iIndex < 0) {
      ThrowF_t(LOCALIZE("Invalid attachment number"));
    }

    CModelData *pmd = (CModelData *)_pmo->GetData();

    if (iIndex >= pmd->md_aampAttachedPosition.Count()) {
      ThrowF_t(LOCALIZE("Attachment %d does not exist in that model"), iIndex);
    }

    CModelObject *pmoRestore = _pmo;
    CAttachmentModelObject *pamoRestore = _pamo;

    // Find existing attachment or add a new one
    _pamo = _pmo->GetAttachmentModel(iIndex);
    if (_pamo == NULL) _pamo = _pmo->AddAttachmentModel(iIndex);

    _pmo = &_pamo->amo_moModelObject;

    Parse_t();

    _pmo = pmoRestore;
    _pamo = pamoRestore;

  } else if (!CSimpleConfig::ProcessProperty(strProp, strValue)) {
    ThrowF_t(TRANS("Unknown property '%s'"), strProp.str_String);
  }

  return TRUE;
};

BOOL CModelConfig::SetModel(CModelConfig &cfg, const CTString &strConfigFile, CTString &strName)
{
  try {
    cfg.Open_t(strConfigFile);

    const SLONG slBeforeName = cfg._strmFile.GetPos_t();
    CTString strLine = cfg.GetNonEmptyLine_t();

    // Read optional name
    if (strLine.RemovePrefix("Name:")) {
      strName = strLine;
      strName.TrimSpacesLeft();

    } else {
      strName = "<unnamed>";
      cfg._strmFile.SetPos_t(slBeforeName);
    }

    cfg.Parse_t();
    return TRUE;

  } catch (char *strError) {
    CPrintF(TRANS("Cannot load model config:\n%s (%d) : %s\n"), strConfigFile.str_String, cfg._ctLine, strError);
  }

  return FALSE;
};
