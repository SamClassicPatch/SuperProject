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

static const CTFileName *_pfnm = NULL;

inline SQRESULT PushPath(sq::VM &vm) {
  sq_pushroottable(vm);
  sq_pushstring(vm, _pfnm->str_String, -1);
  return SQ_OK;
};

void IGameEvents_OnGameStart(void) {
  RunCustomScripts("OnGameStart", NULL);
};

void IGameEvents_OnChangeLevel(void) {
  RunCustomScripts("OnChangeLevel", NULL);
};

void IGameEvents_OnGameStop(void) {
  RunCustomScripts("OnGameStop", NULL);
};

void IGameEvents_OnGameSave(const CTFileName &fnmSave) {
  _pfnm = &fnmSave;
  RunCustomScripts("OnGameSave", &PushPath);
};

void IGameEvents_OnGameLoad(const CTFileName &fnmSave) {
  _pfnm = &fnmSave;
  RunCustomScripts("OnGameLoad", &PushPath);
};
