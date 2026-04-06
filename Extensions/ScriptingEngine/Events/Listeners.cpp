/* Copyright (c) 2026 Dreamy Cecil
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

#include <Extras/XGizmo/Vanilla/EntityEvents.h>

static CEntity *_pen = NULL;
static const CEntityEvent *_pee = NULL;

// Arguments
static BOOL _bState;
static INDEX _iNumber;
static FLOAT _fNumber;
static CTString _strString;
static CEntity *_penEntity = NULL;

static INDEX _iAmmo1, _iAmmo2, _iAmmo3, _iAmmo4, _iAmmo5, _iAmmo6, _iAmmo7, _iAmmo8;
static BOOL _bPickedUp;

__forceinline SQRESULT PushEvent(sq::VM &vm) {
  sq_pushroottable(vm);
  PushNewPointer(vm.Root(), "CEntityPointer", _pen);
  sq_pushinteger(vm, _pee->ee_slEvent);
  return SQ_OK;
};

inline SQRESULT PushNoArgs(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Empty argument array
  sq_newarray(vm, 0);
  return SQ_OK;
};

inline SQRESULT PushInt(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

inline SQRESULT PushString(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushstring(vm, _strString.str_String, -1);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

inline SQRESULT PushEntity(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  PushNewPointer(vm.Root(), "CEntityPointer", _penEntity);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

inline SQRESULT PushIntEntity(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);
  PushNewPointer(vm.Root(), "CEntityPointer", _penEntity);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

inline SQRESULT PushBoolEntity(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushbool(vm, _bState);
  sq_arrayappend(vm, -2);
  PushNewPointer(vm.Root(), "CEntityPointer", _penEntity);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

inline SQRESULT PushStringFloatInt(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushstring(vm, _strString.str_String, -1);
  sq_arrayappend(vm, -2);
  sq_pushfloat(vm, _fNumber);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);
  return SQ_OK;
};

static void CallOnEvent(CEntity *pen, const CEntityEvent &ee, const CTString &strFunc) {
  // Restrict listener functions for remote players
  if (INetwork::IsPlayingOnRemoteServer()) return;

  static const SQChar *strScriptFuncGeneric = "GenericEvent";
  static const SQChar *strScriptFuncEnemy   = "EnemyEvent";
  static const SQChar *strScriptFuncPlayer  = "PlayerEvent";

  _pen = pen;
  _pee = &ee;

  switch (ee.ee_slEvent) {
    // Generic
    case EVENTCODE_VNL_EActivate:
    case EVENTCODE_VNL_EDeactivate:
    case EVENTCODE_VNL_EEnvironmentStart:
    case EVENTCODE_VNL_EEnvironmentStop:
    case EVENTCODE_VNL_ETeleportMovingBrush:
    case EVENTCODE_VNL_EEnd:
    case EVENTCODE_VNL_EStop: {
      RunCustomScripts("On" + strFunc + strScriptFuncGeneric, &PushNoArgs);
    } break;

    case EVENTCODE_VNL_EStart:
    case EVENTCODE_VNL_ETrigger: {
      const VNL_EStart &eeStart = (const VNL_EStart &)ee;
      _penEntity = eeStart.penCaused;
      RunCustomScripts("On" + strFunc + strScriptFuncGeneric, &PushEntity);
    } break;

    case EVENTCODE_VNL_EReminder: {
      const VNL_EReminder &eeReminder = (const VNL_EReminder &)ee;
      _iNumber = eeReminder.iValue;
      RunCustomScripts("On" + strFunc + strScriptFuncGeneric, &PushInt);
    } break;

    // WorldSettingsController
  #if VANILLA_ENTITY_EVENTS_FOR_TSE
    case EVENTCODE_VNL_EScroll:
    case EVENTCODE_VNL_ETextFX:
    case EVENTCODE_VNL_EHudPicFX:
    case EVENTCODE_VNL_ECredits: {
      const VNL_EScroll &eeWSC = (const VNL_EScroll &)ee;
      _bState = eeWSC.bStart;
      _penEntity = eeWSC.penSender;
      RunCustomScripts("On" + strFunc + strScriptFuncGeneric, &PushBoolEntity);
    } break;
  #endif

    // EnemyBase
    case EVENTCODE_VNL_EStartAttack:
    case EVENTCODE_VNL_EStopAttack:
    case EVENTCODE_VNL_EStopBlindness:
    case EVENTCODE_VNL_EStopDeafness:
    case EVENTCODE_VNL_EHitBySpaceShipBeam:
    case EVENTCODE_VNL_ERestartAttack:
    case EVENTCODE_VNL_EReconsiderBehavior:
    case EVENTCODE_VNL_EForceWound: {
      RunCustomScripts("On" + strFunc + strScriptFuncEnemy, &PushNoArgs);
    } break;

    case EVENTCODE_VNL_ESound: {
      const VNL_ESound &eeSound = (const VNL_ESound &)ee;
      _iNumber = eeSound.EsndtSound;
      _penEntity = eeSound.penTarget;
      RunCustomScripts("On" + strFunc + strScriptFuncEnemy, &PushIntEntity);
    } break;

    // Player, PlayerWeapons
    case EVENTCODE_VNL_EKilledEnemy:
    case EVENTCODE_VNL_ESecretFound:
  #if VANILLA_ENTITY_EVENTS_FOR_TSE
    case EVENTCODE_VNL_EWeaponChanged:
  #endif
    case EVENTCODE_VNL_EBoringWeapon:
    case EVENTCODE_VNL_EFireWeapon:
    case EVENTCODE_VNL_EReleaseWeapon:
    case EVENTCODE_VNL_EReloadWeapon: {
      RunCustomScripts("On" + strFunc + strScriptFuncPlayer, &PushNoArgs);
    } break;

    case EVENTCODE_VNL_ECenterMessage: {
      const VNL_ECenterMessage &eeMessage = (const VNL_ECenterMessage &)ee;
      _strString = eeMessage.strMessage;
      _fNumber = eeMessage.tmLength;
      _iNumber = eeMessage.mssSound;
      RunCustomScripts("On" + strFunc + strScriptFuncPlayer, &PushStringFloatInt);
    } break;

    case EVENTCODE_VNL_EComputerMessage:
    case EVENTCODE_VNL_EVoiceMessage: {
      const VNL_EComputerMessage &eeMessage = (const VNL_EComputerMessage &)ee;
      _strString = eeMessage.fnmMessage;
      RunCustomScripts("On" + strFunc + strScriptFuncPlayer, &PushString);
    } break;

    case EVENTCODE_VNL_EReceiveScore: {
      const VNL_EReceiveScore &eeScore = (const VNL_EReceiveScore &)ee;
      _iNumber = eeScore.iPoints;
      RunCustomScripts("On" + strFunc + strScriptFuncPlayer, &PushInt);
    } break;

    case EVENTCODE_VNL_ESelectWeapon: {
      const VNL_ESelectWeapon &eeSelect = (const VNL_ESelectWeapon &)ee;
      _iNumber = eeSelect.iWeapon;
      RunCustomScripts("On" + strFunc + strScriptFuncPlayer, &PushInt);
    } break;
  }
};

void IListenerEvents_OnSendEvent(CEntity *pen, const CEntityEvent &ee) {
  // Runs any of the following:
  // - OnSendGenericEvent
  // - OnSendEnemyEvent
  // - OnSendPlayerEvent
  CallOnEvent(pen, ee, "Send");
};

void IListenerEvents_OnCallProcedure(CEntity *pen, const CEntityEvent &ee) {
  // Runs any of the following:
  // - OnReceiveGenericEvent
  // - OnReceiveEnemyEvent
  // - OnReceivePlayerEvent
  CallOnEvent(pen, ee, "Receive");
};

inline SQRESULT PushAmmoItem(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo1);
  sq_arrayappend(vm, -2);

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

inline SQRESULT PushAmmoPack(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iAmmo1);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo2);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo3);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo4);
  sq_arrayappend(vm, -2);
#if VANILLA_ENTITY_EVENTS_FOR_TSE
  sq_pushinteger(vm, _iAmmo5);
  sq_arrayappend(vm, -2);
#endif
  sq_pushinteger(vm, _iAmmo6);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo7);
  sq_arrayappend(vm, -2);
#if VANILLA_ENTITY_EVENTS_FOR_TSE
  sq_pushinteger(vm, _iAmmo8);
  sq_arrayappend(vm, -2);
#endif

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

inline SQRESULT PushVitalItem(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushfloat(vm, _fNumber);
  sq_arrayappend(vm, -2);
  sq_pushbool(vm, _bState);
  sq_arrayappend(vm, -2);

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

inline SQRESULT PushKeyOrPowerUpItem(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

inline SQRESULT PushMessageItem(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushstring(vm, _strString.str_String, -1);
  sq_arrayappend(vm, -2);

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

inline SQRESULT PushWeaponItem(sq::VM &vm) {
  if (SQ_FAILED(PushEvent(vm))) return SQ_ERROR;

  // Arguments
  sq_newarray(vm, 0);
  sq_pushinteger(vm, _iNumber);
  sq_arrayappend(vm, -2);
  sq_pushinteger(vm, _iAmmo1);
  sq_arrayappend(vm, -2);
  sq_pushbool(vm, _bState);
  sq_arrayappend(vm, -2);

  // Picked up item
  sq_pushbool(vm, _bPickedUp);
  return SQ_OK;
};

void IListenerEvents_OnReceiveItem(CEntity *penPlayer, const CEntityEvent &ee, BOOL bPickedUp) {
  // Restrict listener functions for remote players
  if (INetwork::IsPlayingOnRemoteServer()) return;

  static const SQChar *strScriptFunc = "OnPlayerItem";

  _pen = penPlayer;
  _pee = &ee;
  _bPickedUp = bPickedUp;

  switch (ee.ee_slEvent) {
    case EVENTCODE_VNL_EAmmoItem: {
      const VNL_EAmmoItem &eeAmmo = (const VNL_EAmmoItem &)ee;
      _iNumber = eeAmmo.EaitType;
      _iAmmo1 = eeAmmo.iQuantity;
      RunCustomScripts(strScriptFunc, &PushAmmoItem);
    } break;

    case EVENTCODE_VNL_EAmmoPackItem: {
      const VNL_EAmmoPackItem &eePack = (const VNL_EAmmoPackItem &)ee;
      _iAmmo1 = eePack.iShells;
      _iAmmo2 = eePack.iBullets;
      _iAmmo3 = eePack.iRockets;
      _iAmmo4 = eePack.iGrenades;
    #if VANILLA_ENTITY_EVENTS_FOR_TSE
      _iAmmo5 = eePack.iNapalm;
    #endif
      _iAmmo6 = eePack.iElectricity;
      _iAmmo7 = eePack.iIronBalls;
    #if VANILLA_ENTITY_EVENTS_FOR_TSE
      _iAmmo8 = eePack.iSniperBullets;
    #endif
      RunCustomScripts(strScriptFunc, &PushAmmoPack);
    } break;

    case EVENTCODE_VNL_EArmor:
    case EVENTCODE_VNL_EHealth: {
      const VNL_EArmor &eeVital = (const VNL_EArmor &)ee;
      _fNumber = eeVital.fArmor;
      _bState = eeVital.bOverTopArmor;
      RunCustomScripts(strScriptFunc, &PushVitalItem);
    } break;

    case EVENTCODE_VNL_EKey: {
      const VNL_EKey &eeKey = (const VNL_EKey &)ee;
      _iNumber = eeKey.kitType;
      RunCustomScripts(strScriptFunc, &PushKeyOrPowerUpItem);
    } break;

    case EVENTCODE_VNL_EMessageItem: {
      const VNL_EMessageItem &eeMessage = (const VNL_EMessageItem &)ee;
      _strString = eeMessage.fnmMessage;
      RunCustomScripts(strScriptFunc, &PushMessageItem);
    } break;

  #if VANILLA_ENTITY_EVENTS_FOR_TSE
    case EVENTCODE_VNL_EPowerUp: {
      const VNL_EPowerUp &eePowerUp = (const VNL_EPowerUp &)ee;
      _iNumber = eePowerUp.puitType;
      RunCustomScripts(strScriptFunc, &PushKeyOrPowerUpItem);
    } break;
  #endif

    case EVENTCODE_VNL_EWeaponItem: {
      const VNL_EWeaponItem &eeWeapon = (const VNL_EWeaponItem &)ee;
      _iNumber = eeWeapon.iWeapon;
      _iAmmo1 = eeWeapon.iAmmo;
      _bState = eeWeapon.bDropped;
      RunCustomScripts(strScriptFunc, &PushWeaponItem);
    } break;
  }
};
