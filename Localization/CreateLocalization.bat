@echo off

set /p LANGCODE="Enter language code (example: ENG): "

set MAINDIR=%~dp0
set RESDIR=%MAINDIR%..\Resources\
set LOCALEDIR=%MAINDIR%%LANGCODE%\

set UPDATESCRIPT=Update%LANGCODE%.bat
set PACKSCRIPT=Assemble%LANGCODE%.bat

:: Controls
echo.
echo Copying "Controls" files...

xcopy "%RESDIR%Controls\06-ClassicsPatchCtrl1.des" "%LOCALEDIR%Controls\"
xcopy "%RESDIR%Controls\06-ClassicsPatchCtrl2.des" "%LOCALEDIR%Controls\"
xcopy "%RESDIR%Controls\06-ClassicsPatchCtrl3.des" "%LOCALEDIR%Controls\"
xcopy "%RESDIR%Controls\06-ClassicsPatchCtrl4.des" "%LOCALEDIR%Controls\"

:: Data
echo.
echo Copying "Data" files...

xcopy "%RESDIR%Data\ClassicsPatch\LevelCategories\00_TFE.des" "%LOCALEDIR%Data\ClassicsPatch\LevelCategories\"
xcopy "%RESDIR%Data\ClassicsPatch\LevelCategories\01_TSE.des" "%LOCALEDIR%Data\ClassicsPatch\LevelCategories\"
xcopy "%RESDIR%Data\ClassicsPatch\LevelCategories\02_SSR.des" "%LOCALEDIR%Data\ClassicsPatch\LevelCategories\"

:: Scripts
echo.
echo Copying "Scripts" files...

xcopy "%RESDIR%Scripts\Addons\ClassicsPatch_Full.des"                  "%LOCALEDIR%Scripts\Addons\"
xcopy "%RESDIR%Scripts\Addons\ClassicsPatch_Minimal.des"               "%LOCALEDIR%Scripts\Addons\"
xcopy "%RESDIR%Scripts\Addons\ClassicsPatch_Vanilla.des"               "%LOCALEDIR%Scripts\Addons\"

xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\00_Green.des"     "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\01_Blue.des"      "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\02_Yellow.des"    "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\03_Bleached.des"  "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\04_Demonic.des"   "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\05_Retro.des"     "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\06_Candy.des"     "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\07_BlackMesa.des" "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"
xcopy "%RESDIR%Scripts\ClassicsPatch\HUDColorPresets\08_Inverted.des"  "%LOCALEDIR%Scripts\ClassicsPatch\HUDColorPresets\"

xcopy "%RESDIR%Scripts\ClassicsPatch\Themes\00_SyncWithHUD.des"        "%LOCALEDIR%Scripts\ClassicsPatch\Themes\"
xcopy "%RESDIR%Scripts\ClassicsPatch\Themes\01_TFE.des"                "%LOCALEDIR%Scripts\ClassicsPatch\Themes\"
xcopy "%RESDIR%Scripts\ClassicsPatch\Themes\02_Warped.des"             "%LOCALEDIR%Scripts\ClassicsPatch\Themes\"
xcopy "%RESDIR%Scripts\ClassicsPatch\Themes\03_TSE.des"                "%LOCALEDIR%Scripts\ClassicsPatch\Themes\"
xcopy "%RESDIR%Scripts\ClassicsPatch\Themes\04_SSR.des"                "%LOCALEDIR%Scripts\ClassicsPatch\Themes\"

xcopy "%RESDIR%Scripts\ClassicsPatch\00_ConfigProperties.des"          "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\01_PatchSettings.des"             "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\02_ServerSettings.des"            "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\03_GameplaySettings.des"          "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\20_PluginSettings.des"            "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\50_ModSettings.des"               "%LOCALEDIR%Scripts\ClassicsPatch\"
xcopy "%RESDIR%Scripts\ClassicsPatch\90_VanillaSettings.des"           "%LOCALEDIR%Scripts\ClassicsPatch\"

xcopy "%RESDIR%Scripts\NetSettings\__ClassicsPatch.des"                "%LOCALEDIR%Scripts\NetSettings\"

