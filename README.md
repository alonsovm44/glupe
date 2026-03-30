# Glupe

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Release](https://img.shields.io/github/v/release/alonsovm44/glupe)](https://github.com/alonsovm44/glupe/releases)
![C++17](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B)
![Platforms](https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-lightgrey)
![AI-Powered](https://img.shields.io/badge/AI-Powered-purple)

## 🎬 Glupe in Action

| Generate | Build | Fix Errors |
|----------|-------|------------|
| ![](./assets/demo.gif) | ![](./assets/demo2.gif) | ![](./assets/demo3.gif) |

> **Glupe is a Semantic Compiler. Think of it as "Docker for Logic."**  
> It attempts to solve one of the biggest problem in AI code generation: **trust**.

Standard AI tools operate on an "all-or-nothing" basis, rewriting entire files and risking your existing architecture. Glupe introduces **Semantic Containers (`$${...}$$`)**—isolated blocks where AI can generate code without touching your hand-written structure.

You define the architecture, and the AI fills in the blanks. Your code stays safe. 

### The Core Workflow: Isolate and Fill

1.  **Isolate Intent:** Write your high-level architecture and place semantic containers where you need implementation details.

    ```cpp
    // file: main.cpp
    #include <iostream>
    #include <vector>

    void process_data(std::vector<int>& data) {
        // The AI is only allowed to write code inside this block.
        $${
            // 1. Filter out all negative numbers.
            // 2. Sort the remaining data in descending order.
            // 3. Remove any duplicate values.
        }$$
    }

    int main() {
        std::vector<int> my_data = {5, -1, 10, 2, 10, -5, 2};
        process_data(my_data);
        // ... print the result ...
    }
    ```

2.  **Fill In-Place:** Use the `-fill` command to populate the containers directly within your source file.

    ```bash
    glupe main.cpp -fill -local
    ```

    Glupe reads `main.cpp`, sends the container's intent (`"Filter out..."`) to the LLM, and injects the generated C++ code back into the `$${...}$$` block, leaving your `main` function and includes untouched. This gives you surgical precision and control over AI-assisted development.

---

## Installation

### Quick Install (Recommended)

**Windows**
1.  Press `Win + R`, type `cmd`, and press Enter.
2.  In the command prompt, type `powershell` and press Enter.
3.  Run this command:
    ```powershell
    irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
    ```

**Linux/macOS**
1.  Open your terminal and run the following command:
    ```bash
    curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash
    ```

### Quick Start
```bash
# Initialize a sample project
glupe --init

# Fill the containers in the sample file and compile it
glupe hello.glp -o hello.exe -cpp -local

# Run the output
./hello.exe
```
--- 

### The risk of AI generated code
Every developer knows the risk of asking AI to modify a file: it’s like giving a junior developer root access to your production server. It's **all or nothing**. They might fix the bug, but they might also refactor your working code or delete critical comments.

We built Glupe to solve this trust problem.

Instead of a Host System we have a Source Code File. The Containers are the `$${  }$$` blocks, isolated zones where the AI is allowed to work. Instead of isolating runtimes, Glupe isolates code blocks. The A.I is aware of the context outside the blocks, it undestands the logic of the rest of your program, but is forbbiden to touch it; it can run wild inside the container, generating complex logic, but it won't touch the host. It is safely contained.

Glupe allows for incremental builds via container hashing, if a container hasn't changed, it uses a cached code snippet to bypass LLM calls.

This turns AI from a chaotic re-writer from scratch into a precision tool. You maintain architectural control, while the AI handles the implementation details *you want* to give to it.


### Glupe as a "compiler"

Traditional compilers (GCC, Clang, rustc) translate code based on syntax and do not attempt to fix errors for you. When a build fails, you are left to interpret compiler messages, search documentation, and debug the issue manually.

Glupe is a command-line tool that sits between your intent (written in plain text or pseudo-code) and your existing build tools. It uses a configured language model (local or cloud) to generate source code, writes the output to disk, and optionally runs the compiler or build script.

If compilation fails, Glupe can attempt to fix the problem by re-running the model with the compiler error output, up to a configurable number of retries.

Glupe is not a deterministic compiler or a formal transpiler. It is an orchestrator that relies on external compilers and the quality of the configured language model, using LLMs to assist with code generation and build orchestration

---
 ## Key Features
 ### AI-Powered Code Generation
Generate executable code from natural language, mixed languages, or existing files:

```bash
glupe utils.py myalgorithm.c -o myprogram.exe -cpp -cloud
```
Combine Python, C, and intent → get a native C++ binary

### Multi-File Project Generation
Use EXPORT: blocks to define entire projects in a single .glp file:

```glupe
EXPORT: "mylib.h"
$$ myfunc { define a function 'myfunction()' that returns square of a number }$$
EXPORT: END

EXPORT: "myprogram.cpp"
#include <iostream>
#include <vector>
#include "mylib.h"

int main(){
  int x = 3;
  $$ main { 
    make a vector V containing [1,2,3,4,5]
    print "hello world" and vector V
    print(myfunction(x))  // should print 9
  }$$
}
EXPORT: END
```
Run this script
```bash
glupe idea.txt -make -cloud
```
### Full Control: You Drive, AI Fills
Unlike "all-or-nothing" AI generators, Glupe lets you decide exactly where AI touches your code:

You control structure, includes, and architecture
AI only fills $${ ... }$$ blocks
Perfect for production code where safety matters

### Self-Healing Compilation
Failed build? Glupe retries with compiler feedback:

```bash
[Pass 1] Missing #include <map>
[Pass 2] Wide string mismatch  
[Pass 3] BUILD SUCCESSFUL!
```
### One-Step Execution
```bash
glupe app.glp -o app.exe -cpp -local -run
```
# Compiles AND runs immediately
Model-Agnostic
Works with any LLM backend:

Local: Ollama (privacy, zero cost)

Cloud: OpenAI, Google Gemini (more power)

Custom: Any OpenAI-compatible API

Utility Commands

### fix – Apply Smart Edits
Add changes to your code via natural language:

```bash
glupe fix project.c "fix segfault in line 1023" -local
```

### explain – Auto-Generate Documentation
Create a thoroughly commented copy of your file:

```bash
glupe explain main.cpp -cloud english
```
Creates main_doc.cpp with detailed comments

### diff – Semantic Change Analysis
Generate a Markdown report of what changed, not just what text changed:

```bash
glupe diff version1.py version2.py -cloud
```
Outputs human-readable change summary
### sos – Terminal Tech Support
Get AI help without leaving your terminal:
```bash
glupe sos english -local "KeyError: 'name' in my pandas script"
```
### TL;DR
Glupe is a semantic meta-compiler that:
- Parses EXPORT: blocks to create project files
- Copies your literal code exactly as written
- Lets AI fill only the $${ ... }$$ blocks you designate
- Uses -series to build files sequentially (prevents AI fatigue)
- Gives you full control—unlike black-box AI generators

## Configuration
Setup Local AI (Privacy First)
```bash

glupe config model-local qwen2.5-coder:latest
```
Setup Cloud AI (Max Reasoning)
```bash
glupe config api-key "YOUR_KEY"
glupe config model-cloud gemini-1.5-flash
```

### Usage Examples
1. Basic Compilation
````bash
glupe main.cpp -o app.exe -cpp -cloud
````
## What Glupe is NOT

To set clear expectations:

1. Not a compiler: It does not compile code itself. It relies on existing compilers and interpreters.
2. Not deterministic: Output depends on the model and prompt, so results may vary between runs.
3. Not a production build system: It does not track dependencies or perform incremental builds.
4. Not a formal transpiler: It generates code via LLMs, not via syntax tree translation.

The Vision
Programming has traditionally required years of study to master syntax and memory management. Glupe aims to lower the barrier of software engineering so a broader audience can access computational resources for their professional fields.

Glupe transforms the compiler from a syntax checker into a partner in creation.
---

# Read the White Paper
https://github.com/alonsovm44/glupe/blob/master/.DOCUMENTATION/paper.md

# Syntax highlight extension
In the releases section
https://github.com/alonsovm44/glupe-tutorial 

## CONTRIBUTORS
Thanks very much for all contributions to Glupe

- Alonso Velazquez (Mexico) since Jan 15 26
- Krzysztof Dudek (Poland) since Feb 23 26

MIT License
---