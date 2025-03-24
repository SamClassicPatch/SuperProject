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

#ifndef CECIL_INCL_SQSCRIPTVM_H
#define CECIL_INCL_SQSCRIPTVM_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

namespace sq {

class VM {
  private:
    HSQUIRRELVM m_vm;
    CTString m_strErrors; // Error message buffer

  public:
    VM();
    ~VM();

    // Retrieve handler to a raw Squirrel VM
    __forceinline HSQUIRRELVM GetVM(void) const {
      return m_vm;
    };

    // Get last VM error
    __forceinline const char *GetError(void) const {
      // No error
      if (m_strErrors == "") return "no error";

      return m_strErrors.str_String;
    };

    // Push one VM error
    __forceinline void PushError(const CTString &strError) {
      m_strErrors += strError;
    };

    // Clear the VM error
    __forceinline void ClearError(void) {
      m_strErrors = "";
    };

  // Squirrel wrappers
  public:

    // Retrieve current VM state
    __forceinline SQInteger GetState(void) const {
      return sq_getvmstate(m_vm);
    };

  // Running
  public:

    // Compile script from a source file
    SQRESULT CompileSource(const CTString &strSourceFile);

    // Compile script from a string with a given function name
    SQRESULT CompileScript(const CTString &strScript, const char *strSourceName);
};

}; // namespace

#endif
