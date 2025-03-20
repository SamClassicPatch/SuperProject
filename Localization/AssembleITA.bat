@echo off

pushd %~dp0
IF EXIST SE1_ClassicsPatch_ITA.gro DEL SE1_ClassicsPatch_ITA.gro /q

pushd ITA\
7z.exe a -tzip ..\SE1_ClassicsPatch_ITA.gro *

popd
popd
