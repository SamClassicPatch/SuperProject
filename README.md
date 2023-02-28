# Serious Sam Classics Patch

This custom Serious Sam Classics patch includes a lot of modern enhancements that aren't present in the original code, nor the official Steam patches.

This super project gathers everything together under a single solution for proper development.

**Projects include:**
- [`CoreLib`](https://github.com/SamClassicPatch/CoreLib) - library with core functionality that's used by other projects.
- [`DedicatedServer`](https://github.com/SamClassicPatch/DedicatedServer) - dedicated server application for launching your own classic servers.
- [`EnginePatches`](https://github.com/SamClassicPatch/EnginePatches) - library with dynamic patches for Serious Engine functions that can be selectively applied to specific modules. 
- [`ExamplePlugin`](https://github.com/SamClassicPatch/ExamplePlugin) - example of how user plugins can be made for Serious Sam Classics Patch.
- [`GameExecutable`](https://github.com/SamClassicPatch/GameExecutable) - executable file for playing the game that includes many quality-of-life improvements.
- [`PatchedGui`](https://github.com/SamClassicPatch/PatchedGui) - patched EngineGUI library from the engine that allows to integrate new features into different tools.
- [`Plugins`](https://github.com/SamClassicPatch/Plugins) - projects with various plugins that come with Serious Sam Classics Patch.
- [`Shaders`](https://github.com/SamClassicPatch/Shaders) - patched Shaders library with certain fixes for SKA models and shaders used by them.
- [`WorldEditor`](https://github.com/SamClassicPatch/WorldEditor) - patched Serious Editor application that works on 64-bit systems.

Original source code of `DedicatedServer`, `GameExecutable`, `PatchedGui`, `Shaders` and `WorldEditor` projects is taken from [Serious Engine 1.10](https://github.com/Croteam-official/Serious-Engine).

## Building

Before building the code, make sure to load in all of the submodules. Use `git submodule update --init --recursive` command to load files for all projects.

To compile the code, you'll need to use a compiler from Microsoft Visual C++ 6.0.

Full guide: https://github.com/DreamyCecil/SE1-ModSDK#building

### Notes

Once the projects are compiled for the `Release_TSE107` configuration, you can manually copy them into the `Bin` directory one level above the solution directory by using the `DeployToBin.bat` script (e.g. into `C:/SeriousSam/Bin` if this repository is in `C:/SeriousSam/SuperProject`).

## License

This project is licensed under GNU GPL v2 (see LICENSE file).

Some of the code included with the SDK may not be licensed under the GNU GPL v2:

- zlib (located in `Extras/zlib`) by Jean-loup Gailly and Mark Adler
