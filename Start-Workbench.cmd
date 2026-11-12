@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0workbench\Start-Dev.ps1" -Action Workbench
if errorlevel 1 pause
