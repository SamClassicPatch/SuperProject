:: You need to specify 7-Zip installation directory in the PATH environment variable for this script to work!

:: This script copies the GRO package into a new location and adds remaining resources to it
:: If you run the script on its own, it will just pack everything from this directory into the GRO

@echo off

pushd %~dp0
copy /B SE1_ClassicsPatch.gro "%~f1"

7z.exe u -tzip "%~f1SE1_ClassicsPatch.gro" -r * -x!*.gro -x!*.bat
popd
