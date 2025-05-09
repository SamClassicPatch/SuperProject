/* Copyright (c) 2023-2025 Dreamy Cecil
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

#if _PATCHCONFIG_ENGINEPATCHES

#include "Worlds.h"

#include <Engine/Templates/Stock_CEntityClass.h>
#include <Core/Interfaces/ResourceFunctions.h>

// [Cecil] Determine world format before loading the world itself
void CWorldPatch::DetermineWorldFormat(const CTFileName &fnmWorld, CTFileStream &strmFile) {
  // Loading from the current game directory
  ELevelFormat &eWorld = _EnginePatches._eWorldFormat;
  eWorld = E_LF_CURRENT;

#if CLASSIC_TSE_FUSION_MODE
  // Check if the level is being loaded from TFE
  if (IsFileFromDir(GAME_DIR_TFE, fnmWorld)) {
    eWorld = E_LF_TFE;
  }
#endif

  // Determine world format from world info
  {
    strmFile.ExpectID_t("BUIV");

    INDEX iDummy;
    strmFile >> iDummy;

    // Levels from other games
    if (iDummy != 10000) {
      eWorld = E_LF_150;
    }

    strmFile.ExpectID_t("WRLD");

    // World info may set a different value to CPatches::_eWorldFormat
    P_ReadInfo(&strmFile, FALSE);

    // Reset the stream
    strmFile.SetPos_t(0);
  }

  // Force a specific format
  const ELevelFormat eWanted = (ELevelFormat)_EnginePatches._iWantedWorldFormat;

  if (eWanted >= 0 && eWanted < E_LF_FORMATCOUNT) {
    eWorld = eWanted;
  }
};

void CWorldPatch::P_Load(const CTFileName &fnmWorld) {
  // Open the file
  wo_fnmFileName = fnmWorld;

  CTFileStream strmFile;
  strmFile.Open_t(fnmWorld);

  // [Cecil] Determine world format
  ELevelFormat &eWorld = _EnginePatches._eWorldFormat;
  DetermineWorldFormat(fnmWorld, strmFile);

  // [Cecil] Set converter for the world format and reset it
  static HPatchPlugin hConverters = ClassicsExtensions_GetExtensionByName("PATCH_EXT_wldconverters");

  ExtArgWorldConverter_t convData;
  bool bConverter = ClassicsExtensions_CallSignal(hConverters, "GetConverterForFormat", &convData.iID, &eWorld);

  // Prepare world converter before using it
  if (bConverter) {
    // Pass converters from the command
    convData.pData = _EnginePatches._strWorldConverters.str_String;
    ClassicsExtensions_CallSignal(hConverters, "PrepareConverter", NULL, &convData);
  }

  // Check engine build
  BOOL bNeedsReinit;
  _pNetwork->CheckVersion_t(strmFile, TRUE, bNeedsReinit);

  // Read the world
  Read_t(&strmFile);

  strmFile.Close();

  // [Cecil] Convert the world some specific way while in game
  if (!ClassicsCore_IsEditorApp() && bConverter) {
    SetProgressDescription(LOCALIZE("converting from old version"));
    CallProgressHook_t(0.0f);

    // Must be in 24bit mode when managing entities
    CSetFPUPrecision FPUPrecision(FPT_24BIT);
    CTmpPrecachingNow tpn;

    // Use prepared world converter
    ClassicsExtensions_CallSignal(hConverters, "ConvertWorld", NULL, this);

    CallProgressHook_t(1.0f);

  // Reinitialize and resave old levels, if needed
  } else if (bNeedsReinit) {
    SetProgressDescription(LOCALIZE("converting from old version"));
    CallProgressHook_t(0.0f);
    ReinitializeEntities();
    CallProgressHook_t(1.0f);

    SetProgressDescription(LOCALIZE("saving converted file"));
    CallProgressHook_t(0.0f);
    Save_t(fnmWorld);
    CallProgressHook_t(1.0f);
  }

  // [Cecil] Call API method after loading the world
  IHooks::OnWorldLoad(this, fnmWorld);
};

void CWorldPatch::P_LoadBrushes(const CTFileName &fnmWorld) {
  // Open the file
  wo_fnmFileName = fnmWorld;

  CTFileStream strmFile;
  strmFile.Open_t(fnmWorld);

  // [Cecil] Determine world format
  DetermineWorldFormat(fnmWorld, strmFile);

  // Check engine build
  BOOL bNeedsReinit;
  _pNetwork->CheckVersion_t(strmFile, FALSE, bNeedsReinit);
  ASSERT(!bNeedsReinit);

  strmFile.ExpectID_t("WRLD");

  // Read world brushes from the file
  ReadBrushes_t(&strmFile);
};

// Read world information
void CWorldPatch::P_ReadInfo(CTStream *strm, BOOL bMaybeDescription) {
  // Read entire world info
  if (strm->PeekID_t() == CChunkID("WLIF")) {
    strm->ExpectID_t(CChunkID("WLIF"));

    // [Cecil] "DTRS" in the EXE as is gets picked up by the Depend utility
    static const CChunkID chnkDTRS(CTString("DT") + "RS");

    if (strm->PeekID_t() == chnkDTRS) {
      strm->ExpectID_t(chnkDTRS);
    }

    // [Cecil] Rev: Read new world info
    {
      CTString strLeaderboard = "";
      ULONG aulExtra[3] = { 0, 0, 0 };

      // Read leaderboard
      if (strm->PeekID_t() == CChunkID("LDRB")) {
        strm->ExpectID_t("LDRB");
        *strm >> strLeaderboard;

        _EnginePatches._eWorldFormat = E_LF_SSR;
      }

      // Read unknown values
      if (strm->PeekID_t() == CChunkID("Plv0")) {
        strm->ExpectID_t("Plv0");
        *strm >> aulExtra[0];
        *strm >> aulExtra[1];
        *strm >> aulExtra[2];

        _EnginePatches._eWorldFormat = E_LF_SSR;
      }

      // Set default or read values
      #if SE1_GAME == SS_REV
        wo_strLeaderboard = strLeaderboard;
        wo_ulExtra3 = aulExtra[0];
        wo_ulExtra4 = aulExtra[1];
        wo_ulExtra5 = aulExtra[2];
      #endif
    }

    // Read display name
    *strm >> wo_strName;

    // Read flags
    *strm >> wo_ulSpawnFlags;

    // [Cecil] Rev: Read special gamemode chunk
    if (strm->PeekID_t() == CChunkID("SpGM")) {
      strm->ExpectID_t("SpGM");
      _EnginePatches._eWorldFormat = E_LF_SSR;

    } else {
      // Otherwise remove some flags
      #if SE1_GAME == SS_REV
        wo_ulSpawnFlags &= ~0xE00000;
      #endif
    }

    // Read world description
    *strm >> wo_strDescription;

  // Only read description
  } else if (bMaybeDescription) {
    *strm >> wo_strDescription;
  }
};

// Create a new entity of a given class
CEntity *CWorldPatch::P_CreateEntity(const CPlacement3D &plPlacement, const CTFileName &fnmClass) {
  CEntityClass *pecClass = NULL;
  CTFileName fnmCopy = fnmClass;

  // [Cecil] Try obtaining a new entity class
  try {
    pecClass = _pEntityClassStock->Obtain_t(fnmCopy);

  // [Cecil] If the current file can't be loaded for any reason
  } catch (char *strError) {
    (void)strError;

    // Try again with an explicit replacement
    CTFileName fnmReplacement;

    if (IRes::ReplaceFile(fnmCopy, fnmReplacement, "Entity Class Links (*.ecl)\0*.ecl\0" FILTER_END)) {
      pecClass = _pEntityClassStock->Obtain_t(fnmReplacement);

    // Otherwise send the error further up
    } else {
      throw;
    }
  }

  // Create an entity with that class (release because it obtains it once more)
  CEntity *penNew = CreateEntity(plPlacement, pecClass);
  _pEntityClassStock->Release(pecClass);

  return penNew;
};

#endif // _PATCHCONFIG_ENGINEPATCHES
