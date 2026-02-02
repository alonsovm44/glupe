# Yori Compiler

The compiler that fixes your code before you even see the error.

## Why use Yori?

Most compilers just complain when you make a mistake. Yori solves it.

Yori is an AI-native compiler wrapper that bridges the gap between high-level logic and native machine code. It allows you to write in Python, pseudo-code, or your own DSL and compiles it into high-performance binaries (C++, Rust, Go, etc.) seamlessly (provided you give the necessary libraries).
---

## You should use Yori if
1. You want to prototype fast: Describe features in your own DSL or pseudocode and Yori does the heavy lifting for you.
2. You hate debugging syntax errors: Yori captures compiler errors, understands them, fixes the source code, and recompiles automatically.
3. You value privacy: Full support for Local LLMs (Ollama) means your code never has to leave your machine.
4. You need performance: Write in "Python-like" logic, but get a compiled C++ or Rust binary out.

## Key Features

- Semantic Transpilation & Compilation

Yori doesn't just translate syntax; it translates intent. You can feed it a .py file but ask for a C++ binary. Yori ensures the logic is ported using native libraries (no Python.h hacks), creating standalone executables.

- Autohealing toolchain

1. Yori runs in a loop with your system's compiler (GCC, Rustc, Go, etc.).
2. Yori generates code.
3. Yori attempts to compile.
If it fails: Yori reads the error log, understands why it failed (missing semi-colon? wrong library?), patches the code, and tries again.

- Model Agnostic
1. Local: Auto-detects your installed Ollama models (Llama 3, Qwen, DeepSeek). Zero cost, full privacy.
2. Cloud: Native integration with Groq, OpenAI, and Google Gemini for when you need maximum reasoning power.

## Utilities

Yori includes quality of life utilities for programmers. No more diving into StackOverflow for hours

1. `yori fix file.cpp "fix segfault at line 203`-> applies semantic fixes
2. `yori explain file.rs` Makes a copy of the file commented and documented for you
3. `yori diff fileA.cpp fileB.cpp` Explains semantic changes, not just the diffs
4. `yori sos "help me fix this error...` instant AI techsupport

## Manual install (Windows)
1. Go to Releases and download the latest release, unzip and run the [installer.ps1]
2. To run the script right-click → Run with PowerShell
3. Follow prompts (Ollama, model, compiler auto-setup)
---
## Build Yori yourself

```Bash
g++ yoric.cpp -o yori -std=c++17 

Move to path (Linux/Mac)
sudo my yori /usr/local/bin/
```
# Configuration
Yori works out the box but you can config it as you see fit
```bash
yori config model-local 
```
Cloud Setup
```bash
yori config cloud-protocol google # or openai
yori config api-key "YOUR_API_KEY"
yori condig model-cloud "gemini-1.5-flash"
```

##  The Vision
Programming has traditionally required years of study to master syntax and memory management. Yori aims to lower the entry barrier of software engineering so a broader audience can access computational resources for their professional fields.  

# Yori is best meant for

- **Software developers** who want to prototype an MVP fast and better, from weeks to days or hours.
- **CS professors and CS101 students** who want to exercise computational thinking without wrestling syntax again.
- **Domain professionals** (physicists, biologists, economists) who need to simulate systems but don't understand language syntax—Yori lets them code in their domain vocabulary and transpile their code to working python scripts or native c++ executables.
- **Indie gamedevs** building multilingual prototypes and rapid iteration builds.
- **Data scientists** needing Python ML models packaged as single standalone executables.
- **Junior developers** who want to understand systems without syntax pain.
- **Product managers** specifying features that auto-compile to working demos for stakeholder validation.
- **DevOps engineers** generating infrastructure-as-code and deployment configs from high-level intent.
- **UX/Design teams** creating functional prototypes to validate interaction flows before frontend dev.
- **Hardware engineers** who need embedded firmware prototypes without toolchain setup.
- **Financial analysts** building risk models, dashboards, and trading algorithms without learning Python/R.
- **Marketers** generating landing pages, A/B tests, and conversion funnels as standalone deploys.
- **Open source maintainers** rapidly scaffolding new features, APIs, or CLIs for contributor feedback.
- **Technical writers** embedding working code examples directly in documentation that auto-updates.
- **CEO/Founders** sketching SaaS ideas that become shippable MVPs for investor demos.

---

