/* Copyright (c) 2024-2025 Dreamy Cecil
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
112
%{
#include "StdH.h"
%}

class CControlZoneEntity : CRationalEntity {
name      "Control Zone";
thumbnail "Thumbnails\\ControlZone.tbn";
features  "HasName", "IsTargetable";

properties:
 1 CTString m_strName "Name" 'N' = "Control Zone",

{
  CFieldSettings m_fsField;
}

components:
 1 texture TEXTURE_ZONE "Models\\Editor\\ControlZoneBox.tex",

functions:
  void SetupFieldSettings(void) {
    m_fsField.fs_toTexture.SetData(GetTextureDataForComponent(TEXTURE_ZONE));
    m_fsField.fs_colColor = C_WHITE | CT_OPAQUE;
  };

  CFieldSettings *GetFieldSettings(void) {
    if (m_fsField.fs_toTexture.GetData() == NULL) {
      SetupFieldSettings();      
    }
    return &m_fsField;
  };

procedures:
  LogicLoop() {
    // [Cecil] TODO: It needs GameInfo and new Player fields to be recreated in order to work
    /*while (TRUE)
    {
      wait (0.05f) {
        on (EPass ePass) : {
          // Mark player as being inside this zone
          if (IsOfClass(ePass.penOther, "Player")) {
            ((CPlayer &)*ePass.penOther).m_penInControlZone = this;
          }
          resume;
        }

        on (ETimer) : {
          for (INDEX i = 0; i < CEntity::GetMaxPlayers(); i++) {
            CPlayer *penPlayer = (CPlayer *)CEntity::GetPlayerEntity(i);

            // No player or not inside any zone
            // [Cecil] NOTE: This probably needs to be 'penPlayer->m_penInControlZone == this' instead of using 'IsEntityInside(penPlayer)' below
            if (penPlayer == NULL || penPlayer->m_penInControlZone == NULL) {
              continue;
            }

            // Player is inside this zone and alive
            if (IsEntityInside(penPlayer) && penPlayer->GetFlags() & ENF_ALIVE) {
              CGameInfoEntity *penGameInfo = (CGameInfoEntity *)GetGameInfo();

              if (penPlayer->m_iTeam == 0) {
                // Add score to red team
                penGameInfo->m_iControlZoneTeam0++;

              } else {
                // Add score to blue team
                penGameInfo->m_iControlZoneTeam1++;
              }

            } else {
              penPlayer->m_penInControlZone = NULL;
            }
          }
          stop;
        }
      }
    }*/

    return;
  };

  Main() {
    InitAsFieldBrush();
    SetPhysicsFlags(EPF_BRUSH_FIXED);
    SetCollisionFlags(((ECBI_MODEL) << ECB_TEST) | ((ECBI_BRUSH) << ECB_IS) | ((ECBI_MODEL) << ECB_PASS));

    autowait(0.1f);
    jump LogicLoop();
  };
};
