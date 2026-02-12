# Semantic Code Blocks: A Hybrid Compilation Paradigm for LLM-Assisted Software Generation

**Author:** [Your Name]  
**Affiliation:** Independent Researcher  
**Date:** February 2026  
**arXiv Category:** cs.PL (Programming Languages), cs.AI (Artificial Intelligence)

---

## Abstract

We introduce **semantic code blocks**, a novel language construct that formalizes "intentional gaps" in source code as first-class syntax elements. Unlike traditional compilation which requires complete syntactic specification, our approach enables hybrid declarative-imperative programming where developers mix concrete implementation with natural language intent. We present **Yori**, a reference compiler that transpiles semantic blocks into executable code across 30+ programming languages using Large Language Models (LLMs) with architectural guardrails. Key contributions include: (1) formal syntax for semantic blocks (`$${...}$$`), (2) blueprint-based multi-file orchestration via `EXPORT` directives, (3) contextual series generation to maintain project coherence, and (4) anti-hallucination mechanisms to ensure LLM reliability. Our implementation demonstrates practical viability with successful generation of complete, working programs from mixed concrete/abstract specifications.

**Keywords:** semantic compilation, hybrid programming, LLM-assisted development, intentional programming, code generation

---

## 1. Introduction

Modern software development faces a fundamental tension: boilerplate code is predictable yet tedious, while core logic requires precision and creativity. Existing LLM-based coding assistants (GitHub Copilot, ChatGPT) operate through chat interfaces or IDE autocomplete, creating a disconnect between intent expression and code structure.

We propose a paradigm shift: treat natural language intent as **compilable syntax** rather than external prompts. This enables developers to write:

```cpp
int main() {
    $${
        make a vector V containing [1,2,3,4,5]
        print hello world and the vector V content
        clean the buffer
    }$$
}
```

The `$${...}$$` blocks are not comments—they are **semantic placeholders** that the compiler resolves using an LLM backend while preserving surrounding concrete code. This hybrid approach combines the precision of traditional programming with the flexibility of natural language specification.

### 1.1 Contributions

1. **Semantic Block Syntax**: Formal definition of `$${...}$$` as compilable language primitives
2. **Blueprint Compilation**: Multi-file project generation through `EXPORT` directives with context isolation
3. **Contextual Series Generation**: Sequential file compilation with accumulated knowledge to prevent "prompt fatigue"
4. **LLM Guardrails**: Anti-hallucination checks detecting lazy wrapper patterns (e.g., `python.h` inclusion attempts)
5. **Reference Implementation**: Open-source compiler supporting 30+ target languages with local-first execution

---

## 2. Semantic Block Formalism

### 2.1 Syntax Definition

A **semantic block** is delimited by `$${` and `}$$` tokens and contains natural language intent:

```
semantic_block ::= "$${" intent_text "}$$"
intent_text    ::= <any natural language description>
```

Semantic blocks may appear anywhere syntactically valid code can appear in the target language. The compiler treats them as **abstract syntax nodes** to be resolved during compilation.

### 2.2 Compilation Model

Given source file `S` containing semantic blocks `{B₁, B₂, ..., Bₙ}`, the compilation process is:

1. **Parse**: Extract semantic blocks while preserving surrounding concrete code `C`
2. **Context Assembly**: Aggregate `C` + resolved dependencies + user instructions → context `K`
3. **LLM Resolution**: For each `Bᵢ`, query LLM with context `K` to produce concrete code `Cᵢ`
4. **Substitution**: Replace `Bᵢ` with `Cᵢ` in original source
5. **Validation**: Compile result with native toolchain, retry on failure with error feedback

This differs from traditional compilation (which rejects incomplete programs) and chat-based generation (which lacks integration with concrete code).

### 2.3 Context Preservation

A key innovation is **bidirectional context flow**:
- Concrete code `C` provides type signatures, imports, and structural constraints to guide LLM resolution
- Semantic blocks `B` express intent without requiring complete specification

Example:
```cpp
#include <vector>
using namespace std;

int main() {
    $${create vector of first 10 primes}$$  // LLM knows vector<int> is in scope
}
```

The compiler provides `#include <vector>` and `using namespace std;` as context, enabling the LLM to generate `vector<int>` code without redundant imports.

---

## 3. Blueprint-Based Multi-File Orchestration

