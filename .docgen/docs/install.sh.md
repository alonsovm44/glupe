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
- **Bash**: Ensures Bash is available (required for Alpine Linux).
- **Package Manager**: Detects and uses the system's package manager (apk, apt, dnf, pacman, brew) to install build dependencies if needed.
- **C++ Compiler**: Verifies the presence of `g++` or `clang++`. Offers to install dependencies if missing. Exits if installation fails.
- **Ollama**: Checks for Ollama (Local AI). Offers optional installation if absent, with a warning for minimal distros like Alpine.

### 2. **Environment Setup**
- **Installation Directory**: Creates `$HOME/.glupe` for storing files. Falls back to current directory if `$HOME` is not set.
- **Source Code Download**: Fetches Glupe source files and `json.hpp` from GitHub.

### 3. **Parser and Lexer Generation**
- Generates parser (`glupe.tab.c`) and lexer (`lex.yy.c`) files using Bison and Flex if available. Falls back to downloading pre-generated files if generation fails.

### 4. **Tree-Sitter Integration**
- Downloads and builds Tree-sitter core and language parsers for C++, Python, JavaScript, Java, Go, and Rust. Object files are linked during compilation for optional language support.

### 5. **Compilation**
- Compiles `glupec.cpp`, `lex.yy.c`, `glupe.tab.c`, and Tree-sitter object files using the detected C++ compiler with C++17 and optimizations.
- Links Tree-sitter objects for language parsing support.
- Handles filesystem library linking on Linux by adding `-lstdc++fs`.

### 6. **Configuration**
- Generates a default `config.json` if it doesn't exist, with settings for local and cloud AI models.

### 7. **PATH Integration**
- Adds the installation directory to the user's `PATH` in their shell configuration file (`~/.bashrc`, `~/.zshrc`, or `~/.profile`).
- Notifies the user to restart the terminal or source the config file.

### 8. **Completion**
- Confirms successful installation and provides a command to get started.

## Error Handling
- Exits with an error message if:
  - C++ compiler is missing and cannot be installed.
  - Source file downloads fail.
  - Compilation fails.

## Notes
- Source files are retained for transparency and manual recompilation.
- Ollama installation is optional but recommended for local AI functionality.
- Tree-sitter integration is best-effort; language parsing may be limited if dependencies are missing.

## Requirements
- Internet connection for downloading files.
- Administrative privileges may be required for certain installations.

This script ensures Glupe is installed correctly, configured, and ready for use with minimal user intervention.