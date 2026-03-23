Here is the updated documentation, revised **only where the code has changed**, while preserving the original structure and intent.

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
- **G++ Compiler**: Verifies the presence of `g++`.  
  - If missing, downloads and installs a portable MinGW toolchain (w64devkit v1.21.0) inside the Glupe installation directory and temporarily updates the session PATH.
- **Ollama (Local AI)**: Checks for the `ollama` command.  
  - If not found, prompts the user to optionally download and run the official Ollama installer.

### 2. **Directory Setup**
- Creates the installation directory (`%USERPROFILE%\.glupe`) if it does not exist.
- Creates a `src` subdirectory to store all source files.

### 3. **Source Code Download**
- Downloads all Glupe source files from the GitHub repository, including:
  - `glupec.cpp`, multiple header files, `glupe.l`, `glupe.y`
- Downloads the latest `json.hpp` from the official nlohmann/json release.

### 3.5 **Parser and Lexer Generation**
- Attempts to generate:
  - `glupe.tab.c` using **Bison**
  - `lex.yy.c` using **Flex**
- If Bison or Flex are unavailable, the script warns the user. Compilation may still succeed if generated C files already exist.

### 4. **Compilation**
- Compiles Glupe using:
  - `g++ glupec.cpp lex.yy.c glupe.tab.c`
  - C++17 standard
  - Full static linking (`-static`, `-static-libgcc`, `-static-libstdc++`)
  - Filesystem library (`-lstdc++fs`)
  - Optimization (`-O3`)
- Produces a fully static `glupe.exe` in the installation directory.

### 5. **Configuration**
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

### 6. **PATH Integration**
- Adds the Glupe installation directory to the user's PATH if not already present.
- MinGW is **not** added permanently to PATH to keep the environment clean.
- A terminal restart is required for PATH changes to take effect.

### 7. **Cleanup**
- Retains all source files (`*.cpp`, `*.hpp`, `json.hpp`, lexer/parser files) for future manual recompilation or updates.
- Removes the MinGW zip file if it exists.

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

If you'd like, I can also generate a diff-style summary showing exactly what changed from the previous documentation.