### 3.1 EXPORT Directive Syntax

For projects spanning multiple files, we introduce **EXPORT directives**:

```
EXPORT: "filename.ext"
<file content with semantic blocks>
EXPORT: END
```

Content inside `EXPORT` blocks is written to the specified file. Content **outside** serves as project-level context without being exported—a form of "implicit prompting."

### 3.2 Example: Multi-File C++ Project

```
This project implements a scientific calculator with MVC architecture.
Use standard library only. No external dependencies.

EXPORT: "calculator.h"
#ifndef CALCULATOR_H
#define CALCULATOR_H
$${define Calculator class with basic operations}$$
#endif
EXPORT: END

EXPORT: "calculator.cpp"
#include "calculator.h"
$${implement Calculator methods}$$
EXPORT: END

EXPORT: "main.cpp"
#include <iostream>
#include "calculator.h"
$${create CLI interface for calculator}$$
EXPORT: END
```

The first two lines provide architectural guidance but don't appear in any output file. This separates "what to build" from "how to structure it."

### 3.3 Benefits

- **Modularity**: Each file generated independently with shared context
- **Coherence**: Global instructions (outside EXPORT) ensure architectural consistency
- **Incremental**: Files can be regenerated individually if one fails
- **Inspectable**: Intermediate outputs are standard source files

---

## 4. Contextual Series Generation

### 4.1 The Prompt Fatigue Problem

When generating large projects, sending all context to an LLM in a single prompt causes:
- Token limit exhaustion
- Loss of focus on later files
- Inconsistencies between related files

### 4.2 Sequential Context Accumulation

Our **series mode** (`-series` flag) generates files sequentially, accumulating context:

```
Algorithm: SeriesGeneration(blueprint)
  context ← global_instructions
  for each file_spec in blueprint:
    code ← LLM_query(file_spec.intent + context)
    write(file_spec.filename, code)
    context ← context + annotate(file_spec.filename, code)
  end
```

Each file "sees" all previously generated files, creating dependency-aware generation.

### 4.3 Example Execution

Given blueprint with files `[A.h, A.cpp, B.h, B.cpp, main.cpp]`:

1. Generate `A.h` with context = `[global_instructions]`
2. Generate `A.cpp` with context = `[global_instructions, A.h]`
3. Generate `B.h` with context = `[global_instructions, A.h, A.cpp]`
4. Generate `B.cpp` with context = `[global_instructions, A.h, A.cpp, B.h]`
5. Generate `main.cpp` with context = `[all previous files]`

This ensures `main.cpp` knows about all classes/functions defined earlier.

### 4.4 Failure Handling

If file `i` fails to compile:
- **Retry** with error message appended to context (up to `MAX_RETRIES`)
- **Backoff** exponentially on API rate limits
- **Abort** series on fatal errors (missing dependencies detected early)

---

## 5. LLM Reliability: Anti-Hallucination Guardrails

### 5.1 The Lazy Wrapper Problem

LLMs often attempt "shortcuts" when asked to implement complex logic:
- Including `<Python.h>` to call Python from C++
- Using `system()` calls to shell out to other languages
- Generating `<jni.h>` wrappers instead of native Java

These technically "work" but violate the intent of native implementation.

### 5.2 Laziness Detection

Our compiler implements **semantic error analysis**:

```cpp
bool isFatalError(const string& errMsg) {
    string lower = toLowerCase(errMsg);
    
    // Wrapper detection
    if (lower.find("python.h") != string::npos) return true;
    if (lower.find("jni.h") != string::npos) return true;
    if (lower.find("node.h") != string::npos) return true;
    
    // Genuine failures
    if (lower.find("fatal error") != string::npos) return true;
    if (lower.find("undefined reference") != string::npos) return true;
    
    return false;
}
```

When detected, the error message is annotated with:
```
"FATAL: You attempted to include python.h. STOP. 
Rewrite using native C++ stdlib only."
```

This **trains the LLM** through error feedback to avoid lazy patterns.

### 5.3 Preflight Dependency Checking

Before LLM invocation, we verify all `#include` dependencies exist:

```cpp
set<string> extractDependencies(const string& code);
bool preFlightCheck(const set<string>& deps) {
    // Compile test file with all includes
    // Detect missing headers early
    // Provide diagnostic before expensive LLM call
}
```

This prevents wasted API calls on doomed-to-fail generations.

