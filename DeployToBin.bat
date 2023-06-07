@echo off
echo Copying TSE 1.07 binaries...
echo ----------------------------

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
copy /B /-Y Bin\Release_TSE107\EngineGUI.dll ..\Bin\

echo ----------------------------
echo Copying mod binaries...
echo ----------------------------

echo Entities
copy /B Bin\Release_TSE107\Entities_Custom.dll ..\Bin\

echo Game
copy /B Bin\Release_TSE107\Game_Custom.dll ..\Bin\

echo GameGUI
copy /B Bin\Release_TSE107\GameGUI_Custom.dll ..\Bin\

echo ----------------------------
echo Copying TSE 1.07 plugins...
echo ----------------------------

copy /B Bin\Release_TSE107\Plugins\*.dll ..\Bin\Plugins\

echo ----------------------------
echo Finished deploying!
