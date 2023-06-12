# Serious Sam Classics Patch

This fan-made patch for Serious Sam Classics includes a lot of modern enhancements that aren't present in the open-source release, nor in the official Steam patches since 2014.

This super project gathers everything together under a single solution for proper development.

### Project structure
- [`CoreLib`](../../../CoreLib) - library with core functionality that's used by other projects.
- [`DedicatedServer`](../../../DedicatedServer) - dedicated server application for launching your own classic servers.
- [`EnginePatches`](../../../EnginePatches) - library with dynamic patches for Serious Engine functions that can be selectively applied to specific modules. 
- [`ExamplePlugin`](../../../ExamplePlugin) - example of how user plugins can be made for Serious Sam Classics Patch.
- [`GameExecutable`](../../../GameExecutable) - executable file for playing the game that includes many quality-of-life improvements.
- [`Mod`](../../../Mod) - libraries of a special mod that fixes more intricate issues and expands gameplay possibilities.
- [`PatchedGui`](../../../PatchedGui) - patched EngineGUI library from the engine that allows to integrate new features into different tools.
- [`Plugins`](../../../Plugins) - projects with various plugins that come with Serious Sam Classics Patch.
- [`Shaders`](../../../Shaders) - patched Shaders library with certain fixes for SKA models and shaders used by them.
- [`WorldEditor`](../../../WorldEditor) - patched Serious Editor application that works on 64-bit systems.
- `Builds` - directory for assembling final builds of Serious Sam Classics Patch for every supported game version.
  - [`Builds/TSE107/Mods/ClassicsPatchMod`](../../../ModShell) - shell of a special mod that includes resources necessary for properly running it.
- `Docs` - up-to-date contents of the [wiki](../../wiki) for any specific release.
- `Extras` - third-party dependencies that the projects utilize.
- `Includes` - [Serious Engine 1 SDK](https://github.com/DreamyCecil/SE1-ModSDK/tree/includes) that includes static libraries and engine headers for different game versions.
- `Localization` - translations of Serious Sam Classics Patch contents to different languages.
- `Properties` - common properties for use in project files.
- `Resources` - resources in text & binary formats that are included into final builds.

Original source code of `DedicatedServer`, `GameExecutable`, `Mod`, `PatchedGui`, `Shaders` and `WorldEditor` projects is taken from [Serious Engine 1.10](https://github.com/Croteam-official/Serious-Engine).

## Building

Before building the code, make sure to load in all of the submodules. Use `git submodule update --init --recursive` command to load files for all projects.

To compile the code, you'll need to use a compiler from Microsoft Visual C++ 6.0.

Full guide: https://github.com/DreamyCecil/SE1-ModSDK#building

### Notes

After the project solution is built for the `Release_TSE107` configuration, you can apply the patch to your build of TSE 1.07 one level above by using the `DeployToBin.bat` script (e.g. to `C:/SeriousSam` if this repository resides is in `C:/SeriousSam/SuperProject`).

## License

This project is licensed under GNU GPL v2 (see LICENSE file).

Some of the code included with the SDK may not be licensed under the GNU GPL v2:

- zlib (located in `Extras/zlib`) by Jean-loup Gailly and Mark Adler
