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

#include "StdH.h"

#include "Networking/Modules/ClientLogging.h"

// Auto update shadows upon loading into worlds
INDEX gam_bAutoUpdateShadows = TRUE;

// Queue shadow updating for the next connection to a server
static BOOL _bQueueShadowUpdate = TRUE;

// Fix broken shadows and lights by updating them
void UpdateShadows(void)
{
  FOREACHINDYNAMICCONTAINER(IWorld::GetWorld()->wo_cenEntities, CEntity, iten) {
    // CLight_ClassID
    if (!IsDerivedFromID(&*iten, 200)) continue;

    // Update shadow layers for each light
    CLightSource *pls = iten->GetLightSource();

    if (pls != NULL) {
      pls->FindShadowLayers(FALSE);
    }
  }

  // Update shadows from the sun and such
  IWorld::GetWorld()->CalculateDirectionalShadows();
};

// Called every simulation tick
void IHooks::OnTick(void)
{
  // Call step function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_processing->OnStep == NULL) continue;

    itPlugin->pm_events.m_processing->OnStep();
  }
};

// Called every render frame
void IHooks::OnFrame(CDrawPort *pdp)
{
  // Interact with Steam
  GetSteamAPI()->Update();

  // Update input values
  for (INDEX iAxis = 0; iAxis < MAX_OVERALL_AXES; iAxis++) {
    inp_afAxisValues[iAxis] = _pInput->GetAxisValue(iAxis);
  }

  // Call frame function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_processing->OnFrame == NULL) continue;

    itPlugin->pm_events.m_processing->OnFrame(pdp);
  }
};

// Called before redrawing game view
void IHooks::OnPreDraw(CDrawPort *pdp)
{
  // Call pre-draw function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_rendering->OnPreDraw == NULL) continue;

    itPlugin->pm_events.m_rendering->OnPreDraw(pdp);
  }
};

// Called after redrawing game view
void IHooks::OnPostDraw(CDrawPort *pdp)
{
  // Call post-draw function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_rendering->OnPostDraw == NULL) continue;

    itPlugin->pm_events.m_rendering->OnPostDraw(pdp);
  }
};

// Called after rendering the world
void IHooks::OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp)
{
  // Call render view function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_rendering->OnRenderView == NULL) continue;

    itPlugin->pm_events.m_rendering->OnRenderView(wo, penViewer, apr, pdp);
  }
};

// Called after starting world simulation
void IHooks::OnGameStart(void)
{
  // Call game start function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_game->OnGameStart == NULL) continue;

    itPlugin->pm_events.m_game->OnGameStart();
  }
};

// Called after changing the level
void IHooks::OnChangeLevel(void)
{
  // Call level change function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_game->OnChangeLevel == NULL) continue;

    itPlugin->pm_events.m_game->OnChangeLevel();
  }
};

// Called before stopping world simulation
void IHooks::OnGameStop(void)
{
  // Reset CAM for the next start
  GetGameAPI()->GetCamera().Reset();

  // Call game stop function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_game->OnGameStop == NULL) continue;

    itPlugin->pm_events.m_game->OnGameStop();
  }

  // Queue shadow updating again
  _bQueueShadowUpdate = TRUE;

  // Reset all clients
  CActiveClient::ResetAll();

  // Save client log by the end of the game
  IClientLogging::SaveLog();
};

// Called after saving the game
void IHooks::OnGameSave(const CTFileName &fnmSave)
{
  // Call game save function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_game->OnGameSave == NULL) continue;

    itPlugin->pm_events.m_game->OnGameSave(fnmSave);
  }
};

// Called after loading a saved game
void IHooks::OnGameLoad(const CTFileName &fnmSave)
{
  // Call game load function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_game->OnGameLoad == NULL) continue;

    itPlugin->pm_events.m_game->OnGameLoad(fnmSave);
  }

  // Update shadow maps
  if (gam_bAutoUpdateShadows) {
    UpdateShadows();
  }
};

// Called every time a new player is added
void IHooks::OnAddPlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player addition function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_network->OnAddPlayer == NULL) continue;

    itPlugin->pm_events.m_network->OnAddPlayer(plt, bLocal);
  }

  // Logic for connecting players
  if (bLocal && !_pNetwork->IsServer())
  {
    // Update shadow maps only if queued
    if (_bQueueShadowUpdate) {
      _bQueueShadowUpdate = FALSE;

      if (gam_bAutoUpdateShadows) {
        UpdateShadows();
      }
    }
  }
};

// Called every time a player is removed
void IHooks::OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player removal function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_network->OnRemovePlayer == NULL) continue;

    itPlugin->pm_events.m_network->OnRemovePlayer(plt, bLocal);
  }
};

// Called after starting demo playback
void IHooks::OnDemoPlay(const CTFileName &fnmDemo)
{
  // Start CAM for the demo
  GetGameAPI()->GetCamera().Start(fnmDemo);

  // Call demo play function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_demo->OnDemoPlay == NULL) continue;

    itPlugin->pm_events.m_demo->OnDemoPlay(fnmDemo);
  }

  // Update shadow maps
  if (gam_bAutoUpdateShadows) {
    UpdateShadows();
  }
};

// Called after starting demo recording
void IHooks::OnDemoStart(const CTFileName &fnmDemo)
{
  // Call demo start function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_demo->OnDemoStart == NULL) continue;

    itPlugin->pm_events.m_demo->OnDemoStart(fnmDemo);
  }
};

// Called after stopping demo recording
void IHooks::OnDemoStop(void)
{
  // Call demo stop function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_demo->OnDemoStop == NULL) continue;

    itPlugin->pm_events.m_demo->OnDemoStop();
  }
};

// Called after finishing reading the world file
void IHooks::OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld)
{
  // Call world load function for each plugin
  FOREACHPLUGIN(itPlugin) {
    if (itPlugin->pm_events.m_world->OnWorldLoad == NULL) continue;

    itPlugin->pm_events.m_world->OnWorldLoad(pwo, fnmWorld);
  }
};

// Interface initialization
namespace IInitAPI {

void Hooks(void) {
  // Update shadows in the current world
  _pShell->DeclareSymbol("user void UpdateShadows(void);", &UpdateShadows);
  _pShell->DeclareSymbol("persistent user INDEX gam_bAutoUpdateShadows;", &gam_bAutoUpdateShadows);
};

}; // namespace
