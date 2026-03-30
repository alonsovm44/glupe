# Glupe

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Release](https://img.shields.io/github/v/release/alonsovm44/glupe)](https://github.com/alonsovm44/glupe/releases)
![C++17](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B)
![Platforms](https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-lightgrey)
![AI-Powered](https://img.shields.io/badge/AI-Powered-purple)

> **Use LLMs in your codebase without letting them rewrite your codebase.**

LLMs are powerful, but unconstrained generation is dangerous. Glupe is a CLI tool that acts as a **strict constraint layer** (or firewall) for AI code generation. You write the load-bearing architecture; the AI is only allowed to fill in the explicit blanks.

---

## 🎬 Glupe in Action

| Refine | Build  | Fix Errors (auto-retry) |
| ---------------------- | ----------------------- | ----------------------- |
| ![](./assets/demo4.gif) | ![](./assets/demo2.gif) | ![](./assets/demo3.gif) |

---

## 🧠 The problem

Using LLMs for code today looks like this:

- paste prompt → get code → paste into project  
- or let an agent rewrite entire files  

This creates real issues:

- ❌ no boundaries (AI can change anything)  
- ❌ hard to review (what actually changed?)  
- ❌ not reproducible  
- ❌ easy to break working code  

---

## ✅ What Glupe does

Glupe adds **explicit boundaries** around AI-generated code.

```cpp
int add(int a, int b) {
    $${
        // implement addition
    }$$
}
```

- AI can **only write inside `$$ { ... } $$`**
- Everything else is **guaranteed unchanged**
- You control structure, AI fills implementation

---

## ⚡ TL;DR

- You write the structure  
- AI fills small, isolated regions  
- Your codebase stays stable  

---

## 🔧 The workflow

### 1. Write normal code + intent

```cpp
#include <vector>

void process_data(std::vector<int>& data) {
    $${
        // 1. Remove negative numbers
        // 2. Sort descending
        // 3. Remove duplicates
    }$$
}
```

---

### 2. Run Glupe

```bash
glupe main.cpp -fill -local
```

Glupe will:

- extract the container
- send only that part to the LLM
- inject the result back into the file

---

## 🤔 Why not just prompt?

You can — and for small tasks, you probably should.

Glupe is useful when:

- you care about **not touching the rest of the file**
- you want **repeatable structure**
- you want **reviewable diffs**
- you’re integrating AI into a **real codebase**

---

## ⚖️ What this is (and isn’t)

### ✔️ This is

- a **constraint layer for LLM code generation**
- a way to **keep AI edits local and predictable**
- a tool for **gradual adoption of AI in real projects**

### ❌ This is NOT

- not a new programming language  
- not a replacement for compilers  
- not deterministic (LLMs are still LLMs)  
- not magic — bad prompts still produce bad code  

---

## ⚡ Installation

### Recommended: Pre-compiled Binaries
Download the standalone executable for your OS directly from the [Releases page](https://github.com/alonsovm44/glupe/releases).

### Alternative: Quick Install Scripts
> **Note:** Piping scripts directly to your shell carries inherent security risks. We recommend using the pre-compiled binaries above, but provide these scripts for convenience. You can review `install.ps1` and `install.sh` in the repository.

**Windows**

1. Press `Win + R`, type `cmd`, and press Enter.  
2. In the command prompt, type `powershell` and press Enter.  
3. Run:

```powershell
irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
```

**Linux/macOS**

```bash
curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash
```

---

## 🚀 Quick Start

```bash
glupe --init
glupe hello.glp -o hello.exe -cpp -local
./hello.exe
```

---

## ⚙️ How it works

Glupe is a thin layer between your code and an LLM:

1. scans your file  
2. finds `$$ { ... } $$` blocks  
3. sends only those blocks to the model  
4. injects generated code  
5. optionally compiles + retries on failure  

---

## 🔒 Determinism & Caching

A major issue with AI code generation is non-determinism: regenerating an app might introduce completely new bugs. 

Glupe solves this via **caching**. Once a `$$ block { ... } $$` block successfully compiles and passes your tests, its hash is locked in a `.glupe.lock` file. 

Re-running the CLI will **not** regenerate that block unless you explicitly change the prompt inside the container. It behaves like incremental compilation: once a block works, it stays frozen safely.

---

## 🔥 Features

### Scoped AI generation

Only touch what you explicitly allow.

---

### Multi-file output

```glupe
EXPORT: "mylib.h"
$$ myfunc { define a function that returns square }$$
EXPORT: END
```

---

### Auto-fix compile errors

```bash
[Pass 1] Missing include
[Pass 2] Type error
[Pass 3] BUILD SUCCESSFUL
```

---

### One-step run

```bash
glupe app.glp -o app.exe -cpp -local -run
```

---

## 👤 Who this is for

- developers experimenting with LLM-assisted coding  
- people who **don’t want AI rewriting entire files**  
- teams that want **controlled integration of AI**  

---

## ⚠️ Tradeoffs

- LLM output is still **non-deterministic**
- requires writing inside containers
- adds an extra step vs raw prompting

This is intentional:

> Glupe trades speed for **control and safety**

---

## ⚙️ Configuration

### Local model

```bash
glupe config model-local qwen2.5-coder:latest
```

### Cloud model

```bash
glupe config api-key "YOUR_KEY"
glupe config model-cloud gemini-1.5-flash
```

---

## 🧰 Utility Commands

### fix

```bash
glupe fix project.c "fix segfault" -local
```

### explain

```bash
glupe explain main.cpp -cloud english
```

### diff

```bash
glupe diff v1.py v2.py -cloud
```

### sos

```bash
glupe sos english -local "KeyError in pandas"
```

---

## 🌍 Vision

LLMs are useful, but unsafe by default.

Glupe’s goal is simple:

> make LLM-assisted programming **controllable enough to use in real systems**

---

## 📄 White Paper

https://github.com/alonsovm44/glupe/blob/master/.DOCUMENTATION/paper.md

---

## 🧩 Syntax Highlight Extension

https://github.com/alonsovm44/glupe-tutorial

---

## 👥 Contributors

- Alonso Velazquez (Mexico)  
- Krzysztof Dudek (Poland)  

---

## 📜 License

MIT License
