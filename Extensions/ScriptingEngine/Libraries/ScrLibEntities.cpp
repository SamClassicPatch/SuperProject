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

// CEntity class methods
namespace SqEntity {

// Make sure the entity in the pointer exists
#define ASSERT_ENTITY { if (val == NULL) return sq_throwerror(v, "CEntityPointer is NULL"); }

static SQInteger Constructor(HSQUIRRELVM v, CEntityPointer &val) {
  CEntityPointer *pOther = InstanceValueOfType(v, 2, CEntityPointer);
  if (pOther != NULL) val = *pOther;

  return 0;
};

static SQInteger ToString(HSQUIRRELVM v, CEntityPointer &val) {
  if (val == NULL) {
    sq_pushstring(v, "(none)", -1);
    return 1;
  }

  CTString strEntity(0, "(%s : %u)", val->GetClass()->ec_pdecDLLClass->dec_strName, val->en_ulID);
  sq_pushstring(v, strEntity.str_String, -1);
  return 1;
};

static SQInteger Equal(HSQUIRRELVM v, CEntityPointer &val) {
  // Compare against null
  if (sq_gettype(v, 2) == OT_NULL) {
    sq_pushbool(v, val == NULL);
    return 1;
  }

  // Compare against another pointer
  CEntityPointer *pOther = InstanceValueOfType(v, 2, CEntityPointer);
  if (pOther == NULL) return sq_throwerror(v, "expected CEntityPointer or null value");

  sq_pushbool(v, val == *pOther);
  return 1;
};

static SQInteger GetID(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->en_ulID);
  return 1;
};

static SQInteger GetRenderType(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetRenderType());
  return 1;
};

static SQInteger GetFlags(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetFlags());
  return 1;
};

static SQInteger GetSpawnFlags(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetSpawnFlags());
  return 1;
};

static SQInteger GetPhysicsFlags(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetPhysicsFlags());
  return 1;
};

static SQInteger GetCollisionFlags(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetCollisionFlags());
  return 1;
};

static SQInteger IsPredictor(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredictor());
  return 1;
};

static SQInteger IsPredicted(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredicted());
  return 1;
};

static SQInteger IsPredictable(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushbool(v, val->IsPredictable());
  return 1;
};

static SQInteger GetPredictor(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val->GetPredictor();
  return 1;
};

static SQInteger GetPredicted(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val->GetPredicted();
  return 1;
};

static SQInteger GetSpatialClassificationRadius(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushfloat(v, val->en_fSpatialClassificationRadius);
  return 1;
};

static SQInteger GetSpatialClassificationBox(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  *pbox = val->en_boxSpatialClassification;
  return 1;
};

static SQInteger GetBoundingBox(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  val->GetBoundingBox(*pbox);
  return 1;
};

static SQInteger GetSize(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a box instance
  FLOATaabbox3D *pbox;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATaabbox3D", &pbox)) return SQ_ERROR;

  val->GetSize(*pbox);
  return 1;
};

static SQInteger GetEntityPointRatio(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get point ratio
  FLOAT3D *pvRatio = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvRatio == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Get optional lerped flag
  SQBool bLerped;
  if (SQ_FAILED(sq_getbool(v, 3, &bLerped))) bLerped = false;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  val->GetEntityPointRatio(*pvRatio, *pv, bLerped);
  return 1;
};

static SQInteger GetEntityPointFixed(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get fixed point
  FLOAT3D *pvFixed = InstanceValueOfType(v, 2, FLOAT3D);
  if (pvFixed == NULL) return sq_throwerror(v, "expected FLOAT3D value");

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  val->GetEntityPointFixed(*pvFixed, *pv);
  return 1;
};

static SQInteger GetPlacement(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a placement instance
  CPlacement3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("CPlacement3D", &ppl)) return SQ_ERROR;

  *ppl = val->GetPlacement();
  return 1;
};

