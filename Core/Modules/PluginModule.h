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

#ifndef CECIL_INCL_PLUGINMODULE_H
#define CECIL_INCL_PLUGINMODULE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Abstract class that represents loaded plugin library
class CORE_API CPluginModule : public CSerial {
  public:
    // Plugin method types
    typedef PLUGINMODULEPROTOTYPE_GETINFO((*FInfoFunc));
    typedef PLUGINMODULEPROTOTYPE_STARTUP((*FModuleStartupFunc));
    typedef PLUGINMODULEPROTOTYPE_SHUTDOWN((*FModuleShutdownFunc));

  private:
    HINSTANCE pm_hLibrary; // Library handle
    BOOL pm_bInitialized; // Plugin has been initialized

    CStaticStackArray<HFuncPatch> pm_aPatches; // Custom patches added on plugin startup

    // Hooked methods
    FInfoFunc pm_pGetInfoFunc; // Retrieve information about the plugin
    FModuleStartupFunc pm_pStartupFunc; // Entry point for the plugin
    FModuleShutdownFunc pm_pShutdownFunc; // Plugin cleanup before releasing it

  public:
    PluginInfo_t pm_info; // Plugin information
    CIniConfig pm_props; // Loaded plugin properties
    PluginEvents_t pm_events; // Interface of plugin events

    ExtensionProp_t *pm_aExtensionProps; // Array of properties for communicating with the extension
    size_t pm_ctExtensionProps; // Amount of extension properties in the array

    ExtensionSignal_t *pm_aExtensionSignals; // Array of signals for interacting with the extension
    size_t pm_ctExtensionSignals; // Amount of extension signals in the array

  public:
    // Constructor
    CPluginModule();

    // Destructor
    virtual ~CPluginModule();

    // Return library handle
    inline HINSTANCE GetHandle(void) const {
      return pm_hLibrary;
    };

    // Check if plugin has been initialized
    inline BOOL IsInitialized(void) const {
      return pm_bInitialized;
    };

    // Module initialization
    virtual void Initialize(void);

    // Module deactivation
    virtual void Deactivate(void);

    // Reset class fields
    virtual void ResetFields(void);

    // Add new function patch on startup
    virtual void AddPatch(HFuncPatch hPatch);

    // Get specific symbol from the module
    virtual void *GetSymbol_t(const char *strSymbolName);

    // Get specific symbol from the module (must be a pointer to the pointer variable)
    template<class Type> void GetSymbol_t(Type *ppSymbol, const char *strSymbolName) {
      *ppSymbol = (Type)GetSymbol_t(strSymbolName);
    };

    // Find extension property in the plugin
    ExtensionProp_t *FindProperty(const char *strProperty);

    // Find extension property of a specific type in the plugin
    ExtensionProp_t *FindPropertyOfType(const char *strProperty, ExtensionProp_t::EType eType);

    // Find function of some extension signal in the plugin
    FExtensionSignal FindSignal(const char *strSignal);

    // Call function of some extension signal in the plugin
    // Returns false if the signal cannot be called (not found)
    // piResult - pointer to the variable that will hold the result from the function call (may be NULL)
    // pData - optional data that will be passed into the signal call (see FExtensionSignal prototype)
    bool CallSignal(const char *strSignal, int *piResult, void *pData);

  // CSerial overrides
  public:

    // Module cleanup
    virtual void Clear(void);

    // Write to stream (obsolete method)
    virtual void Write_t(CTStream *ostrFile) {
      ASSERTALWAYS("Plugin modules cannot be written!");
    };

    // Read from stream (obsolete method)
    virtual void Read_t(CTStream *istrFile) {
      ASSERTALWAYS("Plugin modules cannot be read! Use CPluginStock for loading plugins!");
    };

    // Load plugin module (override non-virtual CSerial::Load_t)
    virtual void Load_t(const CTFileName &fnmDLL);

    // Return amount of used memory in bytes
    virtual SLONG GetUsedMemory(void) {
      return sizeof(CPluginModule);
    };

    // Check if this kind of objects is can be freed automatically
    virtual BOOL IsAutoFreed(void) {
      return FALSE;
    };
};

#endif
