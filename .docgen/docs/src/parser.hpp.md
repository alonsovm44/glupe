### `decommentGlupeSyntax` Function

**Purpose:**  
Extracts and processes Glupe syntax embedded within comments in the source code. This function handles block comments (`/* ... */`), line comments (`// ...`), and Glupe-specific comments (`%{ ... }%` and `% ...`). It identifies and preserves Glupe syntax containers (both block and inline) while removing other comment content.

**Behavior:**  
1. **Block Comments (`/* ... */`):**  
   - Scans for block comments and extracts their content.  
   - Preserves content if it starts and ends with `$$` (block container) or `$` (inline container).  
   - Removes other comment content.  

2. **Line Comments (`// ...`):**  
   - Scans for line comments and extracts their content.  
   - Preserves content if it starts and ends with `$` (inline container).  
   - Removes other comment content.  

3. **Glupe Block Comments (`%{ ... }%`):**  
   - Scans for Glupe block comments and extracts their content.  
   - Preserves content if it starts and ends with `$$` (block container) or `$` (inline container).  
   - Removes other comment content.  

4. **Glupe Line Comments (`% ...`):**  
   - Scans for Glupe line comments and extracts their content.  
   - Preserves content if it starts and ends with `$` (inline container).  
   - Removes other comment content.  

**Usage:**  
This function is used to preprocess code before further analysis or execution, ensuring that Glupe syntax is correctly identified and isolated from regular comments.

---

### `resolveImports` Function

**Purpose:**  
Resolves import directives (`IMPORT:`) in the code by recursively processing imported files. It handles both single-line and block imports, detects cyclic imports, and manages local modifications within import blocks.

**Behavior:**  
1. **Import Detection:**  
   - Scans for `IMPORT:` directives and extracts the file name.  
   - Supports quoted and unquoted file names.  

2. **Import Resolution:**  
   - Locates the imported file relative to the base path.  
   - Recursively processes the imported file, adding it to the import stack to detect cycles.  
   - Handles local modifications within `IMPORT: END` blocks.  

3. **Error Handling:**  
   - Detects cyclic imports and logs errors.  
   - Handles missing imports with warnings.  

**Usage:**  
This function is used to preprocess code that relies on external modules or files, ensuring all dependencies are resolved before further processing.

---

### `stripAllContainers` Function

**Purpose:**  
Removes all Glupe syntax containers from the code using AST parsing. This function ensures that only raw code remains, excluding any Glupe-specific syntax.

**Behavior:**  
1. **AST Parsing:**  
   - Uses Flex/Bison to parse the code into an AST.  
   - Identifies and skips `ContainerNode` and `VariableNode` elements.  

2. **Code Reconstruction:**  
   - Rebuilds the code from `RawCodeNode` elements only.  

**Usage:**  
This function is used to strip Glupe syntax for validation, execution, or further processing that requires raw code.

---

### `validateContainers` Function

**Purpose:**  
Validates Glupe syntax containers in the code, ensuring they are properly named, unique, and correctly structured. It detects malformed containers and duplicate IDs.

**Behavior:**  
1. **AST Parsing:**  
   - Parses the code into an AST to identify container nodes.  

2. **Validation:**  
   - Ensures container IDs are unique.  
   - Checks for unclosed containers and malformed syntax.  
   - Tracks active (non-abstract) containers.  

3. **Error Reporting:**  
   - Logs errors for duplicate IDs and malformed syntax.  

**Usage:**  
This function is used to ensure the structural integrity of Glupe syntax containers before further processing or execution.

---

### `processExports` Function

**Purpose:**  
Processes `EXPORT:` directives in the code, writing specified content to files. It handles file creation, directory management, and template stripping during export.

**Behavior:**  
1. **Export Detection:**  
   - Scans for `EXPORT:` directives and extracts the file name.  
   - Supports quoted and unquoted file names.  

2. **File Writing:**  
   - Creates directories if necessary.  
   - Writes content to the specified file, stripping templates using `stripAllContainers`.  

3. **Error Handling:**  
   - Logs errors if file writing fails.  

**Usage:**  
This function is used to export generated code or content to files based on `EXPORT:` directives embedded in the code.

---

### `extractDependencies` Function

**Purpose:**  
Extracts dependencies from the code, including missing imports and `#include` directives. This function helps identify external dependencies for dependency checking.

**Behavior:**  
1. **Dependency Detection:**  
   - Scans for `// [WARN] IMPORT NOT FOUND:` lines.  
   - Parses `#include` directives for C/C++ dependencies.  

2. **Extraction:**  
   - Collects dependencies into a set for uniqueness.  

**Usage:**  
This function is used to gather dependencies for pre-flight checks or dependency resolution.

---

### `preFlightCheck` Function

**Purpose:**  
Performs a pre-flight check to verify that all dependencies are available locally. It compiles a temporary file containing the dependencies and checks for missing ones.

**Behavior:**  
1. **Dependency Compilation:**  
   - Creates a temporary file with dependency code.  
   - Compiles the file using the language's build command.  

2. **Error Detection:**  
   - Identifies missing dependencies from compilation errors.  
   - Logs warnings and errors for missing dependencies.  

3. **Cleanup:**  
   - Removes temporary files after checking.  

**Usage:**  
This function is used to ensure all dependencies are present before proceeding with code execution or further processing.

---

### `splitSourceCode` Function

**Purpose:**  
Splits source code into semantic chunks for refined processing, such as AI-assisted code generation or analysis. This function ensures chunks are manageable in size while preserving code structure.

