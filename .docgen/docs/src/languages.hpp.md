# Code Documentation: Language and Format Selection System

## Overview
This module provides a system for selecting programming languages, 3D model formats, and image formats based on file extensions or user input. It leverages a database of language profiles and format specifications to facilitate code generation, model creation, or image processing tasks. The system integrates with Tree-sitter for language parsing and includes a comprehensive database of language and format profiles.

## Core Components

### **1. Generation Modes (`GenMode`)**
- **Purpose**: Defines the context for selection (code, 3D model, or image generation).
- **Values**:
  - `CODE`: For programming language selection.
  - `MODEL_3D`: For 3D model format selection.
  - `IMAGE`: For image format selection.
- **Usage**: Determines which database (`LANG_DB`, `MODEL_DB`, or `IMAGE_DB`) to use during selection.

### **2. Language/Format Profiles (`LangProfile`)**
- **Purpose**: Stores metadata for languages/formats, including file extensions, version commands, build instructions, and optional syntax check commands.
- **Fields**:
  - `id`: Unique identifier (e.g., "cpp", "obj").
  - `name`: Human-readable name (e.g., "C++", "Wavefront OBJ").
  - `extension`: File extension (e.g., ".cpp", ".obj").
  - `versionCmd`: Command to check tool version (e.g., `g++ --version`).
  - `buildCmd`: Command to compile/build (e.g., `g++ -std=gnu++17`).
  - `producesBinary`: Indicates if the output is a binary (e.g., `true` for C++).
  - `checkCmd` (optional): Command to validate syntax (e.g., `python -m py_compile`).

### **3. Tree-sitter Integration**
- **Purpose**: Provides language parsing capabilities using Tree-sitter.
- **Functionality**:
  - `get_ts_language`: Retrieves the Tree-sitter language parser for a given language name.
  - Supports languages: C++, Python, JavaScript, Java, Go, Rust.

### **4. Databases**
- **Purpose**: Maps file extensions to their respective profiles.
- **Databases**:
  - `LANG_DB`: Programming languages (e.g., C++, Python, Rust, and many more).
  - `MODEL_DB`: 3D model formats (e.g., OBJ, STL, glTF).
  - `IMAGE_DB`: Image formats (e.g., SVG, EPS).

### **5. Selection Mechanism (`selectTarget`)**
- **Purpose**: Handles ambiguous targets by prompting the user to select a language/format.
- **Behavior**:
  1. Determines the appropriate database based on `CURRENT_MODE`.
  2. Lists available options in a numbered format (up to 4 per line).
  3. Accepts user input and updates `CURRENT_LANG` with the selected profile.
  4. Defaults to `cpp` (CODE), `obj` (MODEL_3D), or `svg` (IMAGE) if input is invalid.

## Usage
- **Setting Mode**: Update `CURRENT_MODE` to switch between code, 3D model, or image generation contexts.
- **Selecting Target**: Call `selectTarget()` when the target is ambiguous (e.g., multiple file extensions match).
- **Accessing Profile**: Use `CURRENT_LANG` to retrieve the selected profile's metadata.
- **Tree-sitter Parsing**: Use `get_ts_language` to obtain a Tree-sitter parser for a specific language.

## Examples
```cpp
// Set mode to code generation
CURRENT_MODE = GenMode::CODE;

// Select a language (e.g., if file extension is ambiguous)
selectTarget();

// Access selected language profile
string extension = CURRENT_LANG.extension;
string buildCmd = CURRENT_LANG.buildCmd;

// Get Tree-sitter parser for C++
const TSLanguage* cppParser = get_ts_language("cpp");
```

## Notes
- The system is designed to be extensible; new languages/formats can be added to the databases.
- Platform-specific commands (e.g., `g++`, `rustc`) are assumed to be available in the environment.
- Error handling for invalid user input is minimal; defaults are used if input is out of range.
- Tree-sitter integration is limited to languages with available parsers in the provided code.
- The `LangProfile` struct includes an optional `checkCmd` field for syntax validation.