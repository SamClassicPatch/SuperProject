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

#ifndef CECIL_INCL_RESOURCEFUNCTIONS_H
#define CECIL_INCL_RESOURCEFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Templates/Stock_CMesh.h>
#include <Engine/Templates/Stock_CSkeleton.h>
#include <Engine/Templates/Stock_CAnimSet.h>
#include <Engine/Templates/Stock_CTextureData.h>

// File dialog filters
#define FILTER_ALL "All files (*.*)\0*.*\0"
#define FILTER_TEX "Textures (*.tex)\0*.tex\0"
#define FILTER_MDL "Models (*.mdl)\0*.mdl\0"
#define FILTER_ANI "Animations (*.ani)\0*.ani\0"
#define FILTER_END "\0"

// Interface of methods for resource serialization
namespace IRes {

// Open file dialog for selecting a new file for a replacement
inline void CallFileRequester(CTFileName &fnmReplacement, char *achrTitle, char *achrSelectedFile, char *pFilter) {
  const CTString strLibPath = IDir::AppPath() + IDir::FullLibPath("EngineGUI");
  fnmReplacement = CTString("");

  // Try loading the library
  HMODULE hLib = GetModuleHandleA(strLibPath.str_String);

  if (hLib == NULL) {
    WarningMessage(LOCALIZE("Cannot load %s:\n%s\nCannot replace files!"), strLibPath.str_String, GetWindowsError(GetLastError()));
    return;
  }

  // Try calling the file requester
  typedef CTFileName (*CFileRequester)(char *, char *, char *, char *);
  CFileRequester pFileRequester = (CFileRequester)GetProcAddress(hLib, "?FileRequester@@YA?AVCTFileName@@PAD000@Z");

  if (pFileRequester == NULL) {
    WarningMessage(LOCALIZE("Error in %s:\nFileRequester() function not found\nCannot replace files!"), strLibPath.str_String);
    return;
  }

  fnmReplacement = pFileRequester(achrTitle, pFilter, "Replace file directory", achrSelectedFile);
};

// Get replacement for some non-existing file
inline BOOL ReplaceFile(const CTFileName &fnmSourceFile, CTFileName &fnmReplacement, char *pFilter) {
  // Peek into the base file without adding a new replacement entry in-game
  const BOOL bReadOnly = !ClassicsCore_IsEditorApp();

  if (!bReadOnly) {
    static CSymbolPtr pbUseBase("wed_bUseBaseForReplacement");

    // Don't replace files
    if (!pbUseBase.Exists() || !pbUseBase.GetIndex()) return FALSE;
  }

  const CTString strBaseFile("Data\\BaseForReplacingFiles.txt");

  // Try to find a replacement in the base file
  try {
    char strLine[256];
    char strSource[256];
    char strRemap[256];

    // Read list with file remaps
    CTFileStream strm;
    strm.Open_t(strBaseFile);

    while (!strm.AtEOF()) {
      IData::GetLineFromStream_t(strm, strLine, 256);
      sscanf(strLine, "\"%[^\"]\" \"%[^\"]\"", strSource, strRemap);

      // Skip remapping to itself
      if (CTString(strSource) ==  strRemap) continue;

      // Get remapped file
      if (strSource == fnmSourceFile) {
        fnmReplacement = CTString(strRemap);
        return TRUE;
      }
    }

  } catch (char *strError) {
    (void)strError;
  }

  // No replacement found
  if (bReadOnly) return FALSE;

  // Call file requester for substituting the file
  const CTString strTitle(0, LOCALIZE("For:\"%s\""), fnmSourceFile.str_String);
  const CTString strDefaultFile = fnmSourceFile.FileName() + fnmSourceFile.FileExt();

  IRes::CallFileRequester(fnmReplacement, strTitle.str_String, strDefaultFile.str_String, pFilter);

  // No replacement file
  if (fnmReplacement == "") return FALSE;

  // Try adding new remapping at the end
  try {
    CTString strBase;

    if (FileExists(strBaseFile)) {
      strBase.Load_t(strBaseFile);
    }

    strBase += CTString(0, "\"%s\" \"%s\"\n", fnmSourceFile.str_String, fnmReplacement.str_String);
    strBase.Save_t(strBaseFile);

  } catch (char *strError) {
    WarningMessage("%s", strError);
    return FALSE;
  }

  return TRUE;
};

// Set data to some animation object from a file
template<class Type> inline
void SetDataFromFile_t(Type &object, CTFileName &fnmData, char *strFilters)
{
  while (TRUE) {
    try {
      object.SetData_t(fnmData);
      break;

    } catch (char *strError) {
      (void)strError;

      // Find replacement file
      CTFileName fnmReplacement;

      if (IRes::ReplaceFile(fnmData, fnmReplacement, strFilters)) {
        fnmData = fnmReplacement;

      } else {
        ThrowF_t(LOCALIZE("Cannot find substitution for \"%s\""), fnmData.str_String);
      }
    }
  }
};

namespace Anims {

// Reimplementation of CAnimObject::Write_t()
inline void WriteClass_t(CAnimObject &ao, CTStream &strm) {
  strm.WriteID_t("ANOB");

  strm << (FLOAT)ao.ao_tmAnimStart;
  strm << (INDEX)ao.ao_iCurrentAnim;
  strm << (INDEX)ao.ao_iLastAnim;
  strm << (INDEX)ao.ao_ulFlags;
};

// Reimplementation of CAnimObject::Read_t()
inline void ReadClass_t(CAnimObject &ao, CTStream &strm) {
  INDEX iDummy;

  if (strm.PeekID_t() == CChunkID("ANOB")) {
    strm.ExpectID_t("ANOB");

    FLOAT fDummy;
    strm >> fDummy;
    ao.ao_tmAnimStart = fDummy;

    strm >> iDummy;
    ao.ao_iCurrentAnim = iDummy;

    strm >> iDummy;
    ao.ao_iLastAnim = iDummy;

    strm >> iDummy;
    ao.ao_ulFlags = iDummy;

  } else {
    FLOAT fDummy;
    strm >> fDummy;
    ao.ao_tmAnimStart = fDummy;

    strm >> iDummy;
    ao.ao_iCurrentAnim = iDummy;

    ao.ao_iLastAnim = ao.ao_iCurrentAnim;
    ao.ao_ulFlags = 0;
  }

  // Clamp animation
  if (ao.ao_AnimData == NULL || ao.ao_iCurrentAnim >= ao.GetAnimsCt()) {
    ao.ao_iCurrentAnim = 0;
  }

  if (ao.ao_AnimData == NULL || ao.ao_iLastAnim >= ao.GetAnimsCt()) {
    ao.ao_iLastAnim = 0;
  }
};

inline void Read_t(CTStream &strm, CAnimObject &ao) {
  // Read animation file
  CTFileName fnmAnim;
  strm >> fnmAnim;

  // Try to set it
  SetDataFromFile_t(ao, fnmAnim, FILTER_ANI FILTER_END);

  // Read animation object
  ReadClass_t(ao, strm);
};

inline void Skip_t(CTStream &strm) {
  // Skip animation file and animation object
  CTFileName fnmDummy;
  strm >> fnmDummy;

  CAnimObject aoDummy;
  ReadClass_t(aoDummy, strm);
};

inline void Write_t(CTStream &strm, CAnimObject &ao) {
  // Write animation file
  CAnimData *pad = (CAnimData *)ao.GetData();

  if (pad != NULL) {
    strm << pad->GetName();
  } else {
    strm << CTFileName(CTString(""));
  }

  // Write animation object
  WriteClass_t(ao, strm);
};

}; // Anims namespace

namespace Textures {

inline void Read_t(CTStream &strm, CTextureObject &to) {
  // Read texture file
  CTFileName fnmTexture;
  strm >> fnmTexture;

  // Try to set it
  SetDataFromFile_t(to, fnmTexture, FILTER_TEX FILTER_END);

  // Read texture object
  to.Read_t(&strm);
};

inline void Skip_t(CTStream &strm) {
  // Skip texture file and texture object
  CTFileName fnmDummy;
  strm >> fnmDummy;

  CTextureObject toDummy;
  toDummy.Read_t(&strm);
};

inline void Write_t(CTStream &strm, CTextureObject &to) {
  // Write texture file
  CTextureData *ptd = (CTextureData *)to.GetData();

  if (ptd != NULL) {
    strm << ptd->GetName();
  } else {
    strm << CTFileName(CTString(""));
  }

  // Write texture object
  to.Write_t(&strm);
};

}; // Textures namespace

namespace Models {

inline void Skip_t(CTStream &strm);

inline void Read_t(CTStream &strm, CModelObject &mo) {
  // Read model file
  CTFileName fnModel;
  strm >> fnModel;

  // Try to set it
  SetDataFromFile_t(mo, fnModel, FILTER_MDL FILTER_END);

  // Read model object
  mo.Read_t(&strm);

  // Read multiple textures
  if (strm.PeekID_t() == CChunkID("MTEX")) {
    strm.ExpectID_t("MTEX");

    IRes::Textures::Read_t(strm, mo.mo_toTexture);
    IRes::Textures::Read_t(strm, mo.mo_toBump);
    IRes::Textures::Read_t(strm, mo.mo_toReflection);
    IRes::Textures::Read_t(strm, mo.mo_toSpecular);

  // Read main texture
  } else {
    IRes::Textures::Read_t(strm, mo.mo_toTexture);
  }

  // Read attachments
  if (strm.PeekID_t() == CChunkID("ATCH")) {
    strm.ExpectID_t("ATCH");

    INDEX ctAttachments;
    strm >> ctAttachments;

    while (--ctAttachments >= 0) {
      // Read attachment position
      INDEX iPosition;
      strm >> iPosition;

      // Add new attachment
      CAttachmentModelObject *pamo = mo.AddAttachmentModel(iPosition);

      if (pamo != NULL) {
        // Read placement and model
        strm >> pamo->amo_plRelative;
        Read_t(strm, pamo->amo_moModelObject);

      } else {
        // Skip placement and model
        CPlacement3D plDummy;
        strm >> plDummy;
        Skip_t(strm);
      }
    }
  }
};

void Skip_t(CTStream &strm) {
  // Skip model file and model object
  CTFileName fnmDummy;
  strm >> fnmDummy;

  CModelObject moDummy;
  moDummy.Read_t(&strm);

  // Skip mutliple textures
  if (strm.PeekID_t() == CChunkID("MTEX")) {
    strm.ExpectID_t("MTEX");

    IRes::Textures::Skip_t(strm);
    IRes::Textures::Skip_t(strm);
    IRes::Textures::Skip_t(strm);
    IRes::Textures::Skip_t(strm);

  // Skip main texture
  } else {
    IRes::Textures::Skip_t(strm);
  }

  // Skip attachments
  if (strm.PeekID_t() == CChunkID("ATCH")) {
    strm.ExpectID_t("ATCH");

    INDEX ctAttachments;
    strm >> ctAttachments;

    while (--ctAttachments >= 0) {
      // Skip position, placement and model
      INDEX iPosition;
      strm >> iPosition;

      CPlacement3D plDummy;
      strm >> plDummy;

      Skip_t(strm);
    }
  }
};

inline void Write_t(CTStream &strm, CModelObject &mo) {
  // Write model file
  CAnimData *pad = (CAnimData *)mo.GetData();

  if (pad != NULL) {
    strm << pad->GetName();
  } else {
    strm << CTFileName(CTString(""));
  }

  // Write model object
  mo.Write_t(&strm);

  // Write all textures
  strm.WriteID_t("MTEX");

  IRes::Textures::Write_t(strm, mo.mo_toTexture);
  IRes::Textures::Write_t(strm, mo.mo_toBump);
  IRes::Textures::Write_t(strm, mo.mo_toReflection);
  IRes::Textures::Write_t(strm, mo.mo_toSpecular);

  // Write attachments
  if (!mo.mo_lhAttachments.IsEmpty()) {
    strm.WriteID_t("ATCH");
    strm << mo.mo_lhAttachments.Count();

    FOREACHINLIST(CAttachmentModelObject, amo_lnInMain, mo.mo_lhAttachments, itamo) {
      // Write position, placement and model
      strm << itamo->amo_iAttachedPosition;
      strm << itamo->amo_plRelative;

      Write_t(strm, itamo->amo_moModelObject);
    }
  }
};

}; // Models namespace

#if SE1_VER >= SE1_107

namespace SKA {

inline void Read_t(CTStream &strm, CModelInstance &mi);

inline void WriteMeshInstances_t(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("MSHI");

  // Write all mesh instances
  INDEX ct = mi.mi_aMeshInst.Count();
  strm << ct;

  for (INDEX i = 0; i < ct; i++) {
    MeshInstance &mshi = mi.mi_aMeshInst[i];

    strm.WriteID_t("MESH");
    strm << mshi.mi_pMesh->GetName();

    strm.WriteID_t("MITS");

    // Write textures
    INDEX ctTex = mshi.mi_tiTextures.Count();
    strm << ctTex;

    for (INDEX iTex = 0; iTex < ctTex; iTex++) {
      TextureInstance &ti = mshi.mi_tiTextures[iTex];

      strm.WriteID_t("TITX");

      // Write texture file and texture ID
      CTextureData *ptd = (CTextureData *)ti.ti_toTexture.GetData();
      strm << ptd->GetName();
      strm << ska_GetStringFromTable(ti.GetID());
    }
  }
};

inline void WriteSkeleton_t(CTStream &strm, CModelInstance &mi) {
  // Write model skeleton
  CSkeleton *pSkeleton = mi.mi_psklSkeleton;
  BOOL bHasSkeleton = (pSkeleton != NULL);

  strm.WriteID_t("SKEL");
  strm << bHasSkeleton;

  if (bHasSkeleton) {
    strm << pSkeleton->GetName();
  }
};

inline void WriteAnimSets(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("ANAS");

  // Write anim sets
  INDEX ct = mi.mi_aAnimSet.Count();
  strm << ct;

  for (INDEX i = 0; i < ct; i++) {
    strm << mi.mi_aAnimSet[i].GetName();
  }
};

inline void WriteColisionBoxes(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("MICB");

  // Write all collision boxes
  INDEX ct = mi.mi_cbAABox.Count();
  strm << ct;

  for (INDEX i = 0; i < ct; i++) {
    ColisionBox &cb = mi.mi_cbAABox[i];

    strm << cb.Min();
    strm << cb.Max();
    strm << cb.GetName();
  }

  // Write bounding box of all frames
  strm.WriteID_t("AFBB");
  strm << mi.mi_cbAllFramesBBox.Min();
  strm << mi.mi_cbAllFramesBBox.Max();
};

inline void WriteAnimQueue_t(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("MIAQ");

  AnimQueue &aq = mi.mi_aqAnims;

  // Write animation queue
  INDEX ct = aq.aq_Lists.Count();
  strm << ct;

  for (INDEX i = 0; i < ct; i++) {
    AnimList &al = aq.aq_Lists[i];

    strm.WriteID_t("AQAL");

    strm << al.al_fStartTime;
    strm << al.al_fFadeTime;

    // Write played anims
    INDEX ctPlayed = al.al_PlayedAnims.Count();
    strm << ctPlayed;

    for (INDEX iAnim = 0; iAnim < ctPlayed; iAnim++) {
      PlayedAnim &pa = al.al_PlayedAnims[iAnim];

      strm.WriteID_t("ALPA");

      strm << pa.pa_fStartTime;
      strm << pa.pa_ulFlags;
      strm << pa.pa_Strength;
      strm << pa.pa_GroupID;

      strm << ska_GetStringFromTable(pa.pa_iAnimID);

      // Write animation speed
      strm.WriteID_t("PASP");
      strm << pa.pa_fSpeedMul;
    }
  }
}

inline void WriteOffsetAndChildren(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("MIOF");

  // Write model offset and parent bone
  strm.Write_t(&mi.mi_qvOffset, sizeof(QVect));
  strm << ska_GetStringFromTable(mi.mi_iParentBoneID);

  strm.WriteID_t("MICH");

  // Write model children
  INDEX ct = mi.mi_cmiChildren.Count();
  strm << ct;

  for (INDEX i = 0; i < ct; i++) {
    WriteModelInstance_t(strm, mi.mi_cmiChildren[i]);
  }
};

inline void Write_t(CTStream &strm, CModelInstance &mi) {
  strm.WriteID_t("MI03");

  // Write model instance name
  strm << mi.GetName();

  // Write collision box index, stretch and color
  strm << mi.mi_iCurentBBox;
  strm << mi.mi_vStretch;
  strm << mi.mi_colModelColor;

  WriteMeshInstances_t(strm,mi);
  WriteSkeleton_t(strm, mi);
  WriteAnimSets(strm, mi);
  WriteAnimQueue_t(strm, mi);
  WriteColisionBoxes(strm, mi);
  WriteOffsetAndChildren(strm, mi);

  strm.WriteID_t("ME03");
};

inline void ReadMeshInstances_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  if (bNew) strm.ExpectID_t("MSHI");

