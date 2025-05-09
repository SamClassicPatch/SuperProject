:: You need to specify 7-Zip installation directory in the PATH environment variable for this script to work!

:: This script gathers all resources for each game build and packs them into their own ZIP packages

@echo off

:: Get current date
for /f "delims=" %%a in ('wmic OS Get localdatetime  ^| find "."') do set CURDATEFULL=%%a
set CURDATE=%CURDATEFULL:~0,4%_%CURDATEFULL:~4,2%_%CURDATEFULL:~6,2%

:: Current builds directory and a repository directory above
set BUILDS=%~dp0
set REPO=%BUILDS%..\
set STEAMLIB=%REPO%Extras\Steamworks\redistributable_bin\steam_api.dll
set SDLLIB=%REPO%Extras\SDL\lib\x86\SDL3.dll

echo Copying GRO archives...
echo ----------------------------

cd %REPO%

:: Copy extra GRO files that are released alongside the final builds
call Localization\AssembleITA.bat
copy /B Localization\SE1_ClassicsPatch_ITA.gro %BUILDS%
call Localization\AssembleRUS.bat
copy /B Localization\SE1_ClassicsPatch_RUS.gro %BUILDS%

copy /B Resources\SE1_RevResources.gro         %BUILDS%

echo ----------------------------
echo Copying TFE 1.05 binaries...
echo ----------------------------

cd %REPO%

:: Collect files for the TFE 1.05 build
copy /B %STEAMLIB%                                     %BUILDS%TFE105\Bin\
copy /B %SDLLIB%                                       %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\ClassicsCore.dll            %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\ClassicsExtras_Custom.dll   %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\DedicatedServer_Custom.exe  %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\SeriousSam_Custom.exe       %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\SeriousEditor_Custom.exe    %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\EngineGUI.dll               %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\Entities_Custom.dll         %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\Game_Custom.dll             %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\GameGUI_Custom.dll          %BUILDS%TFE105\Bin\
copy /B Bin\Release_TFE105\Plugins\Accessibility.dll   %BUILDS%TFE105\Bin\Plugins\
copy /B Bin\Release_TFE105\Plugins\AdvancedHUD.dll     %BUILDS%TFE105\Bin\Plugins\
copy /B Bin\Release_TFE105\Plugins\ExtendedInput.dll   %BUILDS%TFE105\Bin\Plugins\
copy /B Bin\Release_TFE105\Plugins\LocalCheats.dll     %BUILDS%TFE105\Bin\Plugins\
copy /B Bin\Release_TFE105\Plugins\ServerUtilities.dll %BUILDS%TFE105\Bin\Plugins\
copy /B Bin\Release_TFE105\Plugins\WorldConverters.dll %BUILDS%TFE105\Bin\Plugins\
call Resources\DeployResources.bat                     %BUILDS%TFE105\
robocopy %BUILDS%TSE107\Mods\                          %BUILDS%TFE105\Mods\ /E

echo ----------------------------
echo Copying TSE 1.05 binaries...
echo ----------------------------

cd %REPO%

:: Collect files for the TSE 1.05 build
copy /B %STEAMLIB%                                     %BUILDS%TSE105\Bin\
copy /B %SDLLIB%                                       %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\ClassicsCore.dll            %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\ClassicsExtras_Custom.dll   %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\DedicatedServer_Custom.exe  %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\SeriousSam_Custom.exe       %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\SeriousEditor_Custom.exe    %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\EngineGUI.dll               %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\Entities_Custom.dll         %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\Game_Custom.dll             %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\GameGUI_Custom.dll          %BUILDS%TSE105\Bin\
copy /B Bin\Release_TSE105\Plugins\Accessibility.dll   %BUILDS%TSE105\Bin\Plugins\
copy /B Bin\Release_TSE105\Plugins\AdvancedHUD.dll     %BUILDS%TSE105\Bin\Plugins\
copy /B Bin\Release_TSE105\Plugins\ExtendedInput.dll   %BUILDS%TSE105\Bin\Plugins\
copy /B Bin\Release_TSE105\Plugins\LocalCheats.dll     %BUILDS%TSE105\Bin\Plugins\
copy /B Bin\Release_TSE105\Plugins\ServerUtilities.dll %BUILDS%TSE105\Bin\Plugins\
copy /B Bin\Release_TSE105\Plugins\WorldConverters.dll %BUILDS%TSE105\Bin\Plugins\
call Resources\DeployResources.bat                     %BUILDS%TSE105\
robocopy %BUILDS%TSE107\Mods\                          %BUILDS%TSE105\Mods\ /E

echo ----------------------------
echo Copying TSE 1.07 binaries...
echo ----------------------------

cd %REPO%

:: Collect files for the TSE 1.07 build
copy /B %STEAMLIB%                                     %BUILDS%TSE107\Bin\
copy /B %SDLLIB%                                       %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\ClassicsCore.dll            %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\ClassicsExtras_Custom.dll   %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\DedicatedServer_Custom.exe  %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\SeriousSam_Custom.exe       %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\SeriousEditor_Custom.exe    %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\EngineGUI.dll               %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\Entities_Custom.dll         %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\Game_Custom.dll             %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\GameGUI_Custom.dll          %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\Shaders.dll                 %BUILDS%TSE107\Bin\
copy /B Bin\Release_TSE107\Plugins\Accessibility.dll   %BUILDS%TSE107\Bin\Plugins\
copy /B Bin\Release_TSE107\Plugins\AdvancedHUD.dll     %BUILDS%TSE107\Bin\Plugins\
copy /B Bin\Release_TSE107\Plugins\ExtendedInput.dll   %BUILDS%TSE107\Bin\Plugins\
copy /B Bin\Release_TSE107\Plugins\LocalCheats.dll     %BUILDS%TSE107\Bin\Plugins\
copy /B Bin\Release_TSE107\Plugins\ServerUtilities.dll %BUILDS%TSE107\Bin\Plugins\
copy /B Bin\Release_TSE107\Plugins\WorldConverters.dll %BUILDS%TSE107\Bin\Plugins\
call Resources\DeployResources.bat                     %BUILDS%TSE107\

echo ----------------------------
echo Cleaning up GRO packages...
echo ----------------------------

cd %BUILDS%

:: Remove unnecessary files from specific games
7z.exe d -tzip TFE105\SE1_ClassicsPatch.gro -i@CleanupTFE105.txt
7z.exe d -tzip TSE105\SE1_ClassicsPatch.gro -i@CleanupTSE105.txt
7z.exe d -tzip TSE107\SE1_ClassicsPatch.gro -i@CleanupTSE107.txt

echo ----------------------------
echo Zipping build packages...
echo ----------------------------

:: Remove old builds
del TFE1.05_%CURDATE%.zip
del TSE1.05_%CURDATE%.zip
del TSE1.07_%CURDATE%.zip

:: Assemble a TFE 1.05 package
cd %BUILDS%TFE105\
7z.exe a -tzip ..\TFE1.05_%CURDATE%.zip * -x!Mods\ClassicsPatchMod\.git*

:: Assemble a TSE 1.05 package
cd %BUILDS%TSE105\
7z.exe a -tzip ..\TSE1.05_%CURDATE%.zip * -x!Mods\ClassicsPatchMod\.git*

:: Assemble a TSE 1.07 package
cd %BUILDS%TSE107\
7z.exe a -tzip ..\TSE1.07_%CURDATE%.zip * -x!Mods\ClassicsPatchMod\.git*

echo ----------------------------
echo Finished assembling!
