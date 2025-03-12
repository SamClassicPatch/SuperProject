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
809
%{
#include "StdH.h"
#include "Models/Items/ItemHolder/ItemHolder.h"
%}

uses "Items/Item";

event EFlagItem {
  INDEX iTeam,
  INDEX iLoose,
  CEntityPointer penFlag, // Entity that sent the event
};

class CFlagItem : CItem {
name      "FlagItem";
thumbnail "Thumbnails\\FlagItem.tbn";

properties:
 1 INDEX m_iTeam "Owner team" 'T' = 0,
 2 INDEX m_iLoose "Loose" = 0,
 3 INDEX m_iPicked = 0,
 5 FLOAT m_tmRecovered = 0.0f,
 6 CEntityPointer m_penLooseReminder,

// [Cecil] Replacement for Revolution's CEntity::en_plOrigin field
10 CPlacement3D m_plOrigin = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)),

components:
  0 class   CLASS_BASE        "Classes\\Item.ecl",
  1 model   MODEL_FLAG        "Models\\Flag\\Flag.mdl",
  2 texture TEXTURE_FLAG      "Models\\Flag\\Flag.tex",
  3 texture TEXTURE_FLAGBLUE  "Models\\Flag\\FlagBlue.tex",

functions:
  void SetProperties(void) {
    m_fRespawnTime = 0.0f;

    if (m_iTeam == 0) {
      AddItem(MODEL_FLAG, TEXTURE_FLAG, 0, 0, 0);
    } else {
      AddItem(MODEL_FLAG, TEXTURE_FLAGBLUE, 0, 0, 0);
    }

    StretchItem(FLOAT3D(2.5f, 2.5f, 2.5f));
  };

  // Remove flags from non-CTF gamemodes
  void AdjustDifficulty(void) {
    if (GetSP()->sp_gmGameMode != CSessionProperties::GM_CTF) {
      Destroy();
    }
  };

procedures:
  ItemCollected(EPass epass) : CItem::ItemCollected {
    // Pick up the flag
    EFlagItem eFlag;
    eFlag.iTeam = m_iTeam;
    eFlag.iLoose = m_iLoose;
    eFlag.penFlag = this;

    if (!m_iPicked || m_iLoose) {
      if (m_penLooseReminder != NULL) {
        m_penLooseReminder->Destroy();
      }

      if (epass.penOther->ReceiveItem(eFlag)) {
        SwitchToEditorModel();
        m_iPicked = TRUE;
      }
    }

    return;
  };

  Main() {
    Initialize();
    StartModelAnim(ITEMHOLDER_ANIM_SMALLOSCILATION, AOF_LOOPING|AOF_NORESTART);
    ForceCollisionBoxIndexChange(ITEMHOLDER_COLLISION_BOX_SMALL);
    SetProperties();

    autowait(0.1f);
    m_plOrigin = GetPlacement(); // [Cecil]

    wait() {
      on (EBegin) : {
        call CItem::ItemLoop();
      }

      // [Cecil] NOTE: Reminder is probably created by the player that drops the flag
      on (EReminder) : {
        if (m_iLoose) {
          Teleport(m_plOrigin, FALSE);
          m_iLoose = FALSE;
          m_iPicked = FALSE;
        }
        resume;
      }
    }
  };
};
