# Glupe

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Release](https://img.shields.io/github/v/release/alonsovm44/glupe)](https://github.com/alonsovm44/glupe/releases)
![C++17](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B)
![Platforms](https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-lightgrey)
![AI-Powered](https://img.shields.io/badge/AI-Powered-purple)

> **Let AI write code — without touching your code.**

Glupe is an AI-powered compiler that generates code only inside explicit regions (`$${...}$$`), keeping your architecture safe, predictable, and under your control.

---

## 🎬 Glupe in Action

| Refine | Build  | Fix Errors (auto-retry) |
| ---------------------- | ----------------------- | ----------------------- |
| ![](./assets/demo4.gif) | ![](./assets/demo2.gif) | ![](./assets/demo3.gif) |

---

## 🧠 Why Glupe exists

AI coding tools today operate in an **all-or-nothing** way:

* They rewrite entire files
* Break structure and intent
* Introduce subtle bugs

This makes them hard to trust in real projects.

**Glupe fixes this by isolating where AI is allowed to write.**

---

## 🧩 The Core Idea: Semantic Containers

```cpp
$${
  // describe what you want here
}$$
```

* AI can **only** generate code inside these blocks
* Everything outside remains untouched
* You keep full architectural control

---

## ⚡ TL;DR

* You write structure
* AI fills `$${ ... }$$`
* Your code stays safe

---

## 🔧 The Core Workflow

### 1. Define structure + intent

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

### 2. Fill in-place

```bash
glupe main.cpp -fill -local
```

Glupe:

* reads your file
* sends the container prompt to an LLM
* injects generated code into the block

---

## ⚡ Install in 5 seconds

### Quick Install (Recommended)

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

Glupe sits between your intent and your compiler:

* Reads your file
* Extracts semantic containers
* Sends prompts to an LLM
* Injects generated code
* Runs your build
* Retries on failure using compiler feedback

---

## 🔥 Key Features

### AI-Powered Code Generation

```bash
glupe utils.py myalgorithm.c -o myprogram.exe -cpp -cloud
```

---

### Multi-File Project Generation

```glupe
EXPORT: "mylib.h"
$$ myfunc { define a function that returns square }$$
EXPORT: END
```

---

### Self-Healing Compilation

```bash
[Pass 1] Missing include
[Pass 2] Type error
[Pass 3] BUILD SUCCESSFUL
```

---

### One-Step Execution

```bash
glupe app.glp -o app.exe -cpp -local -run
```

---

## ⚖️ How Glupe is different

| Tool              | Behavior                         |
| ----------------- | -------------------------------- |
| Copilot / ChatGPT | Rewrite entire files             |
| Glupe             | Writes only inside `$${ ... }$$` |

👉 Glupe gives control. Others take it away.

---

## 👤 Who is this for?

* Developers who don’t trust AI rewriting their code
* People building real systems (not just demos)
* Anyone who wants AI as a precise tool

---

## ⚠️ What Glupe is NOT

* Not a compiler (uses existing compilers)
* Not deterministic (LLM-based)
* Not a build system
* Not a transpiler

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

Programming is limited by syntax and complexity.

Glupe aims to:

* lower the barrier to building software
* let developers focus on structure and intent
* turn AI into a controlled, reliable tool

---

## 📄 White Paper

https://github.com/alonsovm44/glupe/blob/master/.DOCUMENTATION/paper.md

---

## 🧩 Syntax Highlight Extension

https://github.com/alonsovm44/glupe-tutorial

---

## 👥 Contributors

* Alonso Velazquez (Mexico)
* Krzysztof Dudek (Poland)

---

## 📜 License

MIT License
