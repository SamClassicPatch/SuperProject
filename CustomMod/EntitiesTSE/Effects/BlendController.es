/* Copyright (c) 2002-2012 Croteam Ltd. 
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

612
%{
#include "StdH.h"
#include "Effects/WorldSettingsController.h"
#include "Tools/BackgroundViewer.h"
%}

uses "Tools/Marker";

enum BlendControllerType {
  0 BCT_NONE                        "None",                      // no FX
  1 BCT_PYRAMID_PLATES              "Appear pyramid plates",     // effect of appearing of pyramid plates
  2 BCT_ACTIVATE_PLATE_1            "Activate plate 1",          // plate 1 activating
  3 BCT_ACTIVATE_PLATE_2            "Activate plate 2",          // plate 2 activating
  4 BCT_ACTIVATE_PLATE_3            "Activate plate 3",          // plate 3 activating
  5 BCT_ACTIVATE_PLATE_4            "Activate plate 4",          // plate 4 activating
  6 BCT_ACTIVATE_PYRAMID_MORPH_ROOM "Pyramid morph room",        // pyramid morph room activated

  // [Cecil] Rev: New blend modes
  7 BCT_TOGGLE_LIGHTS_1            "[SSR] Toggle lights 1",
  8 BCT_TOGGLE_LIGHTS_2            "[SSR] Toggle lights 2",
  9 BCT_TOGGLE_LIGHTS_3            "[SSR] Toggle lights 3",
 10 BCT_TOGGLE_LIGHTS_4            "[SSR] Toggle lights 4",
 11 BCT_TOGGLE_CONTROLLED_LIGHTS_1 "[SSR] Toggle controlled lights 1",
 12 BCT_TOGGLE_CONTROLLED_LIGHTS_2 "[SSR] Toggle controlled lights 2",
 13 BCT_TOGGLE_CONTROLLED_LIGHTS_3 "[SSR] Toggle controlled lights 3",
 14 BCT_TOGGLE_CONTROLLED_LIGHTS_4 "[SSR] Toggle controlled lights 4",
};

class CBlendController: CMarker
{
name      "Blend controller";
thumbnail "Thumbnails\\BlendController.tbn";
features "IsImportant";

properties:

  1 enum BlendControllerType m_bctType  "Blend type" 'Y' = BCT_NONE,         // type of effect

components:

  1 model   MODEL_CONTROLLER          "Models\\Editor\\BlendController.mdl",
  2 texture TEXTURE_CONTROLLER        "Models\\Editor\\BlendController.tex",


functions:

  /* Handle an event, return false if the event is not handled. */
  BOOL HandleEvent(const CEntityEvent &ee)
  {
    // obtain world settings controller
    CWorldSettingsController *pwsc = GetWSC(this);
    if( pwsc == NULL) {
      return FALSE;
    }
    FLOAT tmNow = _pTimer->CurrentTick();

    // [Cecil] Rev: Timers for new blend modes
    CStaticArray<FLOAT> &atm = pwsc->m_atmToggledLights;

    if (ee.ee_slEvent==EVENTCODE_EActivate)
    {
      switch(m_bctType)
      {
        case BCT_PYRAMID_PLATES:
          pwsc->m_tmPyramidPlatesStart = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_1:
          pwsc->m_tmActivatedPlate1 = tmNow;
          pwsc->m_tmDeactivatedPlate1 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_2:
          pwsc->m_tmActivatedPlate2 = tmNow;
          pwsc->m_tmDeactivatedPlate2 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_3:
          pwsc->m_tmActivatedPlate3 = tmNow;
          pwsc->m_tmDeactivatedPlate3 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_4:
          pwsc->m_tmActivatedPlate4 = tmNow;
          pwsc->m_tmDeactivatedPlate4 = 1e6;
          break;
        case BCT_ACTIVATE_PYRAMID_MORPH_ROOM:
          pwsc->m_tmPyramidMorphRoomActivated = tmNow;
          break;

        // [Cecil] Rev: New blend types
        case BCT_TOGGLE_LIGHTS_1:
          atm[0] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_2:
          atm[2] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_3:
          atm[4] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_4:
          atm[6] = tmNow;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_1:
          atm[8] = tmNow;
          atm[9] = 1e6;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_2:
          atm[10] = tmNow;
          atm[11] = 1e6;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_3:
          atm[12] = tmNow;
          atm[13] = 1e6;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_4:
          atm[14] = tmNow;
          atm[15] = 1e6;
          break;
      }
    }
    else if (ee.ee_slEvent==EVENTCODE_EDeactivate)
    {
      switch(m_bctType)
      {
        case BCT_ACTIVATE_PLATE_1:
          pwsc->m_tmDeactivatedPlate1 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_2:
          pwsc->m_tmDeactivatedPlate2 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_3:
          pwsc->m_tmDeactivatedPlate3 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_4:
          pwsc->m_tmDeactivatedPlate4 = tmNow;
          break;

        // [Cecil] Rev: New blend types
        case BCT_TOGGLE_LIGHTS_1:
          atm[1] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_2:
          atm[3] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_3:
          atm[5] = tmNow;
          break;

        case BCT_TOGGLE_LIGHTS_4:
          atm[7] = tmNow;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_1:
          atm[8] = 1e6;
          atm[9] = tmNow;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_2:
          atm[10] = 1e6;
          atm[11] = tmNow;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_3:
          atm[12] = 1e6;
          atm[13] = tmNow;
          break;

        case BCT_TOGGLE_CONTROLLED_LIGHTS_4:
          atm[14] = 1e6;
          atm[15] = tmNow;
          break;
      }
    }
    return FALSE;
  }

procedures:

  Main()
  {
    // [Cecil]
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_LIGHTS_1, BCT_ACTIVATE_PLATE_1);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_LIGHTS_2, BCT_ACTIVATE_PLATE_2);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_LIGHTS_3, BCT_ACTIVATE_PLATE_3);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_LIGHTS_4, BCT_ACTIVATE_PLATE_4);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_CONTROLLED_LIGHTS_1, BCT_ACTIVATE_PLATE_1);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_CONTROLLED_LIGHTS_2, BCT_ACTIVATE_PLATE_2);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_CONTROLLED_LIGHTS_3, BCT_ACTIVATE_PLATE_3);
    ResetCompatibilityValue(m_bctType, BCT_TOGGLE_CONTROLLED_LIGHTS_4, BCT_ACTIVATE_PLATE_4);

    // init model
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    // set appearance
    SetModel(MODEL_CONTROLLER);
    SetModelMainTexture(TEXTURE_CONTROLLER);

    return;
  }
};

