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
243
%{
#include "StdH.h"
%}

class CSpectatorCamera : CEntity {
name      "Spectator Camera";
thumbnail "Thumbnails\\SpectatorCamera.tbn";
features  "HasName", "IsTargetable";

properties:
 1 CTString m_strName "Name" 'N' = "Spectator Camera",
 2 FLOAT m_fFOV "FOV" 'F' = 90.0f,
 3 CEntityPointer m_penNext "Next spectator camera" 'V',
 4 CTString m_strTitle "Title" 'T' = "",

components:
 1 model   MODEL_MARKER   "Models\\Editor\\CameraMarker.mdl",
 2 texture TEXTURE_MARKER "Models\\Editor\\CameraMarker.tex",

functions:

procedures:
  Main() {
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);

    if (m_penNext != NULL && !IsOfClass(m_penNext, "Spectator Camera")) {
      WarningMessage("Entity '%s' is not of Spectator Camera class!", m_penNext->GetName());
      m_penNext = NULL;
    }

    return;
  };
};