  // Read mesh instances
  INDEX ct = 0;
  strm>>ct;

  mi.mi_aMeshInst.New(ct);

  for (INDEX i = 0; i < ct; i++) {
    MeshInstance &mshi = mi.mi_aMeshInst[i];

    if (bNew) strm.ExpectID_t("MESH");

    CTFileName fnmMesh;
    strm >> fnmMesh;

    mshi.mi_pMesh = _pMeshStock->Obtain_t(fnmMesh);

    if (bNew) strm.ExpectID_t("MITS");

    // Read textures
    INDEX ctTex = 0;
    strm >> ctTex;

    mshi.mi_tiTextures.New(ctTex);

    for (INDEX iTex = 0; iTex < ctTex; iTex++) {
      TextureInstance &ti = mshi.mi_tiTextures[iTex];

      if (bNew) strm.ExpectID_t("TITX");

      // Read texture file and texture ID
      CTFileName fnTex;
      CTString strTexID;
      strm >> fnTex;
      strm >> strTexID;

      ti.SetName(strTexID);
      ti.ti_toTexture.SetData_t(fnTex);
    }
  }
};

inline void ReadSkeleton_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  // Read model skeleton
  mi.mi_psklSkeleton = NULL;

  if (bNew) strm.ExpectID_t("SKEL");

