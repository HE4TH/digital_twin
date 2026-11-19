@echo off

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-Quest3.ps1" %*

set "install_exit=%errorlevel%"

pause

exit /b %install_exit%

