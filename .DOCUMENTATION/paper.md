# Intention is all you need

**Author:** Alonso Velazquez  
**Affiliation:** Sinaloa Autonomous University  
**Date:** March 29, 2026  
**arXiv Category:** cs.SE (Software Engineering), cs.PL (Programming Languages)

---

## Abstract

We introduce **Semantic Metaprogramming**, a novel paradigm where natural language intent is treated as first-class, computable data. To demonstrate this, we present **Glupe**, a proof-of-concept meta-compiler that enables developers to mix concrete code with high-level pseudocode or natural language intent within the same source file. Using a simple delimiter syntax (`$${...}$$`), programmers can write hybrid programs where architectural "load-bearing walls" are specified in traditional code, while algorithmic implementation details are expressed via human intent. 

Glupe transpiles these semantic containers to executable code using Large Language Models (LLMs), but unlike traditional AI assistants, it enforces strict architectural sandboxing. We introduce concepts such as **Prompt Algebra** and **Semantic Subtraction** ($Spec — Impl$) to mathematically verify that AI-generated code conforms to requested constraints without omission. Key features include targeted dependency injection via a Tree-sitter global symbol graph, AST-driven structural verification, and iterative semantic equivalence loops. 

**Keywords:** LLM-assisted programming, hybrid programming paradigm, semantic metaprogramming, code generation, AST-driven verification, prompt algebra

## 1. Introduction

Large Language Models have shown remarkable ability to generate code from natural language descriptions. However, current LLM-assisted programming tools operate primarily through two paradigms:

1. **Autonomous Agents / Chat interfaces** (Devin, ChatGPT): Full program generation that often operates outside the deterministic constraints of existing architectures.
2. **Autocomplete** (GitHub Copilot): Inline suggestions that act as advanced predictive text but lack formal compilation guarantees.

Both approaches treat natural language as **external** to the programming process. We propose a third paradigm: **Semantic Metaprogramming**, which treats natural language intent as first-class, compilable syntax within source files.

### 1.1 Motivating Example

Consider a developer who knows the high-level algorithm but not the language-specific APIs:

```cpp
#include <vector>
#include <iostream>

int main() {
    $${
        create a vector with the first 10 prime numbers
        print each one with its index
        calculate and print the sum
    }$$
    return 0;
}
```

The `$${...}$$` blocks are not comments, they are **semantic placeholders** compiled to concrete code by an LLM while preserving the surrounding C++ structure (includes, main signature, return statement).

This enables:
- **Gradual specification**: Start with intent, refine to code incrementally
- **Language learning**: Express what you want, learn syntax from generated output  
- **Rapid prototyping**: Focus on architecture, delegate boilerplate
- **Mixed expertise**: Domain experts collaborate with language experts in same file

### 1.2 Contributions

This essay describes the design and implementation of Glupe, a working meta-compiler supporting this paradigm. Our contributions are:

1. **Lightweight syntax convention** for embedding semantic blocks in source code
2. **Multi-file orchestration** through `EXPORT` directives for project generation
3. **Iterative refinement with error feedback** to improve LLM reliability
4. **AST-Driven structural verification and Semantic equivalence loops** to catch LLM omissions and anti-patterns
5. **Targeted dependency injection** using a Tree-Sitter based global symbol graph
6. **Integrated developer tooling** for code repair, documentation, and analysis
7. **Multi-provider LLM backend** supporting local and cloud models

---

## 2. System Design

### 2.1 Semantic Block Syntax

A **semantic block** is delimited by `$${` and `}$$` containing natural language or a custom DSL:

```
semantic_block ::= "$${" natural_language_text "}$$"
```

Design rationale:
- **Distinctive markers** (`$${`, `}$$`) avoid conflicts with existing language syntax
- **Natural language content**: No special formatting required
- **Position-independent**: Can appear anywhere, code can appear in target language
- **Identity**: Containers can be named like this:
```
$$ my_container { intent goes here }$$
```
**Benefits of Semantic Containers**: it effectively sandboxes AI generation in constrained spaces, but inherits the surrounding file as context, leaving it intact. This way developers can safely leverage the speed of AI generation without risking the stability of their existing architecture. They maintain absolute control over the file's structure, imports, and critical logic, effectively treating the AI as a specialized contractor who is strictly forbidden from touching the load-bearing walls of the application. This resolves the "trust gap" that currently prevents AI from being adopted in professional, mission-critical codebases.

**Current limitations:**
- No nesting support (`$${  $${...}$$  }$$` fails)
- Multi-line handling requires careful parsing
- No type constraints or formal semantics

**2.1.1 Native Comments**
Glupe introduces its own comment syntax to annotate semantic logic without affecting the host language:
- **Line comments**: `% This is a comment`
- **Block comments**: `%{ This is a multiline comment }%`

### 2.2 Compilation Pipeline

