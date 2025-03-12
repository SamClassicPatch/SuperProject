/* Copyright (c) 2022-2024 Dreamy Cecil
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

#include <CoreLib/Networking/ExtPackets.h>
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

// Define plugin
CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagGame | k_EPluginFlagEditor, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Local Cheats",
  "Local client cheats such as noclip and weapon creation that can be used regardless of gamemode or mod. Not multiplayer synchronized!");

CPluginSymbol _psAutoKill(SSF_USER, INDEX(0));
CPluginSymbol _psAutoKillRange(SSF_PERSISTENT | SSF_USER, 256.0f);

// Container of local player entities
CDynamicContainer<CPlayerEntity> _cenPlayers;

// Walk on any solid polygon
static void WallWalking(void) {
  SERVER_CLIENT_ONLY;

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;

    const BOOL bDisable = !!(pen->GetPhysicsFlags() & EPF_STICKYFEET);

  #if _PATCHCONFIG_EXT_PACKETS
    CExtEntityFlags pck;
    pck("ulEntity", (int)pen->en_ulID);
    pck.PhysicalFlags(EPF_STICKYFEET, bDisable);
    pck.SendToClients();

  #else
    if (bDisable) {
      pen->SetPhysicsFlags(pen->GetPhysicsFlags() & ~EPF_STICKYFEET);
    } else {
      pen->SetPhysicsFlags(pen->GetPhysicsFlags() | EPF_STICKYFEET);
    }
  #endif

    CPrintF(TRANS("%s^r - wall walking: %s\n"), pen->GetName(), (bDisable ? "^cff0000OFF" : "^c00ff00ON"));
  }
};

// Fly around the map
static void Noclip(void) {
  SERVER_CLIENT_ONLY;

  const ULONG ulWorldCollision = (((1UL << 0) | (1UL << 1)) << ECB_TEST);
  const ULONG ulPhysFlags = (EPF_TRANSLATEDBYGRAVITY | EPF_ORIENTEDBYGRAVITY);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;

    const BOOL bDisable = !(pen->GetCollisionFlags() & ulWorldCollision);

    if (bDisable) {
      pen->SetCollisionFlags(pen->GetCollisionFlags() | ulWorldCollision);
      pen->SetPhysicsFlags(pen->GetPhysicsFlags() | ulPhysFlags);

    } else {
      pen->SetCollisionFlags(pen->GetCollisionFlags() & ~ulWorldCollision);
      pen->SetPhysicsFlags(pen->GetPhysicsFlags() & ~ulPhysFlags);

      pen->en_plViewpoint.pl_OrientationAngle = ANGLE3D(0.0f, 0.0f, 0.0f);
    }

    CPrintF(TRANS("%s^r - noclip: %s\n"), pen->GetName(), (bDisable ? "^cff0000OFF" : "^c00ff00ON"));
  }
};

// Set health for all local players
static void SetHealth(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iHealth = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;

  #if _PATCHCONFIG_EXT_PACKETS
    CExtEntityHealth pck;
    pck("ulEntity", (int)pen->en_ulID);
    pck("fHealth", (FLOAT)iHealth);
    pck.SendToClients();
  #else
    pen->SetHealth(iHealth);
  #endif

    CPrintF(TRANS("Set %s^r health to %d\n"), pen->GetName(), iHealth);
  }
};

// Trigger an entity at the crosshair position
static void Trigger(void) {
  SERVER_CLIENT_ONLY;

  VNL_ETrigger eTrigger;

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CPlacement3D plView = IWorld::GetViewpoint(pen, FALSE);

    CCastRay crRay(pen, plView);
    crRay.cr_bHitTranslucentPortals = FALSE;
    crRay.cr_bPhysical = FALSE;
    crRay.cr_ttHitModels = CCastRay::TT_COLLISIONBOX;

    IWorld::GetWorld()->CastRay(crRay);

    if (crRay.cr_penHit == NULL) continue;

  #if _PATCHCONFIG_EXT_PACKETS
    (ULONG &)eTrigger.penCaused = pen->en_ulID;

    CExtEntityEvent pck;
    pck("ulEntity", (int)crRay.cr_penHit->en_ulID);
    pck.SetEvent(eTrigger, sizeof(eTrigger));
    pck.SendToClients();

    (ULONG &)eTrigger.penCaused = NULL;
  #else
    eTrigger.penCaused = pen;
    crRay.cr_penHit->SendEvent(eTrigger);
  #endif
  }
};

// Create specific item of a specific type
static void CreateItem(CEntity *penPlayer, const CTString &strClass,
                       const CTString &strPropName, INDEX iPropID, INDEX iType) {
#if _PATCHCONFIG_EXT_PACKETS
  // Send packet to create the entity
  CExtEntityCreate pckCreate;
  pckCreate("fnmClass", strClass);
  pckCreate("plPos", penPlayer->GetPlacement());
  pckCreate.SendToClients();

  // Send packet to change the property
  CExtEntityProp pckProp;
  pckProp("ulEntity", (int)0);
  if (strPropName != "") {
    pckProp.SetProperty(strPropName);
  } else {
    pckProp.SetProperty(iPropID);
  }
  pckProp.SetValue(iType);
  pckProp.SendToClients();

  // Send packet to initialize the entity
  CExtEntityInit pckInit;
  pckInit("ulEntity", (int)0);
  pckInit.SetEvent(EVoid(), sizeof(EVoid));
  pckInit.SendToClients();

  // Report entity name with the type ID instead of the type name with an entity ID
  CTString strClassName = ((const CTFileName &)strClass).FileName();
  CPrintF(TRANS("%s^r created '%s' item (%u)\n"), penPlayer->GetName(), strClassName, iType);

#else
  // Create weapon item
  CEntity *penWeapon = IWorld::GetWorld()->CreateEntity_t(penPlayer->GetPlacement(), strClass);

  // Retrieve type property
  CEntityProperty *pep = IWorld::PropertyForNameOrId(penWeapon, CEntityProperty::EPT_ENUM, strPropName, iPropID);
  CTString strItem = "";

  // Property found
  if (pep != NULL) {
    strItem = pep->ep_pepetEnumType->NameForValue(iType);
  }

  // Unknown item type
  if (strItem == "") {
    strItem = TRANS("<unknown>");
    iType = 1; // Default valid type
  }

  // Initialize the item
  ENTITYPROPERTY(penWeapon, pep->ep_slOffset, INDEX) = iType;
  penWeapon->Initialize();

  CPrintF(TRANS("%s^r created '%s' item (%u)\n"), penPlayer->GetName(), strItem, penWeapon->en_ulID);
#endif
};

// Create weapon entity
static void CreateWeapon(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iType = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CreateItem(pen, "Classes\\WeaponItem.ecl", "Type", (0x322 << 8) + 1, iType);
  }
};

// Create ammo entity
static void CreateAmmo(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iType = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CreateItem(pen, "Classes\\AmmoItem.ecl", "Type", (0x323 << 8) + 1, iType);
  }
};

// Create health entity
static void CreateHealth(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iType = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CreateItem(pen, "Classes\\HealthItem.ecl", "Type", (0x321 << 8) + 1, iType);
  }
};

// Create armor entity
static void CreateArmor(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iType = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CreateItem(pen, "Classes\\ArmorItem.ecl", "Type", (0x324 << 8) + 1, iType);
  }
};

// Create powerup entity
static void CreatePowerUp(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  SERVER_CLIENT_ONLY;

  INDEX iType = NEXT_ARG(INDEX);

  FOREACHPLAYER(iten) {
    CPlayerEntity *pen = iten;
    CreateItem(pen, "Classes\\PowerUpItem.ecl", "Type", (0x328 << 8) + 1, iType);
  }
};

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(CIniConfig &props, PluginEvents_t &events)
{
  // Register plugin events
  events.m_processing->OnStep = &IProcessingEvents_OnStep;

  // Custom symbols
  _psAutoKill.Register("cht_iAutoKill");
  _psAutoKillRange.Register("cht_fAutoKillRange");

  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_WallWalking",  "void",  &WallWalking);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_Noclip",       "void",  &Noclip);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_SetHealth",    "INDEX", &SetHealth);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_Trigger",      "void",  &Trigger);

  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_CreateWeapon",  "INDEX", &CreateWeapon);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_CreateAmmo",    "INDEX", &CreateAmmo);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_CreateHealth",  "INDEX", &CreateHealth);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_CreateArmor",   "INDEX", &CreateArmor);
  GetPluginAPI()->RegisterMethod(TRUE, "void", "cht_CreatePowerUp", "INDEX", &CreatePowerUp);
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(CIniConfig &props)
{
};
