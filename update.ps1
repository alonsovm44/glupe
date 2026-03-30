<#
 Safe updater for glupe (Windows PowerShell)
 # Usage: .\update.ps1  (run in admin or normal session; will attempt to use writable locations / prompt)
#>
[CmdletBinding()]
param()

$RepoRaw = "https://raw.githubusercontent.com/alonsovm44/glupe/master"
$JsonUrl = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"

Write-Host "--- Glupe Update Script (Windows) ---" -ForegroundColor Cyan

# locate installed glupe
$cmd = Get-Command glupe -ErrorAction SilentlyContinue
if (-not $cmd) {
    Write-Error "glupe not found in PATH. Ensure it's installed."
    exit 1
}
$currentGlupePath = $cmd.Source
$glupeDir = Split-Path $currentGlupePath -Parent
Write-Host "Current glupe path: $currentGlupePath"

$SrcDir = Join-Path $glupeDir "src"
if (-not (Test-Path $SrcDir)) { New-Item -ItemType Directory -Force -Path $SrcDir | Out-Null }

# download sources (best-effort)
$files = @("glupec.cpp","common.hpp","utils.hpp","config.hpp","languages.hpp","ai.hpp","cache.hpp","parser.hpp","processor.hpp","hub.hpp","ast.hpp","ast_utils.hpp","glupe.l","glupe.y")
foreach ($f in $files) {
    $url = "$RepoRaw/src/$f"
    try { Invoke-WebRequest -Uri $url -OutFile (Join-Path $SrcDir $f) -UseBasicParsing -ErrorAction Stop; Write-Host "Fetched $f" }
    catch { Write-Warn "Could not fetch $f; continuing." }
}
try { Invoke-WebRequest -Uri $JsonUrl -OutFile (Join-Path $SrcDir "json.hpp") -UseBasicParsing -ErrorAction Stop; Write-Host "Fetched json.hpp" } catch { Write-Warn "Could not fetch json.hpp" }

# parser/lexer gen or fallback
$genOk = $false
if (Get-Command bison -ErrorAction SilentlyContinue -and Get-Command flex -ErrorAction SilentlyContinue) {
    try {
        & bison -d -o (Join-Path $SrcDir "glupe.tab.c") (Join-Path $SrcDir "glupe.y")
        & flex -o (Join-Path $SrcDir "lex.yy.c") (Join-Path $SrcDir "glupe.l")
        $genOk = $true
        Write-Host "Generated parser/lexer"
    } catch { Write-Warn "bison/flex generation failed" }
}
if (-not $genOk) {
    Write-Host "Attempting to download pre-generated parser/lexer..."
    try { Invoke-WebRequest -Uri "$RepoRaw/src/glupe.tab.c" -OutFile (Join-Path $SrcDir "glupe.tab.c") -UseBasicParsing; Write-Host "Fetched glupe.tab.c" } catch { }
    try { Invoke-WebRequest -Uri "$RepoRaw/src/lex.yy.c" -OutFile (Join-Path $SrcDir "lex.yy.c") -UseBasicParsing; Write-Host "Fetched lex.yy.c" } catch { }
}

# choose compiler (prefer g++/clang++)
$compiler = (Get-Command g++ -ErrorAction SilentlyContinue)?.Source
if (-not $compiler) { $compiler = (Get-Command clang++ -ErrorAction SilentlyContinue)?.Source }
if (-not $compiler) {
    Write-Error "No g++/clang++ compiler found. Install MSYS2/w64devkit or Visual Studio build tools and retry."
    exit 1
}
Write-Host "Using compiler: $compiler"

# compile to temp exe
$tempExe = Join-Path $env:TEMP ("glupe_new_{0}.exe" -f ([System.Guid]::NewGuid().ToString()))
$srcGlupec = Join-Path $SrcDir "glupec.cpp"
$lex = Join-Path $SrcDir "lex.yy.c"
$tab = Join-Path $SrcDir "glupe.tab.c"

$tsObjs = @()
$vendor = Join-Path $glupeDir "vendor"
$fallbackVendor = Join-Path $env:USERPROFILE ".glupe\vendor"
if (-not (Test-Path $vendor) -and (Test-Path $fallbackVendor)) {
    $vendor = $fallbackVendor
}

if (-not (Test-Path $vendor)) { New-Item -ItemType Directory -Force -Path $vendor | Out-Null }