```
Input: Source file(s) with semantic blocks
Output: Executable binary or source code

Pipeline:
1. PREPROCESS: Resolve IMPORT directives (recursive module inclusion) and perform Directory Sucking
2. PARSE: Parse files using Flex/Bison frontend to extract semantic blocks and EXPORT directives, producing Glupe IR
3. GRAPH: Build Global Symbol Graph using Tree-sitter for targeted dependency injection
4. OPTIMIZE: If valid source without blocks → direct compile (bypass LLM)
5. GENERATE: Query LLM with context (surrounding code + errors + targeted injected dependencies)
6. VALIDATE: Compile with native toolchain and perform AST verification
7. ITERATE: On failure, retry with error feedback and semantic equivalence loop (max N attempts)
8. OUTPUT: Save binary or source file
```

**Key insight**: Unlike traditional compilers that reject incomplete programs, Glupe treats semantic blocks as **compilation tasks** rather than syntax errors.

### 2.3 Context Preservation

When generating code for a semantic block, Glupe provides the LLM with:

```
CONTEXT = {
    surrounding_code,      // Imports, type signatures, existing functions
    target_language,       // C++, Python, etc.
    previous_errors,       // Compilation failures from prior attempts
    user_instructions,     // Optional constraints via "*prompt"
    dependency_analysis    // Extracted #includes, imports
}
```

Example:
```cpp
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> data = {5, 2, 8, 1, 9};
    
    $${sort the vector and remove duplicates}$$
    
    for(auto x : data) std::cout << x << " ";
}
```

The LLM sees `#include <algorithm>` and knows to use `std::sort`, `std::unique` rather than reimplementing sorting.

### 2.4 Multi-File Projects: EXPORT Directives

For multi-file projects, Glupe introduces `EXPORT` blocks:

```
EXPORT: "filename.ext"
<file content with optional semantic blocks>
EXPORT: END
```

Content inside `EXPORT` blocks is written to the specified file. Content **outside** acts as project-level instructions without being exported.

**Example:**
```
This project implements a calculator with separate interface/logic.
Use only standard library. No external dependencies.

EXPORT: "calculator.h"
#ifndef CALCULATOR_H
#define CALCULATOR_H
$${define Calculator class with add, subtract, multiply, divide methods}$$
#endif
EXPORT: END

EXPORT: "calculator.cpp"
#include "calculator.h"
$${implement Calculator methods}$$
EXPORT: END

EXPORT: "main.cpp"
#include <iostream>
#include "calculator.h"
$${create CLI interface for the calculator}$$
EXPORT: END
```

The first two lines provide architectural context but don't appear in any output file. This separates **project intent** from **file structure**.

**Implementation:**
- Simple line-by-line parser scanning for `EXPORT:` markers
- Filename extraction via regex for quoted strings
- Automatic directory creation for nested paths
- Sequential or parallel file generation.

### 2.5 Advanced Semantics: Inheritance and Abstraction (v5.8+)

Glupe v5.8 introduced object-oriented concepts to semantic containers, enabling architectural reuse and policy enforcement.

**1. Abstract Containers**
Containers marked with `ABSTRACT` do not generate code themselves but serve as templates for other containers.

```glupe
$$ABSTRACT secure_api {
    All database queries must use parameterized statements.
    All inputs must be sanitized.
    Use try-catch blocks for all network calls.
}$$
```

**2. Container Inheritance**
Containers can inherit intent from parents using the `->` operator. The child container inherits the "DNA" (instructions) of the parent, automatically applying constraints or logic.

```glupe
$$ login_handler -> secure_api {
    Implement the user login function.
}$$
```

**3. Multiple Inheritance**
Glupe supports mixing multiple architectural traits:

```glupe
$$ critical_endpoint -> logging, metrics {
    Handle the payment processing request.
}$$
```

This solves the "Drift" problem: if the security policy changes in the parent, every inheriting container updates automatically upon recompilation.

### 2.6 Prompt Algebra Basics

When natural language intent is treated as a first-class data type, we can impose strict mathematical topology on it. Instead of blindly concatenating strings to send to an LLM, Glupe acts as an algebraic "calculator," manipulating Semantic Nodes using structural operators to compute a deterministic Final State of Intent.

Let I represent a Semantic Node (an Intent).

The core operations of Glupe's Intent Algebra include:

**1. Addition (Contextual Merging)**
Combining two or more distinct intents into a unified semantic space using Multiple Inheritance (`-> A, B`).
*Formalization:* $\mathcal{I}_{child} = \mathcal{I}_{A} \oplus \mathcal{I}_{B}$
*Glupe Syntax:* `$$ C -> A, B { build endpoint }$$`
The compiler mathematically merges the constraints into a single, unified prompt vector before transpilation. If the conditions are contradictory it throws a compile time error.

**2. Subtraction (Constraint Overriding)**
Removing specific instruction members or negating inherited rules. In Glupe, explicit contradiction in a child block or right-to-left precedence acts as a subtractive override.
*Formalization:* I= I_parent — C_conflict
*Example:* A parent dictates "Log all user data," while a child overrides with "Do not log passwords." The conflict is mathematically bounded by the child's explicit subtraction.

**3. Multiplication (Functional Amplification)**
Iterative execution or mapping of one intent over another (Context Injection). By passing a small, isolated intent `x` into a functional container `F(x)`, the AI multiplies the effect of `x` across an entire domain.
*Formalization:* $F(\mathcal{I}_x) = \mathcal{I}_x \otimes Iterator$
*Glupe Syntax:* `$$ process_database(x) { apply x to every column }$$`

