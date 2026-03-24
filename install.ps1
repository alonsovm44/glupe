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
    Start-Sleep -Seconds 1
    $ans = Read-Host "Do you want to install Ollama? [y/N]"
    if ($ans -match "^[Yy]") {
        Write-Host "Downloading Ollama installer..."
        $OllamaExe = "$env:TEMP\OllamaSetup.exe"
        try {
            Invoke-WebRequest -Uri "https://ollama.com/download/OllamaSetup.exe" -OutFile $OllamaExe
            Write-Host "Running Ollama installer..."
            Start-Process -FilePath $OllamaExe
            Write-Host "[OK] Ollama installer launched. Please complete the setup." -ForegroundColor Green
        } catch {
            Write-Host "[WARN] Could not install Ollama automatically. Please install manually." -ForegroundColor Yellow
        }
    }
}

# 2. Create Directory
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
    Write-Host "[OK] Created installation directory: $InstallDir" -ForegroundColor Green
}

# Create src directory
$SrcDir = "$InstallDir\src"
if (-not (Test-Path $SrcDir)) {
    New-Item -ItemType Directory -Force -Path $SrcDir | Out-Null
}

# 3. Download Source
Write-Host "[INFO] Downloading source code..."
try {
    $SourceFiles = @("glupec.cpp", "common.hpp", "utils.hpp", "config.hpp", "languages.hpp", "ai.hpp", "cache.hpp", "parser.hpp", "processor.hpp", "hub.hpp", "ast.hpp", "ast_utils.hpp", "glupe.l", "glupe.y")
    foreach ($file in $SourceFiles) {
        Invoke-WebRequest -Uri "$RepoUrl/src/$file" -OutFile "$SrcDir\$file"
    }
    Invoke-WebRequest -Uri $JsonUrl -OutFile "$SrcDir\json.hpp"
} catch {
    Write-Host "[ERROR] Failed to download source files." -ForegroundColor Red
    Write-Host "Ensure you have internet connection and the repository URL is correct."
    Write-Host $_
    exit 1
}

# 3.5 Generate Parser and Lexer
Write-Host "[INFO] Generating parser and lexer with Bison and Flex..."
try {
    Invoke-Expression "bison -d -o `"$SrcDir\glupe.tab.c`" `"$SrcDir\glupe.y`""
    Invoke-Expression "flex -o `"$SrcDir\lex.yy.c`" `"$SrcDir\glupe.l`""
} catch {
    Write-Host "[WARN] Parser/Lexer generation failed. Compilation might fail if C files are missing." -ForegroundColor Yellow
}

# 3.8 Download and Build Tree-sitter
Write-Host "[INFO] Downloading and Building Tree-sitter libraries..."
$VendorDir = "$InstallDir\vendor"
if (-not (Test-Path $VendorDir)) { New-Item -ItemType Directory -Force -Path $VendorDir | Out-Null }

if (-not (Test-Path "$VendorDir\tree-sitter")) {
    $tsZip = "$VendorDir\tree-sitter.zip"
    Invoke-WebRequest -Uri "https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.zip" -OutFile $tsZip
    Expand-Archive -Path $tsZip -DestinationPath $VendorDir -Force
    Rename-Item "$VendorDir\tree-sitter-0.22.6" -NewName "tree-sitter"
    Remove-Item $tsZip
}
if (-not (Test-Path "$VendorDir\tree-sitter-cpp")) {
    $tsCppZip = "$VendorDir\tree-sitter-cpp.zip"
    Invoke-WebRequest -Uri "https://github.com/tree-sitter/tree-sitter-cpp/archive/refs/tags/v0.22.0.zip" -OutFile $tsCppZip
    Expand-Archive -Path $tsCppZip -DestinationPath $VendorDir -Force
    Rename-Item "$VendorDir\tree-sitter-cpp-0.22.0" -NewName "tree-sitter-cpp"
    Remove-Item $tsCppZip
}

Invoke-Expression "gcc -O3 -I`"$VendorDir\tree-sitter\lib\include`" -I`"$VendorDir\tree-sitter\lib\src`" -c `"$VendorDir\tree-sitter\lib\src\lib.c`" -o `"$VendorDir\tree-sitter.o`""
Invoke-Expression "gcc -O3 -I`"$VendorDir\tree-sitter-cpp\src`" -c `"$VendorDir\tree-sitter-cpp\src\parser.c`" -o `"$VendorDir\parser.o`""
if (Test-Path "$VendorDir\tree-sitter-cpp\src\scanner.c") {
    Invoke-Expression "gcc -O3 -I`"$VendorDir\tree-sitter-cpp\src`" -c `"$VendorDir\tree-sitter-cpp\src\scanner.c`" -o `"$VendorDir\scanner.o`""
} elseif (Test-Path "$VendorDir\tree-sitter-cpp\src\scanner.cc") {
    Invoke-Expression "g++ -O3 -I`"$VendorDir\tree-sitter-cpp\src`" -c `"$VendorDir\tree-sitter-cpp\src\scanner.cc`" -o `"$VendorDir\scanner.o`""
}

# 4. Compile
Write-Host "[INFO] Compiling Glupe..."
# [FIX] Added -static to ensure the exe runs on any machine without DLLs
 $BuildCmd = "g++ `"$SrcDir\glupec.cpp`" `"$SrcDir\lex.yy.c`" `"$SrcDir\glupe.tab.c`" `"$VendorDir\tree-sitter.o`" `"$VendorDir\parser.o`" `"$VendorDir\scanner.o`" -o `"$ExePath`" -std=c++17 -static -static-libgcc -static-libstdc++ -lstdc++fs -O3 -I `"$SrcDir`" -I `"$VendorDir\tree-sitter\lib\include`""
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
# We generally don't need to add MinGW to permanent PATH for the user, only for this session.
# Keeping the user's PATH clean is better. Glupe is now static, so it doesn't need it.

if ($UpdatePath) {
    Write-Host "[INFO] Adding Glupe to PATH..."
    [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
    Write-Host "[OK] Added to PATH. Please restart your terminal to use 'glupe'." -ForegroundColor Yellow
} else {
    Write-Host "[OK] Already in PATH." -ForegroundColor Green
}

# 7. Cleanup
# [FIX] We KEEP glupec.cpp and json.hpp so users can recompile manually if they want, 
# or if an update feature is added later.
# Only remove the downloaded zip if it exists.
if (Test-Path "$InstallDir\mingw.zip") { Remove-Item "$InstallDir\mingw.zip" }

Write-Host "`n[SUCCESS] Glupe installed successfully!" -ForegroundColor Cyan
Write-Host "Run 'glupe --help' to get started."