  BOOL bHasSkeleton;
  strm >> bHasSkeleton;

  if (bHasSkeleton) {
    CTFileName fnmSkeleton;
    strm >> fnmSkeleton;
    mi.mi_psklSkeleton = _pSkeletonStock->Obtain_t(fnmSkeleton);
  }
};

inline void ReadAnimSets_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  if (bNew) strm.ExpectID_t("ANAS");

  // Read anim sets
  INDEX ct = 0;
  strm >> ct;

  while (--ct >= 0) {
    CTFileName fnmAnimSet;
    strm >> fnmAnimSet;

    CAnimSet *pas = _pAnimSetStock->Obtain_t(fnmAnimSet);
    mi.mi_aAnimSet.Add(pas);
  }
};

inline void ReadAnimQueue_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  if (bNew) strm.ExpectID_t("MIAQ");

  AnimQueue &aq = mi.mi_aqAnims;

  // Read animation lists
  INDEX ct = 0;
  strm >> ct;

  if (ct > 0) aq.aq_Lists.Push(ct);

  for (INDEX i = 0; i < ct; i++) {
    AnimList &al = aq.aq_Lists[i];

    if (bNew) strm.ExpectID_t("AQAL");

    strm >> al.al_fStartTime;
    strm >> al.al_fFadeTime;

    // Read played animations
    INDEX ctPlayed = 0;
    strm >> ctPlayed;

    if (ctPlayed > 0) al.al_PlayedAnims.Push(ctPlayed);

    for (INDEX iAnim = 0; iAnim < ctPlayed; iAnim++) {
      PlayedAnim &pa = al.al_PlayedAnims[iAnim];

      if (bNew) strm.ExpectID_t("ALPA");

      strm >> pa.pa_fStartTime;
      strm >> pa.pa_ulFlags;
      strm >> pa.pa_Strength;
      strm >> pa.pa_GroupID;

      CTString strAnimID;
      strm >> strAnimID;

      pa.pa_iAnimID = ska_GetIDFromStringTable(strAnimID);

      // Read animation speed
      if (bNew && strm.PeekID_t() == CChunkID("PASP")) {
        strm.ExpectID_t("PASP");
        strm >> pa.pa_fSpeedMul;
      }
    }
  }
};

