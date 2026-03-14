```markdown
# Glupe Update Script Documentation

## Overview
This script automates the process of updating the `glupe` executable on Linux/macOS systems. It downloads the latest source code, compiles it, and replaces the existing `glupe` binary with the newly compiled version.

## Purpose
- **Update Automation**: Ensures `glupe` is up-to-date without manual intervention.
- **Cross-Platform Compatibility**: Designed for Linux and macOS environments.
- **Error Handling**: Provides clear feedback on failures during download, compilation, or replacement.

## Usage
1. **Prerequisites**:
   - Ensure `glupe` is installed and accessible via `PATH`.
   - `g++` compiler must be installed with C++17 support.
   - `curl` is required for downloading files.

2. **Execution**:
   - Run the script directly:
     ```bash
     ./update_glupe.sh
     ```
   - If permissions are insufficient, use `sudo`:
     ```bash
     sudo ./update_glu_pe.sh
     ```

## Behavior
1. **Environment Detection**:
   - Locates the current `glupe` executable using `command -v`.
   - Exits if `glupe` is not found in `PATH`.

2. **Source Code Download**:
   - Downloads source files from the official Glupe repository (`https://raw.githubusercontent.com/M-MACHINE/glupe/main`).
   - Fetches `json.hpp` from the nlohmann/json GitHub releases.
   - Stores files in a `src` directory adjacent to the current `glupe` binary.

3. **Compilation**:
   - Compiles `glupec.cpp` with optimizations (`-O3`), threading support (`-pthread`), and C++17 standard.
   - Outputs the new binary to a temporary location (`/tmp/glupe.new`).

4. **Executable Replacement**:
   - Replaces the current `glupe` binary with the newly compiled version.
   - Provides instructions to reload the updated binary if necessary.

5. **Error Handling**:
   - Exits with an error message if any step fails (download, compilation, or replacement).
   - Cleans up temporary files on failure.

## Notes
- **Permissions**: The script may require elevated privileges (`sudo`) if the current `glupe` binary is owned by root.
- **Terminal Restart**: A terminal or shell restart may be needed to load the updated `glupe` binary into memory.
- **Dependencies**: Ensure all dependencies (`g++`, `curl`) are installed before running the script.
```