@echo off
setlocal

REM Low-hassle Windows entry point for users blocked by PowerShell execution policy.
REM This changes the execution policy only for this PowerShell process.

set "ROOT_DIR=%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT_DIR%scripts\bootstrap-windows.ps1" %*
exit /b %ERRORLEVEL%
