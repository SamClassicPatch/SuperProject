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

// [Cecil] Rev: Recreation of an entity from Revolution
110
%{
#include "StdH.h"
%}

class CAchievementEntity : CRationalEntity {
name      "Achievement";
thumbnail "Thumbnails\\Achievement.tbn";
features  "HasName", "IsTargetable";

properties:
 1 CTString m_strName        "Name" 'N' = "Achievement",
 2 CTString m_strAchievement "Achievement / stat" 'A' = "",
 3 BOOL  m_bSinglePlayerOnly "Single player only" = FALSE,
 4 BOOL  m_bStat       "Is stat" = FALSE,
 5 BOOL  m_bStatAdd    "Stat adds" = TRUE,
 6 BOOL  m_bStatFloat  "Stat is float" = FALSE,
 7 INDEX m_iStatAmount "Stat amount (INT)" = 0,
 8 FLOAT m_fStatAmount "Stat amount (FLOAT)" = 0.0f,

components:
 1 model   MODEL_MARKER   "Models\\Editor\\NavigationMarker.mdl",
 2 texture TEXTURE_MARKER "Models\\Editor\\NavigationMarker.tex",

functions:
  // [Cecil] Rev: This used to be written inline for ETrigger event
  void TriggerAchievement(CEntity *penCaused) {
    if (m_bSinglePlayerOnly && !GetSP()->sp_bSinglePlayer) {
      return;
    }

    if (!m_bStat) {
    #if SE1_GAME == SS_REV
      CAchievement *pAch = _pSteam->GetAchievementByID(m_strAchievement);

      if (pAch != NULL) {
        pAch->Unlock();
        return;
      }
    #endif

      CPrintF("^cff0000Warning: Achievement '%s' can't be found!\n", m_strAchievement);
      return;
    }

  #if SE1_GAME == SS_REV
    CSteamStat *pStat = _pSteam->GetStatByID(m_strAchievement);
    const BOOL bNoStat = (pStat == NULL);
  #else
    const BOOL bNoStat = TRUE;
  #endif

    if (bNoStat) {
      CPrintF("^cff0000Warning: Stat '%s' can't be found!\n", m_strAchievement);
      return;
    }

  #if SE1_GAME == SS_REV
    if (m_bStatFloat) {
      if (m_bStatAdd) {
        pStat->AddFLOAT(m_fStatAmount);
      } else {
        pStat->SetFLOAT(m_fStatAmount);
      }

    } else {
      if (m_bStatAdd) {
        pStat->AddINT(m_iStatAmount);
      } else {
        pStat->SetINT(m_iStatAmount);
      }
    }
  #endif
  };

procedures:
  Main() {
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);

    GetModelObject()->StretchModel(FLOAT3D(0.35f, 0.35f, 0.35f));
    ModelChangeNotify();

    autowait(0.1f);

    wait() {
      on (ETrigger eTrigger) : {
        TriggerAchievement(eTrigger.penCaused);
        resume;
      }
    }

    return;
  };
};
