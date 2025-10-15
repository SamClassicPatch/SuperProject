@echo off

pushd %~dp0
if exist SE1_ClassicsPatch_ITA.gro del SE1_ClassicsPatch_ITA.gro /q

pushd ITA\
7z.exe a -tzip ..\SE1_ClassicsPatch_ITA.gro *

popd
popd
