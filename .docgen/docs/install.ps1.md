Here is the updated documentation, revised **only where the code has changed**, while preserving the original structure and intent:

---

# Glupe Installer for Windows

## Purpose
This PowerShell script automates the installation of Glupe, a C++ application, on Windows systems. It handles dependency installation, parser/lexer generation, compilation, and configuration to ensure a seamless setup experience.

## Usage
To execute the installer, run the following command in PowerShell:
```powershell
irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
```

## Behavior

### 1. **Dependency Checks and Installation**
- **C++ Compiler**: Checks for `g++`, `clang++`, or `cl.exe`.  
  - If none are found, downloads and installs a portable MinGW toolchain (w64devkit v1.21.0) inside the Glupe installation directory and temporarily updates the session PATH.
- **Ollama (Local AI)**: Checks for the `ollama` command.  
  - If not found, informs the user to install Ollama separately (no automatic installation).

### 2. **Directory Setup**
- Creates the installation directory (`%USERPROFILE%\.glupe`) and `src` and `vendor` subdirectories if they do not exist.

### 3. **Source Code Download**
- Downloads all Glupe source files from the GitHub repository, including:
  - `glupec.cpp`, multiple header files, `glupe.l`, `glupe.y`
- Downloads the latest `json.hpp` from the official nlohmann/json release (v3.11.3).

### 3.5 **Parser and Lexer Generation**
- Attempts to generate:
  - `glupe.tab.c` using **Bison**
  - `lex.yy.c` using **Flex**
- If Bison or Flex are unavailable, downloads pre-generated parser/lexer files from the repository.

### 4. **Tree-Sitter Integration**
- Downloads and extracts the Tree-sitter library (v0.22.6) and language grammars for C++, Python, JavaScript, Java, Go, and Rust.
- Compiles Tree-sitter components into object files for static linking.

### 5. **Compilation**
- Compiles Glupe using the detected compiler (`g++`, `clang++`, or `cl.exe`):
  - Links Tree-sitter object files for language support.
  - Uses C++17 standard, optimization (`-O3`), and includes necessary directories.
  - Produces a fully static `glupe.exe` in the installation directory.

### 6. **Configuration**
- Creates a default `config.json` if none exists.  
- Updated default configuration includes:
  - **Local model**  
    - `model_id`: `"qwen2.5-coder:latest"`  
    - `api_url`: `"http://localhost:11434/api/generate"`
  - **Cloud model**  
    - `protocol`: `"openai"`  
    - `model_id`: `"gpt-4o"`  
    - `api_url`: `"https://api.openai.com/v1/chat/completions"`
  - `max_retries`: `15`

### 7. **PATH Integration**
- Adds the Glupe installation directory to the user's PATH if not already present.
- A terminal restart is required for PATH changes to take effect.

### 8. **Cleanup**
- Retains all source files and Tree-sitter components for future manual recompilation or updates.
- Removes temporary download files (e.g., `.zip` archives).

### Error Handling
- Stops execution on critical errors such as download failures or compilation issues.
- Provides clear, color-coded error messages.

### Output
- Displays color-coded status messages:
  - `[OK]` for success
  - `[INFO]` for informational steps
  - `[WARN]` for non-critical issues
  - `[ERROR]` for failures

## Post-Installation
After installation, run:
```
glupe --help
```
to explore available commands and options.

--- 

### Key Changes Summary:
1. **Compiler Detection**: Added support for `clang++` and `cl.exe` (MSVC).
2. **Tree-Sitter Integration**: Downloads and compiles Tree-sitter for language support.
3. **Compilation**: Improved handling of object files and compiler flags.
4. **Configuration**: Updated `config.json` structure with separate `local` and `cloud` sections.
5. **PATH Integration**: Non-destructive addition to user PATH.
6. **Error Handling**: Enhanced error messages and warnings.