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

static INDEX _iClient, _iPlayer, _iResent;
static ULONG _ulFrom, _ulTo;

static CPlayerCharacter *_ppc = NULL;
static CPlayerAction *_ppa = NULL;
static const CTString *_pstrFrom = NULL;
static CTString *_pstrSender = NULL;
static CTString *_pstrMessage = NULL;

static SQBool _bKeepReturn;
static BOOL _bReturnValue;

inline bool ReturnValue(sq::VM &vm) {
  sq_getbool(vm, -1, &_bKeepReturn);
  if (!_bKeepReturn) _bReturnValue = FALSE; // At least one script doesn't want to return TRUE
  return true;
};

inline SQRESULT PushCharConnect(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushinteger(vm, _iClient);
  PushNewPointer(vm.Root(), "CPlayerCharacter", *_ppc);
  return SQ_OK;
};

void IPacketEvents_OnCharacterConnect(INDEX iClient, CPlayerCharacter &pc) {
  _iClient = iClient;
  _ppc = &pc;
  RunCustomScripts("OnCharacterConnect", &PushCharConnect);
};

inline SQRESULT PushCharChange(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushinteger(vm, _iClient);
  sq_pushinteger(vm, _iPlayer);
  PushNewPointer(vm.Root(), "CPlayerCharacter", *_ppc);
  return SQ_OK;
};

BOOL IPacketEvents_OnCharacterChange(INDEX iClient, INDEX iPlayer, CPlayerCharacter &pc) {
  _iClient = iClient;
  _iPlayer = iPlayer;
  _ppc = &pc;

  _bKeepReturn = SQTrue;
  _bReturnValue = TRUE;
  RunCustomScripts("OnCharacterChange", &PushCharChange, &ReturnValue);
  return _bReturnValue;
};

inline SQRESULT PushActionArgs(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushinteger(vm, _iClient);
  sq_pushinteger(vm, _iPlayer);
  PushNewPointer(vm.Root(), "CPlayerAction", *_ppa);
  sq_pushinteger(vm, _iResent);
  return SQ_OK;
};

void IPacketEvents_OnPlayerAction(INDEX iClient, INDEX iPlayer, CPlayerAction &pa, INDEX iResent) {
  _iClient = iClient;
  _iPlayer = iPlayer;
  _ppa = &pa;
  _iResent = iResent;
  RunCustomScripts("OnPlayerAction", &PushActionArgs);
};

inline SQRESULT PushChatArgs(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushinteger(vm, _iClient);
  sq_pushinteger(vm, _ulFrom);
  sq_pushinteger(vm, _ulTo);
  sq_pushstring(vm, _pstrMessage->str_String, -1);
  return SQ_OK;
};

BOOL IPacketEvents_OnChatMessage(INDEX iClient, ULONG ulFrom, ULONG ulTo, CTString &strMessage) {
  _iClient = iClient;
  _ulFrom = ulFrom;
  _ulTo = ulTo;
  _pstrMessage = &strMessage;

  _bKeepReturn = SQTrue;
  _bReturnValue = TRUE;
  RunCustomScripts("OnChatMessage", &PushChatArgs, &ReturnValue);
  return _bReturnValue;
};

inline SQRESULT PushChatOutArgs(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushinteger(vm, _ulFrom);
  sq_pushstring(vm, _pstrFrom->str_String, -1);
  sq_pushstring(vm, _pstrSender->str_String, -1);
  sq_pushstring(vm, _pstrMessage->str_String, -1);
  return SQ_OK;
};

inline bool ReturnNewOutputValues(sq::VM &vm) {
  if (sq_gettype(vm, -1) != OT_ARRAY) {
    return true;
  }

  CTString strNewSender;
  CTString strNewMessage;

  // Update sender and message if they can be retrieved from the array
  if (vm.ArrayPopFirst(-1, &strNewSender) && vm.ArrayPopFirst(-1, &strNewMessage)) {
    *_pstrSender = strNewSender;
    *_pstrMessage = strNewMessage;
  }
  return true;
};

void IPacketEvents_OnChatOutput(const ULONG ulFrom, const CTString &strFrom, CTString &strSender, CTString &strMessage) {
  _ulFrom = ulFrom;
  _pstrFrom = &strFrom;

  // These two are also modified by pointer in ReturnNewOutputValues() aside from simply being pushed as arguments
  _pstrSender = &strSender;
  _pstrMessage = &strMessage;

  RunCustomScripts("OnChatOutput", &PushChatOutArgs, &ReturnNewOutputValues);
};
