# Here is the roadmap to upgrade -refine from a powerful prototype to an enterprise-grade intent extractor.

1. From Heuristic Chunking to AST-Driven Slicing

The Current Problem: Right now, in parser.hpp, splitSourceCode uses a line-target (500 lines) and simple brace counting ({ vs }) to chunk the file. On real codebases with complex macros, #ifdef blocks, or deeply nested lambdas, this will break the code at awkward places, confusing the LLM.

- The Upgrade: You already have the groundwork for this in get_refine_query(const string& lang_id). We need to fully integrate Tree-sitter to parse the target language into an Abstract Syntax Tree (AST).

How it works: Instead of feeding the LLM "Chunk 1 to 5," we feed it exact logical units: "Class ResourceManager", "Function processPayment()".
This guarantees that the LLM always receives complete, syntactically whole thoughts to refine.

2. Global Symbol Graph (Replacing Sliding Context)

The Current Problem: In glupec.cpp, the refine loop uses previousContext to pass signatures from Chunk 1 to Chunk 2. If a function in Chunk 50 relies on a struct defined in Chunk 1, the sliding window might have forgotten it, leading to hallucinated intent.

The Upgrade: Implement a Two-Pass Refinement Pipeline:

Pass 1 (The Mapper): Run a fast, cheap AST pass over the entire codebase (or use an LSP like clangd/pylsp) to extract all function signatures, class definitions, and global variables into a project-wide JSON dictionary.
Pass 2 (The Refiner): When the LLM is asked to refine function X(), we query the global dictionary for only the dependencies X actually uses. We inject this exact graph into the prompt. This provides perfect context without blowing up the token limit.

3. The "Semantic Equivalence" Verification Loop
The Current Problem: The LLM might oversimplify. It might read a 100-line cryptographic hash function and refine it as $${ calculate hash }$$. While technically true, if we try to recompile that intent, the AI backend won't know which hash algorithm to use, breaking the system.

The Upgrade: We can use the compiler to grade the refinement automatically using a round-trip test.

Refine: Target Code $\rightarrow$ .glp Blueprint.
Transpile: .glp Blueprint $\rightarrow$ New Target Code (in a temp directory).
Diff: Use an AST-diff tool to compare the original function with the newly generated function.
Auto-Heal: If the ASTs are vastly different (e.g., missing loops, missing error handling), Glupe automatically prompts the LLM: "Your refined intent $${ calculate hash }$$ lost critical details. The original code handled SHA-256 padding. Try again, but be more granular."

4. Granularity Dials (Zoom Levels)

Different users want different things from -refine. A senior architect wants a high-level overview, while a maintainer wants a 1:1 functional map.

The Upgrade: Introduce refinement flags to control the prompt algebra:

glupe source.cpp -refine --level=arch: Strips out all logic. Refines the file into empty semantic containers showing only the architecture ($$ func1 {} $$, $$ classA {} $$).
glupe source.cpp -refine --level=algo: (The current behavior) Refines logic into numbered algorithmic steps.
glupe source.cpp -refine --level=strict: Forces the LLM to map every single line of control flow to a GIR pseudo-code equivalent, ensuring zero loss of detail.

5. Project-Level Refinement (Directory Sucking)
The Current Problem: Currently, -refine operates file-by-file.

The Upgrade: We need it to understand architecture across boundaries.

glupe ./src -refine -make
Glupe walks the directory tree. It detects #include "my_header.h" and automatically translates that relationship into Glupe's IMPORT: "my_header.glp" syntax.
It generates a single master .glp file containing EXPORT: blocks for the entire repository, instantly turning a legacy C codebase into a modern, language-agnostic Glupe project.