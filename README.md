# Serious Sam Classics Patch

This custom Serious Sam Classic patch includes a lot of modern enhancements that aren't present in the original code, nor the official Steam patches.

This super project gathers everything together under a single solution for proper development.

**Projects include:**
- [`CoreLib`](https://github.com/SamClassicPatch/CoreLib) - static library with core functionality that's used by other projects.
- [`DedicatedServer`](https://github.com/SamClassicPatch/DedicatedServer) - dedicated server application for launching your own classic servers.
- [`EngineGUI`](https://github.com/SamClassicPatch/EngineGUI) - patched Serious Engine library that allows to integrate new features into different tools.
- [`ExamplePlugin`](https://github.com/SamClassicPatch/ExamplePlugin) - example of how user plugins can be made for the Serious Sam Classics patch.
- [`GameExecutable`](https://github.com/SamClassicPatch/GameExecutable) - executable file for playing the game that includes many quality-of-life improvements.
- [`Plugins`](https://github.com/SamClassicPatch/Plugins) - projects with various plugins that come with the Serious Sam Classics patch.
- [`WorldEditor`](https://github.com/SamClassicPatch/WorldEditor) - patched Serious Editor application that works on 64-bit systems.

Original source code of `DedicatedServer`, `EngineGUI`, `GameExecutable` and `WorldEditor` projects is taken from [Serious Engine 1.10](https://github.com/Croteam-official/Serious-Engine).

## Building

Before building the code, make sure to load in all of the submodules. Use `git submodule update --init --recursive` command to load files for all projects.

To compile the code, you'll need to use a compiler from Microsoft Visual C++ 6.0.

Full guide: https://github.com/DreamyCecil/SE1-ModSDK#building

### Notes

Once the projects are compiled for the `Release_TSE107` configuration, you can manually copy them into the `Bin` directory one level above the solution directory by using the `DeployToBin.bat` script (e.g. into `C:/SeriousSam/Bin` if this repository is in `C:/SeriousSam/SuperProject`).

## License

This project is licensed under GNU GPL v2 (see LICENSE file).

Some of the code included with the SDK may not be licensed under the GNU GPL v2:

* DirectX8 SDK (Headers & Libraries) (`d3d8.h`, `d3d8caps.h` and `d3d8types.h` located in `Includes` folder) by Microsoft
