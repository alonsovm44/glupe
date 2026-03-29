<#
name: install.ps1
purpose: Improved Glupe installer for Windows (portable & more robust)
usage: irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
#>

[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

$RepoRaw = "https://raw.githubusercontent.com/alonsovm44/glupe/master"
$InstallDir = Join-Path $env:USERPROFILE ".glupe"
$SrcDir = Join-Path $InstallDir "src"
$VendorDir = Join-Path $InstallDir "vendor"
$ExePath = Join-Path $InstallDir "glupe.exe"
$JsonUrl = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
$w64devkitUrl = "https://github.com/skeeto/w64devkit/releases/download/v1.21.0/w64devkit-1.21.0.zip"

function Write-Info { param($m) Write-Host "[INFO] $m" -ForegroundColor Cyan }
function Write-Ok   { param($m) Write-Host "[OK]   $m" -ForegroundColor Green }
function Write-Warn { param($m) Write-Host "[WARN] $m" -ForegroundColor Yellow }
function Write-Err  { param($m) Write-Host "[ERROR] $m" -ForegroundColor Red; exit 1 }

Write-Host "--- Glupe Installer (Windows) ---" -ForegroundColor Cyan
Write-Info "Install dir: $InstallDir"

# Ensure directories exist
New-Item -ItemType Directory -Force -Path $SrcDir | Out-Null
New-Item -ItemType Directory -Force -Path $VendorDir | Out-Null
Write-Ok "Created $InstallDir and subdirectories"

# Helper to run commands and surface exit code
function Run-Command {
    param(
        [Parameter(Mandatory=$true)] [string] $File,
        [string[]] $Args = @(),
        [switch] $Wait = $true
    )
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $File
    $psi.Arguments = ($Args -join ' ')
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.UseShellExecute = $false
    $p = New-Object System.Diagnostics.Process
    $p.StartInfo = $psi
    $p.Start() | Out-Null
    if ($Wait) {
        $stdout = $p.StandardOutput.ReadToEnd()
        $stderr = $p.StandardError.ReadToEnd()
        $p.WaitForExit()
        return [pscustomobject]@{ ExitCode = $p.ExitCode; Stdout = $stdout; Stderr = $stderr }
    } else {
        return $null
    }
}

# 1) Find a C++ toolchain
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
$clangpp = Get-Command clang++ -ErrorAction SilentlyContinue
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue

$Compiler = $null
if ($gpp) { $Compiler = "g++"; Write-Ok "Found g++ at $($gpp.Source)" }
elseif ($clangpp) { $Compiler = "clang++"; Write-Ok "Found clang++ at $($clangpp.Source)" }
elseif ($cl) { $Compiler = "cl"; Write-Ok "Found MSVC (cl.exe)" }

if (-not $Compiler) {
    Write-Warn "No C++ compiler found in PATH."
    $installDevkit = Read-Host "Download a portable MinGW (w64devkit) and use it for building? [Y/n]"
    if (($installDevkit -eq "") -or ($installDevkit -match "^[Yy]")) {
        $zipPath = Join-Path $InstallDir "w64devkit.zip"
        Write-Info "Downloading w64devkit..."
        try {
            Invoke-WebRequest -Uri $w64devkitUrl -OutFile $zipPath -UseBasicParsing
            Write-Info "Extracting w64devkit..."
            Expand-Archive -Path $zipPath -DestinationPath $InstallDir -Force
            Remove-Item $zipPath -Force -ErrorAction SilentlyContinue
            # w64devkit folder name may vary; find bin
            $devkitDir = Get-ChildItem -Path $InstallDir -Directory | Where-Object { $_.Name -like "w64devkit*" } | Select-Object -First 1
            if ($devkitDir) {
                $devkitBin = Join-Path $devkitDir.FullName "bin"
                $env:Path = "$devkitBin;$env:Path"
                Write-Ok "w64devkit available and added to PATH for this session ($devkitBin)."
                $gpp = Get-Command g++ -ErrorAction SilentlyContinue
                if ($gpp) { $Compiler = "g++"; Write-Ok "Using g++ from w64devkit: $($gpp.Source)" }
            } else {
                Write-Warn "Could not find extracted w64devkit folder; please extract manually."
            }
        } catch {
            Write-Err "Failed to download/extract w64devkit: $_"
        }
    } else {
        Write-Err "Aborting. Please install Visual Studio Build Tools, MSYS2, or w64devkit and re-run."
    }
}

if (-not $Compiler) { Write-Err "No usable compiler found. Exiting." }

# 1.5) Check for Ollama (optional)
$ollama = Get-Command ollama -ErrorAction SilentlyContinue
if ($ollama) { Write-Ok "Ollama found: $($ollama.Source)" } else {
    Write-Warn "Ollama not found. Skipping automatic install. If you want a local LLM, install Ollama separately."
    # do not auto-run installers on Windows silently
}

# 2) Download source files
Write-Info "Downloading source files..."
$sourceFiles = @(
    "glupec.cpp","common.hpp","utils.hpp","config.hpp","languages.hpp",
    "ai.hpp","cache.hpp","parser.hpp","processor.hpp","hub.hpp","ast.hpp",
    "ast_utils.hpp","glupe.l","glupe.y"
)
foreach ($f in $sourceFiles) {
    $url = "$RepoRaw/src/$f"
    $dest = Join-Path $SrcDir $f
    try {
        Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing
        Write-Ok "Fetched $f"
    } catch {
        Write-Warn "Could not download $f (maybe file missing in repo). Continuing; build may fail."
    }
}
# json.hpp
try { Invoke-WebRequest -Uri $JsonUrl -OutFile (Join-Path $SrcDir "json.hpp") -UseBasicParsing; Write-Ok "Fetched json.hpp" } catch { Write-Warn "Failed to fetch json.hpp" }

# 3) Generate parser/lexer if bison/flex available
$bison = Get-Command bison -ErrorAction SilentlyContinue
$flex = Get-Command flex -ErrorAction SilentlyContinue
$genOk = $false
if ($bison -and $flex) {
    try {
        Write-Info "Generating parser/lexer with bison/flex..."
        & bison -d -o (Join-Path $SrcDir "glupe.tab.c") (Join-Path $SrcDir "glupe.y")
        & flex -o (Join-Path $SrcDir "lex.yy.c") (Join-Path $SrcDir "glupe.l")
        Write-Ok "Generated glupe.tab.c and lex.yy.c"
        $genOk = $true
    } catch {
        Write-Warn "bison/flex generation failed: $_"
    }
} else {
    Write-Warn "bison/flex not found in PATH. Attempting to download pre-generated parser/lexer from repo."
}

if (-not $genOk) {
    try {
        Invoke-WebRequest -Uri "$RepoRaw/src/glupe.tab.c" -OutFile (Join-Path $SrcDir "glupe.tab.c") -UseBasicParsing
        Invoke-WebRequest -Uri "$RepoRaw/src/lex.yy.c" -OutFile (Join-Path $SrcDir "lex.yy.c") -UseBasicParsing
        Write-Ok "Downloaded pre-generated parser/lexer (if available)."
    } catch {
        Write-Warn "Pre-generated parser/lexer not available; compilation may fail."
    }
}

# 4) Download tree-sitter sources (best-effort)
try {
    $tsUrl = "https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.zip"
    $tsZip = Join-Path $VendorDir "tree-sitter.zip"
    if (-not (Test-Path (Join-Path $VendorDir "tree-sitter"))) {
        Write-Info "Downloading tree-sitter (0.22.6)..."
        Invoke-WebRequest -Uri $tsUrl -OutFile $tsZip -UseBasicParsing
        Expand-Archive -Path $tsZip -DestinationPath $VendorDir -Force
        Remove-Item $tsZip -Force
        Get-ChildItem $VendorDir -Directory | Where-Object { $_.Name -like "tree-sitter-*" } | ForEach-Object { Rename-Item $_.FullName -NewName "tree-sitter" -Force }
    }
    
    $tsLangs = [ordered]@{ "cpp"="0.22.0"; "python"="0.21.0"; "javascript"="0.21.2"; "java"="0.21.0"; "go"="0.21.2"; "rust"="0.21.2" }
    foreach ($lang in $tsLangs.Keys) {
        $ver = $tsLangs[$lang]
        $langDir = "tree-sitter-$lang"
        if (-not (Test-Path (Join-Path $VendorDir $langDir))) {
            Write-Info "Downloading $langDir ($ver)..."
            $langUrl = "https://github.com/tree-sitter/$langDir/archive/refs/tags/v$ver.zip"
            $langZip = Join-Path $VendorDir "$langDir.zip"
            Invoke-WebRequest -Uri $langUrl -OutFile $langZip -UseBasicParsing
            Expand-Archive -Path $langZip -DestinationPath $VendorDir -Force
            Remove-Item $langZip -Force
            Get-ChildItem $VendorDir -Directory | Where-Object { $_.Name -like "$langDir-*" } | ForEach-Object { Rename-Item $_.FullName -NewName $langDir -Force }
        }
    }
    Write-Ok "Fetched tree-sitter sources (best-effort)."
} catch {
    Write-Warn "Failed to fetch/build tree-sitter. Build may still work without it depending on code paths."
}

# 5) Build (best-effort). Use g++/clang++ when available; if MSVC, try cl (simple path).
Write-Info "Compiling Glupe..."
$srcFilesToPass = @(
    (Join-Path $SrcDir "glupec.cpp"),
    (Join-Path $SrcDir "lex.yy.c"),
    (Join-Path $SrcDir "glupe.tab.c")
) -join " "

# Collect object files for tree-sitter if present
$tsObjs = @()
$tsTree = Join-Path $VendorDir "tree-sitter"
if (Test-Path (Join-Path $tsTree "lib/src/lib.c")) {
    $tsObj = Join-Path $VendorDir "tree-sitter.o"
    & $env:COMSPEC /c "gcc -O3 -I`"$tsTree\lib\include`" -I`"$tsTree\lib\src`" -c `"$tsTree\lib\src\lib.c`" -o `"$tsObj`"" 2>&1 | Out-Null
    if (Test-Path $tsObj) { $tsObjs += "`"$tsObj`"" }
}
$langs = @("cpp", "python", "javascript", "java", "go", "rust")
foreach ($lang in $langs) {
    $langDir = Join-Path $VendorDir "tree-sitter-$lang"
    $srcDirTS = Join-Path $langDir "src"
    if (Test-Path (Join-Path $srcDirTS "parser.c")) {
        $pObj = Join-Path $VendorDir "${lang}_parser.o"
        & $env:COMSPEC /c "gcc -O3 -I`"$srcDirTS`" -c `"$srcDirTS\parser.c`" -o `"$pObj`"" 2>&1 | Out-Null
        if (Test-Path $pObj) { $tsObjs += "`"$pObj`"" }
    }
    if (Test-Path (Join-Path $srcDirTS "scanner.c")) {
        $sObj = Join-Path $VendorDir "${lang}_scanner.o"
        & $env:COMSPEC /c "gcc -O3 -I`"$srcDirTS`" -c `"$srcDirTS\scanner.c`" -o `"$sObj`"" 2>&1 | Out-Null
        if (Test-Path $sObj) { $tsObjs += "`"$sObj`"" }
    } elseif (Test-Path (Join-Path $srcDirTS "scanner.cc")) {
        $sObj = Join-Path $VendorDir "${lang}_scanner.o"
        $cxx = if ($Compiler) { $Compiler } else { "g++" }
        & $env:COMSPEC /c "$cxx -O3 -I`"$srcDirTS`" -c `"$srcDirTS\scanner.cc`" -o `"$sObj`"" 2>&1 | Out-Null
        if (Test-Path $sObj) { $tsObjs += "`"$sObj`"" }
    }
}

# Build command
if ($Compiler -in @("g++","clang++")) {
    $cmd = @($Compiler, "-std=c++17", "-O3", "-I", $SrcDir, "-I", (Join-Path $tsTree "lib\include"))
    $cmd += (Join-Path $SrcDir "glupec.cpp")
    if (Test-Path (Join-Path $SrcDir "lex.yy.c")) { $cmd += (Join-Path $SrcDir "lex.yy.c") }
    if (Test-Path (Join-Path $SrcDir "glupe.tab.c")) { $cmd += (Join-Path $SrcDir "glupe.tab.c") }
    foreach ($o in $tsObjs) { $cmd += $o }
    $cmd += "-o"; $cmd += $ExePath
    # Avoid forcing -static on Windows; let the builder decide.
    $fullCmd = $cmd -join ' '
    Write-Info "Running: $fullCmd"
    $result = Run-Command -File $cmd[0] -Args $cmd[1..($cmd.Length-1)]
    if ($result.ExitCode -ne 0) {
        Write-Warn "Compiler returned non-zero exit code. Stdout:`n$result.Stdout`nStderr:`n$result.Stderr"
        Write-Err "Compilation failed."
    } else {
        Write-Ok "Compiled Glupe to: $ExePath"
    }
} elseif ($Compiler -eq "cl") {
    Write-Warn "MSVC detected (cl.exe). The provided build path uses g++/clang++. To build with MSVC you need a Visual Studio Developer prompt and a suitable project/CMake configuration. See README for instructions."
    Write-Err "No automatic MSVC build implemented in this installer."
}

# 6) Create config.json
$configPath = Join-Path $InstallDir "config.json"
if (-not (Test-Path $configPath)) {
    $conf = @{
        local = @{ model_id = "qwen2.5-coder:latest"; api_url = "http://localhost:11434/api/generate" }
        cloud = @{ protocol = "openai"; api_key = ""; model_id = "gpt-4o"; api_url = "https://api.openai.com/v1/chat/completions" }
        max_retries = 15
    } | ConvertTo-Json -Depth 4
    Set-Content -Path $configPath -Value $conf -Encoding UTF8
    Write-Ok "Created default config.json"
}

# 7) Add to user PATH (non-destructive)
try {
    $userPath = [Environment]::GetEnvironmentVariable("Path","User") -split ';' | Where-Object { $_ -ne '' }
    if ($userPath -notcontains $InstallDir) {
        $newPath = ($userPath + $InstallDir) -join ';'
        [Environment]::SetEnvironmentVariable("Path",$newPath,"User")
        Write-Warn "Added $InstallDir to user PATH. You will need to restart shells to pick it up."
    } else {
        Write-Ok "$InstallDir already in user PATH"
    }
} catch {
    Write-Warn "Could not update user PATH automatically: $_"
}

Write-Host "`n[SUCCESS] Glupe installer finished." -ForegroundColor Cyan
Write-Host "Try: `"$ExePath --help`" (or open a new shell and run `glupe --help` if PATH was updated)."