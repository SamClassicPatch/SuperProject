This directory is for assembling final builds of Classics Patch for every supported game version.

After the project solution is built, run AssembleBuilds.bat and it will first gather all needed files under dedicated game folders and then pack them into their own ZIP packages, suited for distribution.

The builds include a GRO package with patch resources for a specific game and original vanilla DLL files in the Bin directory that the patch replaces with its own.

The builds are also able to pack a Steamworks API library (steam_api.dll) that is located next to AssembleBuilds.bat to be used as a bridge between the game and the Steam client but it is not included in this repository and needs to be downloaded separately with the Steamworks SDK from https://partner.steamgames.com/ or taken from one of the release builds of Classics Patch.
