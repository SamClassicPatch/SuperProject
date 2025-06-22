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

goto BeginCopy

:: Function for copying files for each game
:: %1 - Project directory
:: %2 - Bin directory
:: %3 - Binary filename
:: %4 - Binary extension
:: %5 - Destination directory
:: %6 - Subdirectory
:CopyFile
mkdir %5DebugFiles\
copy /B %REPO%%2%6%3%4   %5%6
copy /B %REPO%%2%6%3.map %5DebugFiles\
goto :EOF

:BeginCopy

echo ----------------------------
echo Copying TFE 1.05 binaries...
echo ----------------------------

cd %REPO%
set GAMEBUILD=TFE105

:: Collect files for the TFE 1.05 build
call :CopyFile  Core\                        Bin\Release_%GAMEBUILD%\  ClassicsCore            .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomDedicatedServer\       Bin\Release_%GAMEBUILD%\  DedicatedServer_Custom  .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomEngineGUI\             Bin\Release_%GAMEBUILD%\  EngineGUI               .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomGameClient\            Bin\Release_%GAMEBUILD%\  SeriousSam_Custom       .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomWorldEditor\           Bin\Release_%GAMEBUILD%\  SeriousEditor_Custom    .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\EntitiesTFE\       Bin\Release_%GAMEBUILD%\  Entities_Custom         .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\ExtraEntities\     Bin\Release_%GAMEBUILD%\  ClassicsExtras_Custom   .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\Game\              Bin\Release_%GAMEBUILD%\  Game_Custom             .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\GameGUI\           Bin\Release_%GAMEBUILD%\  GameGUI_Custom          .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  Extensions\Accessibility\    Bin\Release_%GAMEBUILD%\  Accessibility           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\AdvancedHUD\      Bin\Release_%GAMEBUILD%\  AdvancedHUD             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ExtendedInput\    Bin\Release_%GAMEBUILD%\  ExtendedInput           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\LocalCheats\      Bin\Release_%GAMEBUILD%\  LocalCheats             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ScriptingEngine\  Bin\Release_%GAMEBUILD%\  ScriptingEngine         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ServerUtilities\  Bin\Release_%GAMEBUILD%\  ServerUtilities         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\WorldConverters\  Bin\Release_%GAMEBUILD%\  WorldConverters         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
copy /B %STEAMLIB%                  %BUILDS%%GAMEBUILD%\Bin\
copy /B %SDLLIB%                    %BUILDS%%GAMEBUILD%\Bin\
call Resources\DeployResources.bat  %BUILDS%%GAMEBUILD%\
robocopy %BUILDS%TSE107\Mods\       %BUILDS%%GAMEBUILD%\Mods\ /E

echo ----------------------------
echo Copying TSE 1.05 binaries...
echo ----------------------------

cd %REPO%
set GAMEBUILD=TSE105

:: Collect files for the TSE 1.05 build
call :CopyFile  Core\                        Bin\Release_%GAMEBUILD%\  ClassicsCore            .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomDedicatedServer\       Bin\Release_%GAMEBUILD%\  DedicatedServer_Custom  .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomEngineGUI\             Bin\Release_%GAMEBUILD%\  EngineGUI               .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomGameClient\            Bin\Release_%GAMEBUILD%\  SeriousSam_Custom       .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomWorldEditor\           Bin\Release_%GAMEBUILD%\  SeriousEditor_Custom    .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\EntitiesTFE\       Bin\Release_%GAMEBUILD%\  Entities_Custom         .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\ExtraEntities\     Bin\Release_%GAMEBUILD%\  ClassicsExtras_Custom   .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\Game\              Bin\Release_%GAMEBUILD%\  Game_Custom             .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\GameGUI\           Bin\Release_%GAMEBUILD%\  GameGUI_Custom          .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  Extensions\Accessibility\    Bin\Release_%GAMEBUILD%\  Accessibility           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\AdvancedHUD\      Bin\Release_%GAMEBUILD%\  AdvancedHUD             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ExtendedInput\    Bin\Release_%GAMEBUILD%\  ExtendedInput           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\LocalCheats\      Bin\Release_%GAMEBUILD%\  LocalCheats             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ScriptingEngine\  Bin\Release_%GAMEBUILD%\  ScriptingEngine         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ServerUtilities\  Bin\Release_%GAMEBUILD%\  ServerUtilities         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\WorldConverters\  Bin\Release_%GAMEBUILD%\  WorldConverters         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
copy /B %STEAMLIB%                  %BUILDS%%GAMEBUILD%\Bin\
copy /B %SDLLIB%                    %BUILDS%%GAMEBUILD%\Bin\
call Resources\DeployResources.bat  %BUILDS%%GAMEBUILD%\
robocopy %BUILDS%TSE107\Mods\       %BUILDS%%GAMEBUILD%\Mods\ /E

echo ----------------------------
echo Copying TSE 1.07 binaries...
echo ----------------------------

cd %REPO%
set GAMEBUILD=TSE107

:: Collect files for the TSE 1.07 build
call :CopyFile  Core\                        Bin\Release_%GAMEBUILD%\  ClassicsCore            .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomDedicatedServer\       Bin\Release_%GAMEBUILD%\  DedicatedServer_Custom  .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomEngineGUI\             Bin\Release_%GAMEBUILD%\  EngineGUI               .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomGameClient\            Bin\Release_%GAMEBUILD%\  SeriousSam_Custom       .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomShaders\               Bin\Release_%GAMEBUILD%\  Shaders                 .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomWorldEditor\           Bin\Release_%GAMEBUILD%\  SeriousEditor_Custom    .exe  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\EntitiesTFE\       Bin\Release_%GAMEBUILD%\  Entities_Custom         .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\ExtraEntities\     Bin\Release_%GAMEBUILD%\  ClassicsExtras_Custom   .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\Game\              Bin\Release_%GAMEBUILD%\  Game_Custom             .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  CustomMod\GameGUI\           Bin\Release_%GAMEBUILD%\  GameGUI_Custom          .dll  %BUILDS%%GAMEBUILD%\Bin\  ""
call :CopyFile  Extensions\Accessibility\    Bin\Release_%GAMEBUILD%\  Accessibility           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\AdvancedHUD\      Bin\Release_%GAMEBUILD%\  AdvancedHUD             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ExtendedInput\    Bin\Release_%GAMEBUILD%\  ExtendedInput           .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\LocalCheats\      Bin\Release_%GAMEBUILD%\  LocalCheats             .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ScriptingEngine\  Bin\Release_%GAMEBUILD%\  ScriptingEngine         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\ServerUtilities\  Bin\Release_%GAMEBUILD%\  ServerUtilities         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
call :CopyFile  Extensions\WorldConverters\  Bin\Release_%GAMEBUILD%\  WorldConverters         .dll  %BUILDS%%GAMEBUILD%\Bin\  "Plugins\"
copy /B %STEAMLIB%                  %BUILDS%%GAMEBUILD%\Bin\
copy /B %SDLLIB%                    %BUILDS%%GAMEBUILD%\Bin\
call Resources\DeployResources.bat  %BUILDS%%GAMEBUILD%\

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
