:: This script will copy built binaries of the patch for TSE 1.07 into the game's Bin directory
:: one level above this one, including a GRO package with all the needed resources packed

@echo off

set BUILTDIR=Bin\Debug_TSE107\
set GAMEDIR=%~dp0..\
set BINDIR=%GAMEDIR%Bin\Debug\

:: Copy TSE 1.07 binaries
echo ClassicsCore
copy /B %BUILTDIR%ClassicsCore.dll %BINDIR%

echo ClassicsExtras
copy /B %BUILTDIR%ClassicsExtras_CustomD.dll %BINDIR%

echo DedicatedServer
copy /B %BUILTDIR%DedicatedServer_Custom.exe %BINDIR%

echo SeriousSam
copy /B %BUILTDIR%SeriousSam_Custom.exe %BINDIR%

echo SeriousEditor
copy /B %BUILTDIR%SeriousEditor_Custom.exe %BINDIR%

echo Shaders
copy /B %BUILTDIR%ShadersD.dll %BINDIR%

echo EngineGUI
copy /B %BUILTDIR%EngineGUID.dll %BINDIR%

:: Copy mod files
echo Entities
copy /B %BUILTDIR%Entities_CustomD.dll %BINDIR%

echo Game
copy /B %BUILTDIR%Game_CustomD.dll %BINDIR%

echo GameGUI
copy /B %BUILTDIR%GameGUI_CustomD.dll %BINDIR%

echo ClassicsPatchMod
robocopy Builds\TSE107\Mods %GAMEDIR%Mods\ /E

:: Copy TSE 1.07 plugins
mkdir %BINDIR%Plugins
copy /B %BUILTDIR%Plugins\*.dll %BINDIR%Plugins\

:: Copy SDL library
copy /B Extras\SDL\lib\x86\SDL3.dll %BINDIR%

:: Pack resources
call Resources\DeployResources.bat %GAMEDIR%

echo.
echo Finished deploying!
