Here is the updated documentation with **only the sections that changed**, preserving the original structure and wording as closely as possible.

```markdown
# Glupe Update Script Documentation

## Overview
This script automates the process of updating the `glupe` executable on Linux/macOS systems. It downloads the latest source code, generates parser/lexer files (or fetches pre‑generated ones), compiles the project using an available C++ compiler, and replaces the existing `glupe` binary with the newly compiled version.

## Purpose
- **Update Automation**: Ensures `glupe` is up-to-date without manual intervention.
- **Cross-Platform Compatibility**: Designed for Linux and macOS environments.
- **Error Handling**: Provides clear feedback on failures during download, parser/lexer generation, compilation, or replacement.

## Usage
1. **Prerequisites**:
   - Ensure `glupe` is installed and accessible via `PATH`.
   - A C++17‑capable compiler is required (**g++** or **clang++**).
   - `curl` is required for downloading files.
   - **Flex and Bison** are recommended but no longer required; the script will fall back to downloading pre‑generated parser/lexer files if needed.

2. **Execution**:
   - Run the script directly:
     ```bash
     ./update_glupe.sh
     ```
   - If permissions are insufficient, use `sudo`:
     ```bash
     sudo ./update_glupe.sh
     ```

## Behavior
1. **Environment Detection**:
   - Locates the current `glupe` executable using `command -v`.
   - Exits if `glupe` is not found in `PATH`.
   - Creates a `src` directory next to the installed `glupe` binary.

2. **Source Code Download**:
   - Downloads source files from the updated repository (`https://raw.githubusercontent.com/alonsovm44/glupe/master`).
   - Fetches `json.hpp` from the nlohmann/json GitHub releases.
   - Stores files in the `src` directory adjacent to the current `glupe` binary.
   - Downloads an expanded set of source files, including:
     - `glupec.cpp`, multiple headers (`*.hpp`), and grammar files (`glupe.l`, `glupe.y`).
   - Missing files no longer cause failure; the script continues with warnings.

3. **Parser and Lexer Generation**:
   - Attempts to run **Bison** and **Flex** to generate `glupe.tab.c` and `lex.yy.c`.
   - If generation fails or Flex/Bison are not installed, the script automatically downloads pre‑generated parser/lexer files from the repository.
   - This fallback mechanism ensures the update can proceed even without local parser tools.

4. **Compilation**:
   - Automatically selects the first available compiler: `g++` or `clang++`.
   - Compiles `glupec.cpp` along with generated or downloaded parser/lexer sources.
   - Uses optimizations (`-O3`), threading support (`-pthread`), and C++17 standard.
   - Includes the `src` directory via `-I`.
   - Links `-lstdc++fs` on Linux for compatibility with older libstdc++ versions.
   - Automatically includes Tree‑sitter object files (`tree-sitter.o`, language parsers/scanners) if present in `vendor/`.
   - Outputs the new binary to a secure temporary file in `/tmp`.

5. **Executable Replacement**:
   - Creates a timestamped backup of the existing `glupe` binary when possible.
   - Attempts an in‑place replacement using `mv`.
   - If permissions prevent replacement, the script retries using `sudo`.
   - Ensures the final binary is marked executable.

6. **Error Handling**:
   - Exits with an error message if compilation or replacement fails.
   - Warns but does not abort on missing source files or parser/lexer generation failures.
   - Leaves the temporary binary intact if replacement fails, allowing manual recovery.

## Notes
- **Permissions**: The script may require elevated privileges (`sudo`) if the current `glupe` binary is owned by root or installed system‑wide.
- **Terminal Restart**: A terminal or shell restart may be needed to load the updated `glupe` binary into memory.
- **Dependencies**: Ensure required tools (`curl`, `g++` or `clang++`, optional `flex` and `bison`) are installed before running the script.
```

If you'd like, I can also generate a **diff-style comparison** showing exactly what changed from the previous documentation.