if (-not (Test-Path (Join-Path $vendor "tree-sitter.o")) -or -not (Test-Path (Join-Path $vendor "cpp_parser.o"))) {
    Write-Host "Tree-sitter objects missing. Fetching and building..." -ForegroundColor Cyan
    try {
        $tsUrl = "https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.zip"
        $tsZip = Join-Path $vendor "tree-sitter.zip"
        if (-not (Test-Path (Join-Path $vendor "tree-sitter"))) {
            Write-Host "Downloading tree-sitter (0.22.6)..."
            Invoke-WebRequest -Uri $tsUrl -OutFile $tsZip -UseBasicParsing
            Expand-Archive -Path $tsZip -DestinationPath $vendor -Force
            Remove-Item $tsZip -Force
            Get-ChildItem $vendor -Directory | Where-Object { $_.Name -like "tree-sitter-*" } | ForEach-Object { Rename-Item $_.FullName -NewName "tree-sitter" -Force }
        }
        
        $tsTree = Join-Path $vendor "tree-sitter"
        if (Test-Path (Join-Path $tsTree "lib\src\lib.c")) {
            $tsObj = Join-Path $vendor "tree-sitter.o"
            & $env:COMSPEC /c "gcc -O3 -I`"$tsTree\lib\include`" -I`"$tsTree\lib\src`" -c `"$tsTree\lib\src\lib.c`" -o `"$tsObj`"" 2>&1 | Out-Null
        }
        
        $tsLangs = [ordered]@{ "cpp"="0.22.0"; "python"="0.21.0"; "javascript"="0.21.2"; "java"="0.21.0"; "go"="0.21.2"; "rust"="0.21.2" }
        foreach ($lang in $tsLangs.Keys) {
            $ver = $tsLangs[$lang]
            $langDir = "tree-sitter-$lang"
            if (-not (Test-Path (Join-Path $vendor $langDir))) {
                Write-Host "Downloading $langDir ($ver)..."
                $langUrl = "https://github.com/tree-sitter/$langDir/archive/refs/tags/v$ver.zip"
                $langZip = Join-Path $vendor "$langDir.zip"
                Invoke-WebRequest -Uri $langUrl -OutFile $langZip -UseBasicParsing
                Expand-Archive -Path $langZip -DestinationPath $vendor -Force
                Remove-Item $langZip -Force
                Get-ChildItem $vendor -Directory | Where-Object { $_.Name -like "$langDir-*" } | ForEach-Object { Rename-Item $_.FullName -NewName $langDir -Force }
            }
            
            $srcDirTS = Join-Path $vendor "$langDir\src"
            if (Test-Path (Join-Path $srcDirTS "parser.c")) { & $env:COMSPEC /c "gcc -O3 -I`"$srcDirTS`" -c `"$srcDirTS\parser.c`" -o `"$vendor\${lang}_parser.o`"" 2>&1 | Out-Null }
            if (Test-Path (Join-Path $srcDirTS "scanner.c")) { & $env:COMSPEC /c "gcc -O3 -I`"$srcDirTS`" -c `"$srcDirTS\scanner.c`" -o `"$vendor\${lang}_scanner.o`"" 2>&1 | Out-Null }
            elseif (Test-Path (Join-Path $srcDirTS "scanner.cc")) { $cxx = if ($compiler) { $compiler } else { "g++" }; & $env:COMSPEC /c "$cxx -O3 -I`"$srcDirTS`" -c `"$srcDirTS\scanner.cc`" -o `"$vendor\${lang}_scanner.o`"" 2>&1 | Out-Null }
        }
    } catch { Write-Warn "Failed to fetch/build tree-sitter. Compilation may fail." }
}

if (Test-Path (Join-Path $vendor "tree-sitter.o")) { $tsObjs += (Join-Path $vendor "tree-sitter.o") }
$langs = @("cpp", "python", "javascript", "java", "go", "rust")
foreach ($lang in $langs) {
    if (Test-Path (Join-Path $vendor "${lang}_parser.o")) { $tsObjs += (Join-Path $vendor "${lang}_parser.o") }
    if (Test-Path (Join-Path $vendor "${lang}_scanner.o")) { $tsObjs += (Join-Path $vendor "${lang}_scanner.o") }
}

$tsInclude = Join-Path $vendor "tree-sitter\lib\include"
$args = @("-std=c++17","-O3","-I",$SrcDir,"-I",$tsInclude,$srcGlupec)
if (Test-Path $lex) { $args += $lex }
if (Test-Path $tab) { $args += $tab }
$args += $tsObjs
$args += @("-o",$tempExe)

Write-Host "Compiling to $tempExe ..."
$proc = Start-Process -FilePath $compiler -ArgumentList $args -NoNewWindow -Wait -PassThru -ErrorAction SilentlyContinue
if ($proc.ExitCode -ne 0) {
    Write-Error "Compilation failed (exit $($proc.ExitCode)). See compiler output in your shell."
    if (Test-Path $tempExe) { Remove-Item $tempExe -ErrorAction SilentlyContinue }
    exit 1
}

if (-not (Test-Path $tempExe)) {
    Write-Error "Compilation did not produce $tempExe"
    exit 1
}

# backup and attempt replace
$backup = "$currentGlupePath.$([DateTime]::UtcNow.ToString('yyyyMMddHHmmss')).bak"
try {
    Copy-Item -Path $currentGlupePath -Destination $backup -Force -ErrorAction Stop
    Write-Host "Backup created: $backup"
} catch { Write-Warn "Could not create backup; continuing." }

# Try move; if file locked, instruct user
try {
    Move-Item -Path $tempExe -Destination $currentGlupePath -Force -ErrorAction Stop
    Write-Host "Updated glupe executable in place. Old backup: $backup" -ForegroundColor Green
} catch {
    Write-Error "Failed to move new exe into place. The file might be in use. Close terminals using glupe and re-run updater, or manually replace $currentGlupePath."
    # attempt restore cleanup
    if (Test-Path $tempExe) { Remove-Item $tempExe -ErrorAction SilentlyContinue }
    exit 1
}

Write-Host "Update complete. Restart terminals if necessary." -ForegroundColor Cyan