**4. The Identity Matrix (Abstract Variables)**
A node that shapes other nodes without possessing its own executable mass. It acts purely as a mathematical limit (e.g., `$ABSTRACT: MAX_LATENCY -> 50ms`).
*Formalization:* I_{final} = I_target * I_abstract

Because Glupe resolves this algebra *before* calling the LLM, it evaluates the Abstract Syntax Tree (AST) of intent, substitutes variables, merges bounds, and calculates the simplified deterministic equation. If two intents demand mutually exclusive output states, their sum mathematically resolves to an empty set (0), allowing the compiler to throw a Compile-Time Error before any token is wasted on inference.

---

## 3. Reliability and Verification Mechanisms

LLMs are non-deterministic and prone to failures. Glupe implements several mechanisms to improve reliability:

### 3.1 Iterative Compilation with Error Feedback

Rather than single-shot generation, Glupe uses a **retry loop with accumulated error history**:

```
error_history = ""
for attempt in 1..MAX_RETRIES:
    code = LLM_generate(intent + context + error_history)
    result = native_compile(code)
    
    if result.success:
        return code
    
    error_history += "\n--- Attempt " + attempt + " failed ---\n"
    error_history += result.compiler_errors
    
    if is_fatal_error(result.errors):
        break
end
```

Compiler error messages provide **ground truth feedback** that guides the LLM toward valid solutions. We observe that most errors are fixed within 2-4 iterations.

### 3.2 Pattern-Based Failure Detection

LLMs often attempt "lazy" solutions rather than native implementation:

**Common anti-patterns:**
- Including `<Python.h>` to embed Python in C++
- Using `system("python script.py")` to shell out
- Generating JNI wrappers instead of Java code
- Literal translation (writing Python syntax in a C++ file)

**Detection strategy:**
```cpp
bool isFatalError(const string& compilerOutput) {
    string lower = toLowerCase(compilerOutput);
    
    // Wrapper detection
    if (lower.find("python.h") != string::npos) return true;
    if (lower.find("jni.h") != string::npos) return true;
    
    // Literal translation detection
    if (lower.find("def ") != string::npos) return true;  // Python function
    if (lower.find("print(") != string::npos) return true; // Python print
    
    // Genuine missing dependencies
    if (lower.find("no such file") != string::npos) return true;
    
    return false;
}
```

When detected, Glupe annotates the error message:
```
"FATAL: You attempted to include python.h. STOP. 
Rewrite using native C++ standard library only."
```

This **trains the LLM** through reinforcement to avoid lazy patterns.

### 3.3 Preflight Dependency Checking

Before invoking the LLM, Glupe extracts all dependencies (`#include`, `import`, etc.) and verifies they exist:

```cpp
dependencies = extract_dependencies(code)
compile_test_file_with_dependencies(dependencies)

if compilation_fails:
    report_missing_dependencies()
    abort_before_expensive_LLM_call()
```

This prevents wasted API calls and tokens on doomed-to-fail generations. 
>Note: as of version 5.8 this is only implemented for python and c/c++

### 3.4 Direct Compilation Optimization

**Key insight**: If input files are already valid source code in the target language and contain no semantic blocks, bypass LLM entirely.

```cpp
if (all_files_match_target_extension && no_semantic_blocks) {
    compile_directly_with_native_toolchain();
    // Zero overhead for traditional workflows
}
```

This makes Glupe a **drop-in replacement** for native compilers when no AI features are needed.

### 3.5 AST-Driven Structural Verification

LLMs are notorious for being "lazy," often oversimplifying code when asked to refactor or comment, replacing critical logic with `// ... implementation goes here ...`. To counter this, Glupe integrates Tree-sitter for Abstract Syntax Tree (AST) manipulation. 

During modification operations (like `fix` or `explain`), Glupe mathematically compares the node count of structural milestones (`function_definition`, `if_statement`, `for_statement`, etc.) between the original code and the AI-generated output. If the complexity ratio drops below a strict threshold (e.g., `< 0.7`), indicating the AI dropped significant structural logic, Glupe physically rejects the output and forces the AI to try again with an explicit system alert. This structural feedback loop forces non-deterministic AI models to maintain algorithmic fidelity.

### 3.6 Semantic Subtraction (The "Omission Blindspot" Guardrail)

A critical flaw in autonomous coding agents is their inability to detect omissions. LLMs are decent at catching contradictions but terrible at realizing what they forgot to implement from a specification. Standard unit tests only verify *if* written code runs, not *how* it was architected or if constraints were silently ignored.

Glupe introduces a mathematical verification gate called **Semantic Subtraction ($Spec — Impl$)**. 
1. The developer provides the Expected Intent (`spec.glp`).
2. Glupe reverse-engineers the AI-generated code (`agent_output.cpp`) back into a semantic blueprint (`actual_impl.glp`) using AST-driven refinement.
3. Glupe performs a two-way Set Subtraction between both blueprints.

