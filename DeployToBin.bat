@echo off
echo Copying TSE 1.07 binaries...
echo ----------------------------

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
echo Copying TSE 1.07 plugins...
echo ----------------------------

copy /B Bin\Release_TSE107\Plugins\*.dll ..\Bin\Plugins\

echo ----------------------------
echo Finished deploying!
