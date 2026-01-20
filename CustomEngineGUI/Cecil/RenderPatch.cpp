/* Copyright (c) 2022-2026 Dreamy Cecil
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

// Original function pointers
static void (CModelObject::*pMdlSetupFunc)(CRenderModel &) = NULL;
static void (*pMdlBeginFunc)(CAnyProjection3D &, CDrawPort *) = NULL;
static void (*pMdlEndFunc)(BOOL) = NULL;
static void (*pSkaBeginFunc)(CAnyProjection3D &, CDrawPort *) = NULL;
static void (*pSkaEndFunc)(BOOL) = NULL;

// Hooked render space
static CDrawPort *_pdpAfterModel = NULL;

// Patched CModelObject::SetupModelRendering() method
class CModelObjectPatch : public CModelObject {
  public:
    void P_SetupModelRendering(CRenderModel &rm) {
      // Call original function
      (this->*pMdlSetupFunc)(rm);

      // Check for Modeler models
      CTString strModel = "";
      CTString strTexture = "";
      CTString strReflection = "";
      CTString strSpecular = "";

      if (GetData() != NULL) {
        strModel = GetData()->GetName();
      }

      if (mo_toTexture.GetData() != NULL) {
        strTexture = mo_toTexture.GetData()->GetName();
      }

      if (mo_toReflection.GetData() != NULL) {
        strReflection = mo_toReflection.GetData()->GetName();
      }

      if (mo_toSpecular.GetData() != NULL) {
        strSpecular = mo_toSpecular.GetData()->GetName();
      }

      // Reflection and specular creation
      BOOL bTeapot = (strModel.Matches("Models\\Editor\\Teapot.mdl") && strTexture.Matches("Models\\Editor\\Teapot.tex"))
                  && (strReflection.Matches("Temp\\ReflectionTemp.tex") || strSpecular.Matches("Temp\\SpecularTemp.tex"));

      // Modeler floor
      BOOL bFloor = (strModel.Matches("Models\\Editor\\Floor.mdl") && strTexture.Matches("Models\\Editor\\Floor.tex"));

      // Modeler lamp
      BOOL bLamp = (strModel.Matches("Models\\Editor\\Lamp.mdl") && strTexture.Matches("Models\\Editor\\SpotLight.tex"));

      // Modeler collision box
      BOOL bCollision = (strModel.Matches("Models\\Editor\\CollisionBox.mdl") && strTexture.Matches("Models\\Editor\\CollisionBox.tex"));

      // If any of the Modeler models
      if (bTeapot || bFloor || bLamp || bCollision) {
        // Don't draw after the model
        _pdpAfterModel = NULL;
      }
    };
};

// Patched beginning of vertex (MDL) model rendering
static void P_BeginMdlRendering(CAnyProjection3D &apr, CDrawPort *pdp) {
  // Call original function
  (*pMdlBeginFunc)(apr, pdp);

  // Hook render space for Modeler
  _pdpAfterModel = pdp;
};

// Patched ending of vertex (MDL) model rendering
static void P_EndMdlRendering(BOOL bRestoreOrtho) {
  // Call original function
  (*pMdlEndFunc)(bRestoreOrtho);

  // Call API after rendering the model
  if (_pdpAfterModel != NULL) {
    IHooks::OnFrame(_pdpAfterModel);
  }

  // Reset render space
  _pdpAfterModel = NULL;
};

// SKA methods are only available in SE1.07 and newer
#if SE1_VER >= SE1_107

// Patched beginning of skeletal (SKA) mesh rendering
static void P_BeginSkaRendering(CAnyProjection3D &apr, CDrawPort *pdp) {
  // Call original function
  (*pSkaBeginFunc)(apr, pdp);

  // Hook render space for SKA Studio
  _pdpAfterModel = pdp;
};

// Patched ending of skeletal (SKA) mesh rendering
static void P_EndSkaRendering(BOOL bRestoreOrtho) {
  // Call original function
  (*pSkaEndFunc)(bRestoreOrtho);

  // Call API after rendering the model
  if (_pdpAfterModel != NULL) {
    IHooks::OnFrame(_pdpAfterModel);
  }

  // Reset render space
  _pdpAfterModel = NULL;
};

#endif

void ClassicsPatch_ModelRenderPatches(void) {
  // MDL rendering methods
  pMdlSetupFunc = &CModelObject::SetupModelRendering;
  CreatePatch(pMdlSetupFunc, &CModelObjectPatch::P_SetupModelRendering, "CModelObject::SetupModelRendering(...)");

  pMdlBeginFunc = &BeginModelRenderingView;
  CreatePatch(pMdlBeginFunc, &P_BeginMdlRendering, "::BeginModelRenderingView(...)");

  pMdlEndFunc = &EndModelRenderingView;
  CreatePatch(pMdlEndFunc, &P_EndMdlRendering, "::EndModelRenderingView(...)");

  // SKA rendering methods
  #if SE1_VER >= SE1_107
    pSkaBeginFunc = &RM_BeginRenderingView;
    CreatePatch(pSkaBeginFunc, &P_BeginSkaRendering, "::RM_BeginRenderingView(...)");

    pSkaEndFunc = &RM_EndRenderingView;
    CreatePatch(pSkaEndFunc, &P_EndSkaRendering, "::RM_EndRenderingView(...)");
  #endif
};