The `glupe audit` command mathematically evaluates the absence of requested logic, catching missed requirements (Missing) and architectural liberties (Hallucinated). Equipped with a `--ignore-scaffold` flag to forgive standard language boilerplate, this provides a deterministic, CI/CD-ready guardrail that can automatically block AI Pull Requests if they drift from the master architectural intent.

---

## 4. LLM Backend Architecture

### 4.1 Multi-Provider Support

Glupe abstracts LLM providers through a unified interface:

```cpp
string callAI(string prompt) {
    json body;
    
    if (PROTOCOL == "google") {
        body["contents"][0]["parts"][0]["text"] = prompt;
        url += "?key=" + API_KEY;
    } 
    else if (PROTOCOL == "openai") {
        body["messages"][0]["role"] = "user";
        body["messages"][0]["content"] = prompt;
        headers += "Authorization: Bearer " + API_KEY;
    }
    else { // ollama (local)
        body["model"] = MODEL_ID;
        body["prompt"] = prompt;
        body["stream"] = false;
    }
    
    response = curl_post(url, body);
    return extract_code(response);
}
```

**Supported backends:**
- **Ollama** (default): Local models, zero cost, privacy-preserving
- **OpenAI**: GPT-4, GPT-5 via API
- **Google**: Gemini Pro, Flash via API
- **Compatible APIs**: Groq, together.ai, any OpenAI-compatible endpoint

Configuration via `config.json`:
```json
{
  "local": {
    "model_id": "qwen2.5-coder:3b",
    "api_url": "http://localhost:11434/api/generate"
  },
  "cloud": {
    "protocol": "openai",
    "model_id": "gpt-4-turbo",
    "api_key": "sk-...",
    "api_url": "https://api.openai.com/v1/chat/completions"
  },
  "max_retries": 15
```
This will open the browser in apifreellm.com website, where users can access to free API keys. 

Users can switch providers with flags:
```bash
glupe hello.glp -local   # Use Ollama
glupe hello.glp -cloud   # Use configured cloud provider
```

### 4.2 Rate Limiting and Backoff

Cloud providers enforce rate limits. Glupe implements exponential backoff:

```cpp
for (int attempt = 0; attempt < 3; attempt++) {
    response = api_call(prompt);
    
    if (response.contains("429") || response.contains("Rate limit")) {
        wait_seconds = 5 * (attempt + 1);
        sleep(wait_seconds);
        continue;
    }
    break;
}
```

### 4.3 Response Parsing

LLM responses vary by provider. Glupe extracts code blocks:

```cpp
string extractCode(string jsonResponse) {
    json j = parse(jsonResponse);
    
    // OpenAI format
    if (j.contains("choices"))
        text = j["choices"][0]["message"]["content"];
    
    // Google format
    else if (j.contains("candidates"))
        text = j["candidates"][0]["content"]["parts"][0]["text"];
    
    // Ollama format
    else if (j.contains("response"))
        text = j["response"];
    
    // Strip markdown code fences
    if (text.contains("```")) {
        return extract_between_fences(text);
    }
    
    return text;
}
```

---

## 5. Developer Tools

Beyond transpilation, Glupe provides AI-powered development utilities:

### 5.1 Code Repair

```bash
glupe fix myfile.cpp "change the sorting algorithm in line 644 to use quicksort"
```

**Workflow:**
1. Read existing file
2. Send to LLM with instruction: "Apply this change to the code"
3. Replace file with fixed version

**Use cases:**
- Fix specific bugs without full rewrite
- Apply refactorings ("extract this into a function")
- Update deprecated APIs

### 5.2 Interactive Help (SOS)

```bash
glupe sos english "error: no matching function for call to 'std::vector<int>::push'"
```

**Workflow:**
1. Send error message + language context to LLM
2. Get explanation + suggested fix
3. Display in terminal

**Use cases:**
- Debug unfamiliar errors
- Learn language-specific idioms
- Get unstuck quickly

### 5.3 Documentation Generation

```bash
glupe explain myfile.cpp Spanish
```

**Workflow:**
1. Read source file
2. Prompt LLM: "Add technical comments explaining this code in [language]"
3. Save as `myfile_doc.cpp`

**Use cases:**
- Document legacy code
- Generate multi-language documentation
- Create teaching materials

### 5.4 Semantic Diff Analysis

```bash
glupe diff version1.cpp version2.cpp
```

**Workflow:**
1. Read both files
2. Prompt LLM: "Analyze semantic differences, identify changed behavior"
3. Generate Markdown report

**Output example:**
```markdown
## Summary
Function `calculate()` changed from iterative to recursive implementation.

## Changed Functions
- `calculate(int n)`: Now uses memoization via static map

