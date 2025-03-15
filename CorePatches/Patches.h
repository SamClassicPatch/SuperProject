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

#if _PATCHCONFIG_ENGINEPATCHES

// Import library for use
#ifndef ENGINEPATCHES_EXPORTS
  #pragma comment(lib, "ClassicsPatches.lib")

  #define PATCHES_API __declspec(dllimport)
#else
  #define PATCHES_API __declspec(dllexport)
#endif

// Check if the fusion mode is available (only for TSE)
#define TSE_FUSION_MODE (SE1_GAME == SS_TSE)

// Available engine patches
class PATCHES_API CPatches : public ICorePatches {
  public:
    // Rendering
    INDEX _bAdjustForAspectRatio;
    INDEX _bUseVerticalFOV;
    FLOAT _fCustomFOV;
    FLOAT _fThirdPersonFOV;
    INDEX _bCheckFOV;

    // Sound library
    BOOL _bNoListening; // Don't listen to in-game sounds

    // Unpage streams
    ULONG _ulMaxWriteMemory; // Enough memory for writing
    INDEX _bUsePlaceholderResources; // Automatically replace missing resources with placeholders

    // Worlds
    ELevelFormat _eWorldFormat; // Format of the last loaded world

    // Which world converter to use:
    // -1 - disabled
    //  0 - reinit every entity
    //  1 - force TFE converter
    INDEX _iWorldConverter;

  public:
    // Constructor
    CPatches();

    // Clean up on Core shutdown (only for patches set by CorePatches() method)
    virtual void Cleanup(void);

    // Apply core patches (called after Core initialization!)
    void CorePatches(void);

  // Patches after Serious Engine and Core initializations
  private:

    // Enhance entities usage
    void Entities(void);

    // Fix timers for entity logic
    void LogicTimers(void);

    // Enhance network library usage
    void Network(void);

    // Enhance rendering
    void Rendering(void);

    // Enhance sound library usage
    void SoundLibrary(void);

    // Enhance strings usage
    void Strings(void);

    // Enhance texture usage
    void Textures(void);

    // Enhance worlds
    void Worlds(void);

  // Patches before Serious Engine and Core initializations
  public:

    // Customize core file handling in the engine
    void FileSystem(void);

  // [Cecil] TODO: Make SKA patches work in Debug
  #if SE1_VER >= SE1_107 && defined(NDEBUG)
    // Fix SKA models
    void Ska(void);
  #endif

    // Don't use memory paging in streams
    void UnpageStreams(void);
};

// Singleton for patching
PATCHES_API extern CPatches _EnginePatches;

#endif // _PATCHCONFIG_ENGINEPATCHES
