Here is the updated documentation, modified **only where the code has changed**, while preserving the original structure and intent.

---

# Glupe Update Script Documentation

## Purpose
This PowerShell script automates the process of updating the `glupe.exe` executable on a Windows system. It downloads the latest source code, generates parser and lexer files using Bison and Flex when available (or downloads pre‑generated versions as a fallback), compiles the code using an available C++ compiler, and replaces the existing executable with the newly built version. The script ensures a safe update process by creating backups and handling file‑locking scenarios.

## Usage
1. **Prerequisites**:
   - Ensure a C++ compiler (`g++` or `clang++`) is installed and available in the system's PATH.
   - Optional but recommended: `bison` and `flex` for generating parser and lexer files.
   - The existing `glupe.exe` must be in the system's PATH and accessible.

2. **Execution**:
   - Run the script in a PowerShell terminal. Administrative privileges are not strictly required; the script attempts to use writable locations and provides guidance if replacement fails.

## Behavior

### 1. **Environment Setup**
   - **Repository Base URL**: `https://raw.githubusercontent.com/alonsovm44/glupe/master`
   - **JSON Library URL**: `https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp`
   - **Temporary Binary Path**: A unique filename in the system temp directory (e.g., `glupe_new_<GUID>.exe`).

### 2. **Current Executable Detection**
   - Locates the installed `glupe` executable using `Get-Command`.
   - Exits with an error if `glupe` is not found in the PATH.
   - Determines the installation directory and creates a `src` subdirectory if it does not exist.

### 3. **Source Code Download**
   - Downloads source files into the `src` directory.  
   - Updated file list includes:
     - `glupec.cpp`, `common.hpp`, `utils.hpp`, `config.hpp`, `languages.hpp`, `ai.hpp`,  
       `cache.hpp`, `parser.hpp`, `processor.hpp`, `hub.hpp`, `ast.hpp`, `ast_utils.hpp`,  
       `glupe.l`, `glupe.y`
   - Attempts to download `json.hpp`.
   - Download failures now produce warnings rather than stopping execution.

### 4. **Parser and Lexer Generation**
   - If both `bison` and `flex` are available:
     - Generates `glupe.tab.c` and `lex.yy.c`.
   - If generation fails or tools are unavailable:
     - Attempts to download pre‑generated `glupe.tab.c` and `lex.yy.c` from the repository.
   - The script continues even if generation or fallback downloads fail.

### 5. **Compilation**
   - Selects the first available compiler: `g++` or `clang++`.  
   - Exits with an error if neither is found.
   - Compilation includes:
     - `glupec.cpp`, plus generated or downloaded `lex.yy.c` and `glupe.tab.c` when present.
     - Tree‑sitter object files found in the `vendor` directory:
       - `tree-sitter.o`
       - `<lang>_parser.o` and `<lang>_scanner.o` for: `cpp`, `python`, `javascript`, `java`, `go`, `rust`
     - Tree‑sitter include directory: `vendor/tree-sitter/lib/include`
   - Uses the following flags:
     - `-std=c++17`
     - `-O3`
     - `-I <src directory>`
     - `-I <tree-sitter include directory>`
   - Produces a temporary executable in the system temp directory.
   - Exits with an error if compilation fails or no output is produced.

### 6. **Executable Replacement**
   - Creates a timestamped backup of the existing executable (e.g., `glupe.exe.20260101123045.bak`).
   - Attempts to replace the existing executable with the newly compiled one.
   - If replacement fails (e.g., file in use):
     - Displays instructions for manual replacement.
     - Removes the temporary executable.
     - Does not delete the backup.

### 7. **Completion**
   - Displays a success message when the update completes.
   - Recommends restarting terminals to ensure the new version is used.

## Error Handling
- **Download Failures**: Logged as warnings; script continues.
- **Parser/Lexer Generation Failure**: Falls back to downloading pre‑generated files.
- **Compiler Not Found**: Stops execution with an error.
- **Compilation Failure**: Stops execution and removes the temporary executable.
- **File Locking**: Provides guidance for manual replacement and preserves backups.
- **Critical Errors**: Attempts to maintain system stability by keeping the original executable intact.

## Notes
- The script is designed to be resilient: missing tools or partial failures do not necessarily stop the update.
- Tree‑sitter integration is now supported when object files are present in the `vendor` directory.
- Administrative privileges are not required unless the installation directory is protected.
- Manual intervention may be required if the executable is in use during replacement.

---

If you'd like, I can also generate a concise changelog summarizing what changed between the old and new script.