$RepoBaseUrl = "https://raw.githubusercontent.com/M-MACHINE/glupe/main"
$JsonUrl = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
$glupeBinName = "glupe.exe"
$tempBin = Join-Path $env:TEMP "$glupeBinName.new"

Write-Host "--- Glupe Update Script (Windows PowerShell) ---" -ForegroundColor Cyan

# Get the path of the currently running glupe executable
$currentGlupePath = (Get-Command glupe -ErrorAction SilentlyContinue).Source
if (-not $currentGlupePath) {
    Write-Error "Error: 'glupe.exe' command not found in PATH."
    Write-Error "Please ensure glupe is installed and in your PATH."
    exit 1
}
$glupeDir = Split-Path -Path $currentGlupePath -Parent

Write-Host "Current glupe path: $currentGlupePath"

$SrcDir = Join-Path $glupeDir "src"
if (-not (Test-Path $SrcDir)) { New-Item -ItemType Directory -Force -Path $SrcDir | Out-Null }

Write-Host "Downloading source code from $RepoBaseUrl..."
try {
    $SourceFiles = @("glupec.cpp", "common.hpp", "utils.hpp", "config.hpp", "languages.hpp", "ai.hpp", "cache.hpp", "parser.hpp", "processor.hpp", "hub.hpp")
    foreach ($file in $SourceFiles) {
        Invoke-WebRequest -Uri "$RepoBaseUrl/src/$file" -OutFile (Join-Path $SrcDir $file) -ErrorAction Stop
    }
    Invoke-WebRequest -Uri $JsonUrl -OutFile (Join-Path $SrcDir "json.hpp") -ErrorAction Stop
} catch {
    Write-Error "Error: Failed to download source files. $($_.Exception.Message)"
    exit 1
}

Write-Host "Compiling Glupe..."
$BuildCmd = "g++ `"$SrcDir\glupec.cpp`" -o `"$tempBin`" -std=c++17 -static -static-libgcc -static-libstdc++ -lstdc++fs -O3 -I `"$SrcDir`""
try {
    Invoke-Expression $BuildCmd
} catch {
    Write-Error "Error: Compilation failed. $($_.Exception.Message)"
    exit 1
}

if (-not (Test-Path $tempBin)) {
    Write-Error "Error: Compilation failed (Output file not found)."
    exit 1
}

Write-Host "Replacing current glupe executable..."

# Handle file locking: rename the old executable, then move the new one
$oldGlupeBackup = "$currentGlupePath.old"
try {
    if (Test-Path $oldGlupeBackup) { Remove-Item $oldGlupeBackup -Force -ErrorAction SilentlyContinue }
    Rename-Item -Path $currentGlupePath -NewName $oldGlupeBackup -Force -ErrorAction Stop
} catch {
    Write-Error "Error: Could not rename old glupe.exe. It might be in use. Please close all terminals and try again. $($_.Exception.Message)"
    Remove-Item $tempBin -ErrorAction SilentlyContinue
    exit 1
}

try {
    Move-Item -Path $tempBin -Destination $currentGlupePath -Force -ErrorAction Stop
    Write-Host "Successfully updated glupe to the latest version!" -ForegroundColor Green
    Write-Host "The old executable was backed up to '$oldGlupeBackup'."
    Write-Host "Please restart your terminal or shell to ensure the new version is loaded."
    Remove-Item $oldGlupeBackup -ErrorAction SilentlyContinue # Clean up backup if successful
} catch {
    Write-Error "Error: Failed to move new glupe.exe into place. Attempting to restore old executable. $($_.Exception.Message)"
    try {
        Move-Item -Path $oldGlupeBackup -Destination $currentGlupePath -Force -ErrorAction Stop
        Write-Error "Old glupe.exe restored."
    } catch {
        Write-Error "CRITICAL ERROR: Failed to restore old glupe.exe. Your glupe installation might be corrupted. $($_.Exception.Message)"
    }
    Remove-Item $tempBin -ErrorAction SilentlyContinue
    exit 1
}

Write-Host "--- Update Complete ---" -ForegroundColor Cyan