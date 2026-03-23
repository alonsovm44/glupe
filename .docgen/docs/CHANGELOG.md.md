# Project Documentation  
*A high‑level technical overview of the system’s capabilities, workflows, and evolution.*

## Overview

This project is a **multi‑language AI‑assisted code generation and build automation tool**.  
It integrates LLM‑driven refinement, multi‑file orchestration, build‑system detection, containerized code structures, and direct compilation paths to minimize token usage and reduce human intervention.

Over time, the tool has evolved from a simple code generator into a **full development pipeline** capable of:

- Parsing, generating, refining, and updating multi‑file projects  
- Managing container‑based abstractions and inheritance  
- Detecting build systems and executing them automatically  
- Synchronizing source and binary artifacts  
- Providing diagnostics, diffs, explanations, and automated fixes  
- Supporting 20+ programming languages and embedded code blocks  
- Integrating authentication, remote operations, and caching  
- Generating 3D assets and supporting advanced code analysis

The following sections summarize the system’s functional domains and how they behave.

---

# Core Concepts

## 1. **Modes of Operation**

### **Refine Mode (`-refine`)**
Refine mode iteratively improves existing code.  
Recent additions include:

- **Scaffold generation**: The tool can now produce export‑block project structures before refinement begins.
- **Chunk‑based refinement fixes**: Ensures large files are processed reliably without losing context.
- **AST-Driven source code slicing**: Prevents code fracturing during refinement.
- **Tree-sitter C++ parsing**: Enhances context understanding for C++ code.

**Purpose:**  
To evolve existing codebases without regenerating them from scratch.

---

### **Series Mode (`-series`)**
Processes multi‑file projects **sequentially** to avoid LLM context fatigue.

**Behavior:**

- Files are generated or refined one at a time.
- API errors no longer interrupt the sequence.
- Ensures consistent architecture across large codebases.

**Use Case:**  
Large projects where global context must be preserved without overwhelming the model.

---

### **Make Mode (`-make`)**
A build‑oriented mode that:

- Scans export blocks and creates the required directory/file structure.
- Detects build systems (Makefile, CMake, scripts) and executes them.
- Supports language‑agnostic defaults.
- Integrates with `-run` to build before execution.
- Added `--ignore-scaffold` flag for skipping scaffold generation.

**Purpose:**  
To turn AI‑generated project descriptions into runnable software automatically.

---

### **Run Mode (`-run`)**
Executes the compiled binary **only after a successful build** when used with Architect Mode.

**Purpose:**  
To ensure execution never occurs on stale or invalid binaries.

---

## 2. **Containers & Abstractions**

The system uses **containers** as modular code units.

### Features:
- Named containers with collision detection  
- Multiple inheritance with edge‑case handling  
- Abstract containers  
- Inline containers with syntax validation  
- Semantic vectors for advanced context modeling  
- `.yori.lock` for container hashing  
- Cached containers for faster rebuilds  
- Semantic equivalence verification loop  
- Global symbol graph using Tree-sitter  

**Purpose:**  
To structure AI‑generated code into reusable, composable modules.

---

## 3. **Import & Export System**

### Import Enhancements
- Embedded code parsing inside comments (`// /**/`)
- Explicit import markers for clarity
- IMPORT/END blocks for controlled inclusion
- Directory sucking for bulk imports

### Export Enhancements
- Context detection: If content appears outside an export block, the tool requests a target language.
- Scaffold generation for project layout

**Purpose:**  
To give the AI a deterministic, structured context for multi-file generation.

---

## 4. **Compilation & Build Pipeline**

### Direct Compilation
When all input files share a language, the system:

1. Attempts native compilation first  
2. Only invokes the LLM if compilation errors occur  

**Purpose:**  
To reduce token usage and accelerate iteration.

### Build Caching
- Caches build artifacts to avoid redundant work
- Supports multi‑file caching

### Configurable Toolchains
- Build commands can be overridden via `config.json`
- `-build` flag allows overriding default build command

---

## 5. **Developer Assistance Tools**

### **Fix Command**
```
yori fix "file.ext" "description" [-local | -cloud]
```
Rewrites a file according to a natural‑language description.

### **Explain Command**
```
yori explain <path> [-cloud | -local] [language]
```
Generates a natural‑language explanation of a file.

### **Diff Command**
```
yori diff <pathA> <pathB> [-cloud | -local] [language]
```
Produces a structured Markdown diff with entity‑level analysis.

### **SOS Command**
```
yori sos [language] [-local | -cloud] "prompt"
```
Requests targeted help from the model.

### **Audit Command**
```
yori audit [--ignore-scaffold]
```
Verifies semantic equivalence and project structure.

### **See Command**
```
yori config see
```
Displays current configuration.

### **Max Retries**
```
yori config max-retries <value>
```
Controls the healing loop iteration count.

---

## 6. **Authentication & Remote Operations**

### JWT Authentication
Enables secure communication with remote endpoints.

### Login / Push / Pull
- Push and pull commands with improved error reporting
- Integration with remote hubs for project synchronization

**Purpose:**  
To support cloud‑based workflows and team collaboration.

---

## 7. **Language & Platform Support**

- 20+ programming languages  
- Arduino and ESP32 support  
- Kotlin support  
- Flex/Bison support for Glupe  
- Glupe IR integration  
- Windows, macOS, and Linux installers  

**Purpose:**  
To make the tool usable across diverse ecosystems.

---

## 8. **3D Asset Generation (`-3D`)**
Supports AI‑generated 3D files:

- OBJ  
- STL  
- PLY  
- GLTF  
- SVG  

Example:
```
yori input.ext -o output.ext2 -computing_mode -3d "*instructions"
```

**Purpose:**  
To extend the system beyond source code into asset pipelines.

---

## 9. **Error Handling & Stability**

### Fatal Error Handling
Unrecoverable errors abort the operation and offer analysis options.

### Error Memory
The system remembers previous errors to avoid repeating them.

### Pre‑Flight Checks
- Enhanced dependency checks before token generation.
- Semantic equivalence verification loop.

---

## 10. **New Features (v6.x)**

### **Directory Sucking**
Automatically imports entire directories for bulk processing.

### **Audit Command**
Verifies project structure and semantic equivalence.

### **Global Symbol Graph**
Uses Tree-sitter for advanced code analysis and context modeling.

### **Targeted Dependency Injection**
Drastically reduces API token usage by injecting dependencies directly.

**Purpose:**  
To enhance project stability, reduce token usage, and improve code analysis.