static SQInteger GetLerpedPlacement(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a placement instance
  CPlacement3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("CPlacement3D", &ppl)) return SQ_ERROR;

  *ppl = val->GetLerpedPlacement();
  return 1;
};

static SQInteger GetRotationMatrix(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a matrix instance
  FLOATmatrix3D *pm;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOATmatrix3D", &pm)) return SQ_ERROR;

  *pm = val->GetRotationMatrix();
  return 1;
};

static SQInteger GetViewpoint(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Make sure it's a player entity
  if (!IsDerivedFromID(val, CPlayerEntity_ClassID)) return sq_throwerror(v, "cannot retrieve a viewpoint from a non-player entity");
  CPlayerEntity *penPlayer = (CPlayerEntity *)(CEntity *)val;

  // Get optional lerped flag
  SQBool bLerped;
  if (SQ_FAILED(sq_getbool(v, 2, &bLerped))) bLerped = false;

  // Create a placement instance
  CPlacement3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("CPlacement3D", &ppl)) return SQ_ERROR;

  *ppl = IWorld::GetViewpoint(penPlayer, bLerped);
  return 1;
};

static SQInteger GetPlacementRelativeToParent(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a placement instance
  CPlacement3D *ppl;
  if (!GetVMClass(v).Root().CreateInstanceOf("CPlacement3D", &ppl)) return SQ_ERROR;

  *ppl = val->en_plRelativeToParent;
  return 1;
};

static SQInteger GetParent(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val->GetParent();
  return 1;
};

static SQInteger GetChildOfClass(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  const SQChar *str;
  sq_getstring(v, 2, &str);

  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val->GetChildOfClass(str);
  return 1;
};

static SQInteger GetName(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetName().str_String, -1);
  return 1;
};

static SQInteger GetDescription(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetDescription().str_String, -1);
  return 1;
};

static SQInteger GetTarget(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create an entity instance
  CEntityPointer *ppen;
  if (!GetVMClass(v).Root().CreateInstanceOf("CEntityPointer", &ppen)) return SQ_ERROR;

  *ppen = val->GetTarget();
  return 1;
};

static SQInteger GetClassificationBoxStretch(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Create a vector instance
  FLOAT3D *pv;
  if (!GetVMClass(v).Root().CreateInstanceOf("FLOAT3D", &pv)) return SQ_ERROR;

  *pv = val->GetClassificationBoxStretch();
  return 1;
};

static SQInteger GetClassFile(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetClass()->GetName().str_String, -1);
  return 1;
};

static SQInteger GetClassName(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushstring(v, val->GetClass()->ec_pdecDLLClass->dec_strName, -1);
  return 1;
};

static SQInteger GetClassID(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;
  sq_pushinteger(v, val->GetClass()->ec_pdecDLLClass->dec_iID);
  return 1;
};

