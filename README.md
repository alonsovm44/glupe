# Glupe — Intent-first AI codeblocks for C++ 🌹🌹

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![Release](https://img.shields.io/github/v/release/alonsovm44/glupe)](https://github.com/alonsovm44/glupe/releases) ![C++17](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B) ![Platforms](https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-lightgrey)

One-line pitch
Glupe uses small, well-scoped code containers so AI can implement behavior without ever rewriting your architecture — add a $$ { } $$ block, ask Glupe to fill it, and keep full control.

## 🎬 Glupe in Action

| Refine | Fill | Auto Fix Errors |
|----------|-------|------------|
| ![](./assets/demo4.gif) | ![](./assets/demo2.gif) | ![](./assets/demo3.gif) |

Try it now (copy-paste)
Linux / macOS:
```bash
curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash \
  && glupe --init \
  && glupe hello.glp -o hello.exe -cpp -local \
  && ./hello.exe
```
Windows (PowerShell):
```powershell
irm https://raw.githubusercontent.com/alonsovm44/glupe/master/install.ps1 | iex
# then open PowerShell and run:
glupe --init
glupe hello.glp -o hello.exe -cpp -local
.\hello.exe
```

What makes Glupe different
- Isolation: AI can only write inside your marked containers (no file rewrites).
- Predictable outputs: containers are hashed and cached; unchanged blocks skip LLM calls.
- Self-healing builds: Glupe iteratively fixes missing includes and simple type issues until compilation succeeds.

Quick example
```cpp
#include <iostream>
#include <vector>

void process_data(std::vector<int>& data) {
    /*
    $${ 
        1. Remove negative numbers
        2. Sort descending
        3. Remove duplicates
    }$$
    */
}

int main() {
    std::vector<int> data = {3, -1, 2, 2};
    process_data(data);
    for (auto v : data) std::cout << v << " ";
    return 0;
}
```
Run:
```bash
glupe main.cpp -fill -local
```

Core workflow (3 steps)
1. Write your architecture (files, types, APIs).  
2. Mark containers with $$ { } $$.  
3. Fill: `glupe <file> -fill -local` (or use `-cloud` to call a cloud LLM).

Key commands
- Fill a block: `glupe main.cpp -fill -local`
- Fix code: `glupe fix file.cpp "fix segmentation fault" -local`
- Explain file: `glupe explain main.cpp -cloud english`
- Diff: `glupe diff v1.py v2.py -cloud`

Installation
- Quick install: see Try it now above.
- Manual: run the platform-specific installer scripts in the repo (install.sh / install.ps1).

Configuration
- Local model (recommended): `glupe config model-local qwen2.5-coder:latest`
- Cloud: `glupe config api-key "YOUR_KEY"` and `glupe config model-cloud gemini-1.5-flash`

Documentation & White paper
- White paper: .DOCUMENTATION/paper.md  
- Developer docs: .docgen/docs

Contributing & Community
- Contributors: Alonso Velazquez (Mexico), Krzysztof Dudek (Poland)  
- Good first issues: help improve the top-of-readme “Try it now”, add extra examples under examples/cpp, and add a CI workflow.  
- To contribute: fork → branch → PR. Add tests / small examples for any change.


Roadmap & Philosophy
- You define what. AI handles how. You stay in control.
- Planned: first-class IDE plugin, more sample projects (networking, parsing), CI-run demos on releases.

License
- MIT

Links
- White paper: https://github.com/alonsovm44/glupe/blob/master/.DOCUMENTATION/paper.md
- Syntax highlight / tutorial: https://github.com/alonsovm44/glupe-tutorial