inline void ReadCollisionBoxes_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  if (bNew) strm.ExpectID_t("MICB");

  // Create enough collision boxes
  INDEX ct = 0;
  strm >> ct;

  mi.mi_cbAABox.New(ct);

  // Read all collision boxes
  for (INDEX i = 0; i < ct; i++) {
    ColisionBox &cb = mi.mi_cbAABox[i];

    strm >> cb.Min();
    strm >> cb.Max();

    if (bNew) {
      CTString strName;
      strm >> strName;

      cb.SetName(strName);
    }
  }

  // Read bounding box of all frames
  if (bNew) {
    strm.ExpectID_t("AFBB");

    strm >> mi.mi_cbAllFramesBBox.Min();
    strm >> mi.mi_cbAllFramesBBox.Max();
  }
};

inline void ReadOffsetAndChildren_t(CTStream &strm, CModelInstance &mi, BOOL bNew)
{
  if (bNew) strm.ExpectID_t("MIOF");

  // Read model offset and parent bone
  strm.Read_t(&mi.mi_qvOffset,sizeof(QVect));

  CTString strBoneID;
  strm >> strBoneID;

  mi.mi_iParentBoneID = ska_GetIDFromStringTable(strBoneID);

  if (bNew) strm.ExpectID_t("MICH");

  // Read model instance children
  INDEX ct = 0;
  strm >> ct;

  while (--ct >= 0) {
    // Create empty child and read it
    CModelInstance *pmi = CreateModelInstance("Temp");
    mi.mi_cmiChildren.Add(pmi);

    Read_t(strm, *pmi);
  }
};

