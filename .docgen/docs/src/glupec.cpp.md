# GLUPE Compiler Documentation

## Overview
The GLUPE Compiler is a semantic compiler that translates high-level instructions, blueprints, or requirements into executable code, 3D models, or images. It supports multiple programming languages, 3D modeling formats, and image formats through a unified interface.

## Core Features
- **Multi-Language Support**: Compiles to various programming languages (C++, Python, etc.), 3D formats (OBJ, STL), and image formats (SVG).
- **Semantic Compilation**: Translates natural language instructions or blueprints into code.
- **AI-Assisted Development**: Uses AI models (local or cloud-based) to generate, refine, and repair code.
- **Project Scaffolding**: Generates multi-file project structures from high-level requirements.
- **Code Refinement**: Converts legacy or spaghetti code into semantic blueprints.
- **Asset Generation**: Creates 3D models and images based on textual descriptions.
- **Caching**: Reuses successful builds to speed up recompilation.
- **Error Handling**: Detects fatal errors and provides AI-assisted explanations.
- **Interactive Mode**: Allows user clarification for ambiguous containers.
- **IDE Integration**: Supports interactive mode for IDEs with ambiguous prompt handling.
- **Direct Compilation**: Attempts direct compilation for matching source files.
- **Series Mode**: Generates files sequentially based on a blueprint.
- **Tree Shaking**: Removes unused code from AI-generated output.
- **Blind Mode**: Skips verification for unsupported toolchains.
- **Custom Build Commands**: Override default build commands with `-build`.
- **Audit Mode**: Verifies implementation against specification using semantic analysis.
- **Feedback Integration**: Automatically fixes issues reported in audit JSON reports.

## Command-Line Interface (CLI)

### Core Options
- `-o <file>`: Specify the output filename.
- `-cloud`: Use cloud-based AI provider (configured in `config.json`).
- `-local`: Use local AI provider (Ollama).
- `-u, --update`: Update mode (edits existing files instead of overwriting).
- `-make`: Architect mode (generates multi-file projects sequentially).
- `-refine`: Refine mode (reverse engineer code to `.glp` blueprint).
- `-scaffold`: Generate project structure from requirements.
- `-t, --transpile`: Transpile only (do not compile binary).
- `-run`: Run the output binary after compilation.
- `-crono`: Measure execution time.
- `-fill`: Fill containers in-place (preserves manual code).
- `--feedback <file>`: Use audit JSON report to auto-heal missing/hallucinated logic.
- `-i, --interactive`: Enable interactive mode for ambiguous containers.
- `--ide`: Enable IDE mode (interactive with special ambiguous prompt handling).
- `-dry-run`: Show prompt/context without calling AI.
- `-verbose`: Enable verbose logging.
- `-3d`: 3D model generation mode.
- `-img`: Image generation mode.
- `--clean`: Remove temporary build files.
- `--init`: Initialize project (creates `hello.glp` and `config.json`).

### Commands
- `config <key> <val>`: Update configuration.
- `config model-local`: Interactive local model selection.
- `clean cache`: Clear semantic cache.
- `edit <file> --container <name> "prompt"`: Edit a container's prompt.
- `check <file>`: Validate syntax of a `.glp` file.
- `fix <file> "instr"`: AI-powered code repair.
- `explain <file> [lang]`: Generate documentation.
- `diff <f1> <f2> [lang]`: Semantic diff report.
- `audit <spec> <impl> [--ignore-scaffold] [--output <file.json>]`: Verify implementation against specification.
- `sos [lang] "query"`: Ask AI for help.
- `update`: Check for and apply updates to GLUPE.
- `hub`: Enter interactive GlupeHub mode.
- `login / signup / logout`: GlupeHub authentication.
- `push <file> [tags]`: Upload to GlupeHub.
- `pull <file> <user>`: Download from GlupeHub.
- `info <file.glp>`: Show file metadata.
- `insert-metadata <path>`: Insert metadata template.
- `ast-test <file>`: Test Tree-sitter AST parsing.