static SQInteger GetPropertyForId(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property ID
  SQInteger iID;
  sq_getinteger(v, 2, &iID);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForId(val, iID);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForHash(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property hash
  SQInteger iHash;
  sq_getinteger(v, 2, &iHash);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForHash(val, iHash);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForIdOrOffset(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type, ID and offset
  SQInteger iType, iID, iOffset;
  sq_getinteger(v, 2, &iType);
  sq_getinteger(v, 3, &iID);
  sq_getinteger(v, 4, &iOffset);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForIdOrOffset(val, iType, iID, iOffset);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForName(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type and name
  SQInteger iType;
  sq_getinteger(v, 2, &iType);
  const SQChar *strName;
  sq_getstring(v, 3, &strName);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForName(val, iType, strName);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForNameOrId(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get property type, name and ID
  SQInteger iType, iID;
  sq_getinteger(v, 2, &iType);
  const SQChar *strName;
  sq_getstring(v, 3, &strName);
  sq_getinteger(v, 4, &iID);

  // Make sure the property exists
  CEntityProperty *pepCheck = IWorld::PropertyForNameOrId(val, iType, strName, iID);
  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static SQInteger GetPropertyForVariable(HSQUIRRELVM v, CEntityPointer &val) {
  ASSERT_ENTITY;

  // Get class and variable names
  const SQChar *strClass;
  const SQChar *strVariable;
  sq_getstring(v, 2, &strClass);
  sq_getstring(v, 3, &strVariable);

  // Make sure the property exists
  CEntityProperty *pepCheck = NULL;

#if SE1_GAME != SS_REV
  // Find property data and try to find it by name or ID
  const CEntityProperty *pepData = FindPropertyByVariable(strClass, strVariable);

  if (pepData != NULL) {
    pepCheck = IWorld::PropertyForNameOrId(val, pepData->ep_eptType, pepData->ep_strName, pepData->ep_ulID);
  }
#else
  pepCheck = val->PropertyForVariable(strVariable);
#endif

  if (pepCheck == NULL) return sq_throwerror(v, "property doesn't exist");

  // Create a property instance
  CEntityProperty **ppep;
  Table sqtEntities(GetVMClass(v).Root().GetValue("Entities"));
  if (!sqtEntities.CreateInstanceOf("Property", &ppep)) return SQ_ERROR;

  *ppep = pepCheck;
  return 1;
};

static Method<CEntityPointer> _aMethods[] = {
  { "Equal", &Equal, 2, ".x|o" },

  // Internal
  { "GetID", &GetID, 1, "." },
  { "GetRenderType", &GetRenderType, 1, "." },

  { "GetFlags",          &GetFlags,          1, "." },
  { "GetSpawnFlags",     &GetSpawnFlags,     1, "." },
  { "GetPhysicsFlags",   &GetPhysicsFlags,   1, "." },
  { "GetCollisionFlags", &GetCollisionFlags, 1, "." },

  { "IsPredictor",   &IsPredictor,   1, "." },
  { "IsPredicted",   &IsPredicted,   1, "." },
  { "IsPredictable", &IsPredictable, 1, "." },
  { "GetPredictor",  &GetPredictor,  1, "." },
  { "GetPredicted",  &GetPredicted,  1, "." },

  // Placement
  { "GetSpatialClassificationRadius", &GetSpatialClassificationRadius, 1, "." },
  { "GetSpatialClassificationBox",    &GetSpatialClassificationBox,    1, "." },
  { "GetBoundingBox",      &GetBoundingBox,       1, "." },
  { "GetSize",             &GetSize,              1, "." },
  { "GetEntityPointRatio", &GetEntityPointRatio, -2, ".xb" },
  { "GetEntityPointFixed", &GetEntityPointFixed,  2, ".x" },

  { "GetPlacement",        &GetPlacement,       1, "." },
  { "GetLerpedPlacement",  &GetLerpedPlacement, 1, "." },
  { "GetRotationMatrix",   &GetRotationMatrix,  1, "." },
  { "GetViewpoint",        &GetViewpoint,      -1, ".b" },

  // Parents and children
  { "GetPlacementRelativeToParent", &GetPlacementRelativeToParent, 1, "." },
  { "GetParent",       &GetParent,       1, "." },
  { "GetChildOfClass", &GetChildOfClass, 2, ".s" },

  // Class-specific overloads
  { "GetName",        &GetName,        1, "." },
  { "GetDescription", &GetDescription, 1, "." },
  { "GetTarget",      &GetTarget,      1, "." },
  { "GetClassificationBoxStretch", &GetClassificationBoxStretch, 1, "." },

  // Entity class data
  { "GetClassFile", &GetClassFile, 1, "." },
  { "GetClassName", &GetClassName, 1, "." },
  { "GetClassID",   &GetClassID,   1, "." },

  // Property lookup
  { "GetPropertyForId",         &GetPropertyForId,         2, ".n" },
  { "GetPropertyForHash",       &GetPropertyForHash,       2, ".n" },
  { "GetPropertyForIdOrOffset", &GetPropertyForIdOrOffset, 4, ".nnn" },
  { "GetPropertyForName",       &GetPropertyForName,       3, ".ns" },
  { "GetPropertyForNameOrId",   &GetPropertyForNameOrId,   4, ".nsn" },
  { "GetPropertyForVariable",   &GetPropertyForVariable,   3, ".ss" },
};

}; // namespace

// CEntityProperty class methods
namespace SqProp {

static SQInteger Constructor(HSQUIRRELVM v, CEntityProperty *&val) {
  CEntityProperty **pOther = InstanceValueOfType(v, 2, CEntityProperty *);
  if (pOther != NULL) val = *pOther;

  return 0;
};

static SQInteger GetPropType(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_eptType);
  return 1;
};

static SQInteger GetPropID(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_ulID);
  return 1;
};

static SQInteger GetPropOffset(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_slOffset);
  return 1;
};

static SQInteger GetPropName(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushstring(v, val->ep_strName, -1);
  return 1;
};

static SQInteger GetPropFlags(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_ulFlags);
  return 1;
};

static SQInteger GetPropShortcut(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_chShortcut);
  return 1;
};

