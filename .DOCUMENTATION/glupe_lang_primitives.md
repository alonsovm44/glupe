# Glupe Language Primitives (v5.8.1)

Glupe is a semantic metaprogramming language that operates as a meta-layer above host languages. This document defines the core primitives used to express intent, structure, and state.

The key insight to understand Glupe is this: containers store prompts which are placeholders for code. When you run Glupe, it tells an AI to turn those prompts into working code.

## Semantic Containers (Structural)

Containers are the primary units of logic. They wrap natural language, step instructions (algorithmic) or pseudo-code intent to be resolved into host-specific code. 
We will dig step by step, but first let's introduce the reader to the syntax.

### This is the template for a container
```glupe
$$[ABSTRACT][CONST] name -> parent1, parent2,...,parentN {
    intent goes here
} $$
```
### For inline containers
```
$[ABSTRACT][CONST] name -> parent1, parent2, ... , parentN {intent goes here} $
```
### Multi-line Block Containers ($$)
Used for high-level logic, functions, classes, or architectural patterns.

Headless Block: `$${ logic }$$`

Behavior: Transient. Regenerated on every compilation.

Named Block: `$$identifier { logic }$$`

Behavior: Persistent. Hashed and cached in .glupe.lock.

### Inline Containers ($)
Used for micro-logic, single-line expressions, or variable-level logic.

- Headless Inline: `$ { logic } $`

Behavior: Transient. Resolved during every build.

- Named Inline: `$ identifier { logic } $`

Behavior: Persistent. Hashed and cached in .glupe.lock.

### Abstract Containers
Non-executable blueprints that influence other blocks through inheritance.

Syntax:` $$ABSTRACT identifier { rules }$$` or `$ABSTRACT identifier { rules } $`

Behavior: Does not generate code. Acts as a constraint/template.

## Semantic State (Atomic)
Semantic variables and constants manage state through intent. Version 5.8.1 introduces Caching Symmetry to allow developers to choose between ephemeral and persistent state.

Semantic Variables ($:)
Variables that define state through intent.

Ephemeral Variable: `$: x -> <assignment>`

Behavior: Uncached. The instruction is re-evaluated by the AI on every compilation. Useful for dynamic logic, environmental checks, or volatile implementation.

Persistent Variable: `$$: x -> <assignment>`

Behavior: Cached. The result is stored in .glupe.lock. The AI only re-evaluates if the assignment string or its dependencies change. Best for complex calculations or architectural constants.

Assignment Types:
Semantic Instruction:` $: x -> sum of first 100 numbers`

Result: AI generates code implementing the logic.

Literal String: `$: x -> "sum of first 100 numbers"`

Result: Treated as a raw string literal.
Note: in cases like this `$: x -> print "hi"` Glupe understands that the instruction is to print the literal word in quotations "hi", the literal string mode only triggers when the whole assigned value is wrapped in them.

Literal Value: `$: x -> 10`

Result: Standard numeric assignment.

Inline Composition:` $: x -> $ inline_container { logic } $`

Complex Composition:
```
$: x -> $$ cont {
    intent goes here
} $$ 
```
Result: x inherits the logic/result of the inline container, working as a semantic alias for that intent.

## Edge cases worth studying
### Non cached multiple composition
`$:x -> $ A -> B,C {} $` 
**The Volatility Paradox**
Here, we are assigning an ephemeral variable ($:) to a named inline container (A) that inherits from two parents (B, C).

The Conflict: By your language spec, a named container ($ A { ... } $) is supposed to be persistent and cached. But you are assigning it to an uncached variable ($: x).

The Compiler's Resolution: How should Glupe handle this?

Option A: A is cached in .glupe.lock, but x dynamically decides how to call A based on the environment.

Option B (Better for IOP): The compiler treats the $:  as the dominant command. It tells the AI: "Read the rules of B, then overwrite them with C (Right-to-Left rule), read the intent, and generate the code inline right here, right now. Do not cache this specific execution."

Why it's useful: Imagine B is a general security policy, and C is a specific hardware constraint for the current user's machine. Because x is ephemeral ($:), if the hardware environment changes between builds, the AI will regenerate x seamlessly, while still respecting the abstract rules of B and C.

