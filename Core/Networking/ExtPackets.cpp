/* Copyright (c) 2023-2025 Dreamy Cecil
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

#include "ExtPackets.h"
#include "NetworkFunctions.h"
#include "Modules/PacketCommands.h"

#define VANILLA_EVENTS_ENTITY_ID
#include <Extras/XGizmo/Vanilla/EntityEvents.h>

#if _PATCHCONFIG_EXT_PACKETS

// Report packet actions to the server
INDEX ser_bReportExtPacketLogic = TRUE;

void ClassicsPackets_ServerReport(IClassicsExtPacket *pExtPacket, const char *strFormat, ...)
{
  // Ignore reports
  if (!_pNetwork->IsServer() || !ser_bReportExtPacketLogic) return;

  va_list arg;
  va_start(arg, strFormat);

  CTString str;
  str.VPrintF(strFormat, arg);

  // Append packet name in the beginning
  CPrintF("[%s] %s", pExtPacket->GetName(), str);
  va_end(arg);
};

void ClassicsPackets_SendToClients(IClassicsExtPacket *pExtPacket)
{
  // Not running a server
  if (!_pNetwork->IsServer()) return;

  // Remember last value
  INDEX &iLastSequence = _pNetwork->ga_srvServer.srv_iLastProcessedSequence;
  const INDEX iLastValue = iLastSequence;

  CNetStreamBlock nsbExt = INetwork::CreateServerPacket(pExtPacket->GetType());

  if (pExtPacket->Write(nsbExt)) {
    INetwork::AddBlockToAllSessions(nsbExt);

  // Restore the value since the packet has been discarded
  } else {
    iLastSequence = iLastValue;
  }
};

void ClassicsPackets_SendToServer(IClassicsExtPacket *pExtPacket)
{
  CNetworkMessage nmExt = INetwork::CreateClientPacket(pExtPacket->GetType());

  if (pExtPacket->Write(nmExt)) {
    _pNetwork->SendToServerReliable(nmExt);
  }
};

// [Cecil] TEMP: Display a warning regarding some built-in packet property
#define PACKET_PROP_WARNING(_Packet, _Property, _Warning) \
  CPrintF("[%s] ^cff0000'%s' - %s\n", _Packet->GetName(), _Property, _Warning)

bool ClassicsPackets_GetBoolProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return false;

  return pval->IsTrue();
};

int ClassicsPackets_GetIntProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return 0;

  return pval->ToIndex();
};

double ClassicsPackets_GetFloatProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return 0.0;

  return pval->ToFloat();
};

const char *ClassicsPackets_GetStringProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return "";

  if (pval->GetType() != CAnyValue::E_VAL_STRING) return "";
  return pval->GetString().str_String;
};

bool ClassicsPackets_SetBoolProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, bool bValue) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return false;

  if (pval->GetType() == CAnyValue::E_VAL_BOOL || pval->GetType() == CAnyValue::E_VAL_INDEX) {
    pval->GetIndex() = bValue;

  } else {
    PACKET_PROP_WARNING(pExtPacket, strProperty, "Cannot set property to a bool value!");
    return false;
  }

  return true;
};

bool ClassicsPackets_SetIntProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, int iValue) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return false;

  if (pval->GetType() == CAnyValue::E_VAL_BOOL || pval->GetType() == CAnyValue::E_VAL_INDEX) {
    pval->GetIndex() = iValue;

  } else {
    PACKET_PROP_WARNING(pExtPacket, strProperty, "Cannot set property to an integer value!");
    return false;
  }

  return true;
};

bool ClassicsPackets_SetFloatProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, double fValue) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return false;

  if (pval->GetType() == CAnyValue::E_VAL_FLOAT) {
    pval->GetFloat() = fValue;

  } else if (pval->GetType() == CAnyValue::E_VAL_DOUBLE) {
    pval->GetDouble() = fValue;

  } else {
    PACKET_PROP_WARNING(pExtPacket, strProperty, "Cannot set property to a float value!");
    return false;
  }

  return true;
};

bool ClassicsPackets_SetStringProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, const char *strValue) {
  CExtPacket &pck = *(CExtPacket *)pExtPacket;
  CAnyValue *pval = pck.GetValue(strProperty);
  if (pval == NULL) return false;

  if (pval->GetType() == CAnyValue::E_VAL_STRING) {
    pval->GetString() = strValue;

  } else {
    PACKET_PROP_WARNING(pExtPacket, strProperty, "Cannot set property to a string value!");
    return false;
  }

  return true;
};

IClassicsBuiltInExtPacket *ClassicsPackets_Create(IClassicsExtPacket::EPacketType ePacket) {
  return CExtPacket::CreatePacket(ePacket);
};

void ClassicsPackets_Destroy(IClassicsBuiltInExtPacket *pExtPacket) {
  delete pExtPacket;
};

// Retrieve an entity from an ID
CEntity *CExtEntityPacket::FindExtEntity(ULONG ulID) {
  // Take last created entity if ID is 0
  CEntity *pen = CExtEntityCreate::penLast;

  if (ulID != 0) {
    pen = IWorld::FindEntityByID(IWorld::GetWorld(), ulID);
  }

  return pen;
};

// Make sure to return some entity from the ID
CEntity *CExtEntityPacket::GetEntity(void) {
  if (!IsEntityValid()) {
    ClassicsPackets_ServerReport(this, TRANS("Received invalid entity ID!\n"));
    return NULL;
  }

  const ULONG ulEntity = props["ulEntity"].GetIndex();
  CEntity *pen = FindExtEntity(ulEntity);

  if (pen == NULL) {
    ClassicsPackets_ServerReport(this, TRANS("Received invalid entity ID!\n"));
    return NULL;
  }

  return pen;
};

// Convenient value getter
CAnyValue *CExtPacket::GetValue(const CTString &strVariable) {
  se1::map<CTString, CAnyValue>::iterator it = props.find(strVariable);

  if (it == props.end()) {
    PACKET_PROP_WARNING(this, strVariable, "Property doesn't exist!");
    return NULL;
  }

  return &it->second;
};

// Convenient value setter
bool CExtPacket::operator()(const CTString &strVariable, const CAnyValue &val) {
  se1::map<CTString, CAnyValue>::iterator it = props.find(strVariable);

  if (it == props.end()) {
    PACKET_PROP_WARNING(this, strVariable, "Property doesn't exist!");
    return false;
  }

  if (it->second.GetType() != val.GetType()) {
    PACKET_PROP_WARNING(this, strVariable, CTString(0, "Cannot set value! Expected type %d but got %d!", it->second.GetType(), val.GetType()));
    return false;
  }

  it->second = val;
  return true;
};

// Create new packet from type
CExtPacket *CExtPacket::CreatePacket(EPacketType ePacket)
{
  switch (ePacket) {
    // Server to client
    case k_EPacketType_EntityCreate  : return new CExtEntityCreate();
    case k_EPacketType_EntityDelete  : return new CExtEntityDelete();
    case k_EPacketType_EntityCopy    : return new CExtEntityCopy();
    case k_EPacketType_EntityEvent   : return new CExtEntityEvent();
    case k_EPacketType_EntityItem    : return new CExtEntityItem();
    case k_EPacketType_EntityInit    : return new CExtEntityInit();
    case k_EPacketType_EntityTeleport: return new CExtEntityTeleport();
    case k_EPacketType_EntityPosition: return new CExtEntityPosition();
    case k_EPacketType_EntityParent  : return new CExtEntityParent();
    case k_EPacketType_EntityProp    : return new CExtEntityProp();
    case k_EPacketType_EntityHealth  : return new CExtEntityHealth();
    case k_EPacketType_EntityFlags   : return new CExtEntityFlags();
    case k_EPacketType_EntityMove    : return new CExtEntityMove();
    case k_EPacketType_EntityRotate  : return new CExtEntityRotate();
    case k_EPacketType_EntityImpulse : return new CExtEntityImpulse();
    case k_EPacketType_EntityDirDmg  : return new CExtEntityDirectDamage();
    case k_EPacketType_EntityRadDmg  : return new CExtEntityRangeDamage();
    case k_EPacketType_EntityBoxDmg  : return new CExtEntityBoxDamage();

    case k_EPacketType_ChangeLevel : return new CExtChangeLevel();
    case k_EPacketType_ChangeWorld : return new CExtChangeWorld();
    case k_EPacketType_SessionProps: return new CExtSessionProps();
    case k_EPacketType_GameplayExt : return new CExtGameplayExt();
    case k_EPacketType_PlaySound   : return new CExtPlaySound();

    // Client to server
    // ...
  }

  // Invalid packet
  ASSERT(FALSE);
  return NULL;
};

// [Cecil] TEMP: Get entity of a specific class under a certain index
static INDEX GetEntity(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  const CTString &strClass = *NEXT_ARG(CTString *);
  INDEX iEntity = NEXT_ARG(INDEX);

  CEntities cen;
  IWorld::FindClasses(IWorld::GetWorld()->wo_cenEntities, cen, strClass);

  if (iEntity < cen.Count()) {
    return cen[iEntity].en_ulID;
  }

  return -1;
};

// Register the module
void CExtPacket::RegisterExtPackets(void)
{
  _pShell->DeclareSymbol("persistent user INDEX ser_bReportExtPacketLogic;", &ser_bReportExtPacketLogic);

  // [Cecil] TEMP: Get entity of a specific class under a certain index
  _pShell->DeclareSymbol("user INDEX GetEntity(CTString, INDEX);", &GetEntity);

  // Declare extra symbols
  void DeclareExtraSymbolsForExtPackets(void);
  DeclareExtraSymbolsForExtPackets();

  // Event setup
  _pShell->DeclareSymbol("user void pck_SetupEvent(INDEX);", &IPacketCommands::SetupEvent);
  _pShell->DeclareSymbol("user void pck_EventFieldIndex(INDEX, INDEX);", &IPacketCommands::EventFieldIndex);
  _pShell->DeclareSymbol("user void pck_EventFieldFloat(INDEX, FLOAT);", &IPacketCommands::EventFieldFloat);
  _pShell->DeclareSymbol("user void pck_EventFieldVector(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EventFieldVector);

  // Entity instantiation
  _pShell->DeclareSymbol("user void pck_EntityCreate(CTString);", &IPacketCommands::EntityCreate);
  _pShell->DeclareSymbol("user void pck_EntityDelete(INDEX, INDEX);", &IPacketCommands::EntityDelete);
  _pShell->DeclareSymbol("user void pck_EntityCopy(INDEX, INDEX);", &IPacketCommands::EntityCopy);

  // Entity logic
  _pShell->DeclareSymbol("user void pck_EntityEvent(INDEX);", &IPacketCommands::EntityEvent);
  _pShell->DeclareSymbol("user void pck_EntityItem(INDEX);", &IPacketCommands::EntityItem);
  _pShell->DeclareSymbol("user void pck_EntityInit(INDEX);", &IPacketCommands::EntityInit);
  _pShell->DeclareSymbol("user void pck_EntityInitEvent(INDEX);", &IPacketCommands::EntityInitEvent);

  // Entity placement
  _pShell->DeclareSymbol("user void pck_EntitySetPos(INDEX, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntitySetPos);
  _pShell->DeclareSymbol("user void pck_EntitySetRot(INDEX, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntitySetRot);
  _pShell->DeclareSymbol("user void pck_EntityTeleport(INDEX, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, INDEX);", &IPacketCommands::EntityTeleport);

  // Entity properties
  _pShell->DeclareSymbol("user void pck_EntityParent(INDEX, INDEX);", &IPacketCommands::EntityParent);
  _pShell->DeclareSymbol("user void pck_EntityNumberProp(INDEX, CTString, INDEX, FLOAT);", &IPacketCommands::EntityNumberProp);
  _pShell->DeclareSymbol("user void pck_EntityStringProp(INDEX, CTString, INDEX, CTString);", &IPacketCommands::EntityStringProp);
  _pShell->DeclareSymbol("user void pck_EntityHealth(INDEX, FLOAT);", &IPacketCommands::EntityHealth);

  // Entity flags
  _pShell->DeclareSymbol("user void pck_EntityFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityFlags);
  _pShell->DeclareSymbol("user void pck_EntityPhysicalFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityPhysicalFlags);
  _pShell->DeclareSymbol("user void pck_EntityCollisionFlags(INDEX, INDEX, INDEX);", &IPacketCommands::EntityCollisionFlags);

  // Entity movement
  _pShell->DeclareSymbol("user void pck_EntityMove(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityMove);
  _pShell->DeclareSymbol("user void pck_EntityRotate(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityRotate);
  _pShell->DeclareSymbol("user void pck_EntityImpulse(INDEX, FLOAT, FLOAT, FLOAT);", &IPacketCommands::EntityImpulse);

  // Entity damage
  _pShell->DeclareSymbol("user void pck_SetupDamage(INDEX, INDEX, FLOAT);", &IPacketCommands::SetupDamage);
  _pShell->DeclareSymbol("user void pck_SetDirectDamage(INDEX, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetDirectDamage);
  _pShell->DeclareSymbol("user void pck_SetRangeDamage(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetRangeDamage);
  _pShell->DeclareSymbol("user void pck_SetBoxDamage(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::SetBoxDamage);

  _pShell->DeclareSymbol("user void pck_EntityDamage(void);", &IPacketCommands::EntityDamage);

  // World change
  _pShell->DeclareSymbol("user void pck_ChangeLevel(CTString);", &IPacketCommands::ChangeLevel);
  _pShell->DeclareSymbol("user void pck_ChangeWorld(CTString);", &IPacketCommands::ChangeWorld);

  // Session properties
  _pShell->DeclareSymbol("user void pck_StartSesProps(INDEX);",    &IPacketCommands::StartSesProps);
  _pShell->DeclareSymbol("user void pck_SesPropIndex(INDEX);",     &IPacketCommands::SesPropIndex);
  _pShell->DeclareSymbol("user void pck_SesPropFloat(FLOAT);",     &IPacketCommands::SesPropFloat);
  _pShell->DeclareSymbol("user void pck_SesPropString(CTString);", &IPacketCommands::SesPropString);
  _pShell->DeclareSymbol("user void pck_SeekSesProp(INDEX);",      &IPacketCommands::SeekSesProp);
  _pShell->DeclareSymbol("user void pck_SendSesProps(void);",      &IPacketCommands::SendSesProps);

  // Gameplay extensions
  _pShell->DeclareSymbol("user void pck_GameplayExtNumber(CTString, FLOAT);",    &IPacketCommands::GameplayExtNumber);
  _pShell->DeclareSymbol("user void pck_GameplayExtString(CTString, CTString);", &IPacketCommands::GameplayExtString);

  // Sound playback
  _pShell->DeclareSymbol("user void pck_PlayQuickSound(CTString, INDEX, FLOAT);", &IPacketCommands::PlayQuickSound);
  _pShell->DeclareSymbol("user void pck_PlayGlobalSound(CTString, INDEX, INDEX, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);", &IPacketCommands::PlayGlobalSound);
};

// Write event into a network packet
void EExtEntityEvent::Write(CNetworkMessage &nm, ULONG ctFields) {
  nm << ee_slEvent;

  // Write data
  UBYTE ubData = (ctFields != 0);
  nm.WriteBits(&ubData, 1);

  if (ubData) {
    // Fit 64 fields by writing the 0-63 range
    ubData = UBYTE(ctFields - 1);
    nm.WriteBits(&ubData, 6);

    nm.Write(&aulFields[0], ctFields * sizeof(ULONG));
  }
};

// Read event from a network packet
ULONG EExtEntityEvent::Read(CNetworkMessage &nm) {
  Reset();
  nm >> ee_slEvent;

  // Read data
  UBYTE ubData = 0;
  nm.ReadBits(&ubData, 1);

  if (ubData) {
    ULONG ctFields = 0;

    // Interpret read size as being in the 1-64 range
    nm.ReadBits(&ctFields, 6);
    ctFields++;

    nm.Read(&aulFields[0], ctFields * sizeof(ULONG));
    return ctFields;
  }

  return 0;
};

// Convert fields according to the event type
void EExtEntityEvent::ConvertTypes(void)
{
  // Convert entity IDs into pointers
  switch (ee_slEvent) {
    // First field is an entity
    case EVENTCODE_VNL_EStart:
    case EVENTCODE_VNL_ETrigger:
    case EVENTCODE_VNL_EAirShockwave:
    case EVENTCODE_VNL_EAirWave:
    case EVENTCODE_VNL_EBulletInit:
    case EVENTCODE_VNL_ELaunchCannonBall:
    case EVENTCODE_VNL_ELaunchLarvaOffspring:
    case EVENTCODE_VNL_EAnimatorInit:
    case EVENTCODE_VNL_EWeaponsInit:
    case EVENTCODE_VNL_EWeaponEffectInit:
    case EVENTCODE_VNL_ELaunchProjectile:
    case EVENTCODE_VNL_EReminderInit:
    case EVENTCODE_VNL_ESeriousBomb:
    case EVENTCODE_VNL_EWatcherInit:
    case EVENTCODE_VNL_EWater: {
      VNL_EStart &ee = (VNL_EStart &)*this;
      ee.penCaused = EntityFromID(0);
    } break;

    // Second field is an entity
    case EVENTCODE_VNL_ESound:
    case EVENTCODE_VNL_EScroll:
    case EVENTCODE_VNL_ETextFX:
    case EVENTCODE_VNL_EHudPicFX:
    case EVENTCODE_VNL_ECredits: {
      VNL_ESound &ee = (VNL_ESound &)*this;
      ee.penTarget = EntityFromID(1);
    } break;

    // Two first fields are entities
    case EVENTCODE_VNL_EDevilProjectile:
    case EVENTCODE_VNL_EFlame:
    case EVENTCODE_VNL_EViewInit:
    case EVENTCODE_VNL_ESpinnerInit: {
      VNL_EFlame &ee = (VNL_EFlame &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penAttach = EntityFromID(1);
    } break;

    // Same ID as ETwister
    case EVENTCODE_VNL_ESpawnerProjectile: {
      VNL_ESpawnerProjectile &ee = (VNL_ESpawnerProjectile &)*this;
      ee.penOwner = EntityFromID(0);
      ee.penTemplate = MaybeEntity(1); // Preserves 'ETwister::fSize'
    } break;

    // Skips: sptType, fDamagePower, fSizeMultiplier, vDirection
    case EVENTCODE_VNL_ESpawnSpray: {
      VNL_ESpawnSpray &ee = (VNL_ESpawnSpray &)*this;
      ee.penOwner = EntityFromID(6);
    } break;

    case EVENTCODE_VNL_ESpawnDebris: {
      VNL_ESpawnDebris &ee = (VNL_ESpawnDebris &)*this;
      ee.penFallFXPapa = EntityFromID(22);

      if (ee.penFallFXPapa != NULL) {
        CEntity *pen = (CEntity *)ee.penFallFXPapa;

        ee.pmd = pen->GetModelObject()->GetData(); // 1
        ee.ptd = (CTextureData *)pen->GetModelObject()->mo_toTexture.GetData(); // 3
        ee.ptdRefl = (CTextureData *)pen->GetModelObject()->mo_toReflection.GetData(); // 4
        ee.ptdSpec = (CTextureData *)pen->GetModelObject()->mo_toSpecular.GetData(); // 5
        ee.ptdBump = (CTextureData *)pen->GetModelObject()->mo_toBump.GetData(); // 6
      }
    } break;

    // Skips: eetType, vDamageDir, vDestination, tmLifeTime, fSize, ctCount
    case EVENTCODE_VNL_ESpawnEffector: {
      VNL_ESpawnEffector &ee = (VNL_ESpawnEffector &)*this;
      ee.penModel  = EntityFromID(10);
      ee.penModel2 = EntityFromID(11);
    } break;
  }
};

#else

void ClassicsPackets_ServerReport(IClassicsExtPacket *pExtPacket, const char *strFormat, ...) {
  ASSERT(FALSE);
};

void ClassicsPackets_SendToClients(IClassicsExtPacket *pExtPacket) {
  ASSERT(FALSE);
};

void ClassicsPackets_SendToServer(IClassicsExtPacket *pExtPacket) {
  ASSERT(FALSE);
};

bool ClassicsPackets_GetBoolProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  ASSERT(FALSE);
  return false;
};

int ClassicsPackets_GetIntProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  ASSERT(FALSE);
  return 0;
};

double ClassicsPackets_GetFloatProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  ASSERT(FALSE);
  return 0.0;
};

const char *ClassicsPackets_GetStringProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty) {
  ASSERT(FALSE);
  return "";
};

bool ClassicsPackets_SetBoolProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, bool bValue) {
  ASSERT(FALSE);
  return false;
};

bool ClassicsPackets_SetIntProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, int iValue) {
  ASSERT(FALSE);
  return false;
};

bool ClassicsPackets_SetFloatProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, double fValue) {
  ASSERT(FALSE);
  return false;
};

bool ClassicsPackets_SetStringProp(IClassicsBuiltInExtPacket *pExtPacket, const char *strProperty, const char *strValue) {
  ASSERT(FALSE);
  return false;
};

IClassicsBuiltInExtPacket *ClassicsPackets_Create(IClassicsExtPacket::EPacketType ePacket) {
  ASSERT(FALSE);
  return NULL;
};

void ClassicsPackets_Destroy(IClassicsBuiltInExtPacket *pExtPacket) {
  ASSERT(FALSE);
  delete pExtPacket;
};

#endif // _PATCHCONFIG_EXT_PACKETS
