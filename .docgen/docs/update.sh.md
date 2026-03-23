Here is the updated documentation with **only the sections that changed**, while preserving the original structure and wording wherever possible.

```markdown
# Glupe Update Script Documentation

## Overview
This script automates the process of updating the `glupe` executable on Linux/macOS systems. It downloads the latest source code, generates parser/lexer files, compiles the project, and replaces the existing `glupe` binary with the newly compiled version.

## Purpose
- **Update Automation**: Ensures `glupe` is up-to-date without manual intervention.
- **Cross-Platform Compatibility**: Designed for Linux and macOS environments.
- **Error Handling**: Provides clear feedback on failures during download, parser/lexer generation, compilation, or replacement.

## Usage
1. **Prerequisites**:
   - Ensure `glupe` is installed and accessible via `PATH`.
   - `g++` compiler must be installed with C++17 support.
   - `curl` is required for downloading files.
   - **Flex and Bison** are recommended for generating lexer/parser files.

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

2. **Source Code Download**:
   - Downloads source files from the Glupe repository (`https://raw.githubusercontent.com/M-MACHINE/glupe/master`).
   - Fetches `json.hpp` from the nlohmann/json GitHub releases.
   - Stores files in a `src` directory adjacent to the current `glupe` binary.
   - Downloads an expanded set of source files, including:
     - `glupec.cpp`, headers (`*.hpp`), and grammar files (`glupe.l`, `glupe.y`).

3. **Parser and Lexer Generation**:
   - Runs **Bison** to generate `glupe.tab.c`.
   - Runs **Flex** to generate `lex.yy.c`.
   - Continues even if Flex/Bison are missing, but warns the user.

4. **Compilation**:
   - Compiles `glupec.cpp` along with generated parser/lexer sources.
   - Uses optimizations (`-O3`), threading support (`-pthread`), and C++17 standard.
   - Includes the `src` directory via `-I`.
   - Outputs the new binary to `/tmp/glupe.new`.

5. **Executable Replacement**:
   - Replaces the current `glupe` binary with the newly compiled version.
   - Advises restarting the terminal or shell to load the updated binary.

6. **Error Handling**:
   - Exits with an error message if any step fails (download, parser/lexer generation, compilation, or replacement).
   - Cleans up temporary files on failure.

## Notes
- **Permissions**: The script may require elevated privileges (`sudo`) if the current `glupe` binary is owned by root.
- **Terminal Restart**: A terminal or shell restart may be needed to load the updated `glupe` binary into memory.
- **Dependencies**: Ensure all dependencies (`g++`, `curl`, `flex`, `bison`) are installed before running the script.
```

If you'd like, I can also generate a diff-style version showing exactly what changed.