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

---

## 🚀 AI code generation — without losing control

> **Glupe lets you use AI to write code without ever letting it modify your real code.**

Most AI coding tools are **all-or-nothing**:
- they rewrite entire files  
- they break working code  
- they destroy structure  

**Glupe solves this with isolation.**

---

## 🧠 The idea: Semantic Containers

Glupe introduces **semantic containers**:

```cpp
$${
    // AI writes code here
}$$
```

- Your architecture stays **untouched**
- AI is **strictly limited** to these blocks
- You get **precision instead of chaos**

> Think of it as **“Docker for Logic”** — but for source code.

---

## ⚡ Example: Safe AI in real code

```cpp
#include <iostream>
#include <vector>

void process_data(std::vector<int>& data) {
    $${
        // 1. Remove negative numbers
        // 2. Sort descending
        // 3. Remove duplicates
    }$$
}
```

Run:

```bash
glupe main.cpp -fill -local
```

👉 Glupe fills the block with real C++ code  
👉 Your structure stays exactly the same  

---

## 💥 Why this matters

AI is powerful—but unpredictable.

Using AI on code today is like:
> giving a junior dev root access to your codebase

Glupe changes that:

- ✅ You control structure  
- ✅ AI only fills what you allow  
- ✅ Your code is never rewritten  

---

## 🔑 Core Workflow

### 1. Write your architecture
Define your program normally.

### 2. Add containers
Mark where AI is allowed to work.

### 3. Fill safely
```bash
glupe main.cpp -fill -local
```

---

## 🚀 Installation

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

## ⚡ Quick Start

```bash
# Create sample project
glupe --init

# Fill containers + compile
glupe hello.glp -o hello.exe -cpp -local

# Run
./hello.exe
```

---

## ✨ Key Features

### 🎯 Surgical AI Control
- AI writes **only inside `$$ { } $$`**
- Your architecture is **guaranteed untouched**

---

### ⚡ Cached Containers
- Containers are hashed
- No repeated LLM calls if unchanged

---

### 🔁 Self-Healing Compilation
```bash
[Pass 1] Missing include
[Pass 2] Type error
[Pass 3] BUILD SUCCESSFUL
```

---

### 🧩 Multi-File Project Generation

```glupe
EXPORT: "main.cpp"
$$ main {
    create a program that prints hello world
}$$
EXPORT: END
```

---

### 🛠 Utility Commands

Fix code:
```bash
glupe fix file.cpp "fix segmentation fault" -local
```

Explain code:
```bash
glupe explain main.cpp -cloud english
```

Diff changes:
```bash
glupe diff v1.py v2.py -cloud
```

Terminal help:
```bash
glupe sos english -local "KeyError in pandas"
```

---

## 🧠 What Glupe is (and isn't)

### ✅ What it is
- A **safe AI code generation layer**
- A tool to bridge **intent → implementation**
- A way to make AI **predictable**

### ❌ What it is NOT
- Not a compiler
- Not deterministic
- Not a traditional build system
- Not a transpiler

---

## ⚙️ Configuration

### Local AI (recommended)
```bash
glupe config model-local qwen2.5-coder:latest
```

### Cloud AI
```bash
glupe config api-key "YOUR_KEY"
glupe config model-cloud gemini-1.5-flash
```

---

## 🎯 Philosophy

> You define **what**  
> AI handles **how**  
> You stay in control

---

## 📄 White Paper
https://github.com/alonsovm44/glupe/blob/master/.DOCUMENTATION/paper.md

---

## 🎨 Syntax Highlight Extension
https://github.com/alonsovm44/glupe-tutorial 

---

## 👥 Contributors

- Alonso Velazquez (Mexico)
- Krzysztof Dudek (Poland)

---

## 📜 License
MIT