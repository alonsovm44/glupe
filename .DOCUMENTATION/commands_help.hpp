#pragma once

constexpr const char* COMMANDS_HELP_DOC = R"GLUPE(
===============================================================================
                        GLUPE COMMANDS & FLAGS GUIDE
===============================================================================

This document lists all essential commands, flags, and common combinations 
to help new users navigate the Glupe Semantic Compiler.

-------------------------------------------------------------------------------
1. CORE COMPILATION & GENERATION
-------------------------------------------------------------------------------
Basic usage: glupe [files...] [options] ["*instructions"]

  glupe main.glp -local         # Compile using local AI (Ollama)
  glupe main.glp -cloud         # Compile using cloud AI (OpenAI/Google)
  glupe main.glp -o app.exe     # Specify the output filename
  glupe main.glp -cpp           # Force target language (e.g., -cpp, -py, -rust)
  glupe main.glp -t             # Transpile only (generate code, do not build binary)
  
  [experimental]
  glupe main.glp -3d            # Generate 3D models (e.g., .obj, .stl)
  glupe main.glp -img           # Generate images (e.g., .svg)

-------------------------------------------------------------------------------
2. ARCHITECTURE & MULTI-FILE MODES
-------------------------------------------------------------------------------
These flags dictate HOW Glupe interprets your files and interacts with the host.

  -make        : Architect Mode. Reads `EXPORT:` blocks and generates entire 
                 multi-file projects, creating directories automatically.
  -series      : Sequential Generation. Generates files one-by-one to prevent 
                 AI context fatigue. Often paired with `-make`.
  -fill        : Fill Mode. Surgically injects code ONLY into `$${...}$$` 
                 semantic containers, leaving your manual architecture untouched.
  -u, --update : Update Mode. Reads the existing output file and edits it 
                 incrementally, utilizing the semantic cache where possible.
  -refine      : Refine Mode. Reverse-engineers existing, messy source code 
                 into a clean `.glp` semantic blueprint.
  -scaffold    : Scaffold Mode (used with -refine). Converts plain-text 
                 requirements into a full `.glp` architectural blueprint.

-------------------------------------------------------------------------------
3. EXECUTION & ZERO-TRUST SANDBOXING
-------------------------------------------------------------------------------
  -run         : Compile and immediately execute the output binary/script.
  --sandbox    : [EXPERIMENTAL] Runs the compilation/build tools AND the 
                 resulting execution inside a secure Docker container.

-------------------------------------------------------------------------------
4. ESSENTIAL COMBINATIONS (PRO TIPS)
-------------------------------------------------------------------------------
  The "Safe Implementation"
  $ glupe main.cpp -fill -local
  (AI safely implements missing container logic without rewriting your C++ file)

  The "Full Project Architect"
  $ glupe project.glp -make -series -cloud
  (Generates a multi-file project sequentially using maximum cloud reasoning)

  The "Zero-Trust Build & Run"
  $ glupe main.cpp -o app.exe --sandbox -run
  (Compiles and executes the AI-generated code securely inside Docker)

  The "Legacy Modernization"
  $ glupe legacy.c -refine -cloud
  (Converts an old C file into a semantic .glp blueprint)

-------------------------------------------------------------------------------
5. UTILITY COMMANDS
-------------------------------------------------------------------------------
Glupe acts as a full AI-developer toolchain:

  fix          : AI-powered code repair.
                 $ glupe fix bug.py "fix index out of range" -local

  explain      : Generates heavily commented documentation for a file.
                 $ glupe explain main.cpp -cloud English

  diff         : Generates a semantic markdown report of changes between files.
                 $ glupe diff v1.py v2.py -cloud

  sos          : Ask the AI for terminal help or debugging advice.
                 $ glupe sos cpp "error: no matching function for call"

  audit        : Verifies if an implementation diverges from a specification.
                 $ glupe audit spec.glp impl.glp --output report.json

  check        : Validates the syntax of Glupe containers in a file.
                 $ glupe check file.glp

  edit         : Edits a specific container's prompt directly from the CLI.
                 $ glupe edit file.cpp --container "init" "new instructions"

-------------------------------------------------------------------------------
6. FLAGS & MODIFIERS
-------------------------------------------------------------------------------
  --feedback <file> : Reads an audit JSON report to auto-heal missing logic.
  -i, --interactive : Prompts the user if the AI finds the intent ambiguous.
  -dry-run          : Shows the assembled prompt/context without calling the API.
  -crono            : Measures execution time.
  -verbose          : Enables detailed logging (great for debugging).
  --clean           : Removes temporary build files.
  --init            : Generates a sample `hello.glp` and `config.json`.

-------------------------------------------------------------------------------
7. CONFIGURATION & GLUPEHUB
-------------------------------------------------------------------------------
  config <key> <val>     : Update config (e.g., `glupe config api-key "KEY"`).
  config model-local     : Interactively select an installed Ollama model.
  config see             : Print the current configuration.
  
  clean cache            : Clears the semantic cache (`.glupe.lock`).
  update                 : Checks and applies updates from the master repository.

  hub                    : Enter the interactive GlupeHub CLI mode.
  login / signup / logout: Authentication for GlupeHub.
  push <file> [tags]     : Upload a blueprint to GlupeHub.
  pull <file> <user>     : Download a blueprint from GlupeHub.
  info <file.glp>        : Show metadata for a file.
  insert-metadata <file> : Prepends a metadata block template to a file.

===============================================================================
)GLUPE";