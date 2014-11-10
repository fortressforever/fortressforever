@echo off
powershell -command 'Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force'
powershell -file ../copy_bins_steam.ps1 %1 