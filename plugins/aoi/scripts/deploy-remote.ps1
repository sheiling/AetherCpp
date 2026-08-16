#Requires -Version 5.1
<#
.SYNOPSIS
  Sync build/bin/<Config> to remote debug machine.

.DESCRIPTION
  Remote path priority:
    1) -RemoteRoot parameter
    2) deploy.local.ps1 ($RemoteRoot)
    3) AETHER_REMOTE_DIR environment variable
    4) VS launch.vs.json (deployDirectory / cwd + remoteMachineName as UNC)

  launch.vs.json search paths:
    <project>/.vs/launch.vs.json
    <project>/build/.vs/launch.vs.json
#>
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Config = 'RelWithDebInfo',
    [string]$RemoteRoot,
    [string[]]$TargetExeNames = @('Aether', 'myapp')
)

$ErrorActionPreference = 'Stop'
$ScriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = (Resolve-Path (Join-Path $ScriptDir '..\..\..')).Path
$LocalRoot   = Join-Path $ProjectRoot "build\bin\$Config"

function Test-UsablePath {
    param([string]$Path)
    return $Path -and $Path -notmatch '\$\{'
}

function Convert-RemotePathToUnc {
    param(
        [string]$RemoteMachine,
        [string]$RemotePath
    )
    if (-not (Test-UsablePath $RemotePath)) { return $null }
    if ($RemotePath -match '^\\\\') { return $RemotePath }

    $machine = $RemoteMachine
    if ($machine -match '^(.+?)\s*\(.*$') { $machine = $Matches[1].Trim() }
    if ($machine -match '^([^:]+):(\d+)$') { $machine = $Matches[1] }
    if (-not $machine) { return $RemotePath }

    if ($RemotePath -match '^([A-Za-z]):\\(.*)$') {
        return ('\\{0}\{1}$\{2}' -f $machine, $Matches[1], $Matches[2])
    }
    return $RemotePath
}

function Get-LaunchVsJsonPaths {
    param([string]$Root)
    @(
        (Join-Path $Root '.vs\launch.vs.json'),
        (Join-Path $Root 'build\.vs\launch.vs.json')
    )
}

function Select-RemoteDebugConfiguration {
    param(
        $Configurations,
        [string[]]$ExeNames
    )
    if (-not $Configurations) { return $null }

    $list = @($Configurations)
    foreach ($exe in $ExeNames) {
        $hit = $list | Where-Object {
            ($_.projectTarget -like "*$exe*") -or ($_.name -like "*$exe*")
        } | Select-Object -First 1
        if ($hit) { return $hit }
    }

    $remote = $list | Where-Object {
        $_.type -eq 'remoteWindows' -or (Test-UsablePath $_.remoteMachineName)
    } | Select-Object -First 1
    if ($remote) { return $remote }

    return $list | Select-Object -First 1
}

function Get-RemoteRootFromLaunchVs {
    param(
        [string]$Root,
        [string[]]$ExeNames
    )
    foreach ($path in (Get-LaunchVsJsonPaths $Root)) {
        if (-not (Test-Path $path)) { continue }

        $doc = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
        $configs = @()
        if ($doc.configurations) { $configs += $doc.configurations }

        $cfg = Select-RemoteDebugConfiguration -Configurations $configs -ExeNames $ExeNames
        if (-not $cfg) { continue }

        $remotePath = $null
        foreach ($key in @('deployDirectory', 'cwd', 'remoteDeployDirectory')) {
            $value = $cfg.$key
            if (Test-UsablePath $value) {
                $remotePath = $value
                break
            }
        }
        if (-not $remotePath) { continue }

        $unc = Convert-RemotePathToUnc -RemoteMachine $cfg.remoteMachineName -RemotePath $remotePath
        if ($unc) {
            return @{
                RemoteRoot = $unc
                LaunchFile = $path
                ConfigName = $cfg.name
            }
        }
    }
    return $null
}

$LocalConfig = Join-Path $ScriptDir 'deploy.local.ps1'
if (Test-Path $LocalConfig) {
    . $LocalConfig
}

if (-not $RemoteRoot) {
    $RemoteRoot = $env:AETHER_REMOTE_DIR
}

$LaunchInfo = $null
if (-not $RemoteRoot) {
    $LaunchInfo = Get-RemoteRootFromLaunchVs -Root $ProjectRoot -ExeNames $TargetExeNames
    if ($LaunchInfo) {
        $RemoteRoot = $LaunchInfo.RemoteRoot
    }
}

if (-not $RemoteRoot) {
    Write-Error 'Remote deploy path not found. Set launch.vs.json, deploy.local.ps1, AETHER_REMOTE_DIR, or -RemoteRoot.'
}

if (-not (Test-Path $LocalRoot)) {
    Write-Error ('Local build output missing: {0}. Build {1} first.' -f $LocalRoot, $Config)
}

function Sync-Dir {
    param(
        [string]$RelativePath,
        [string[]]$Include = @('*.dll', '*.exe', '*.pdb')
    )
    $src = if ($RelativePath) { Join-Path $LocalRoot $RelativePath } else { $LocalRoot }
    $dst = if ($RelativePath) { Join-Path $RemoteRoot $RelativePath } else { $RemoteRoot }
    if (-not (Test-Path $src)) { return }

    $null = New-Item -ItemType Directory -Force -Path $dst
    & robocopy $src $dst @Include /XO /R:1 /W:1 /NFL /NDL /NJH /NJS | Out-Null
    $code = $LASTEXITCODE
    if ($code -ge 8) {
        throw ('robocopy failed ({0}): exit {1}' -f $RelativePath, $code)
    }
    $label = if ($RelativePath) { $RelativePath } else { '(root)' }
    Write-Host "[OK] $label -> $dst"
}

Write-Host "Deploy $Config"
Write-Host "  From: $LocalRoot"
Write-Host "  To  : $RemoteRoot"
if ($LaunchInfo) {
    Write-Host ('  Via : {0} ({1})' -f $LaunchInfo.LaunchFile, $LaunchInfo.ConfigName)
}
Write-Host ''

Sync-Dir -RelativePath '' -Include @('*.dll', '*.exe', '*.pdb')
Sync-Dir -RelativePath 'plugins' -Include @('*.dll', '*.pdb')
Sync-Dir -RelativePath 'config\i18n' -Include @('*.qm')

Write-Host ''
Write-Host 'Deploy finished.'
