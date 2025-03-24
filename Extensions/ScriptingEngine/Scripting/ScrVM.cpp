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

#include <Extras/squirrel3/include/sqstdaux.h>

// Helper function for reading one script character at a time for sq_compile()
static SQInteger SqLexerFeed(SQUserPointer pData)
{
  CTFileStream &strm = *(CTFileStream *)pData;

  // No more data to read
  if (strm.AtEOF()) return 0;

  // Read one more character from the stream
  // 'UBYTE' instead of 'char' to preserve the character from some ANSI code page
  UBYTE ch;
  strm >> ch;

  return ch;
};

namespace sq {

// Get a script VM class from a Squirrel VM
__forceinline VM &GetVM(HSQUIRRELVM v) {
  return *(VM *)sq_getsharedforeignptr(v);
};

// Message output
static void HandlerPrintF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  CPutString(strPrint);

  va_end(arg);
};

// Error output
static void HandlerErrorF(HSQUIRRELVM v, const char *str, ...) {
  va_list arg;
  va_start(arg, str);

  CTString strPrint;
  strPrint.VPrintF(str, arg);
  GetVM(v).PushError(strPrint);

  va_end(arg);
};

// Compiler error output
static void HandlerCompilerError(HSQUIRRELVM v,
  const SQChar *strError, const SQChar *strSource, SQInteger iLn, SQInteger iCh)
{
  // Clear the last error
  GetVM(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    pCallback(v, _SC("%s (ln %d, ch %d) error: %s\n"), strSource, iLn, iCh, strError);
  }
};

// Runtime error output
static SQInteger HandlerRuntimeError(HSQUIRRELVM v) {
  // Clear the last error
  GetVM(v).ClearError();

  SQPRINTFUNCTION pCallback = sq_geterrorfunc(v);

  if (pCallback != NULL) {
    const SQChar *strError = NULL;

    if (sq_gettop(v) >= 1) {
      if (SQ_SUCCEEDED(sq_getstring(v, 2, &strError))) {
        pCallback(v, _SC("AN ERROR HAS OCCURRED [%s]\n"), strError);
      } else {
        pCallback(v, _SC("AN ERROR HAS OCCURRED [unknown]\n"));
      }

      sqstd_printcallstack(v);
    }
  }

  return SQ_OK;
};

VM::VM(void) {
  // Create a new VM and bind this wrapper class to it
  m_vm = sq_open(1024);
  sq_setsharedforeignptr(m_vm, this);

  // Register things in the root table
  sq_pushroottable(m_vm);
  {
    // Set handler functions
    sq_setprintfunc(m_vm, &HandlerPrintF, &HandlerErrorF);
    sq_setcompilererrorhandler(m_vm, &HandlerCompilerError);

    sq_newclosure(m_vm, &HandlerRuntimeError, 0);
    sq_seterrorhandler(m_vm);
  }
  sq_poptop(m_vm);
};

VM::~VM() {
  sq_close(m_vm);
};

// Compile script from a source file
SQRESULT VM::CompileSource(const CTString &strSourceFile) {
  // Open the source file using an engine stream
  CTFileStream strm;

  try {
    strm.Open_t(strSourceFile);

  } catch (char *strError) {
    return sq_throwerror(m_vm, strError);
  }

  // Compile the script by reading characters from the stream
  SQRESULT r = sq_compile(m_vm, SqLexerFeed, &strm, strSourceFile.str_String, TRUE);
  strm.Close();

  return r;
};

// Compile script from a string with a given function name
SQRESULT VM::CompileScript(const CTString &strScript, const char *strSourceName) {
  return sq_compilebuffer(m_vm, strScript, strScript.Length(), strSourceName, TRUE);
};

}; // namespace
