# Glupe Installer for Windows
# Usage: irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex

$ErrorActionPreference = "Stop"
$RepoUrl = "https://raw.githubusercontent.com/alonsovm44/glupe/master"
$JsonUrl = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
$InstallDir = "$env:USERPROFILE\.glupe"
$ExePath = "$InstallDir\glupe.exe"

Write-Host "--- Glupe Installer ---" -ForegroundColor Cyan

# 1. Check for G++
$MinGwBin = ""
try {
    $gpp = Get-Command g++ -ErrorAction Stop
    Write-Host "[OK] G++ found: $($gpp.Source)" -ForegroundColor Green
} catch {
    Write-Host "[INFO] G++ not found. Installing portable MinGW (w64devkit)..." -ForegroundColor Yellow
    
    if (-not (Test-Path $InstallDir)) { New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null }
    
    $MinGwUrl = "https://github.com/skeeto/w64devkit/releases/download/v1.21.0/w64devkit-1.21.0.zip"
    $ZipPath = "$InstallDir\mingw.zip"
    
    try {
        Invoke-WebRequest -Uri $MinGwUrl -OutFile $ZipPath
    } catch {
        Write-Host "[ERROR] Failed to download MinGW. Please install manually." -ForegroundColor Red; exit 1
    }
    
    Write-Host "Extracting MinGW..."
    Expand-Archive -Path $ZipPath -DestinationPath $InstallDir -Force
    Remove-Item $ZipPath
    
    $MinGwBin = "$InstallDir\w64devkit\bin"
    $env:Path = "$MinGwBin;$env:Path" # Update session PATH
    Write-Host "[OK] MinGW installed to $MinGwBin" -ForegroundColor Green
}

# 1.5 Check for Ollama
try {
    Get-Command ollama -ErrorAction Stop | Out-Null
    Write-Host "[OK] Ollama found." -ForegroundColor Green
} catch {
    Write-Host "[INFO] Ollama (Local AI) not found." -ForegroundColor Yellow
    $ans = Read-Host "Do you want to install Ollama? [Y/n]"
    if ($ans -eq "" -or $ans -match "^[Yy]") {
        Write-Host "Downloading Ollama installer..."
        $OllamaExe = "$env:TEMP\OllamaSetup.exe"
        Invoke-WebRequest -Uri "https://ollama.com/download/OllamaSetup.exe" -OutFile $OllamaExe
        Write-Host "Running Ollama installer..."
        Start-Process -FilePath $OllamaExe -Wait
        Remove-Item $OllamaExe -ErrorAction SilentlyContinue
        Write-Host "[OK] Ollama installed." -ForegroundColor Green
    }
}

# 2. Create Directory
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
    Write-Host "[OK] Created installation directory: $InstallDir" -ForegroundColor Green
}

# 3. Download Source
Write-Host "[INFO] Downloading source code..."
try {
    # Download main source
    Invoke-WebRequest -Uri "$RepoUrl/glupec.cpp" -OutFile "$InstallDir\glupec.cpp"
    # Download JSON dependency
    Invoke-WebRequest -Uri $JsonUrl -OutFile "$InstallDir\json.hpp"
} catch {
    Write-Host "[ERROR] Failed to download source files." -ForegroundColor Red
    Write-Host "Ensure you have internet connection and the repository URL is correct."
    Write-Host $_
    exit 1
}

# 4. Compile
Write-Host "[INFO] Compiling Glupe..."
$BuildCmd = "g++ `"$InstallDir\glupec.cpp`" -o `"$ExePath`" -std=c++17 -static-libgcc -static-libstdc++ -lstdc++fs -O3"
Invoke-Expression $BuildCmd

if (-not (Test-Path $ExePath)) {
    Write-Host "[ERROR] Compilation failed." -ForegroundColor Red
    exit 1
}
Write-Host "[OK] Compilation successful." -ForegroundColor Green

# 5. Create Config
$ConfigPath = "$InstallDir\config.json"
if (-not (Test-Path $ConfigPath)) {
    $ConfigContent = @{
        local = @{
            model_id = "qwen2.5-coder:latest"
            api_url = "http://localhost:11434/api/generate"
        }
        cloud = @{
            protocol = "openai"
            api_key = ""
            model_id = "gpt-4o"
            api_url = "https://api.openai.com/v1/chat/completions"
        }
        max_retries = 15
    } | ConvertTo-Json -Depth 4
    Set-Content -Path $ConfigPath -Value $ConfigContent
    Write-Host "[OK] Created default config.json" -ForegroundColor Green
}

# 6. Add to PATH
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
$NewPath = $UserPath
$UpdatePath = $false

if ($UserPath -notlike "*$InstallDir*") { $NewPath += ";$InstallDir"; $UpdatePath = $true }
if ($MinGwBin -ne "" -and $UserPath -notlike "*$MinGwBin*") { $NewPath += ";$MinGwBin"; $UpdatePath = $true }

if ($UpdatePath) {
    Write-Host "[INFO] Adding Glupe to PATH..."
    [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
    Write-Host "[OK] Added to PATH. Please restart your terminal to use 'glupe'." -ForegroundColor Yellow
} else {
    Write-Host "[OK] Already in PATH." -ForegroundColor Green
}

# 7. Cleanup
Remove-Item "$InstallDir\glupec.cpp" -ErrorAction SilentlyContinue
Remove-Item "$InstallDir\json.hpp" -ErrorAction SilentlyContinue

Write-Host "`n[SUCCESS] Glupe installed successfully!" -ForegroundColor Cyan
Write-Host "Run 'glupe --help' to get started."