## Observations
- Performance improved for repeated calls
- Risk: Stack overflow for n > 10000
```

**Use cases:**
- Code review
- Understand refactorings
- Identify behavioral changes beyond textual diffs

### 5.5 Semantic Refinement (Compression)

```bash
glupe source.cpp -refine [-local | -cloud]
```

**Workflow:**
1. Reads the source code file.
2. Prompts the LLM to "semantically compress" the implementation details into high-level intent blocks (`$${...}$$`), while preserving the architectural skeleton (imports, class definitions, function signatures).
3. Outputs a `.glp` file (e.g., `source.cpp.glp`).

**Implications:**
- **Legacy Modernization**: Converts "dead" legacy code back into "living" intent. A developer can refine a legacy C file into a `.glp` file, then recompile it targeting Rust or Python.
- **Intent Recovery**: Recovers the "why" behind the code that is often lost in implementation details.
- **Codebase Compression**: Reduces the cognitive load for developers reading the file, as they see the high-level logic first.
- **It allows for 'no rot' software**: Unlike traditional code, intention does not age, a program coded in `.glp` can be recompiled into the same program in 10 or 20 years since it preserves the intent.

### 5.6 Architectural Auditing (Semantic Subtraction)

```bash
glupe audit spec.glp implementation_refined.glp --ignore-scaffold
```

**Workflow:**
1. Read the expected architectural blueprint (`spec.glp`).
2. Read the actual refined blueprint of the generated code (`implementation_refined.glp`).
3. Align containers semantically (resolving AI naming drift).
4. Perform mathematical Set Subtraction to generate a report of missing and hallucinated features.
5. Return a standard exit code (`0` for success, `1` for divergence) to act as a CI/CD pipeline guardrail.

**Use cases:**
- Mathematical verification of AI-generated Pull Requests.
- Test-Driven Architecture (TDA).
- Ensuring strict adherence to global security policies and architectures.

**Implications:**
- **Zero-Trust AI Integration**: Organizations no longer need to blindly trust that an LLM understood the system architecture. They can mathematically prove that no constraints were ignored and no unauthorized logic was hallucinated.
- **Automated Code Review**: Shifts the cognitive burden of reviewing AI-generated code from human reviewers to a deterministic compiler gate.
- **Living Architecture Enforced**: The design blueprint (`spec.glp`) ceases to be a stale wiki page and becomes a strict, enforceable contract governing the repository.

### 5.7 Targeted Implementation (-fill)

```bash
glupe source.cpp -fill [-local | -cloud]
```

**Workflow:**
1. Reads the source code file containing semantic containers (`$${...}$$`).
2. Prompts the LLM to generate the implementation details exclusively for those containers, using the surrounding code as context.
3. In-place replaces the containers with the generated native code, leaving the rest of the file untouched.

**Use cases:**
- **Boilerplate Elimination**: Write the signatures and let the AI fill in the repetitive implementations.
- **Progressive Enhancement**: Iteratively fill in complex logic blocks one by one while manually testing the surrounding architecture.

This calls for a spec-first driven coding paradigm. 

---

## 6. Language Support

### 6.1 Language Profile System

Glupe supports 30+ languages through a declarative profile database:

```cpp
struct LangProfile {
    string extension;        // e.g., ".cpp"
    string versionCmd;       // e.g., "g++ --version"
    string buildCmd;         // e.g., "g++ -std=c++17"
    bool producesBinary;     // true for compiled languages
};

map<string, LangProfile> LANG_DB = {
    {"cpp", {".cpp", "g++ --version", "g++ -std=gnu++17", true}},
    {"py",  {".py",  "python --version", "python -m py_compile", false}},
    {"rust",{".rs",  "rustc --version", "rustc", true}},
    // ... 30+ more
};
```

**Adding new language support:**
1. Add entry to `config.json` with toolchain commands
2. No language-specific parsing required
3. ~5 lines of configuration

**Currently supported:**
- **Compiled**: C, C++, Rust, Go, Swift, Zig, Nim, Haskell, Dart
- **Interpreted**: Python, JavaScript, Ruby, PHP, Perl, Lua, Julia, R
- **JVM**: Java, Kotlin, Scala, Clojure
- **Scripting**: Bash, PowerShell, Batch
- **Specialized**: TypeScript, LaTeX, SQL, HTML, CSS, Markdown

### 6.2 Cross-Language Transpilation

Same semantic description can target multiple languages:

```bash
glupe program.Glupe -cpp -o program.exe
glupe program.Glupe -py  -o program.py
glupe program.Glupe -rust -o program
```

**Use cases:**
- Language exploration ("How would this look in Rust?")
- Prototyping in Python, production in C++
- Educational materials showing same algorithm in multiple languages

### 6.3 Context assimilation

Glupe supports this
```
Glupe script.py file.c scriptB.js -o app.exe -cpp 
```
Glupe acts as a universal translator:

1. Read Phase: It reads script.py (Python), file.c (C), and scriptB.js (JavaScript).
2. Context Extraction: It ignores the syntax differences and extracts the Logic and Intent (including any $${ ... }$$ blocks).
3. Harmonization: It builds a unified "Project Context." It sees that script.py defines a data structure and file.c defines a sorting algorithm.
4. Transpilation: It melts down all that logic and re-casts it into the target language: C++ (-cpp).

Output: It produces a single, unified native binary app.exe.


---

## 7. Implementation Details

### 7.1 Import Resolution

Glupe supports modular semantic code through `IMPORT:` directives:

```
IMPORT: "utils.Glupe"

