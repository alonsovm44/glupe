Below is a **clean, structured, comprehensive Markdown documentation** derived from the changelog you provided.  
I focus on **purpose, behavior, and usage patterns**, not on restating obvious version‑log details.

---

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

The following sections summarize the system’s functional domains and how they behave.

---

# Core Concepts

## 1. **Modes of Operation**

### **Refine Mode (`-refine`)**
Refine mode iteratively improves existing code.  
Recent additions include:

- **Scaffold generation**: The tool can now produce export‑block project structures before refinement begins.
- **Chunk‑based refinement fixes**: Ensures large files are processed reliably without losing context.

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

**Purpose:**  
To structure AI‑generated code into reusable, composable modules.

---

## 3. **Import & Export System**

### Import Enhancements
- Embedded code parsing inside comments (`// /**/`)
- Explicit import markers for clarity
- IMPORT/END blocks for controlled inclusion

### Export Enhancements
- Context detection: If content appears outside an export block, the tool requests a target language.
- Scaffold generation for project layout

**Purpose:**  
To give the AI a deterministic, structured context for multi‑file generation.

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