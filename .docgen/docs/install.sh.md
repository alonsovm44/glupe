# Glupe Installer Script Documentation

## Overview
This Bash script automates the installation of Glupe, a CLI tool, on Linux and macOS systems. It handles dependencies, compilation, and configuration, ensuring a seamless setup process.

## Purpose
The script provides a one-command installation method for Glupe, eliminating manual steps. It checks for required tools, downloads source code, compiles the application, and configures the environment.

## Usage
Execute the script using the following command:
```bash
curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash
```

## Behavior

### 1. **Prerequisites Check**
- **C++ Compiler**: Verifies the presence of `g++` or `clang++`. Exits if neither is found, providing installation instructions.
- **Ollama**: Checks for Ollama (Local AI). Offers installation if absent.

### 2. **Environment Setup**
- **Installation Directory**: Creates `$HOME/.glupe` for storing files.
- **Source Code Download**: Fetches Glupe source files and `json.hpp` from GitHub.

### 3. **Compilation**
- Compiles `glupec.cpp` using the detected C++ compiler with C++17 and optimizations.
- Handles filesystem library linking on Linux.

### 4. **Configuration**
- Generates a default `config.json` if it doesn't exist.

### 5. **PATH Integration**
- Adds the installation directory to the user's `PATH` in their shell configuration file (`~/.bashrc`, `~/.zshrc`, or `~/.profile`).
- Notifies the user to restart the terminal or source the config file.

### 6. **Completion**
- Confirms successful installation and provides a command to get started.

## Error Handling
- Exits with an error message if:
  - C++ compiler is missing.
  - Source file downloads fail.
  - Compilation fails.

## Notes
- Source files are retained for transparency and manual recompilation.
- Ollama installation is optional but recommended for local AI functionality.

## Requirements
- Internet connection for downloading files.
- Administrative privileges may be required for certain installations.

This script ensures Glupe is installed correctly, configured, and ready for use with minimal user intervention.