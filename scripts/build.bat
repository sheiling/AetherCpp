@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul 2>&1

:: ============================================================
::  PluginApp 自动构建脚本
::  用法:
::    build.bat               —— 默认 Release
::    build.bat Release       —— Release
::    build.bat Debug         —— Debug
::    build.bat RelWithDebInfo—— RelWithDebInfo
::    build.bat clean         —— 清理 build 目录后重新构建
:: ============================================================

:: ── 脚本自身所在目录 → 推导项目根目录 ──────────────────────
set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%.."
set "PROJECT_ROOT=%CD%"
popd

:: ── 构建配置（默认 Release）──────────────────────────────────
set "BUILD_CONFIG=Release"
set "CLEAN_BUILD="
if /i "%~1"=="Debug"          set "BUILD_CONFIG=Debug"
if /i "%~1"=="Release"        set "BUILD_CONFIG=Release"
if /i "%~1"=="RelWithDebInfo" set "BUILD_CONFIG=RelWithDebInfo"
if /i "%~1"=="clean"          set "CLEAN_BUILD=1" & set "BUILD_CONFIG=Release"
if /i "%~1"=="clean" if /i "%~2"=="Debug"          set "BUILD_CONFIG=Debug"
if /i "%~1"=="clean" if /i "%~2"=="RelWithDebInfo" set "BUILD_CONFIG=RelWithDebInfo"

set "BUILD_DIR=%PROJECT_ROOT%\build"
set "OUTPUT_DIR=%BUILD_DIR%\bin\%BUILD_CONFIG%"

echo.
echo ========================================================
echo   PluginApp  Build Script
echo   Config  : %BUILD_CONFIG%
echo   Root    : %PROJECT_ROOT%
echo   Build   : %BUILD_DIR%
echo   Output  : %OUTPUT_DIR%
echo ========================================================
echo.

:: ── 检测 CMake ───────────────────────────────────────────────
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found. Please install CMake and add it to PATH.
    goto :FAIL
)
for /f "tokens=*" %%v in ('cmake --version 2^>^&1') do (
    echo [INFO]  %%v
    goto :cmake_ver_done
)
:cmake_ver_done

:: ── 自动检测并激活 MSVC 环境 ─────────────────────────────────
:: 若已在 VS Developer Command Prompt 中则跳过
if defined VCToolsVersion (
    echo [INFO]  MSVC environment already active ^(VCToolsVersion=%VCToolsVersion%^).
    goto :msvc_ready
)

:: 先检查 VSINSTALLDIR 环境变量
if defined VSINSTALLDIR (
    set "_VCVARS=%VSINSTALLDIR%VC\Auxiliary\Build\vcvars64.bat"
    if exist "!_VCVARS!" goto :do_activate
)

:: 把带括号的路径存入变量，再在 for 循环中拼接，避免括号解析问题
set "_VS19=C:\Program Files (x86)\Microsoft Visual Studio\2019"
set "_VS22=C:\Program Files\Microsoft Visual Studio\2022"

for %%e in (Enterprise Professional Community BuildTools) do (
    set "_VCVARS=!_VS19!\%%e\VC\Auxiliary\Build\vcvars64.bat"
    if exist "!_VCVARS!" goto :do_activate
)
for %%e in (Enterprise Professional Community BuildTools) do (
    set "_VCVARS=!_VS22!\%%e\VC\Auxiliary\Build\vcvars64.bat"
    if exist "!_VCVARS!" goto :do_activate
)

echo [WARN]  vcvars64.bat not found; assuming MSVC is already in PATH.
goto :msvc_ready

:do_activate
echo [INFO]  Activating MSVC: !_VCVARS!
call "!_VCVARS!"
if errorlevel 1 (
    echo [ERROR] Failed to activate MSVC environment.
    goto :FAIL
)

:msvc_ready

:: ── Qt5 路径 ─────────────────────────────────────────────────
set "QT_ROOT=C:\Qt\5.15.2\msvc2019_64"
set "QT_CMAKE_FLAG="
if exist "%QT_ROOT%\lib\cmake\Qt5\Qt5Config.cmake" (
    set "QT_CMAKE_FLAG=-DQt5_DIR=%QT_ROOT%\lib\cmake\Qt5"
    echo [INFO]  Qt5 found: %QT_ROOT%
) else (
    echo [WARN]  Qt5 not found at %QT_ROOT%, relying on system CMAKE_PREFIX_PATH.
)

:: ── 清理构建目录（clean 模式）───────────────────────────────
if defined CLEAN_BUILD (
    echo [INFO]  Cleaning build directory...
    if exist "%BUILD_DIR%" (
        rd /s /q "%BUILD_DIR%"
        echo [INFO]  Build directory removed.
    )
)

:: ── CMake Configure（CMakeCache.txt 不存在时自动 configure）──
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo.
    echo [STEP 1/2] CMake Configure ...
    echo.
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cmake -S "%PROJECT_ROOT%" ^
          -B "%BUILD_DIR%" ^
          -G "Visual Studio 16 2019" ^
          -A x64 ^
          %QT_CMAKE_FLAG%
    if errorlevel 1 (
        echo.
        echo [ERROR] CMake configure failed.
        goto :FAIL
    )
    echo [INFO]  Configure succeeded.
) else (
    echo [INFO]  CMakeCache.txt exists, skipping configure.
)

:: ── CMake Build ──────────────────────────────────────────────
echo.
echo [STEP 2/2] CMake Build  ^(config=%BUILD_CONFIG%^) ...
echo.

cmake --build "%BUILD_DIR%" --config %BUILD_CONFIG% --parallel
if errorlevel 1 (
    echo.
    echo [ERROR] Build FAILED.
    goto :FAIL
)

:: ── 构建成功 ─────────────────────────────────────────────────
echo.
echo ========================================================
echo   BUILD SUCCEEDED
echo   Config : %BUILD_CONFIG%
echo   Output : %OUTPUT_DIR%
echo ========================================================
echo.

if exist "%OUTPUT_DIR%\myapp.exe"  echo [EXE]     %OUTPUT_DIR%\myapp.exe
if exist "%OUTPUT_DIR%\plugins\"   echo [PLUGINS] %OUTPUT_DIR%\plugins\
if exist "%OUTPUT_DIR%\plugins\" (
    for %%f in ("%OUTPUT_DIR%\plugins\*.dll") do echo            %%~nxf
)
echo.
endlocal
exit /b 0

:FAIL
echo.
echo ========================================================
echo   BUILD FAILED  ^(see errors above^)
echo ========================================================
echo.
endlocal
exit /b 1
