# Glupe Installer for Windows

## Purpose
This PowerShell script automates the installation of Glupe, a C++ application, on Windows systems. It handles dependencies, compilation, and configuration, ensuring a seamless setup experience.

## Usage
To execute the installer, run the following command in PowerShell:
```powershell
irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
```

## Behavior

### 1. **Dependency Checks and Installation**
- **G++ Compiler**: Verifies the presence of `g++`. If absent, installs a portable MinGW (w64devkit) in the Glupe installation directory.
- **Ollama (Local AI)**: Checks for Ollama. If not found, prompts the user to install it via a downloadable installer.

### 2. **Directory Setup**
- Creates the installation directory (`%USERPROFILE%\.glupe`) and a `src` subdirectory for source files.

### 3. **Source Code Download**
- Downloads Glupe source files (`glupec.cpp`, headers, and `json.hpp`) from the GitHub repository and places them in the `src` directory.

### 4. **Compilation**
- Compiles `glupec.cpp` into `glupe.exe` using `g++` with static linking (`-static`) to ensure portability. The executable is placed in the installation directory.

### 5. **Configuration**
- Generates a default `config.json` file if it does not exist, containing settings for local and cloud AI models.

### 6. **PATH Integration**
- Adds the Glupe installation directory to the user's PATH environment variable if it is not already present. A restart of the terminal is required to use `glupe` globally.

### 7. **Cleanup**
- Retains source files (`glupec.cpp`, `json.hpp`) for potential recompilation. Removes the downloaded MinGW zip file if present.

### Error Handling
- Stops execution on critical errors (e.g., download failures, compilation issues) and provides clear error messages.

### Output
- Provides color-coded feedback for each step, indicating success (`[OK]`), warnings (`[WARN]`), or errors (`[ERROR]`).

## Post-Installation
After successful installation, run `glupe --help` to explore available commands and options.