@echo off
setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%\.."

set "ROOT_DIR=%CD%"
set "BUILD_DIR=%ROOT_DIR%\build"
set "THIRDPARTY_DIR=%ROOT_DIR%\Engine\ThirdParty"
set "CONAN_PROFILE=Automation\conanProfileDebug"
set "CMAKE_GENERATOR=Visual Studio 17 2022"
set "ARCHITECTURE=x64"

echo.
echo [LampyEngine] Generating project files...

call :ensure_tool conan || goto :error
call :ensure_tool cmake || goto :error
call :detect_python || goto :error
call :ensure_python_package PySide6 || goto :error
call :ensure_python_package pyinstaller || goto :error

echo.
echo [Step] Installing dependencies with Conan...
conan install . --output-folder="%THIRDPARTY_DIR%" --build=missing ^
    --profile="%CONAN_PROFILE%" --profile:b="%CONAN_PROFILE%"
if errorlevel 1 goto :error

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo.
echo [Step] Configuring CMake project...
cmake -S . -B "%BUILD_DIR%" -G "%CMAKE_GENERATOR%" -A %ARCHITECTURE% ^
    -DCMAKE_TOOLCHAIN_FILE="%THIRDPARTY_DIR%\conan_toolchain.cmake" --fresh
if errorlevel 1 goto :error

echo.
echo [Success] Project files generated at "%BUILD_DIR%\LampyEngine.sln".
goto :end

:ensure_tool
where "%~1" >nul 2>&1
if errorlevel 1 (
    echo [Error] Required tool '%~1' not found in PATH.
    exit /b 1
)
exit /b 0

:detect_python
set "PYTHON_CMD="
py -3 --version >nul 2>&1
if %errorlevel%==0 (
    set "PYTHON_CMD=py -3"
) else (
    python --version >nul 2>&1
    if %errorlevel%==0 (
        set "PYTHON_CMD=python"
    )
)

if not defined PYTHON_CMD (
    echo [Error] Python 3.9+ is required but was not found.
    exit /b 1
)
echo [Info] Using Python command: %PYTHON_CMD%
exit /b 0

:ensure_python_package
set "PACKAGE=%~1"
%PYTHON_CMD% -m pip show %PACKAGE% >nul 2>&1
if errorlevel 1 (
    echo [Info] Installing Python package %PACKAGE%...
    %PYTHON_CMD% -m pip install %PACKAGE%
    if errorlevel 1 (
        echo [Error] Failed to install Python package %PACKAGE%.
        exit /b 1
    )
) else (
    echo [Info] Python package %PACKAGE% already installed.
)
exit /b 0

:error
echo.
echo [Fail] Generation aborted.
exit /b 1

:end
popd
endlocal
exit /b 0

