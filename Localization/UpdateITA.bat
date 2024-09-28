@echo off
cd %~dp0

set GAMEDIR=%~dp0
set EXTRACTTOOLDIR=ExtractionTool\
set EXTRACTTOOL=%EXTRACTTOOLDIR%Depend.exe
set TRANSDIR=ITA\Data\Translations\

echo Updating "%TRANSDIR%ClassicsPatch_Core.txt"...

%EXTRACTTOOL% t %GAMEDIR% "%EXTRACTTOOLDIR%Trans_Core.txt" "%TRANSDIR%ClassicsPatch_Core.txt"

echo.
echo Updating "%TRANSDIR%ClassicsPatch_Game.txt"...

%EXTRACTTOOL% t %GAMEDIR% "%EXTRACTTOOLDIR%Trans_Game.txt" "%TRANSDIR%ClassicsPatch_Game.txt"

echo.
echo Updating "%TRANSDIR%ClassicsPatch_Mod.txt"...

%EXTRACTTOOL% t %GAMEDIR% "%EXTRACTTOOLDIR%Trans_Mod.txt" "%TRANSDIR%ClassicsPatch_Mod.txt"

echo.
echo Updating "%TRANSDIR%ClassicsPatch_Scripts.txt"...

%EXTRACTTOOL% t %GAMEDIR% "%EXTRACTTOOLDIR%Trans_Scripts.txt" "%TRANSDIR%ClassicsPatch_Scripts.txt"

pause
