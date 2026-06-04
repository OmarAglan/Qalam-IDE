param(
    [string]$QtRoot = $env:QALAM_QT_DIR,
    [string]$BuildDir = 'build/windows-release',
    [string]$PackageDir = 'dist/Qalam-win64',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-Location (Split-Path -Parent $PSScriptRoot)

if (!$SkipBuild) {
    & (Join-Path $PSScriptRoot 'build-windows.ps1') -Configuration Release -QtRoot $QtRoot
}

$exe = Join-Path $BuildDir 'qalam/Qalam.exe'
if (!(Test-Path $exe)) {
    throw "Qalam.exe was not found at $exe. Build the project first."
}

if (Test-Path $PackageDir) { Remove-Item $PackageDir -Recurse -Force }
New-Item -ItemType Directory -Path $PackageDir | Out-Null
Copy-Item $exe $PackageDir

if (!$QtRoot) { $QtRoot = $env:QALAM_QT_DIR }
if (!$QtRoot -and $env:QTDIR) { $QtRoot = $env:QTDIR }
if (!$QtRoot -and (Test-Path 'C:/Qt')) {
    $QtRoot = Get-ChildItem 'C:/Qt' -Directory |
        Sort-Object Name -Descending |
        ForEach-Object { Join-Path $_.FullName 'mingw_64' } |
        Where-Object { Test-Path (Join-Path $_ 'bin/windeployqt.exe') } |
        Select-Object -First 1
}

$windeployqt = if ($QtRoot) { Join-Path $QtRoot 'bin/windeployqt.exe' } else { 'windeployqt.exe' }
if (!(Get-Command $windeployqt -ErrorAction SilentlyContinue)) {
    throw 'windeployqt.exe was not found. Set QALAM_QT_DIR to your Qt MinGW kit path.'
}

& $windeployqt --compiler-runtime --dir $PackageDir (Join-Path $PackageDir 'Qalam.exe')

# Optional: bundle the Baa compiler if the repository contains a local compiler folder.
if (Test-Path 'baa') {
    Copy-Item 'baa' (Join-Path $PackageDir 'baa') -Recurse -Force
}

$zipPath = 'dist/Qalam-win64.zip'
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path (Join-Path $PackageDir '*') -DestinationPath $zipPath

Write-Host "Packaged Qalam successfully:" -ForegroundColor Green
Write-Host "  $zipPath"