static SQInteger GetPropColor(HSQUIRRELVM v, CEntityProperty *&val) {
  sq_pushinteger(v, val->ep_colColor);
  return 1;
};

static SQInteger GetEnumValueCount(HSQUIRRELVM v, CEntityProperty *&val) {
  if (val->ep_pepetEnumType == NULL) {
    return sq_throwerror(v, "entity property does not point to any enum type");
  }

  sq_pushinteger(v, val->ep_pepetEnumType->epet_ctValues);
  return 1;
};

static SQInteger GetEnumValues(HSQUIRRELVM v, CEntityProperty *&val) {
  if (val->ep_pepetEnumType == NULL) {
    return sq_throwerror(v, "entity property does not point to any enum type");
  }

  // Create an array that will contain all enum values
  sq_newarray(v, 0);

  for (INDEX i = 0; i < val->ep_pepetEnumType->epet_ctValues; i++) {
    CEntityPropertyEnumValue &epev = val->ep_pepetEnumType->epet_aepevValues[i];

    // Create an array for each enum value that contains its index and name
    sq_newarray(v, 0);

    sq_pushinteger(v, epev.epev_iValue);
    sq_arrayappend(v, -2);
    sq_pushstring(v, epev.epev_strName, -1);
    sq_arrayappend(v, -2);

    // Append this array to the main one
    sq_arrayappend(v, -2);
  }

  return 1;
};

static Method<CEntityProperty *> _aMethods[] = {
  { "GetEnumValueCount", &GetEnumValueCount, 1, "." },
  { "GetEnumValues", &GetEnumValues, 1, "." },
};

}; // namespace

namespace Entities {

}; // namespace

void VM::RegisterEntities(void) {
  // [Cecil] NOTE: Everything entity-related should be read-only!!!
  // Creation and modification should be handled exclusively through extension packets for proper synchronization!
  Table sqtEntities = Root().RegisterTable("Entities");
  INDEX i;

  // Register classes
  {
    Class<CEntityPointer> sqcEntity(GetVM(), "CEntityPointer", &SqEntity::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqEntity::_aMethods); i++) {
      sqcEntity.RegisterMethod(SqEntity::_aMethods[i]);
    }

    // Metamethods
    sqcEntity.RegisterMetamethod(E_MM_TOSTRING, &SqEntity::ToString);

    Root().AddClass(sqcEntity);
  }
  {
    Class<CEntityProperty *> sqcProp(GetVM(), "Property", &SqProp::Constructor);

    // Methods
    for (i = 0; i < ARRAYCOUNT(SqProp::_aMethods); i++) {
      sqcProp.RegisterMethod(SqProp::_aMethods[i]);
    }

    sqcProp.RegisterVar("type",     &SqProp::GetPropType,     NULL);
    sqcProp.RegisterVar("id",       &SqProp::GetPropID,       NULL);
    sqcProp.RegisterVar("offset",   &SqProp::GetPropOffset,   NULL);
    sqcProp.RegisterVar("name",     &SqProp::GetPropName,     NULL);
    sqcProp.RegisterVar("flags",    &SqProp::GetPropFlags,    NULL);
    sqcProp.RegisterVar("shortcut", &SqProp::GetPropShortcut, NULL);
    sqcProp.RegisterVar("color",    &SqProp::GetPropColor,    NULL);

    sqtEntities.AddClass(sqcProp);
  }

  // Render types
  Enumeration enRenderTypes(GetVM());

