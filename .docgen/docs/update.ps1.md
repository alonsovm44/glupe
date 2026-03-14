```markdown
# Glupe Update Script Documentation

## Purpose
This PowerShell script automates the process of updating the `glupe.exe` executable on a Windows system. It downloads the latest source code, compiles it, and replaces the existing executable with the newly built version. The script ensures a safe update process by handling file locking and providing backup mechanisms.

## Usage
1. **Prerequisites**:
   - Ensure `g++` (MinGW or similar) is installed and available in the system's PATH.
   - The existing `glupe.exe` must be in the system's PATH and accessible.

2. **Execution**:
   - Run the script in a PowerShell terminal with sufficient permissions.

## Behavior
### 1. **Environment Setup**
   - **Repository Base URL**: `https://raw.githubusercontent.com/M-MACHINE/glupe/main`  
   - **JSON Library URL**: `https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp`  
   - **Temporary Binary Path**: Stored in the system's temp directory as `glupe.exe.new`.

### 2. **Current Executable Detection**
   - Locates the path of the currently installed `glupe.exe` using `Get-Command`.  
   - Exits with an error if `glupe.exe` is not found in the PATH.

### 3. **Source Code Download**
   - Creates a `src` directory in the same folder as the current `glupe.exe`.  
   - Downloads specified source files (`glupec.cpp`, `common.hpp`, etc.) from the repository.  
   - Downloads the JSON library (`json.hpp`) from the provided URL.  
   - Exits with an error if any download fails.

### 4. **Compilation**
   - Compiles `glupec.cpp` using `g++` with the following flags:  
     - `-std=c++17`: C++17 standard.  
     - `-static`: Produces a fully static binary.  
     - `-O3`: Maximum optimization.  
     - `-I "$SrcDir"`: Includes the `src` directory for headers.  
   - Exits with an error if compilation fails or the output file is not generated.

### 5. **Executable Replacement**
   - Renames the existing `glupe.exe` to `glupe.exe.old` to handle file locking.  
   - Moves the newly compiled binary (`glupe.exe.new`) to the original location.  
   - If the move fails, attempts to restore the old executable.  
   - Cleans up the backup file (`glupe.exe.old`) if the update is successful.

### 6. **Completion**
   - Displays success or error messages based on the outcome.  
   - Recommends restarting the terminal to ensure the new version is loaded.

## Error Handling
- **Download Failure**: Stops execution and provides details on the download error.  
- **Compilation Failure**: Stops execution and provides compiler error details.  
- **File Locking**: Handles cases where the old executable is in use and provides recovery steps.  
- **Critical Errors**: Attempts to restore the old executable if the update fails, ensuring system stability.

## Notes
- The script is designed to be idempotent, ensuring the system remains functional even if the update fails.  
- Manual intervention may be required if critical errors occur during the update process.
```