inline void ReadOld_t(CTStream &strm, CModelInstance &mi) {
  strm.ExpectID_t("SKMI");

  // Read model instance name
  CTString strName;
  strm >> strName;
  mi.SetName(strName);

  ReadMeshInstances_t(strm, mi, FALSE);
  ReadSkeleton_t(strm, mi, FALSE);
  ReadAnimSets_t(strm, mi, FALSE);
  ReadCollisionBoxes_t(strm, mi, FALSE);

  // Read collision box index, stretch and color
  strm >> mi.mi_iCurentBBox;
  strm >> mi.mi_vStretch;
  strm >> mi.mi_colModelColor;

  ReadAnimQueue_t(strm, mi, FALSE);
  ReadOffsetAndChildren_t(strm, mi, FALSE);
};

inline void ReadNew_t(CTStream &strm, CModelInstance &mi) {
  strm.ExpectID_t("MI03");

  // Read model instance name
  CTString strName;
  strm >> strName;

  mi.SetName(strName);

  // Read collision box index, stretch and color
  strm >> mi.mi_iCurentBBox;
  strm >> mi.mi_vStretch;
  strm >> mi.mi_colModelColor;

  ReadMeshInstances_t(strm, mi, TRUE);
  ReadSkeleton_t(strm, mi, TRUE);
  ReadAnimSets_t(strm, mi, TRUE);
  ReadAnimQueue_t(strm, mi, TRUE);
  ReadCollisionBoxes_t(strm, mi, TRUE);
  ReadOffsetAndChildren_t(strm, mi, TRUE);

  strm.ExpectID_t("ME03");
};

void Read_t(CTStream &strm, CModelInstance &mi) {
  CChunkID cid = strm.PeekID_t();

  // Read old format
  if (cid == CChunkID("SKMI")) {
    ReadOld_t(strm, mi);

  // Read new format
  } else if (cid == CChunkID("MI03")) {
    ReadNew_t(strm, mi);

  // Unknown format
  } else {
    strm.Throw_t("Unknown model instance format");
  }
};

inline void Skip_t(CTStream &strm) {
  CModelInstance miDummy;
  Read_t(strm, miDummy);
};

}; // SKA namespace

#endif

}; // namespace

#endif
