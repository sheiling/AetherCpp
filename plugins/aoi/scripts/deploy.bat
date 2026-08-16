@echo off
setlocal
chcp 65001 >nul 2>&1

set "SCRIPT_DIR=%~dp0"
set "CONFIG=RelWithDebInfo"
if /i "%~1"=="Debug"          set "CONFIG=Debug"
if /i "%~1"=="Release"        set "CONFIG=Release"
if /i "%~1"=="RelWithDebInfo" set "CONFIG=RelWithDebInfo"

REM 默认远程运行目录（与 build\bin\<Config> 结构一致）；改 IP/路径时只改下面一行
set "REMOTE_ROOT=\\192.168.5.222\aether\bin"

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%deploy-remote.ps1" -Config %CONFIG% -RemoteRoot "%REMOTE_ROOT%"
exit /b %ERRORLEVEL%
