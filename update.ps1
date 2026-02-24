$repo = "M-MACHINE/glupe"
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

Write-Host "Fetching latest release information from GitHub..."
try {
    $latestReleaseJson = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest"
    $latestReleaseTag = $latestReleaseJson.tag_name
} catch {
    Write-Error "Error: Could not fetch latest release tag from GitHub. $($_.Exception.Message)"
    exit 1
}

if (-not $latestReleaseTag) {
    Write-Error "Error: Latest release tag is empty."
    exit 1
}

Write-Host "Latest release: $latestReleaseTag"
$downloadUrl = "https://github.com/$repo/releases/download/$latestReleaseTag/$glupeBinName"

Write-Host "Downloading new glupe binary from $downloadUrl to $tempBin..."
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $tempBin -ErrorAction Stop
} catch {
    Write-Error "Error: Failed to download new glupe binary. $($_.Exception.Message)"
    Remove-Item $tempBin -ErrorAction SilentlyContinue
    exit 1
}

if (-not (Test-Path $tempBin)) {
    Write-Error "Error: Downloaded file not found: $tempBin"
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
    Write-Host "Successfully updated glupe to $latestReleaseTag!" -ForegroundColor Green
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