### Examples
```bash
glupe main.glp -o app.exe -cpp
glupe idea.txt -make -series
glupe legacy.c -refine
glupe fix bug.py "fix index out of range"
glupe explain main.cpp -local English
glupe diff v1.cpp v2.cpp -cloud Spanish
glupe audit spec.glp impl.glp --ignore-scaffold --output report.json
glupe sos Python "How to handle CSV files?"
glupe update
glupe hub
glupe login
glupe push project.glp "demo, tutorial"
glupe pull example.glp john_doe
glupe info project.glp
glupe insert-metadata new_project/
glupe ast-test main.cpp
```

## Configuration
GLUPE uses a `config.json` file to store settings for local and cloud AI providers, toolchains, and other preferences. The `loadConfig` function reads this file and sets global variables accordingly.

### Configuration Keys
- `api-key`: Cloud API key.
- `max-retries`: Maximum number of API retries (default: 15).
- `cloud-protocol`: Protocol for cloud AI (`google`, `openai`, `ollama`).
- `model-cloud`: Cloud model ID.
- `url-cloud`: Cloud API URL.
- `model-local`: Local model ID.
- `url-local`: Local API URL.

## AI Integration
GLUPE integrates with AI models via the `callAI` function, which sends prompts to the configured AI provider and retrieves responses. The `extractCode` function parses the response to extract the generated code.

## Caching
The compiler uses a cache to store successful builds, avoiding redundant AI calls and recompilation. The cache is stored in `.glupe_build.cache` and is invalidated when input files change.

## Error Handling
GLUPE detects fatal errors (e.g., missing files, undefined references) and provides AI-assisted explanations. The `isFatalError` function checks for common error patterns, and `explainFatalError` provides detailed insights.

## Audit Mode
The `audit` command compares a specification `.glp` file with an implementation, generating a JSON report of missing, hallucinated, or mismatched logic. The `--feedback` flag allows automatic fixing of reported issues.

## Project Structure
- **common.hpp**: Common utilities, structures, and global flags.
- **utils.hpp**: Logger, system utilities, heuristics, and execution timer.
- **config.hpp**: Configuration management.
- **languages.hpp**: Language database and toolchains.
- **ai.hpp**: AI integration and prompt handling.
- **cache.hpp**: Caching mechanisms.
- **parser.hpp**: Input parsing and blueprint handling.
- **processor.hpp**: Code processing and generation.
- **ast_utils.hpp**: AST-based code analysis and manipulation.
- **hub.hpp**: GlupeHub integration.

## Usage Workflow
1. **Initialize Project**: Use `--init` to create a basic project structure.
2. **Write Instructions**: Create `.glp` files or use plain text files with instructions.
3. **Compile**: Run `glupe <file>` with desired options.
4. **Refine/Repair**: Use `-refine` or `fix` commands to improve code.
5. **Audit**: Use `audit` to verify implementations against specifications.
6. **Deploy**: Use `-run` to execute the generated binary or view assets.

## Advanced Features
- **Series Mode**: Generates files sequentially based on a blueprint.
- **Tree Shaking**: Removes unused code from AI-generated output.
- **Blind Mode**: Skips verification for unsupported toolchains.
- **Custom Build Commands**: Override default build commands with `-build`.
- **Interactive Mode**: Allows user clarification for ambiguous containers.
- **IDE Integration**: Supports interactive mode for IDEs with ambiguous prompt handling.
- **Audit Feedback Integration**: Automatically fixes issues reported in audit reports.

## Troubleshooting
- **API Errors**: Check `config.json` for correct API keys and URLs.
- **Compilation Failures**: Use `-verbose` for detailed logging.
- **Cache Issues**: Clear cache with `clean cache`.
- **Audit Failures**: Use `--feedback` to automatically fix reported issues.

## Contributing
Contributions are welcome! Fork the repository, make changes, and submit a pull request. Ensure all changes are well-documented and tested.

## License
GLUPE is released under the MIT License. See `LICENSE` for details.