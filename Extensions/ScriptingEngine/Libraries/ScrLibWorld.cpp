/* Copyright (c) 2025-2026 Dreamy Cecil
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

namespace sq {

namespace World {

// Safe guard against running world functions outside the game
inline BOOL IsGameOn(void) {
  return GetGameAPI()->IsHooked() && GetGameAPI()->IsGameOn();
};

#define ASSERT_WORLD { if (!IsGameOn()) return sq_throwerror(v, "cannot execute functions from World interface while no game is running"); }

static SQInteger GetBackgroundColor(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushinteger(v, IWorld::GetWorld()->GetBackgroundColor());
  return 1;
};

static SQInteger GetBackgroundViewer(HSQUIRRELVM v) {
  ASSERT_WORLD;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = IWorld::GetWorld()->GetBackgroundViewer();
  return 1;
};

static SQInteger GetWSC(HSQUIRRELVM v) {
  ASSERT_WORLD;
  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = IWorld::GetWSC(IWorld::GetWorld());
  return 1;
};

static SQInteger GetFilename(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushstring(v, IWorld::GetWorld()->wo_fnmFileName.str_String, -1);
  return 1;
};

static SQInteger GetDescription(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushstring(v, IWorld::GetWorld()->GetDescription().str_String, -1);
  return 1;
};

static SQInteger GetName(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushstring(v, IWorld::GetWorld()->GetName().str_String, -1);
  return 1;
};

static SQInteger GetSpawnFlags(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushinteger(v, IWorld::GetWorld()->GetSpawnFlags());
  return 1;
};

static SQInteger GetEntities(HSQUIRRELVM v) {
  ASSERT_WORLD;

  SQBool bPredictors;
  sq_getbool(v, 2, &bPredictors);

  // Create an array that will act as a return value
  sq_newarray(v, 0);

  // Create an entity instance for each entity in the container and append it to the array
  FOREACHINDYNAMICCONTAINER(IWorld::GetWorld()->wo_cenEntities, CEntity, iten) {
    // Skip deleted entities
    if (iten->GetFlags() & ENF_DELETED) continue;

    // Skip predictors, if needed
    if (!bPredictors && iten->IsPredictor()) continue;

    PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
    *ppen = iten;

    sq_arrayappend(v, -2);
  }

  return 1;
};

static SQInteger FindEntityByID(HSQUIRRELVM v) {
  ASSERT_WORLD;

  SQInteger iEntityID;
  sq_getinteger(v, 2, &iEntityID);

  PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
  *ppen = IWorld::FindEntityByID(IWorld::GetWorld(), iEntityID);
  return 1;
};

static SQInteger FindClasses(HSQUIRRELVM v) {
  ASSERT_WORLD;

  const SQChar *strClass;
  sq_getstring(v, 2, &strClass);

  CEntities cOutput;
  IWorld::FindClasses(IWorld::GetWorld()->wo_cenEntities, cOutput, strClass);

  // Create an array that will act as a return value
  sq_newarray(v, 0);

  // Create an entity instance for each entity in the container and append it to the array
  FOREACHINDYNAMICCONTAINER(cOutput, CEntity, iten) {
    PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
    *ppen = iten;

    sq_arrayappend(v, -2);
  }

  return 1;
};

static SQInteger FindClassesByID(HSQUIRRELVM v) {
  ASSERT_WORLD;

  SQInteger iClassID;
  sq_getinteger(v, 2, &iClassID);

  CEntities cOutput;
  IWorld::FindClassesByID(IWorld::GetWorld()->wo_cenEntities, cOutput, iClassID);

  // Create an array that will act as a return value
  sq_newarray(v, 0);

  // Create an entity instance for each entity in the container and append it to the array
  FOREACHINDYNAMICCONTAINER(cOutput, CEntity, iten) {
    PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
    *ppen = iten;

    sq_arrayappend(v, -2);
  }

  return 1;
};

static SQInteger AnyLocalPlayers(HSQUIRRELVM v) {
  ASSERT_WORLD;
  sq_pushbool(v, IWorld::AnyLocalPlayers());
  return 1;
};

static SQInteger GetLocalPlayers(HSQUIRRELVM v) {
  ASSERT_WORLD;

  CPlayerEntities cOutput;
  IWorld::GetLocalPlayers(cOutput);

  // Create an array that will act as a return value
  sq_newarray(v, 0);

  // Create an entity instance for each entity in the container and append it to the array
  FOREACHINDYNAMICCONTAINER(cOutput, CPlayerEntity, iten) {
    PushNewInstance(CEntityPointer, ppen, GetVMClass(v).Root(), "CEntityPointer");
    *ppen = iten;

    sq_arrayappend(v, -2);
  }

  return 1;
};

}; // namespace

// "World" namespace functions
static SQRegFunction _aWorldFuncs[] = {
  // World
  { "GetBackgroundColor",  &World::GetBackgroundColor,  1, "." },
  { "GetBackgroundViewer", &World::GetBackgroundViewer, 1, "." },
  { "GetWSC",              &World::GetWSC,              1, "." },

  { "GetFilename",    &World::GetFilename,    1, "." },
  { "GetDescription", &World::GetDescription, 1, "." },
  { "GetName",        &World::GetName,        1, "." },
  { "GetSpawnFlags",  &World::GetSpawnFlags,  1, "." },

  // Entities
  { "GetEntities",     &World::GetEntities,     2, ".b" },
  { "FindEntityByID",  &World::FindEntityByID,  2, ".n" },
  { "FindClasses",     &World::FindClasses,     2, ".s" },
  { "FindClassesByID", &World::FindClassesByID, 2, ".n" },
  { "AnyLocalPlayers", &World::AnyLocalPlayers, 1, "." },
  { "GetLocalPlayers", &World::GetLocalPlayers, 1, "." },
};

void VM::RegisterWorld(void) {
  // Everything world-related should be read-only!!!
  Table sqtWorld = Root().RegisterTable("World");
  INDEX i;

  // Register functions
  for (i = 0; i < ARRAYCOUNT(_aWorldFuncs); i++) {
    sqtWorld.RegisterFunc(_aWorldFuncs[i]);
  }
};

}; // namespace
