# GlupeHub Interactive CLI Documentation

## Overview
This module provides authentication helpers and an interactive command-line interface (CLI) for interacting with the GlupeHub service. It manages user sessions, executes commands, and handles file operations on the GlupeHub platform.

---

## Session Management

### `getSession()`
**Purpose:** Retrieves the current user session details from the `.glupe_session` file.  
**Returns:** A pair containing the session token and username. If the session file is missing or invalid, returns `{"", ""}`.  
**Usage:** Used internally to check login status and retrieve session details.

### `saveSession(string token, string username)`
**Purpose:** Saves the user's session token and username to the `.glupe_session` file.  
**Behavior:** Overwrites the existing session file with the provided token and username. Logs a success or error message to the console.  
**Usage:** Called after a successful login to persist session data.

### `getSessionToken()`
**Purpose:** Returns the current session token.  
**Usage:** Used to authenticate API requests.

### `getSessionUser()`
**Purpose:** Returns the current session username.  
**Usage:** Used to identify the logged-in user.

### `checkLogin()`
**Purpose:** Verifies if the user is logged in by checking the session token.  
**Behavior:** If not logged in, prints an error message and returns `false`. Otherwise, returns `true`.  
**Usage:** Called before executing commands that require authentication.

---

## Interactive Hub (`startInteractiveHub()`)

**Purpose:** Launches an interactive CLI for managing files and interacting with GlupeHub.  
**Behavior:** Enters a loop where users can input commands. Exits on `exit` or EOF.  

### Available Commands

#### `help`
**Description:** Displays a list of available commands and their usage.  
**Example:**  
```
hub> help
```

#### `search <query>`
**Description:** Searches for files by name.  
**Example:**  
```
hub> search myfile
```

#### `tag <tagname>`
**Description:** Searches for files by tag.  
**Example:**  
```
hub> tag important
```

#### `show <username>`
**Description:** Lists files uploaded by a specific user.  
**Example:**  
```
hub> show john_doe
```
**Output:** Displays a formatted directory listing with file names, sizes, and last modified dates.

#### `view <file_id>`
**Description:** Displays metadata for a specific file.  
**Example:**  
```
hub> view john_doe/file.glp
```
**Output:** Shows metadata fields in a tabular format if available.

#### `pull <file_id>`
**Description:** Downloads a file from GlupeHub.  
**Example:**  
```
hub> pull john_doe/file.glp
```
**Behavior:** Saves the file locally with the same name.

#### `delete <path>`
**Description:** Deletes a file from GlupeHub.  
**Example:**  
```
hub> delete file.glp
```
**Behavior:** Prompts for confirmation before deletion. If the path does not contain a username, the current user's path is prepended.

#### `rename <old> <new>`
**Description:** Renames a file on GlupeHub.  
**Example:**  
```
hub> rename oldfile.glp newfile.glp
```
**Behavior:** If paths do not contain a username, the current user's path is prepended.

---

## Error Handling
- **Authentication Errors:** Commands requiring login will fail if the user is not logged in.  
- **API Errors:** Errors from the GlupeHub API are parsed and displayed as user-friendly messages.  
- **File Operations:** Errors during file operations (e.g., download, delete) are logged and displayed.

---

## Dependencies
- **utils.hpp:** For utility functions like `execCmd`, `log`, and JSON parsing.  
- **nlohmann/json:** For JSON serialization and deserialization.  
- **filesystem (fs):** For file existence checks.  

---

## Usage Example
```bash
$ glupe hub
GlupeHub v-alpha-1.0 MVP
Type 'help' for commands.
hub> help
Available commands:
  search <query>   : Search for files by name.
  tag <tagname>    : Search for files by tag.
  show <username>  : Show files for a user.
  view <file_id>   : View file metadata (e.g., user/file.glp).
  pull <file_id>   : Download a file.
  delete <path>    : Delete a file (e.g. file.glp).
  rename <old> <new>: Rename a file.
  exit             : Exit interactive mode.
hub> search example
# Search results displayed here...
```

This documentation provides a comprehensive overview of the module's functionality, usage, and behavior.