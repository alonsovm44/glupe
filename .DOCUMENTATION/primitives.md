# Glupe Language Primitives (v5.8.1)

Glupe is a semantic metaprogramming language that operates as a meta-layer above host languages. This document defines the core primitives used to express intent, structure, and state.

## Semantic Containers (Structural)

Containers are the primary units of logic. They wrap natural language or pseudo-code intent to be resolved into host-specific code.

### Multi-line Block Containers ($$)

Used for high-level logic, functions, classes, or architectural patterns.

- Headless Block: $${ logic }$$
Behavior: Transient. Regenerated on every compilation.

- Named Block: $$identifier { logic }$$
Behavior: Persistent. Hashed and cached in .glupe.lock.

### Inline Containers ($)

Used for micro-logic, single-line expressions, or variable-level logic.

- Headless Inline: $ { logic } $

Behavior: Transient. Resolved during every build.

- Named Inline: $ identifier { logic } $

Behavior: Persistent. Hashed and cached in .glupe.lock.

### Abstract Containers

Non-executable blueprints that influence other blocks through inheritance.

- Syntax: $$ABSTRACT identifier { rules }$$ or $ABSTRACT identifier { rules } $

Behavior: Does not generate code. Acts as a constraint/template.

## Semantic State (Atomic)

Semantic variables and constants manage state through intent. Version 5.8.1 introduces Caching Symmetry to allow developers to choose between ephemeral and persistent state.

### Semantic Variables ($:)

Variables that define state through intent.

- Ephemeral Variable `$: x -> <assignment>`

Behavior: Uncached. The instruction is re-evaluated by the AI on every compilation. Useful for dynamic logic, environmental checks, or volatile implementation.

Persistent Variable `$$: x -> <assignment>`

Behavior: Cached. The result is stored in .glupe.lock. The AI only re-evaluates if the assignment string or its dependencies change. Best for complex calculations or architectural constants.

## Assignment Types:

- Semantic Instruction: $: x -> sum of first 100 numbers

Result: AI generates code implementing the logic.

- Literal String: $: x -> "text"

Result: Treated as a raw string literal.

- Literal Value: $: x -> 10

Result: Standard numeric assignment.

- Inline Composition: $: x -> $ func(y) { logic } $
- Complex Composition: $: x -> $$ cont{logic }$$ 


Result: x inherits the logic/result of the inline container.

### Semantic Constant ($CONST:)

Immutable semantic nodes. Constants are implicitly cached.

- Syntax: $CONST: identifier -> value

Safety: Redefinition or overriding results in a Compile-Time Error.

### Abstract Variables ($ABSTRACT:)

The "Logic Ghost." These guide AI reasoning but do not consume memory in the binary.

- Ground Truth Constraints: Set "semantic ceilings" (e.g., $ABSTRACT: MARGIN -> 5).

Cross-Language Mapping: Standardizes formats across host languages (C++, Python, etc.).

## The Inheritance and Context System

### Inheritance (->)

The primary operator for directional intent and constraint propagation.

Blocks: $$child -> parent { logic }$$

Variables: $: x -> parent_x (Inherits value and abstract constraints).

### Context Injection ()

Explicitly passes dependencies into an inline logic scope.

Syntax: $ identifier (dependency) { logic } $

Behavior: Tells the compiler exactly which semantic nodes are required, enabling deterministic dependency tracking