## Quick Install
Quick Install Commands
Linux/macOS
<curl -fsSL https://raw.githubusercontent.com/alonsovm44/yori/master/install.sh | bash>
Or with wget:
<wget -qO- https://raw.githubusercontent.com/alonsovm44/yori/master/install.sh | bash>
Windows (PowerShell)
<irm https://raw.githubusercontent.com/alonsovm44/yori/master/install.ps1 | iex>
Or with shorter URL (if you setup a domain):
<irm yori.dev/install.ps1 | iex>
## Installation

### Quick Install (Recommended)

**Linux/macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/alonsovm44/yori/master/install.sh | bash
```

**Windows:**
```powershell
irm https://raw.githubusercontent.com/alonsovm44/yori/master/install.ps1 | iex
```

### Manual Install

1. Download the [latest release](https://github.com/alonsovm44/yori/releases)
2. Extract the archive
3. Run the installer:
   - Windows: `install.bat`
   - Linux: `./install.sh`

### Build from Source
```bash
git clone https://github.com/alonsovm44/yori
cd yori
g++ -std=c++17 yoric.cpp -o bin/yori
./bin/yori --version
```

---

## System Requirements

- **OS**: Windows 10+, Linux (Ubuntu 20.04+, Fedora 33+, Arch)
- **Compiler**: g++ with C++17 support
- **Optional**: Ollama (for local AI mode)

---

## First Steps

After installation:
```bash
# Create example project
yori --init

# Compile your first program
yori hello.yori -o hello.exe -c

# Run it
./hello.exe
```


---
##  What Yori IS NOT

To manage expectations regarding traditional compiler theory and define the tool's scope, it is important to clarify what Yori is not:

-Not a Traditional Deterministic Compiler:

-Unlike GCC, Clang, or Rustc, Yori does not rely on a hand-written lexer, parser, Abstract Syntax Tree (AST), or formal grammar rules to translate code.

-What it is instead: It is an Agentic Build System that leverages Large Language Models (LLMs) to interpret intent and transpile mixed-syntax code into a high-performance target language (like C++).

-Not 100% Deterministic (but can do incremental development with the -u flag):

Due to the probabilistic nature of AI, compiling the same source file twice might result in slightly different underlying C++ implementations (though functionally identical).

Mitigation: Yori employs caching mechanisms and strict prompting to stabilize output, but it does not guarantee bit-for-bit binary reproducibility across builds.

Not "Just a ChatGPT Wrapper":

While it uses AI for code generation, Yori adds a critical layer of Systems Engineering:

1. Context Awareness: Handles multi-file imports and dependencies.

2. Self-Healing: Automatically feeds compiler errors back into the system to fix bugs.

3. Fail-Fast Safety: Detects missing libraries/headers before wasting resources.

4. Toolchain Abstraction: Manages the invocation of native compilers (g++, rustc, etc.) transparently.

### Summary:
 Yori is a Software 2.0 tool. If you need absolute formal verification and clock-cycle precision, use a traditional compiler. If you want to prototype complex ideas in seconds using natural language and mixed logic or your own language, use Yori.


## UNIVERSAL IMPORTS

Yori files support universal imports using IMPORT: keyword. Example:

````cpp
int function1(int x){
    return x + 6;
}
````
````py
def function2(x):
    return x + 7
````
````yori
IMPORT: a.cpp //for function1()
IMPORT: b.py // for function2()

INT x=1
PRINT(function1(x))
PRINT(function2(y))

```` 
````bash
yori example.yori -o myapp.exe -cpp -cloud 
>7
>8
````
Yori follows a You-provide-first philosophy, it won't make libraries from scratch, be sure to provide them if possible. Yori can handle this
````
INCLUDE: raylib.h
INCLUDE: mypy.py

//program logic

````
Yori behaves better with All-in-one libraries
## UNIVERSAL LINKING
Yori now supports universal file compilation (4.5) like so
````
>yori a.cpp b.py c.cs d.js e.acn -o myapp.exe -c -cloud 
//you could also transpile multiple files into a single file
>yori a.py b.py c.py -o d.c -cloud
````
## IMPROVE PERFORMANCE JUST WITH COMMENTS
Compiler directives are universal and can be used in any language.
if you have a python script (or any script) that needs a performance boost just add "#!!! improve performance" (or use the appropiate comment syntax with !!! flag) to tell the compiler what you want. Then you have two options
````
>yori myscript.py -o myscripy_boost.py -cloud
````
````Or
yori myscript.py -o myapp_boost.exe -cpp -cloud
````
