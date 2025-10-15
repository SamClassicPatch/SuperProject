@echo off

pushd %~dp0
if exist SE1_ClassicsPatch_RUS.gro del SE1_ClassicsPatch_RUS.gro /q

pushd RUS\
7z.exe a -tzip ..\SE1_ClassicsPatch_RUS.gro *

popd
popd
