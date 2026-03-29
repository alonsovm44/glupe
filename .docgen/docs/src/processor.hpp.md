### `safeCallAI` Function

**Purpose:**  
Executes an AI call with built-in retry logic, rate limit handling, and error recovery.

**Usage:**  
Used as a wrapper for `callAI` to handle transient errors, API rate limits, and exponential backoff.

**Behavior:**  
- Retries up to `MAX_RETRIES` times on errors.
- Detects rate limit errors (429) and waits dynamically based on API response or exponential backoff.
- Handles other errors with exponential backoff.
- Returns the AI response or an error message if all retries fail.

---

### `substituteVariables` Function

**Purpose:**  
Replaces variable placeholders (`$VAR`) in a string with their corresponding values from the global `SYMBOL_TABLE`.

**Usage:**  
Used to resolve variable references in prompts or code before processing.

**Behavior:**  
- Scans the input string for `$` followed by a valid variable identifier.
- Ignores special syntax like `$$`, `$:`, `${`, and `$ ` (dollar followed by space).
- Looks up the variable in `SYMBOL_TABLE` and substitutes its value if found, respecting file-specific prefixes.
- Handles nested variables and preserves non-variable `$` occurrences.
- Supports optional container resolution via the `allowContainers` flag.

---

### `checkSemanticContradiction` Function

**Purpose:**  
Determines if two instruction sets are logically contradictory using AI analysis.

**Usage:**  
Called during prompt arithmetic to validate logical consistency between terms.

**Behavior:**  
- Constructs a prompt asking the AI to identify contradictions between two sets of instructions.
- Returns `true` if the AI detects a contradiction, otherwise `false`.
- Used to prevent logically impossible combinations in prompt arithmetic.

---

### `performSemanticSubtraction` Function

**Purpose:**  
Performs semantic subtraction of one instruction set from another using AI.

**Usage:**  
Used in prompt arithmetic to handle the `-` operator.

**Behavior:**  
- Constructs a prompt instructing the AI to remove or negate logic from the base prompt based on the subtrahend.
- Returns the modified instruction set after subtraction.
- Handles cases where the subtrahend is not present in the base (adds negation).

---

### `performSemanticMultiplication` Function

**Purpose:**  
Combines two instruction sets semantically using AI, treating them as multiplicative operands.

**Usage:**  
Used in prompt arithmetic to handle the `*` operator.

**Behavior:**  
- If either operand is numeric, treats it as a repetition instruction.
- Otherwise, constructs a prompt instructing the AI to merge the two instruction sets logically.
- Returns the combined instruction set.

---

### `performSemanticDivision` Function

**Purpose:**  
Performs semantic division or inversion of instruction sets using AI.

**Usage:**  
Used in prompt arithmetic to handle the `/` operator.

**Behavior:**  
- If the numerator is `1`, performs inversion of the denominator.
- Otherwise, constructs a prompt instructing the AI to combine the numerator with the inverse of the denominator.
- Returns the resulting instruction set.

---

### `performSemanticExponentiation` Function

**Purpose:**  
Performs semantic exponentiation of instruction sets using AI.

**Usage:**  
Used in prompt arithmetic to handle the `^` operator.

**Behavior:**  
- If the exponent is numeric, treats it as recursive composition.
- Otherwise, constructs a prompt instructing the AI to apply the base logic recursively, modulated by the exponent.
- Returns the resulting instruction set.

---

### `performDeepSubtraction` Function

**Purpose:**  
Performs a deep semantic comparison between expected and actual container logic using AI.

**Usage:**  
Used in blueprint validation to identify missing or hallucinated logic in generated code.

**Behavior:**  
- Constructs a prompt instructing the AI to compare expected and actual logic.
- Identifies missing and hallucinated elements.
- Optionally ignores architectural scaffolding.
- Returns a structured report or "NULL" if semantically equivalent.

---

### `compareBlueprints` Function

**Purpose:**  
Compares expected and actual code blueprints semantically using AI.

**Usage:**  
Used in post-processing to validate generated code against specifications.

**Behavior:**  
- Extracts containers from both blueprints.
- Maps expected containers to actual containers semantically.
- Performs deep subtraction on container logic.
- Generates a detailed comparison report.
- Optionally ignores architectural scaffolding.

---

### `resolvePromptArithmetic` Function

**Purpose:**  
Evaluates arithmetic expressions in prompts using semantic operations.

**Usage:**  
Core function for processing arithmetic operations in prompt logic.

**Behavior:**  
- Parses the expression into terms and operators.
- Applies semantic operations (`+`, `-`, `*`, `/`, `^`) sequentially.
- Handles identity elements, zero properties, and division by zero.
- Checks for semantic contradictions during addition.
- Returns the final resolved instruction set.

---

### `generateIR` Function

**Purpose:**  
Generates Glupe Intermediate Representation (GIR) from human intent.

**Usage:**  
Used in the semantic frontend pass during code generation.

**Behavior:**  
- Constructs a prompt instructing the AI to translate intent into GIR.
- Handles interactive mode for ambiguous inputs.
- Returns the generated GIR or an error message.

---

### `generateTargetCodeFromIR` Function

**Purpose:**  
Generates target code from Glupe Intermediate Representation (GIR).

**Usage:**  
Used in the semantic backend pass during code generation.

**Behavior:**  
- Constructs a prompt instructing the AI to translate GIR into target language code.
- Returns the generated code or an error message.

---

### `processInputWithCache` Function

**Purpose:**  
Pre-processes input code to handle containers, variables, caching, and code generation.

**Usage:**  
First pass in the code processing pipeline.

**Behavior:**  
- Parses containers (`$$...$$`), variables (`$:`, `$$:`), and constants (`$CONST:`).
- Substitutes variables and resolves prompt arithmetic.
- Handles inheritance and context injection for containers.
- Manages caching for containers based on hash comparison.
- Generates code for containers in fill mode or wraps them in markers for global pass.
- Supports interactive and IDE modes with user clarification handling.

---

### `performTreeShaking` Function

**Purpose:**  
Optimizes code by removing unused elements using AI.

**Usage:**  
Called during post-processing to clean up generated code.

**Behavior:**  
- Constructs a prompt instructing the AI to remove unused functions, variables, and imports.
- Preserves essential elements like the `main` function and export directives.
- Returns the optimized code, falling back to the original if errors occur.

---

### `updateCacheFromOutput` Function

**Purpose:**  
Updates the cache with generated code from AI output.

**Usage:**  
Called after AI code generation to persist results.

**Behavior:**  
- Scans the output for `GLUPE_BLOCK_START` and `GLUPE_BLOCK_END` markers.
- Extracts and caches the content of each marked block.
- Removes markers from the final output.
- Saves the updated cache to disk.