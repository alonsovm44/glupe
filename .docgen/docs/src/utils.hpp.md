# Documentation for `common.hpp` and Associated Utilities

## Overview
This documentation covers the functionality provided by `common.hpp` and its associated utilities, focusing on logging, system utilities, and heuristics for code analysis. The code is designed to be modular, reusable, and platform-independent, with specific considerations for Windows and Unix-like systems.

---

## Logger System

### Purpose
Provides a logging mechanism to track application events, errors, and diagnostics. Logs are appended to a file (`glupe.log`) and optionally printed to the console in verbose mode.

### Components

#### `initLogger()`
- **Purpose**: Initializes the logger by opening the log file in append mode and writes a session start message with the current version and timestamp.
- **Usage**: Call at application startup to prepare the logging system.

#### `log(string level, string message)`
- **Purpose**: Logs a message with a specified severity level (`level`) to the log file and optionally to the console if `VERBOSE_MODE` is enabled.
- **Parameters**:
  - `level`: Severity level (e.g., "INFO", "ERROR", "DEBUG").
  - `message`: The message to be logged.
- **Usage**: Use to record events, errors, or diagnostics during application execution.

---

## System Utilities

### Purpose
Provides utility functions for executing system commands, manipulating file names, and formatting durations.

### Components

#### `execCmd(string cmd)`
- **Purpose**: Executes a system command and captures its output and exit code.
- **Return**: `CmdResult` containing the command's output and exit code.
- **Usage**: Use to run external commands and process their results.

#### `stripExt(string fname)`
- **Purpose**: Removes the file extension from a file name.
- **Return**: File name without the extension.
- **Usage**: Useful for processing file names without their extensions.

#### `getExt(string fname)`
- **Purpose**: Extracts the file extension from a file name.
- **Return**: File extension (including the dot) or an empty string if no extension is found.
- **Usage**: Use to determine the file type or extension.

#### `formatDuration(long long seconds)`
- **Purpose**: Formats a duration in seconds into a human-readable string (e.g., "1m 30s").
- **Return**: Formatted duration string.
- **Usage**: Useful for displaying elapsed time in a user-friendly format.

---

## Heuristics

### Purpose
Provides heuristic functions to detect fatal errors, spaghetti code, and track execution time.

### Components

#### `isFatalError(const string& errMsg)`
- **Purpose**: Determines if an error message indicates a fatal error based on specific keywords.
- **Return**: `true` if the error is fatal, otherwise `false`.
- **Usage**: Use to identify critical errors that require immediate attention.

#### `detectIfCodeIsSpaghetti(const string& code)`
- **Purpose**: Analyzes code for indicators of spaghetti or legacy code (e.g., `goto`, excessive indentation, comments like "don't touch").
- **Return**: `true` if the code is likely spaghetti, otherwise `false`.
- **Usage**: Use to flag code that may need refactoring or review.

#### `ExecutionTimer`
- **Purpose**: Tracks execution time for performance analysis.
- **Behavior**: Starts a high-resolution timer on initialization. The `enabled` flag can be used to control timing.
- **Usage**: Instantiate at the start of a timed operation and calculate elapsed time using `std::chrono` utilities.

---

## Global Flags and Constants

### Purpose
Provides global flags and constants for controlling application behavior and versioning.

### Components

#### `VERBOSE_MODE`
- **Purpose**: Enables or disables verbose logging to the console.
- **Type**: `bool` (inline for header-only support).

#### `CURRENT_VERSION`
- **Purpose**: Stores the current version of the application.
- **Type**: `const string` (inline for header-only support).

---

## Platform-Specific Considerations

- **Windows**: Uses `windows.h` and defines `_popen` and `_pclose` for command execution.
- **Unix-like**: Uses `unistd.h` and `limits.h` for command execution.

---

## JSON Library

- **Purpose**: Includes the JSON library for handling JSON data.
- **Namespace**: `nlohmann::json` is aliased as `json`.

---

## Usage Example

```cpp
#include "common.hpp"

int main() {
    initLogger();
    log("INFO", "Application started.");
    
    CmdResult result = execCmd("ls -l");
    log("DEBUG", "Command output: " + result.output);
    
    if (isFatalError(result.output)) {
        log("ERROR", "Fatal error detected.");
    }
    
    log("INFO", "Application finished.");
    return 0;
}
```

This example demonstrates initializing the logger, executing a system command, logging the output, and checking for fatal errors.