:: This script will copy built binaries of the patch for TSE 1.07 into the game's Bin directory
:: one level above this one, including a GRO package with all the needed resources packed

@echo off

:: Copy TSE 1.07 binaries
echo ClassicsCore
copy /B Bin\Release_TSE107\ClassicsCore.dll ..\Bin\

echo ClassicsPatches
copy /B Bin\Release_TSE107\ClassicsPatches.dll ..\Bin\

echo DedicatedServer
copy /B Bin\Release_TSE107\DedicatedServer_Custom.exe ..\Bin\

echo SeriousSam
copy /B Bin\Release_TSE107\SeriousSam_Custom.exe ..\Bin\

echo SeriousEditor
copy /B Bin\Release_TSE107\SeriousEditor_Custom.exe ..\Bin\

echo Shaders
copy /B Bin\Release_TSE107\Shaders.dll ..\Bin\

echo EngineGUI
copy /B Bin\Release_TSE107\EngineGUI.dll ..\Bin\

:: Copy mod files
echo Entities
copy /B Bin\Release_TSE107\Entities_Custom.dll ..\Bin\

echo Game
copy /B Bin\Release_TSE107\Game_Custom.dll ..\Bin\

echo GameGUI
copy /B Bin\Release_TSE107\GameGUI_Custom.dll ..\Bin\

echo ClassicsPatchMod
robocopy Builds\TSE107\Mods ..\Mods\ /E

:: Copy TSE 1.07 plugins
copy /B Bin\Release_TSE107\Plugins\*.dll ..\Bin\Plugins\

:: Pack resources
call Resources\DeployResources.bat %~dp0..\

echo.
echo Finished deploying!