(Note: If you didn't want A to be a permanent concept in the system, you could just write the headless version: $: x -> $ -> B, C { intent } $)

### Cached multiple composition
`$$:x -> A -> B,C {} $`

**The Concrete Dependency Tree**
This is the heavy-lifter of the language. You are telling the compiler to create a permanent, cached artifact ($$:) derived from multiple architectural parents.

Hash Cascading: To cache x in .glupe.lock, the compiler cannot just hash the string "intent". It must create a Composite Hash: Hash(B) + Hash(C) + Hash(intent).

The "Butterfly Effect": If someone changes a single word inside the abstract container B (e.g., changing "Use SHA-256" to "Use AES-GCM"), the hash of B changes. This instantly invalidates the cache for x, forcing the AI to regenerate x upon the next build.

The Semantic Merge: The AI receives a deterministically assembled prompt:

- Context 1: Rules of B.

- Context 2: Rules of C (Overrides B if they conflict).

- Task: Execute intent.

Constraint: Output a standalone, reusable host artifact (like a C++ function) because of the $$: operator.

## Functions
In Glupe, any container can act as a function by explicitly declaring parameters within parentheses `()`. This process is known as **Context Injection**. It tells the compiler that the intent inside the container relies on external inputs, forcing the AI to generate a callable, parameter-accepting structure in the target host language.

### Syntax

**Multi-line Block Function:**
```glupe
$$ name (param1, param2, ...) [-> parent1, parent2] {
    intent utilizing param1, param2, etc.
}$$
```
**Inline Function:**
```
$identifier(param1, param2, ...) { intent utilizing parameters }$
```
#### Behavior and Transpilation
When you define a parameterized container, you are creating a reusable logic blueprint that accepts dynamic inputs:

**Native Artifact Generation:** The AI evaluates the intent and translates the container into a native function in the host language (e.g., double calculate(double a, int b) in C++ or def calculate(a, b): in Python).

**Type Inference:** Because Glupe is intention-oriented, you do not need to strictly type a, b, c. The AI infers the correct host-language data types based on how you describe their usage in the intent block.

**Scope Isolation:** Passing parameters creates a strict semantic boundary. The transpiler instructs the AI to rely only on the explicitly injected variables (and any inherited abstract rules), preventing it from accidentally modifying global state.

#### Examples
 
 1. Defining a Function:

```
$$ apply_discount(base_price, discount_rate) -> financial_policy {
    1. Calculate the reduction by multiplying base_price by discount_rate.
    2. Subtract the reduction from the base_price.
    3. Ensure the final result is never negative.
    4. Return the final price.
}$$
```
2. Calling a Function:
Functions can be invoked inside other containers, or their results can be mapped to Semantic Variables.

```
// Direct invocation inside an inline block
${ print the result of apply_discount(100.0, 0.20) }$

// Assigning the execution result to an ephemeral variable
$: user_checkout_total -> ${ apply_discount(cart_total, 0.15) }$
```
**Caching and Reusability**
Because parameterized blocks like `$$ identifier(a, b) { ... }$$` are Named Containers, they are persistent.

The compiler hashes the intent and the parameter signatures and stores the generated host function in .glupe.lock.

Calling `apply_discount` multiple times in your Glupe file does not cost extra LLM tokens; the compiler simply references the cached host-language function.

## Execution Flow: Ephemeral vs Persistent Composition
When aliasing a container to a variable, the choice between ephemeral `$`: and persistent `$$:` dictates how the AI physically generates the host code.

### Case 1: Ephemeral Alias (Inline Expansion)

```
$: x -> $ a {print "hi"} $
$ b(x) {print x three times} $
```
Behavior: The compiler treats x as a fluid thought. It injects the intent ("print hi") directly into the prompt for b. The generated host code will inline the action (e.g., writing std::cout << "hi"; three times directly inside b's block).

Case 2: Persistent Alias (Concrete Artifact)

Fragmento de código
$$: x -> $ a {print "hi"} $
$ b(x) {print x three times} $
Behavior: The compiler treats x as a permanent, cached fixture. It forces the AI to generate a concrete artifact in the host language (like a physical function print_a()) and stores it in .glupe.lock. Container b will then reference this artifact (e.g., calling print_a(); three times). This saves token costs, allows high reusability, and ensures architectural stability.

Semantic Constant ($CONST:)
Immutable semantic nodes. Constants are implicitly cached.

Syntax: $CONST: identifier -> value

Safety: Redefinition or overriding results in a Compile-Time Error.

Abstract Variables ($ABSTRACT:)
The "Logic Ghost." These guide AI reasoning but do not consume memory in the binary.

Ground Truth Constraints: Set "semantic ceilings" (e.g., $ABSTRACT: MARGIN -> 5).

Cross-Language Mapping: Standardizes formats across host languages (C++, Python, etc.).

The Inheritance and Context System
Inheritance (->)
The primary operator for directional intent and constraint propagation.

Blocks: $$child -> parent { logic }$$

Variables: $: x -> parent_x (Inherits value and abstract constraints).

Context Injection ()
Explicitly passes dependencies into an inline logic scope.

Syntax: $ identifier (dependency) { logic } $

Behavior: Tells the compiler exactly which semantic nodes are required, enabling deterministic dependency tracking.

# Prompt Algebra

In standard API calls to OpenAI or Anthropic, developers treat prompts as string types. They concatenate strings like `"Write a function that " + user_input + " using Python."` It's messy, brittle, and breaks at scale.

In Glupe, the prompt isn't a string. It is an Intent Object (a Semantic Node).
When you write `$: x -> sum of first 100 numbers`, x is not a string. x is a variable of type Intent. It has properties:

- It has a Hash (for caching).

- It has Ancestry (who it inherited from via ->).

- It has Scope (what variables it is allowed to see).

 