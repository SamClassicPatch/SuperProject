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
111
%{
#include "StdH.h"
%}

class CWorldInfoEntity : CEntity {
name      "WorldInfo";
thumbnail "Thumbnails\\WorldInfo.tbn";

properties:
 1 FLOAT m_fSVMedalBronze "Bronze medal seconds" = 30.0f,
 2 FLOAT m_fSVMedalSilver "Silver medal seconds" = 60.0f,
 3 FLOAT m_fSVMedalGold   "Gold medal seconds"   = 90.0f,

components:
 1 model   MODEL_MARKER   "Models\\Editor\\Trigger.mdl",
 2 texture TEXTURE_MARKER "Models\\Editor\\Camera.tex",

functions:

procedures:
  Main() {
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);
    return;
  };
};