#define ADD_RT(_RenderTypeName) enRenderTypes.RegisterValue(#_RenderTypeName, (SQInteger)CEntity::RT_##_RenderTypeName)
  ADD_RT(ILLEGAL);
  ADD_RT(NONE);
  ADD_RT(MODEL);
  ADD_RT(BRUSH);
  ADD_RT(EDITORMODEL);
  ADD_RT(VOID);
  ADD_RT(FIELDBRUSH);
  ADD_RT(SKAMODEL);
  ADD_RT(SKAEDITORMODEL);
  ADD_RT(TERRAIN);
#undef ADD_RT

  Const().AddEnum("RT", enRenderTypes);

#define ADD_FLAG(_FlagType, _FlagName) enFlags.RegisterValue(#_FlagName, (SQInteger)_FlagType##_##_FlagName)
  {
    // Spawn flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(SPF, EASY);
    ADD_FLAG(SPF, NORMAL);
    ADD_FLAG(SPF, HARD);
    ADD_FLAG(SPF, EXTREME);
    ADD_FLAG(SPF, TOURIST);
    ADD_FLAG(SPF, MASK_DIFFICULTY);
    ADD_FLAG(SPF, SINGLEPLAYER);
    ADD_FLAG(SPF, DEATHMATCH);
    ADD_FLAG(SPF, COOPERATIVE);
    ADD_FLAG(SPF, FLYOVER);
    ADD_FLAG(SPF, MASK_GAMEMODE);
    Const().AddEnum("SPF", enFlags);
  }
  {
    // Entity flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(ENF, SELECTED);
    ADD_FLAG(ENF, ZONING);
    ADD_FLAG(ENF, DELETED);
    ADD_FLAG(ENF, ALIVE);
    ADD_FLAG(ENF, INRENDERING);
    ADD_FLAG(ENF, VALIDSHADINGINFO);
    ADD_FLAG(ENF, SEETHROUGH);
    ADD_FLAG(ENF, FOUNDINGRIDSEARCH);
    ADD_FLAG(ENF, CLUSTERSHADOWS);
    ADD_FLAG(ENF, BACKGROUND);
    ADD_FLAG(ENF, ANCHORED);
    ADD_FLAG(ENF, HASPARTICLES);
    ADD_FLAG(ENF, INVISIBLE);
    ADD_FLAG(ENF, DYNAMICSHADOWS);
    ADD_FLAG(ENF, NOTIFYLEVELCHANGE);
    ADD_FLAG(ENF, CROSSESLEVELS);
    ADD_FLAG(ENF, PREDICTABLE);
    ADD_FLAG(ENF, PREDICTOR);
    ADD_FLAG(ENF, PREDICTED);
    ADD_FLAG(ENF, WILLBEPREDICTED);
    ADD_FLAG(ENF, TEMPPREDICTOR);
    ADD_FLAG(ENF, HIDDEN);
    ADD_FLAG(ENF, NOSHADINGINFO);
    Const().AddEnum("ENF", enFlags);
  }
  {
    // Physics flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(EPF, ORIENTEDBYGRAVITY);
    ADD_FLAG(EPF, TRANSLATEDBYGRAVITY);
    ADD_FLAG(EPF, PUSHABLE);
    ADD_FLAG(EPF, STICKYFEET);
    ADD_FLAG(EPF, RT_SYNCHRONIZED);
    ADD_FLAG(EPF, ABSOLUTETRANSLATE);
    ADD_FLAG(EPF, NOACCELERATION);
    ADD_FLAG(EPF, HASLUNGS);
    ADD_FLAG(EPF, HASGILLS);
    ADD_FLAG(EPF, MOVABLE);
    ADD_FLAG(EPF, NOIMPACT);
    ADD_FLAG(EPF, NOIMPACTTHISTICK);
    ADD_FLAG(EPF, CANFADESPINNING);
    ADD_FLAG(EPF, ONSTEEPSLOPE);
    ADD_FLAG(EPF, ORIENTINGTOGRAVITY);
    ADD_FLAG(EPF, FLOATING);
    ADD_FLAG(EPF, FORCEADDED);

    ADD_FLAG(EPF, ONBLOCK_MASK);
    ADD_FLAG(EPF, ONBLOCK_STOP);
    ADD_FLAG(EPF, ONBLOCK_SLIDE);
    ADD_FLAG(EPF, ONBLOCK_CLIMBORSLIDE);
    ADD_FLAG(EPF, ONBLOCK_BOUNCE);
    ADD_FLAG(EPF, ONBLOCK_PUSH);
    ADD_FLAG(EPF, ONBLOCK_STOPEXACT);
    Const().AddEnum("EPF", enFlags);
  }
  {
    // Collision bits
    Enumeration enFlags(GetVM());
    ADD_FLAG(ECB, COUNT);
    ADD_FLAG(ECB, IS);
    ADD_FLAG(ECB, TEST);
    ADD_FLAG(ECB, PASS);
    Const().AddEnum("ECB", enFlags);
  }
  {
    // Collision flags
    Enumeration enFlags(GetVM());
    ADD_FLAG(ECF, MASK);
    ADD_FLAG(ECF, ISMASK);
    ADD_FLAG(ECF, TESTMASK);
    ADD_FLAG(ECF, PASSMASK);
    ADD_FLAG(ECF, IGNOREBRUSHES);
    ADD_FLAG(ECF, IGNOREMODELS);
    Const().AddEnum("ECF", enFlags);
  }
