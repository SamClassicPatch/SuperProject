@echo off

cd %~dp0
del SE1_ClassicsPatch_RUS.gro /q

cd RUS\
7z.exe a -tzip ..\SE1_ClassicsPatch_RUS.gro *
