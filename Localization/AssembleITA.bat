@echo off

cd %~dp0
IF EXIST SE1_ClassicsPatch_ITA.gro DEL SE1_ClassicsPatch_ITA.gro /q

cd ITA\
7z.exe a -tzip ..\SE1_ClassicsPatch_ITA.gro *
