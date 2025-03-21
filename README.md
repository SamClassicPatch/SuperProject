# Serious Sam Classics Patch

This fan-made patch for **Serious Sam Classics** includes lots of enhancements and new functionality for the game client, dedicated servers, tool applications and even gameplay logic.

This super project gathers everything together under a single solution for proper development.

> **See [Classics Patch wiki](../../wiki) for various information about the entire project.**

> **Want to submit an issue? Read [this guide](../../wiki/How-to-Submit-issue) on how to do it!**

## Project structure
- [`API`](../../../API) - Standalone API that exposes `Core` methods via C API.
- `Core` - Library with core functionality that's used by other projects, including dynamic patches of Serious Engine functions.
- `CustomDedicatedServer` - Dedicated server application for launching your own classic servers.
- `CustomEngineGUI` - Patched EngineGUI library from the engine that allows to integrate new features into different tools.
- `CustomGameClient` - Executable file for playing the game that includes many quality-of-life improvements.
- `CustomMod` - Libraries of a special mod that fixes more intricate issues and expands gameplay possibilities.
- `CustomShaders` - Patched Shaders library with certain fixes for SKA models and shaders used by them.
- `CustomWorldEditor` - Patched Serious Editor application that works on 64-bit systems.
- `Extensions` - Projects with various plugins that come with Serious Sam Classics Patch.
  - `Sample` - Sample project that demonstates how to make custom user plugins.
- `Builds` - Directory for assembling final builds of Serious Sam Classics Patch for every supported game version.
  - [`TSE107/Mods/ClassicsPatchMod`](../../../ModShell) - Shell of a special mod that includes resources necessary for properly running it.
- `Docs` - Up-to-date contents of the [wiki](../../wiki) for any specific release.
- `Extras` - Global dependencies that the projects utilize.
  - [`SDL`](https://libsdl.org/) - Cross-platform abstraction library for implementing proper support of game controllers.
  - [`Steamworks`](https://partner.steamgames.com/) - SDK for enabling extended interactions with the Steam client.
  - [`XGizmo`](../../../XGizmo) - Headers with a variety of convenient classes and functions for Serious Engine 1.
  - [`zlib`](https://zlib.net/) - Static library for working with ZIP archives.
- `Includes` - [Serious Engine 1 SDK](https://github.com/DreamyCecil/SE1-ModSDK/tree/includes) that includes static libraries and engine headers for different game versions.
- `Localization` - Translations of Serious Sam Classics Patch contents to different languages.
- `Properties` - Common properties for use in project files.
- `Resources` - Resources in text & binary formats that are included into final builds.

Original source code of `CustomDedicatedServer`, `CustomEngineGUI`, `CustomGameClient`, `CustomMod`, `CustomShaders` and `CustomWorldEditor` projects is taken from [Serious Engine 1.10](https://github.com/Croteam-official/Serious-Engine).

# Building

Building instructions are available here: https://github.com/DreamyCecil/SE1-ModSDK/wiki/Building

> [!NOTE]
> A lot of batch scripts in the repository (`.bat`) utilize [7-Zip](https://www.7-zip.org/) in order to pack files into ZIP archives. Make sure to install it and specify a path to it in the environment variables for the scripts to work.

> [!TIP]
> You can apply the compiled build of the patch to your build of TSE 1.07 one level above by using the `DeployToBin - Debug.bat` and `DeployToBin - Release.bat` scripts (for `Debug_TSE107` and `Release_TSE107` configurations, respectively).  
> For example, if this repository resides in `C:/SeriousSam/SuperProject/`, the builds will be deployed into `C:/SeriousSam/` with the executables in `C:/SeriousSam/Bin/Debug/` (for Debug builds) or in `C:/SeriousSam/Bin/` (for Release builds).

# License

This project is licensed under GNU GPL v2 (see LICENSE file).

Some of the code included with the SDK may not be licensed under the GNU GPL v2:

- Simple DirectMedia Layer (located in `Extras/SDL/`) from https://libsdl.org/
- Steamworks SDK (located in `Extras/Steamworks/`) by Valve Corporation
- zlib (located in `Extras/zlib/`) by Jean-loup Gailly and Mark Adler
