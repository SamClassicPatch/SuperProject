/* Copyright (c) 2022-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_PLUGINSINTERFACE_H
#define CECIL_INCL_PLUGINSINTERFACE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Core/Modules/PluginModule.h>

// API for handling plugin modules
class CORE_API CPluginAPI : public IClassicsPlugins {
  private:
    // Handles to Game & GameGUI plugins
    CPluginModule *m_pGamePlugin;
    CPluginModule *m_pGameGuiPlugin;

  public:
    // Constructor
    CPluginAPI();

    // Destructor
    ~CPluginAPI();

    // Load Game library as a plugin
    void LoadGameLib(const char *strSettingsFile);

    // Load GameGUI library and return a pointer to GameGUI_interface
    void *LoadGameGuiLib(const char *strSettingsFile);

    // Set metadata for the Game plugin
    CPluginModule *LoadGamePlugin(void);

    // Set metadata for the GameGUI plugin
    CPluginModule *LoadGameGuiPlugin(void);

    // Load all user plugins of specific utility types (EPluginFlags)
    void LoadPlugins(ULONG ulUtilityFlags);

    // Release all user plugins of specific utility types (EPluginFlags)
    void ReleasePlugins(ULONG ulUtilityFlags, BOOL bForce = FALSE);

    // Obtain pointer to a plugin module of specific utility types
    CPluginModule *ObtainPlugin_t(const CTFileName &fnmModule, ULONG ulUtilityFlags);

    // Load plugin module without safety checks
    CPluginModule *LoadPlugin_t(const CTFileName &fnmModule);

    // Retrieve loaded plugins
    CDynamicContainer<CPluginModule> &GetPlugins(void);

    // Overridden API methods
    virtual void RegisterSymbol(PluginSymbol_t &ps, const char *strSymbolName, const char *strDefaultValue);
    virtual void RegisterMethod(bool bUser, const char *strReturnType, const char *strFunctionName, const char *strArgumentTypes, void *pFunction);
    virtual int GetExtensionCount(void);
    virtual HPatchPlugin *GetExtensionByIndex(int iExtension);
    virtual HPatchPlugin *GetExtensionByName(const char *strExtension);
};

// Full interface getter
CORE_API CPluginAPI *GetPluginAPI(void);

// Class for defining shell symbols within plugins and retrieving values from them
struct CORE_API CPluginSymbol : public PluginSymbol_t
{
  CPluginSymbol(ULONG ulFlags, INDEX iDefValue) : PluginSymbol_t(ulFlags, iDefValue) {};
  CPluginSymbol(ULONG ulFlags, FLOAT fDefValue) : PluginSymbol_t(ulFlags, fDefValue) {};
  CPluginSymbol(ULONG ulFlags, const char *strDefValue) : PluginSymbol_t(ulFlags, strDefValue) {};

  // Get index value
  INDEX GetIndex(void) const {
    ASSERT(m_eSymbolType == k_ESymbolIndex);
    ASSERT(m_pShellSymbol != NULL);

    return *(INDEX *)m_pShellSymbol->ss_pvValue;
  };

  // Get float value
  FLOAT GetFloat(void) const {
    ASSERT(m_eSymbolType == k_ESymbolFloat);
    ASSERT(m_pShellSymbol != NULL);

    return *(FLOAT *)m_pShellSymbol->ss_pvValue;
  };

  // Get string value
  const CTString &GetString(void) const {
    ASSERT(m_eSymbolType == k_ESymbolString);
    ASSERT(m_pShellSymbol != NULL);

    return *(CTString *)m_pShellSymbol->ss_pvValue;
  };

  // Register this symbol through the API
  void Register(const char *strSymbolName, const char *strPreFunc = "", const char *strPostFunc = "");
};

// Iteration through all plugins
#define FOREACHPLUGIN(_Iter) \
  FOREACHINDYNAMICCONTAINER(GetPluginAPI()->GetPlugins(), CPluginModule, _Iter)

#endif