int main() {
    $${use the helper functions from utils.glp}$$
}
```

**Features:**
- Recursive import resolution
- Cycle detection
- Contextual inclusion (imports are visible to LLM)
>note: as of version 5.8 Glupe does not support tree shaking yet.
### 7.2 Template Stripping

When exporting files, Glupe strips semantic blocks from output:

```cpp
// Input (EXPORT block):
int main() {
    $${initialize variables}$$
    return 0;
}

// After LLM generation:
int main() {
    int x = 0;
    int y = 0;
    return 0;
}

// Output file (templates stripped):
int main() {
    int x = 0;
    int y = 0;
    return 0;
}
```

This ensures generated files contain only concrete code, not semantic placeholders.

### 7.3 File Locking and Retry Logic

On Windows, file locking can prevent immediate file replacement. Glupe implements retry logic:

```cpp
for (int attempt = 0; attempt < 5; attempt++) {
    try {
        remove_if_exists(output_file);
        copy_file(temp_file, output_file);
        success = true;
        break;
    } catch (filesystem_error& e) {
        sleep_milliseconds(250 * (attempt + 1)); // Exponential backoff
    }
}
```

### 7.4 Caching

Glupe hashes input content and skips recompilation if unchanged:

```cpp
current_hash = hash(source_code + target_language + model_id);
```
1. **Global Build Cache**: Glupe hashes the aggregated input content and skips recompilation if the entire project context is unchanged.
2. **Semantic Container Caching (v5.8)**: Named containers (`$$ "id" { ... }$$`) are hashed individually and tracked in a `.Glupe.lock` file. When running in update mode (`-u`), Glupe compares the current prompt hash against the lockfile.
   - **Match**: The cached implementation is injected from `glupe_cache/`, bypassing the LLM.
   - **Mismatch**: The block is regenerated.
   
This enables **incremental builds**, allowing developers to "freeze" working logic while iterating on other parts of the system.
```
if (cached_hash == current_hash && output_exists) {
    cout << "[CACHE] No changes detected. Using existing build.\n";
    return;
}
```

**Cache invalidation triggers:**
- Source file modification
- Target language change
- Model ID change
- Update mode flag (`-u`)
- Explicit cache cleaning (`glupe clean cache`)

### 7.5 Modular Architecture (v5.9)

As of version 5.9, the Glupe compiler has been refactored from a monolithic source file into a modular header-only architecture (`src/*.hpp`). This improves maintainability and allows for easier extension of core components:
- `parser.hpp`: Syntax analysis and container extraction
- `ai.hpp`: LLM provider abstraction
- `cache.hpp`: Semantic hashing and lockfile management
- `processor.hpp`: Core transpilation logic

---

## 8. Problems that can become more accessible with LLMs and Glupe
Here is a list of problems and domains that are now solvable or significantly more accessible thanks to the Glupe paradigm:

1. The "Trust Gap" in Enterprise AI Adoption
Problem: CTOs want the productivity of AI, but they cannot risk the AI hallucinating security vulnerabilities or breaking architecture in production code.

> Glupe Solution: Semantic Containers act as a Firewall. The AI is strictly forbidden from touching the "Architecture" (manual code). It can only operate inside the logic containers, making AI coding safe enough for enterprise use.

2. The "Technical Debt" Spiral
Problem: Over time, code becomes messy ("spaghetti code") because developers take shortcuts to meet deadlines. Refactoring is risky because you might break existing features.

> Glupe Solution: Since the "Source of Truth" is the Intent (the prompt), the code is disposable. If the code becomes messy, you simply change the prompt and re-compile. The "Technical Debt" is erased because the code is temporary; only the Intent is permanent.

3. The "Boilerplate" Burden
Problem: Developers spend 30-50% of their time writing repetitive boilerplate code (API endpoints, CRUD operations, getters/setters, config files).

>Glupe Solution: Create a "Boilerplate Container" once. Use it across the entire project. Change it in one place, and every instance updates instantly.

4. The "Single-Point of Failure" (Bus Factor)
Problem: If a key developer leaves a team, they take the knowledge of "how the code works" with them. The remaining team struggles to understand the complex syntax.

>Glupe Solution: The knowledge is stored in Natural Language inside the source file. A new developer can read the .Glupe file and immediately understand why the code exists, not just what it does. The "Bus Factor" is mitigated by readable source code.

5. The "IoT/Embedded Resource" Constraint
Problem: Writing firmware for microcontrollers (Arduino, ESP32) often requires low-level C/C++ knowledge that hobbyists don't have.

>Glupe Solution: Hobbyists write high-level logic 
```
$${ blink LED if temperature > 30 }$$
``` 
> and Glupe compiles it to the tight, low-level C++ required by the microcontroller.

6. Educational Accessibility
Problem: Computer Science courses lose 50% of students in the first semester because of syntax errors (missing semicolons, pointer confusion).

>Glupe Solution: Students learn Logic and Algorithms first using Acorn. They focus on problem-solving rather than syntax debugging. The "compiler" teaches them by showing the generated code.

7. The "Documentation Drift"
Problem: Code changes, but documentation doesn't. The docs become lies.

>Glupe Solution: The Documentation is the Code. The semantic blocks describe the intent. There is no separate document to maintain, so it never goes out of date.

## 9. Related Work

### 9.1 LLM-Assisted Programming Tools

**GitHub Copilot** [[1]](#references): IDE autocomplete based on GPT Codex. Provides line/block suggestions but requires accepting/rejecting each suggestion. Not integrated into compilation pipeline.

**ChatGPT / Claude**: Full program generation through chat. Excellent for greenfield development but disconnected from existing codebases. Requires copy-paste workflow.

**Cursor**: AI-powered IDE with chat interface and codebase awareness. Still operates through suggestions rather than compilable syntax.

**Key difference**: Glupe treats semantic blocks as **part of the source code** rather than external prompts. Generated code is validated through native toolchains, not just "accepted" by the user.

### 9.2 Intentional Programming

**Intentional Software (Simonyi, 2000s)** [[2]](#references): Domain-specific notations compiled to code through projectional editing. Required custom language workbenches.

**Key difference**: Glupe uses natural language interpreted by LLMs, not custom DSLs. No special tooling required—semantic blocks work in any text editor.

### 9.3 Program Synthesis

**Sketch** [[3]](#references): Constraint-based synthesis from partial programs with holes.  
**Rosette** [[4]](#references): Solver-aided synthesis from formal specifications.

**Key difference**: These systems require formal specifications (types, assertions, examples). Glupe accepts informal natural language.

### 9.4 Natural Language Programming

**AlphaCode** [[5]](#references): Generates competitive programming solutions from problem descriptions.  
**CodeGen** [[6]](#references): Multi-turn program synthesis from conversational prompts.

**Key difference**: These systems generate complete programs from scratch. Glupe enables **hybrid** programming where developers mix concrete and abstract code.

---

## 10. Limitations and Future Work

### 10.1 Current Limitations

**1. No formal semantics**  
Semantic blocks lack type constraints. The LLM may generate code that compiles but has incorrect semantics if the intent is ambiguous.

*Example:* "$${sort the array}$" could mean ascending, descending, or by custom comparator.

**2. Non-determinism**  
Same input may produce different outputs across runs due to LLM sampling.

*Mitigation:* Caching locks generated code for unchanged inputs. Developers can "freeze" satisfactory outputs.

**3. No nesting support**  
Semantic blocks cannot be nested: `$${  $${inner}$$  }$$` fails to parse.

**4. Security risks**  
Generated code executes with user privileges. Malicious LLM output could run arbitrary commands.

*Mitigation needed:* Sandboxed execution, code signing, static analysis.

**5. Token limits**  
Very large projects may exceed LLM context windows (typically 32K-128K tokens).

*Mitigation:* Series mode generates files sequentially with accumulated context.

**6. Dependency on LLM quality**  
Weak models (e.g., 1B parameter) produce poor code. System requires competent models (3B+ parameters recommended).

**7. Platform-specific issues**  
File locking on Windows requires retry logic. Some toolchains (LaTeX, .NET) have complex setup requirements.

### 11 Future Directions
1. **Type-aware semantic blocks:**
```cpp
int $${compute factorial}$$ (int n);  // LLM knows return type
```
Infer constraints from surrounding code (type signatures, variable usage). 


**2. Fine-tuning on project codebases:**
Train domain-specific models that understand project conventions, naming patterns, architecture.

**3. Multi-modal input:**
```cpp
$${
    [diagram.png]  // Flowchart or architecture diagram
    implement this state machine
}$$
```
---

## 12. Conclusion

We presented **Glupe**, a practical system for hybrid AI-assisted programming where natural language intent (or any DSL) and concrete code coexist as equal citizens. Our key insights:

1. **Lightweight syntax** (`$${...}$$`) integrates seamlessly with existing languages
2. **Iterative refinement** with compiler error feedback dramatically improves LLM reliability
3. **Pattern detection** catches common failure modes before wasted compilation attempts
4. **Multi-provider abstraction** enables local-first development with cloud fallback
5. **Developer tooling** (fix, explain, diff, sos) leverages same LLM backend for broader utility

Glupe demonstrates that semantic programming is **practical today** with current LLMs. As models improve, the balance may shift toward more semantic, less syntactic programming—but the developer remains in control, mixing precision where needed with abstraction where beneficial.

**Semantic blocks are not a replacement for programming. They are a new tool in the programmer's arsenal.**

---

## Acknowledgments

Thanks to the open-source community for Ollama, nlohmann/json, and the countless developers whose work enables local-first AI.

Thanks to Google for Gemini 3 VS Code extension, it was very useful all the way.

Thanks to Krzystof Dudek for useful feedback and collaboration. 
---

## References

[1] Chen, M., et al. "Evaluating Large Language Models Trained on Code." *arXiv:2107.03374* (2021).

[2] Simonyi, C. "The Death of Computer Languages, the Birth of Intentional Programming." *NATO Science Committee Conference* (1995).

[3] Solar-Lezama, A., et al. "Combinatorial Sketching for Finite Programs." *ASPLOS* (2006).

[4] Torlak, E., & Bodik, R. "A Lightweight Symbolic Virtual Machine for Solver-Aided Host Languages." *PLDI* (2014).

[5] Li, Y., et al. "Competition-Level Code Generation with AlphaCode." *Science* 378.6624 (2022).

[6] Nijkamp, E., et al. "CodeGen: An Open Large Language Model for Code with Multi-Turn Program Synthesis." *arXiv:2203.13474* (2022).

[7] Roziere, B., et al. "Code Llama: Open Foundation Models for Code." *arXiv:2308.12950* (2023).

[8] OpenAI. "GPT-4 Technical Report." *arXiv:2303.08774* (2023).

---

## Appendix A: Command Reference

### Compilation
```bash
glupe file.glp                      # Compile with local LLM
glupe file.glp -cloud               # Use cloud provider
glupe file.glp -cpp -o program      # Target C++
glupe file.glp -py -o script.py     # Target Python
glupe file.glp -make                # Multi-file project mode
glupe file.glp -run                 # Compile and execute
glupe file.glp -zig -t              # Transpile only (no binary)
```

### Developer Tools
```bash
glupe fix file.cpp "instruction"   # AI-powered code repair
glupe explain file.cpp Spanish     # Generate documentation
glupe diff v1.cpp v2.cpp           # Semantic diff analysis
glupe sos cpp "error message"      # Interactive help
glupe file.cpp -refine             # Reverse engineer to .glp
glupe file.cpp -fill               # Fill semantic containers in-place
```

### Configuration
```bash
glupe config see                   # View current config
glupe config api-key YOUR_KEY      # Set cloud API key
glupe config model-local           # Select Ollama model (interactive)
glupe config model-cloud gpt-4     # Set cloud model
glupe config max-retries 20        # Set retry limit
```

### Utilities
```bash
glupe --init                       # Create project template
glupe --version                    # Show version
glupe --clean                      # Remove temp files
glupe clean cache                  # Clear semantic cache
```

---

## Appendix B: Example Compilation Trace

**Input (`hello.glp`):**
```
EXPORT: "hello.cpp"
#include <iostream>

int main() {
    $${print hello world 5 times}$$
    return 0;
}
EXPORT: END
```

**Compilation Log:**
```
[CHECK] Toolchain for C++...
   [OK] Ready.
[INFO] 'EXPORT:' directive detected. Auto-enabling Architect Mode (-make).
[CHECK] Verifying dependencies locally...
   [OK] Dependencies verified.
   [Pass 1] Architecting Project...
[EXPORT] Writing to hello.cpp...
   Verifying...
BUILD SUCCESSFUL: hello.cpp
   [Source]: hello.cpp
```

**Output (`hello.cpp`):**
```cpp
#include <iostream>

int main() {
    for(int i = 0; i < 5; i++) {
        std::cout << "Hello World" << std::endl;
    }
    return 0;
}
```

---

## Appendix C: Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    Glupe COMPILER                        │
│                                                         │
│  Input: .Glupe files with semantic blocks                │
│         ↓                                               │
│  ┌──────────────────────────────────────────┐           │
│  │ 1. PREPROCESSOR                          │           │
│  │    - Resolve IMPORT: directives          │           │
│  │    - Detect EXPORT: blocks               │           │
│  │    - Check for semantic blocks           │           │
│  └────────────┬─────────────────────────────┘           │
│               ↓                                         │
│  ┌──────────────────────────────────────────┐           │
│  │ 2. OPTIMIZATION CHECK                    │           │
│  │    - If valid source + no blocks         │           │
│  │      → Direct compilation (bypass LLM)   │           │
│  └────────────┬─────────────────────────────┘           │
│               ↓                                         │
│  ┌──────────────────────────────────────────┐           │
│  │ 3. CONTEXT ASSEMBLY                      │           │
│  │    - Extract surrounding code            │           │
│  │    - Analyze dependencies                │           │
│  │    - Prepare LLM prompt                  │           │
│  └────────────┬─────────────────────────────┘           │
│               ↓                                         │
│  ┌──────────────────────────────────────────┐           │
│  │ 4. LLM BACKEND (Multi-Provider)          │           │
│  │    ┌─────────────────────────────────┐   │           │
│  │    │ Ollama (Local)                  │   │           │
│  │    │ OpenAI (Cloud)                  │   │           │
│  │    │ Google (Cloud)                  │   │           │
│  │    └─────────────────────────────────┘   │           │
│  └────────────┬─────────────────────────────┘           │
│               ↓                                         │
│  ┌──────────────────────────────────────────┐           │
│  │ 5. VALIDATION & ITERATION                │           │
│  │    - Compile with native toolchain       │           │
│  │    - Detect errors/anti-patterns         │           │
│  │    - Retry with error feedback           │           │
│  │    - Max 15 attempts                     │           │
│  └────────────┬─────────────────────────────┘           │
│               ↓                                         │
│  ┌──────────────────────────────────────────┐           │
│  │ 6. OUTPUT                                │           │
│  │    - Binary (if compiled language)       │           │
│  │    - Source (if interpreted/requested)   │           │
│  │    - Multi-file exports (if -make mode)  │           │
│  └──────────────────────────────────────────┘           │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

**Project Repository:** `https://github.com/alonsovm44/glupe`  
**License:** MIT  
**Contact:** [alonsovm443@outlook.com]