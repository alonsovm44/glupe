# Read the White Paper
https://github.com/alonsovm44/glupe/blob/master/WHITE_PAPER.md
# Glupe 
Glupe is a semantic metaprogramming language. Unlike traditional metaprogramming systems that operate within a single language (C++ templates, Lisp macros), Glupe operates at the level of human intent and can materialize that intent into any target programming language. This positions Glupe as a 'meta' language not just in the technical sense of generating code, but in the philosophical sense of being one level above all programming languages.

Example meta-program:
```glupe

$$ include {
    standard io library
    vector library

}$$
$$ABSTRACT rules{
    every printed message must end with "!"
}$$

$$ABSTRACT rules2{
    every printed message must begin with "?"
}$$

$$ main -> rules, rules2 {
 let v = vector[1,2,3]
 print v   
}$$

```
Result:
```
? 1,2,3 ! 
```
This code can be compiled into 40+ supported languages or native executables.
--- 
## But what does Glupe do?
### The one liner
"Glupe isolates AI logic into semantic containers, so your manual code stays safe." 
---

Think of Glupe as "Docker for Logic."

Just as Docker packages the Environment to make software independent of the machine, Glupe packages the Intent to make software independent of the language.

They both solve "It works on my machine," but at different layers:

- Docker freezes the Operating System. It ensures your code runs the same on your laptop, the cloud, or a server in 2015.
- Glupe freezes the Algorithm. It ensures your code can be compiled to C++ today, Rust in 2030, or whatever language exists in 2050.
Docker solves Space. Glupe solves Time.

Best of all, they work together. You can store your .glp blueprints in a Docker container—creating software that is immortal in both environment and logic.

## Installation

### Quick Install (Recommended)

**Windows**
Installation guide
1. Press `Win + R` and type `cmd` or open cmd.exe
2. Type  `Powershell`
3. Run this command:
```Powershell 
 irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
```
If you install Ollama be sure to accept all pop up windows.
The installer automatically installs the latest version of Glupe.

**Linux/macOS:**
Installation in Linux/macOS
1. Open bash terminal and run the following command:
```bash
curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash
``` 
**Manual Build**

````bash
g++ glupec.cpp -o glupe -std=c++17 -lstdc++fs
```` 
### **Quick Sart**
```bash
glupe --init
glupe hello.glp -o hello.exe -cpp -local
.\hello
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
glupe idea.txt -make -cloud -series
```
### Full Control: You Drive, AI Fills
Unlike "all-or-nothing" AI generators, Glupe lets you decide exactly where AI touches your code:

You control structure, includes, and architecture
AI only fills $${ ... }$$ blocks
Perfect for production code where safety matters

### Series Mode (Prevents Prompt Fatigue)
```bash
glupe project.glp -make -series
```
Generates files sequentially (not parallel) to ensure AI maintains context and delivers complete, coherent outputs.

### Automatic Build Detection
Glupe automatically detects and runs your build system:

Makefile → runs make

CMakeLists.txt → configures and builds

build.sh / build.bat → executes directly

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
Glupe is a semantic compiler that:
- Parses EXPORT: blocks to create project files
- Copies your literal code exactly as written
- Lets AI fill only the $${ ... }$$ blocks you designate
- Uses -series to build files sequentially (prevents AI fatigue)
- Gives you full control—unlike black-box AI generators

## Why use Glupe?
1. A different approach to build automation

Traditional build systems (Make, CMake, etc.) focus on compiling and linking source code you already wrote, since they won't write code for you. Glupe aims to help bridge the gap between intent and implementation by generating source code and build files based on a text description.

With CMake: you write configuration files and provide source code.

With Glupe: you describe what you want in plain text, and Glupe attempts to generate the source code and build scripts, then run the build.

2. Faster prototyping (with caveats)

Glupe can accelerate early-stage prototyping by letting you express ideas in your own jargon or pseudo-code, then generating and compiling an initial implementation or MVP.

Input: Intent or pseudo-code
Output: Source code and optionally a compiled binary

Note: Results depend on the model, the explicitness and quality of the input. The output may also require refinement. Glupe is not guaranteed to produce production-ready code. Treat the output Glupe makes as a fresh piece out of a 3D printer.

3. Self-healing build loop

Glupe can reduce time spent debugging compilation errors by using an automated feedbacks loop:

A. Generate code
B. Compile
C. If the build fails, send the compiler output back to the model
D. Retry (up to a configured number of attempts)
This can help with common compilation issues, but it is not a replacement for understanding the underlying code or dependencies.

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
```
The Vision
Programming has traditionally required years of study to master syntax and memory management. Glupe aims to lower the barrier of software engineering so a broader audience can access computational resources for their professional fields.

Glupe transforms the compiler from a syntax checker into a partner in creation.
---
License & Contributing
MIT License
---