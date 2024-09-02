:: È necessario specificare la cartella di installazione di 7-Zip nella variabile di ambiente PATH per fare in modo che questo script funzioni!

:: Questo script crea un pacchetto GRO con la localizzazione italiana

@echo off

:: Rimozione del vecchio pacchetto
cd %~dp0
IF EXIST SE1_ClassicsPatch_ITA.gro del SE1_ClassicsPatch_ITA.gro

cd ITA\
7z.exe a -tzip ..\SE1_ClassicsPatch_ITA.gro *
