@echo off
setlocal

for %%I in ("%~dp0.") do set "PROJECT_DIR=%%~fI"
for %%I in ("%PROJECT_DIR%\..\..") do set "WORKSPACE_ROOT=%%~fI"
set "TOOLCHAIN_BIN=%WORKSPACE_ROOT%\.local\w64devkit\bin"

if not exist "%TOOLCHAIN_BIN%\gcc.exe" (
    echo Portable toolchain not found at "%TOOLCHAIN_BIN%".
    exit /b 1
)

set "PATH=%TOOLCHAIN_BIN%;%PATH%"

cmake.exe -S "%PROJECT_DIR%" -B "%PROJECT_DIR%\build" -G Ninja ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_C_COMPILER="%TOOLCHAIN_BIN%\gcc.exe"
if errorlevel 1 exit /b %errorlevel%

cmake.exe --build "%PROJECT_DIR%\build"
exit /b %errorlevel%
