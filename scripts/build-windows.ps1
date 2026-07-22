param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$QtRoot = $env:QALAM_QT_DIR,

    [string]$BuildDir = '',

    [switch]$DeployAfterBuild,

    [switch]$BuildTests
)

$ErrorActionPreference = 'Stop'
Set-Location (Split-Path -Parent $PSScriptRoot)

if (!$BuildDir) {
    $BuildDir = "build/windows-$($Configuration.ToLowerInvariant())"
}

function Invoke-Native {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    & $FilePath @Arguments
    $exitCode = $LASTEXITCODE
    if ($null -eq $exitCode) { $exitCode = 0 }
    if ($exitCode -ne 0) {
        throw "$FilePath failed with exit code $exitCode."
    }
}

function Resolve-QtRoot {
    param([string]$ProvidedRoot)

    $candidates = @()
    if ($ProvidedRoot) { $candidates += $ProvidedRoot }
    if ($env:QTDIR) { $candidates += $env:QTDIR }
    if (Test-Path 'C:/Qt') {
        $candidates += Get-ChildItem 'C:/Qt' -Directory |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName 'mingw_64' }
    }

    foreach ($candidate in $candidates) {
        if (!$candidate) { continue }
        $cmakeConfig = Join-Path $candidate 'lib/cmake/Qt6/Qt6Config.cmake'
        if (Test-Path $cmakeConfig) { return (Resolve-Path $candidate).Path }
    }

    throw 'Qt 6 MinGW kit was not found. Install Qt 6 with MinGW, or set QALAM_QT_DIR to something like C:\Qt\6.10.2\mingw_64.'
}

function Resolve-MingwBin {
    param([string]$ResolvedQtRoot)

    $qtParent = Split-Path -Parent $ResolvedQtRoot
    $qtInstall = Split-Path -Parent $qtParent
    $toolCandidates = @()

    if (Test-Path (Join-Path $qtInstall 'Tools')) {
        $toolCandidates += Get-ChildItem (Join-Path $qtInstall 'Tools') -Directory -Filter 'mingw*_64' |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName 'bin' }
    }

    foreach ($candidate in $toolCandidates) {
        if (Test-Path (Join-Path $candidate 'g++.exe')) { return (Resolve-Path $candidate).Path }
    }

    $pathGxx = Get-Command g++.exe -ErrorAction SilentlyContinue
    if ($pathGxx) { return (Split-Path -Parent $pathGxx.Source) }

    throw 'MinGW g++.exe was not found. Install the MinGW kit from Qt Maintenance Tool, or add MinGW bin to PATH.'
}

$QtRoot = Resolve-QtRoot -ProvidedRoot $QtRoot
$MingwBin = Resolve-MingwBin -ResolvedQtRoot $QtRoot
$MakeProgram = Join-Path $MingwBin 'mingw32-make.exe'
$Gxx = Join-Path $MingwBin 'g++.exe'

$env:PATH = "$MingwBin;$QtRoot/bin;$env:PATH"

$deployFlag = if ($DeployAfterBuild) { 'ON' } else { 'OFF' }
$testsFlag = if ($BuildTests) { 'ON' } else { 'OFF' }

Invoke-Native -FilePath 'cmake' -Arguments @(
    '-S', '.',
    '-B', $BuildDir,
    '-G', 'MinGW Makefiles',
    "-DCMAKE_PREFIX_PATH=$QtRoot",
    "-DCMAKE_CXX_COMPILER=$Gxx",
    "-DCMAKE_MAKE_PROGRAM=$MakeProgram",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    '-DCMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders=TRUE',
    "-DQALAM_DEPLOY_AFTER_BUILD=$deployFlag",
    "-DQALAM_BUILD_TESTS=$testsFlag"
)

Invoke-Native -FilePath 'cmake' -Arguments @('--build', $BuildDir, '--target', 'Qalam', '--parallel')

if ($BuildTests) {
    # Build the complete configured test graph so newly registered CTest targets
    # cannot be skipped by a stale hard-coded executable list.
    Invoke-Native -FilePath 'cmake' -Arguments @('--build', $BuildDir, '--parallel')
    Invoke-Native -FilePath 'ctest' -Arguments @('--test-dir', $BuildDir, '--output-on-failure')
}

Write-Host "Built Qalam successfully:" -ForegroundColor Green
Write-Host "  $BuildDir/qalam/Qalam.exe"
