@echo off
setlocal

call "%~dp0build.cmd"
if errorlevel 1 exit /b %errorlevel%

"%~dp0build\state_sync_simulation.exe"
exit /b %errorlevel%