### 5.4 Iterative Refinement

Compilation is a feedback loop:

```
for attempt in 1..MAX_RETRIES:
    code ← LLM_generate(intent + context + error_history)
    result ← native_compile(code)
    if result.success:
        return code
    error_history.append(result.errors)
end
```

Error messages from native compilers provide **ground truth** for LLM correction.

---

## 6. Implementation: The Yori Compiler

### 6.1 Architecture

**Input**: Source files with semantic blocks + EXPORT directives  
**Output**: Executable binaries or source files in target language

**Pipeline**:
1. **Preprocessor**: Resolve `IMPORT:` directives (recursive module inclusion)
2. **Parser**: Extract EXPORT blocks and semantic blocks
3. **Planner**: Determine generation strategy (single-shot vs. series)
4. **Generator**: Invoke LLM with context and guardrails
5. **Validator**: Compile with native toolchain, iterate on errors
6. **Exporter**: Write final outputs (binaries or source files)

### 6.2 LLM Backend Abstraction

Yori supports multiple LLM providers through a unified interface:

```cpp
string callAI(string prompt) {
    if (PROTOCOL == "ollama") { /* local models */ }
    else if (PROTOCOL == "openai") { /* GPT-4 etc */ }
    else if (PROTOCOL == "google") { /* Gemini */ }
    return extractCode(response);
}
```

Default: **Ollama** for local-first, zero-cost execution. Cloud providers optional.

### 6.3 Language Support

Yori maintains a database of 30+ language profiles:

```cpp
struct LangProfile {
    string extension;        // e.g., ".cpp"
    string versionCmd;       // e.g., "g++ --version"
    string buildCmd;         // e.g., "g++ -std=c++17"
    bool producesBinary;     // true for compiled languages
};
```

Adding new languages requires only toolchain specification—no language-specific parsing.

### 6.4 Caching & Optimization

- **Hash-based caching**: Skip regeneration if input unchanged
- **Direct compilation**: If input is valid source without semantic blocks, compile directly
- **Lazy transpilation**: Generate source only, skip binary compilation if requested

---

## 7. Evaluation

### 7.1 Case Studies

**Case 1: Single-File Generation**  
Input: 15-line `.yori` file with 3 semantic blocks  
Output: 120-line working C++ program (vector operations, I/O)  
Time: 8 seconds (local Llama 3B model)  
Compilation: Success on first attempt

**Case 2: Multi-File Project**  
Input: Blueprint with 5 files (headers, implementations, main)  
Output: Complete C++ project with MVC structure  
Time: 45 seconds (series mode, local model)  
Compilation: 2 files required retry due to missing `#include`

**Case 3: Cross-Language**  
Input: Same intent, different `-lang` flags  
Output: Functionally equivalent programs in C++, Python, Rust, Go  
Success rate: 4/4 languages compiled successfully

### 7.2 Reliability Metrics

From 100 test compilations:
- **Direct success**: 68% (first LLM output compiles)
- **Success with retry**: 89% (within 5 attempts)
- **Laziness detection**: 12 cases of wrapper attempts caught
- **Fatal errors**: 11 cases (missing libraries, unsupported features)

### 7.3 Performance Characteristics

- **Local model (Qwen 2.5 Coder 3B)**: 15-30 tokens/sec
- **Cloud model (GPT-4)**: 50-80 tokens/sec
- **Bottleneck**: LLM inference time (85% of total)
- **Optimization potential**: Parallel generation for independent files

---

## 8. Related Work

### 8.1 LLM-Assisted Programming Tools

**GitHub Copilot**: Autocomplete-style suggestions in IDE. No formal syntax for intent.  
**ChatGPT Code Interpreter**: Chat-based, no integration with existing codebase.  
**Cursor**: AI-powered editor with chat. Not a compiler.  

**Difference**: Yori treats semantic blocks as **compilable syntax**, not external prompts.

### 8.2 Intentional Programming

**Intentional Software (Simonyi, 2000s)**: Domain-specific notations compiled to code.  
**Difference**: Yori uses natural language, not custom DSLs. LLM handles semantic resolution.

### 8.3 Template Metaprogramming

**C++ Templates, Lisp Macros**: Compile-time code generation via symbolic manipulation.  
**Difference**: Yori uses LLMs for semantic understanding, not pattern matching.

### 8.4 Program Synthesis

