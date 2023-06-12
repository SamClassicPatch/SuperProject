:: You need to specify 7-Zip installation directory in the PATH environment variable for this script to work!

:: This script creates a GRO package with Russian localization

@echo off

:: Remove old package
cd %~dp0
del SE1_ClassicsPatch_RUS.gro

cd RUS\
7z.exe a ..\SE1_ClassicsPatch_RUS.gro *
