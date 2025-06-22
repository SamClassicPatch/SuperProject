/* Copyright (c) 2025 Dreamy Cecil
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

namespace Message {

static SQInteger Fatal(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  FatalError("%s", str);
  return 1;
};

static SQInteger Warning(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  WarningMessage("%s", str);
  return 1;
};

static SQInteger Info(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  InfoMessage("%s", str);
  return 1;
};

static SQInteger Question(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);

  bool bYes = MessageBoxA(NULL, str, LOCALIZE("Question"), MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TASKMODAL) == IDYES;
  sq_pushbool(v, bYes ? SQTrue : SQFalse);
  return 1;
};

static SQInteger Log(HSQUIRRELVM v) {
  const SQChar *str = "";
  sq_getstring(v, 2, &str);
  CPutString(str);
  return 1;
};

}; // namespace

// "Message" namespace functions
static SQRegFunction _aMessageFuncs[] = {
  { "Fatal",    &Message::Fatal, 2, ".s" },
  { "Warning",  &Message::Warning, 2, ".s" },
  { "Info",     &Message::Info, 2, ".s" },
  { "Question", &Message::Question, 2, ".s" },
  { "Log",      &Message::Log, 2, ".s" },
};

void VM::RegisterMessage(void) {
  Table sqtMessage = Root().AddTable("Message");

  // Register functions
  for (INDEX i = 0; i < ARRAYCOUNT(_aMessageFuncs); i++) {
    sqtMessage.RegisterFunc(_aMessageFuncs[i]);
  }
};

}; // namespace