**Behavior:**  
1. **Chunking:**  
   - Splits code into chunks based on a target line count.  
   - Ensures chunks are split at safe points (e.g., top-level code).  

2. **Brace Balancing:**  
   - Tracks brace balance to avoid splitting within nested structures.  

**Usage:**  
This function is used to prepare code for refined processing, such as AI-assisted refactoring or generation.

---

### `sliceSourceCodeAST` Function

**Purpose:**  
Splits source code into semantic chunks using Tree-sitter AST parsing. This function ensures chunks are logically separated based on code structure.

**Behavior:**  
1. **AST Parsing:**  
   - Uses Tree-sitter to parse the code into an AST.  
   - Identifies structural blocks (functions, classes, etc.).  

2. **Chunking:**  
   - Groups smaller nodes (macros, includes) into header chunks.  
   - Separates major structural blocks into individual chunks.  

**Usage:**  
This function is used for advanced code slicing, leveraging AST for precise chunking.

---

### `compareASTComplexity` Function

**Purpose:**  
Compares the structural complexity of two code versions using AST diffing. This function helps detect oversimplification or loss of complexity.

**Behavior:**  
1. **AST Parsing:**  
   - Parses both original and new code into ASTs.  

2. **Complexity Analysis:**  
   - Counts critical structural nodes (functions, classes, etc.).  
   - Calculates the ratio of new to original complexity.  

**Usage:**  
This function is used to validate that code refinements maintain structural integrity.

---

### `buildGlobalSymbolGraph` Function

**Purpose:**  
Builds a global symbol graph from the code, mapping symbols to their signatures. This function aids in context generation and symbol tracking.

**Behavior:**  
1. **AST Parsing:**  
   - Uses Tree-sitter to parse the code into an AST.  
   - Identifies functions, classes, and other symbols.  

2. **Graph Construction:**  
   - Maps symbol names to their signatures.  

**Usage:**  
This function is used to create a symbol graph for context-aware processing.

---

### `getRelevantContext` Function

**Purpose:**  
Extracts relevant context from a code chunk based on a global symbol graph. This function ensures that related symbols are included in the context.

**Behavior:**  
1. **Symbol Matching:**  
   - Scans the chunk for symbol names present in the graph.  
   - Includes matching symbol signatures in the context.  

**Usage:**  
This function is used to generate context for AI models or other tools requiring symbol-aware processing.

---

### `extractSignatures` Function

**Purpose:**  
Extracts function, class, and other signatures from the code for context generation. This function helps provide relevant context for AI-assisted tasks.

**Behavior:**  
1. **Signature Detection:**  
   - Scans for function definitions, class declarations, and other relevant constructs.  
   - Ignores comments and irrelevant lines.  

2. **Extraction:**  
   - Collects signatures into a single string.  

**Usage:**  
This function is used to generate context for AI models or other tools that require code signatures.

---

### `stripMetadata` Function

**Purpose:**  
Removes metadata blocks (`META_START ... META_END`) from the code, leaving only the functional code.

**Behavior:**  
1. **Metadata Detection:**  
   - Locates `META_START` and `META_END` markers.  
   - Removes the content between these markers.  

**Usage:**  
This function is used to clean up code before execution or further processing, removing non-functional metadata.

---

### `showMetadata` Function

**Purpose:**  
Displays metadata associated with a file, including inferred metadata and detected dependencies.

**Behavior:**  
1. **Metadata Extraction:**  
   - Parses `META_START ... META_END` blocks for JSON metadata.  
   - Infers metadata if no block is found.  

2. **Dependency Detection:**  
   - Extracts dependencies using `extractDependencies`.  

3. **Display:**  
   - Prints metadata in a formatted JSON structure.  

**Usage:**  
This function is used to inspect metadata associated with a file, aiding in debugging or documentation.

---

### `sanitize_container_syntax` Function

**Purpose:**  
Sanitizes Glupe container syntax in the code, fixing common issues such as malformed starts, ends, and multiline inline containers.

**Behavior:**  
1. **Syntax Fixing:**  
   - Corrects `${` to `$ {`.  
   - Fixes `}$` to `} $` (unless part of `}$$`).  
   - Flattens multiline inline containers.  
   - Ensures block containers have proper opening and closing syntax.  

2. **Stray Character Removal:**  
   - Removes unpaired `$` characters.  

**Usage:**  
This function is used to clean up Glupe syntax after refinement or generation, ensuring correctness and consistency.

---

### `parseBlueprint` Function

**Purpose:**  
Parses a blueprint (series of `EXPORT:` directives) into a structured list of files and their content.

**Behavior:**  
1. **Blueprint Parsing:**  
   - Scans for `EXPORT:` directives and extracts file names and content.  
   - Handles multiline content within export blocks.  

2. **Entry Creation:**  
   - Creates `BlueprintEntry` objects for each file.  

**Usage:**  
This function is used to process blueprints for sequential code generation or file creation.

---

### `get_refine_query` Function

**Purpose:**  
Generates a Tree-sitter query for refining code based on the language ID. This query is used to identify semantic units in the code for targeted refinement.

**Behavior:**  
1. **Query Generation:**  
   - Returns language-specific queries for C++, Python, JavaScript, TypeScript, Java, and Go.  
   - Covers function definitions, class declarations, imports, and other relevant constructs.  

**Usage:**  
This function is used to prepare queries for Tree-sitter-based code analysis or refinement.