#undef ADD_FLAG

  // Property types
  Enumeration enPropTypes(GetVM());

#define ADD_EPT(_PropTypeName) enPropTypes.RegisterValue(#_PropTypeName, (SQInteger)CEntityProperty::EPT_##_PropTypeName)
  ADD_EPT(ENUM);
  ADD_EPT(BOOL);
  ADD_EPT(FLOAT);
  ADD_EPT(COLOR);
  ADD_EPT(STRING);
  ADD_EPT(RANGE);
  ADD_EPT(ENTITYPTR);
  ADD_EPT(FILENAME);
  ADD_EPT(INDEX);
  ADD_EPT(ANIMATION);
  ADD_EPT(ILLUMINATIONTYPE);
  ADD_EPT(FLOATAABBOX3D);
  ADD_EPT(ANGLE);
  ADD_EPT(FLOAT3D);
  ADD_EPT(ANGLE3D);
  ADD_EPT(FLOATplane3D);
  ADD_EPT(MODELOBJECT);
  ADD_EPT(PLACEMENT3D);
  ADD_EPT(ANIMOBJECT);
  ADD_EPT(FILENAMENODEP);
  ADD_EPT(SOUNDOBJECT);
  ADD_EPT(STRINGTRANS);
  ADD_EPT(FLOATQUAT3D);
  ADD_EPT(FLOATMATRIX3D);
  ADD_EPT(FLAGS);
  ADD_EPT(MODELINSTANCE);
#undef ADD_EPT

  Const().AddEnum("EPT", enPropTypes);
};

}; // namespace