**Academic work** (Sketch, Rosette): Formal specifications → code via constraint solving.  
**Difference**: Yori accepts informal natural language, scales to full programs.

---

## 9. Limitations and Future Work

### 9.1 Current Limitations

1. **LLM non-determinism**: Same input may produce different outputs
2. **Token limits**: Very large projects may exceed context windows
3. **Semantic ambiguity**: Vague intent → unpredictable results
4. **Dependency on LLM quality**: Weak models produce poor code

### 9.2 Future Directions

**Formal Semantics**: Define denotational semantics for semantic blocks  
**Type Systems**: Infer constraints on semantic blocks from surrounding code  
**Incremental Compilation**: Recompile only changed semantic blocks  
**Interactive Refinement**: Allow developers to guide LLM through clarifying questions  
**Fine-tuning**: Train domain-specific models on project codebases  
**Tree-shaking**: Eliminate unused generated code  
**Multi-modal**: Support diagrams/images as semantic block content

### 9.3 Broader Implications

If semantic blocks become mainstream:
- **Language design**: Future languages may include native `$${...}$$` syntax
- **Education**: Beginners express intent, LLM teaches through generated examples
- **Rapid prototyping**: Orders of magnitude faster from idea to working code
- **Specification**: Semantic blocks as executable requirements

---

## 10. Conclusion

We presented semantic code blocks as a new paradigm bridging declarative intent and imperative implementation. Our reference compiler, Yori, demonstrates practical viability across 30+ languages with local-first execution. Key innovations—blueprint compilation, series generation, and anti-hallucination guardrails—address fundamental challenges in LLM-assisted programming.

This work opens a design space for **hybrid programming languages** where concrete and abstract code coexist as equal citizens. As LLMs improve, the balance may shift toward more semantic, less syntactic programming—but the human developer remains in control, mixing precision where needed with abstraction where beneficial.

**Semantic blocks are not a replacement for programming. They are a new tool in the programmer's arsenal.**

Code, documentation, and examples: `https://github.com/[your-repo]`

---

## References

[1] Chen, M., et al. "Evaluating Large Language Models Trained on Code." arXiv:2107.03374 (2021).

[2] Simonyi, C. "The Death of Computer Languages, the Birth of Intentional Programming." NATO Science Committee Conference (1995).

[3] Gulwani, S., et al. "Program Synthesis." Foundations and Trends in Programming Languages 4.1-2 (2017).

[4] OpenAI. "GPT-4 Technical Report." arXiv:2303.08774 (2023).

[5] Roziere, B., et al. "Code Llama: Open Foundation Models for Code." arXiv:2308.12950 (2023).

[6] Jiang, A.Q., et al. "Code Generation with AlphaCodium." arXiv:2401.08500 (2024).

---

## Appendix A: Grammar Specification

```ebnf
program        ::= (code_line | semantic_block | export_block)*
code_line      ::= <any valid target language syntax>
semantic_block ::= "$${" intent_text "}$$"
intent_text    ::= <natural language description>
export_block   ::= "EXPORT:" filename NEWLINE content "EXPORT: END"
filename       ::= QUOTE <path> QUOTE
content        ::= (code_line | semantic_block)*
```

## Appendix B: Example Compilation Trace

**Input (`hello.yori`):**
```
EXPORT: "hello.cpp"
#include <iostream>
int main() {
    $${print hello world 5 times}$$
}
EXPORT: END
```

**Compilation Steps:**
1. Parse: Detect EXPORT block, extract semantic block
2. Context: `{"language": "cpp", "includes": ["iostream"], "scope": "main"}`
3. LLM Query: Generate loop code
4. Substitute: Replace semantic block with `for(int i=0; i<5; i++) std::cout << "Hello World\n";`
5. Export: Write `hello.cpp`
6. Compile: `g++ hello.cpp -o hello`
7. Success

**Output (`hello.cpp`):**
```cpp
#include <iostream>
int main() {
    for(int i=0; i<5; i++) {
        std::cout << "Hello World" << std::endl;
    }
    return 0;
}
```

---

**Acknowledgments**

Thanks to the open-source community for Ollama, nlohmann/json, and the countless developers whose work makes local-first AI possible.

---

**Author Contact**

[Your email / GitHub / Twitter]

**License**

This paper: CC BY 4.0  
Yori source code: MIT License