:: Translation tables
echo.
echo Creating translation tables...

xcopy "%MAINDIR%Empty.txt" "%LOCALEDIR%Data\Translations\"
rename "%LOCALEDIR%Data\Translations\Empty.txt" "ClassicsPatch_Core.txt"

xcopy "%MAINDIR%Empty.txt" "%LOCALEDIR%Data\Translations\"
rename "%LOCALEDIR%Data\Translations\Empty.txt" "ClassicsPatch_Game.txt"

xcopy "%MAINDIR%Empty.txt" "%LOCALEDIR%Data\Translations\"
rename "%LOCALEDIR%Data\Translations\Empty.txt" "ClassicsPatch_Mod.txt"

xcopy "%MAINDIR%Empty.txt" "%LOCALEDIR%Data\Translations\"
rename "%LOCALEDIR%Data\Translations\Empty.txt" "ClassicsPatch_Scripts.txt"

:: Translation script
echo.
echo Generating translation update script...

:: ---------- Script start ---------- ::
echo @echo off> %UPDATESCRIPT%
echo cd %%~dp0>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo set GAMEDIR=%%~dp0>> %UPDATESCRIPT%
echo set EXTRACTTOOLDIR=ExtractionTool\>> %UPDATESCRIPT%
echo set EXTRACTTOOL=%%EXTRACTTOOLDIR%%Depend.exe>> %UPDATESCRIPT%
echo set TRANSDIR=%LANGCODE%\Data\Translations\>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo echo Updating "%%TRANSDIR%%ClassicsPatch_Core.txt"...>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo %%EXTRACTTOOL%% t %%GAMEDIR%% "%%EXTRACTTOOLDIR%%Trans_Core.txt" "%%TRANSDIR%%ClassicsPatch_Core.txt">> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo echo.>> %UPDATESCRIPT%
echo echo Updating "%%TRANSDIR%%ClassicsPatch_Game.txt"...>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo %%EXTRACTTOOL%% t %%GAMEDIR%% "%%EXTRACTTOOLDIR%%Trans_Game.txt" "%%TRANSDIR%%ClassicsPatch_Game.txt">> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo echo.>> %UPDATESCRIPT%
echo echo Updating "%%TRANSDIR%%ClassicsPatch_Mod.txt"...>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo %%EXTRACTTOOL%% t %%GAMEDIR%% "%%EXTRACTTOOLDIR%%Trans_Mod.txt" "%%TRANSDIR%%ClassicsPatch_Mod.txt">> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo echo.>> %UPDATESCRIPT%
echo echo Updating "%%TRANSDIR%%ClassicsPatch_Scripts.txt"...>> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo %%EXTRACTTOOL%% t %%GAMEDIR%% "%%EXTRACTTOOLDIR%%Trans_Scripts.txt" "%%TRANSDIR%%ClassicsPatch_Scripts.txt">> %UPDATESCRIPT%

echo.>> %UPDATESCRIPT%
echo pause>> %UPDATESCRIPT%
:: ---------- Script end ---------- ::

:: Assembly script
echo.
echo Generating GRO assembly script...

:: ---------- Script start ---------- ::
echo @echo off> %PACKSCRIPT%

echo.>> %PACKSCRIPT%
echo pushd %%~dp0>> %PACKSCRIPT%
echo del SE1_ClassicsPatch_%LANGCODE%.gro /q>> %PACKSCRIPT%

echo.>> %PACKSCRIPT%
echo pushd %LANGCODE%\>> %PACKSCRIPT%
echo 7z.exe a -tzip ..\SE1_ClassicsPatch_%LANGCODE%.gro *>> %PACKSCRIPT%

echo.>> %PACKSCRIPT%
echo popd>> %PACKSCRIPT%
echo popd>> %PACKSCRIPT%
:: ---------- Script end ---------- ::

echo.
echo Done!
echo To update translation tables with new strings, run "%UPDATESCRIPT%" script.
echo To pack localization into a redistributable GRO package, run "%PACKSCRIPT%" script.

pause
