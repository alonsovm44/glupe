# Yori Compiler Documentation (v4.9.1)

## Overview
Yori is an AI-powered meta-compiler that translates natural language, mixed-syntax code, or multi-language source files into executable binaries or target source code. It handles the entire build pipeline: context aggregation, AI generation, dependency verification, compilation, and error correction.

## Usage Syntax
```bash
yori <input_files...> [-o output] [flags]
```

## Core Commands

### 1. Build / Compile
The default mode when running `yori` with input files.
```bash
yori main.yori -o app.exe -cpp -cloud
```

### 2. AI Code Repair (`fix`)
Applies AI-driven fixes to an existing file based on natural language instructions.
```bash
yori fix <file> "instruction" [-cloud/-local]
```
*Example:* `yori fix main.cpp "Change the loop to use iterators" -local`

### 3. Configuration (`config`)
Manages the `config.json` file for AI providers and toolchains.
```bash
yori config <key> <value>
```
*   **Keys**:
    *   `api-key`: Set Cloud API Key.
    *   `cloud-protocol`: Set protocol (`google`, `openai`).
    *   `model-cloud`: Set Cloud Model ID (e.g., `gemini-1.5-flash`).
    *   `url-cloud`: Set Cloud API Endpoint.
    *   `url-local`: Set Local API Endpoint (default: `http://localhost:11434/api/generate`).
    *   `model-local`: Set Local Model ID.
*   **Interactive Mode**: `yori config model-local` scans for installed Ollama models.

### 4. API Key Management
Opens the Google AI Studio page to generate keys.
```bash
yori get-key
```

## Compilation Flags

### Build Control
*   `-o <file>`: Specify output filename. If omitted, defaults to input name with target extension.
*   `-u` / `--update`: **Update Mode**. Reads the existing output source file (if present) and modifies it based on new input/instructions, rather than generating from scratch.
*   `-t` / `--transpile`: **Transpilation Mode**. Forces the output to be a text file (source code) instead of a binary, even for compiled languages.
*   `-k` / `--keep`: **Keep Source**. Preserves the generated source code file (e.g., `.cpp`) alongside the binary. (Implied by `-u`).
*   `-run` / `--run`: **Run Immediately**. Executes the output binary or script after a successful build.
*   `--clean`: Removes temporary build files (`temp_build*`, `.yori_build.cache`).

### AI Provider
*   `-cloud`: Uses the configured cloud provider (Google/OpenAI). Generally faster and smarter.
*   `-local`: Uses the local Ollama instance. Private and offline. (Default).

### Debugging & Info
*   `-dry-run`: Prints the aggregated context that would be sent to the AI, then exits.
*   `-verbose`: Enables detailed logging of API requests and internal states.
*   `--version`: Displays current version.

## Language Support
Yori automatically detects the target language based on the output extension (e.g., `-o app.py` -> Python). You can also force a language using flags:

| Flag | Language | Extension | Binary? |
| :--- | :--- | :--- | :--- |
| `-cpp` | C++ | `.cpp` | Yes |
| `-c` | C | `.c` | Yes |
| `-rust` | Rust | `.rs` | Yes |
| `-go` | Go | `.go` | Yes |
| `-py` | Python | `.py` | No |
| `-js` | JavaScript | `.js` | No |
| `-ts` | TypeScript | `.ts` | No |
| `-cs` | C# | `.cs` | Yes |
| `-java` | Java | `.java` | No |
| `-zig` | Zig | `.zig` | Yes |
| `-sh` | Bash | `.sh` | No |
| ... | *And many more (Lua, PHP, Ruby, Swift, Kotlin, etc.)* | | |

## Universal Features

### 1. Universal Imports (`IMPORT:`)
Yori supports a custom import syntax that works across all input file types. It resolves the file path, reads the content, and embeds it into the context sent to the AI.
```yori
IMPORT: utils/math.cpp
IMPORT: "libs/logger.py"
```
*   Handles cyclic dependencies automatically.
*   Supports relative paths.

### 2. Preprocessor Directives
Control the AI's creativity level using comment directives in your source code.
*   `// !!!` **Strict/Literal**: "Do exactly what the code says. No improvisation."
*   `// >>>` **Creative/Design**: "Infer intent, optimize, or fill in gaps."
*   `// ???` **Unsure**: "Proceed with caution, ambiguous intent."

### 3. Multi-File Linking
You can pass multiple files of different languages. Yori aggregates them into a single context, allowing the AI to understand the full project logic and generate a unified output.
```bash
yori logic.py main.c utils.js -o app.exe -cpp
```

## Advanced Mechanics

### Pre-Flight Dependency Check
Before calling the AI, Yori scans the input for potential dependencies (e.g., `#include`, `import`). For C/C++, it attempts a local compilation check to verify if headers exist on the system. If missing, it aborts early to save tokens/time.

### Self-Healing Build Loop
1.  **Generate**: AI writes code.
2.  **Verify**: Yori attempts to compile/run the code using the system toolchain.
3.  **Feedback**: If compilation fails, the error log is fed back to the AI.
4.  **Iterate**: The AI generates a fixed version. This repeats up to `MAX_RETRIES` (15).

### Caching
Yori maintains a `.yori_build.cache` file. If the input files, target language, and model haven't changed since the last successful build, it skips generation and reuses the previous output.