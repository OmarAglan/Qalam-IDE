@echo off
setlocal

REM Wrapper for bootstrap-windows.ps1.
REM Useful when Windows says the .ps1 file is not digitally signed.

set "SCRIPT_DIR=%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%bootstrap-windows.ps1" %*
exit /b %ERRORLEVEL%
