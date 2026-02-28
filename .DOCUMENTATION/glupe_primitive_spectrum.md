# The Glupe Primitive Spectrum (v6.0)

This table defines the behavior, storage, and architectural "mass" of every primitive combination in Glupe.

Primitive | Syntax | Format | Persistance | Inheritable | Binary mass | Registry Role|
---------|--------|--------|-------------|------------|-------------|--------------
Persistent block| $$x {}$$ | Multiline|Disk (.lock) | Yes | Yes | Architectural Pillar|
Abstract persisten block | $$ABSTRACT x {}$$ | Multiline |Disk (.lock)| Yes | No |Cached Architectural Blue Print|
Ephemeral block | $${}$$ | Multiline | RAM | No | Yes | Transient Logic|
Abstract Ephemeral Block| $$ABSTRACT { }$$| Multiline|RAM|No|No|Scope-limited Rule (Ghost)|
Persistent Inline|$ x { } $|Inline|Disk (.lock)|Yes|Yes (Snippet)|Semantic Snippet|
Abstract Named Inline|$ABSTRACT x { } $|Inline|Disk (.lock)|Yes|No|Named Logic Rule|
Ephemeral Inline|${ }$|Inline|RAM|No|Yes|Transient Expression|
Abstract Headless Inline| $ABSTRACT { } $|Inline|RAM|No|No|Immediate Context Filter|
Persistent Variable|$$: x -> p|Inline|Disk (.lock)|Yes|Yes (Var/Const)|Cached State|
Abstract Persistent Var|$$ABSTRACT: x -> p|Inline|Disk (.lock)|Yes|No|Cached Logical Guardrail|
Ephemeral Variable|$: x -> p|Inline|RAM|Yes|Yes|Dynamic State|
Abstract Ephemeral Var|$ABSTRACT: x -> p|Inline|RAM|Yes|No|Volatile Logic Ghost|
Abstract Persistent Const|$$ABSTRACT CONST: x -> v|Inline|Disk (.lock)|Yes|No|Cached Hard Logic Limit|
Abstract Ephemeral Const|$ABSTRACT CONST: x -> v|Inline|RAM|Yes|No|Volatile Hard Logic Limit|
Semantic Constant|$CONST: x -> v|Inline|Disk (.lock)|Yes|Yes (Immutable)|Hard Binary Constraint|

## Detailed Property Analysis (Extended)

1. Abstract Inline Containers (The "Context Filters")

### Abstract Named Inline ($ABSTRACT x { } $):

- Usage: Used to define a rule that needs to be referenced by multiple lines but shouldn't ever be compiled into a function.

- Registry: Stored as a reusable node. If another block uses -> x, it inherits these rules.

### Abstract Headless Inline ($ABSTRACT { } $):

Usage: The "One-Shot Ghost." You wrap a specific piece of host-code or another Glupe container in this to apply a temporary constraint that isn't worth naming or saving.

Effect: It influences the resolution of its contents and then vanishes from the compiler's memory.

2. The Persistence of Abstracts ($$ABSTRACT)

While it seems counter-intuitive for an "invisible" rule to be persistent, $$ABSTRACT is used for Architectural Stability:

If you have a massive $$ABSTRACT policy that takes the AI a long time to "reason" through, you use $$ABSTRACT to cache the logic resolution.

The compiler stores the "Resolved Intent" in .glupe.lock so that subsequent builds don't need to re-verify the logic of that blueprint unless the text changes.

3. Abstract Variables vs. Abstract Containers

Variables ($ABSTRACT: x -> v): Used for Values that guide the AI.

Example: $ABSTRACT: MAX_RETRY -> 3. The AI sees this and writes loops that only run 3 times, but the variable MAX_RETRY never exists in the final C++ binary.

Containers ($ABSTRACT x { }): Used for Behavior/Rules that guide the AI.

Example: $ABSTRACT x { Use asynchronous calls }. The AI writes async/await patterns but the container x never exists as a function.

4. Semantic Constants ($CONST:)

Constants are unique because they require binary mass.

A $CONST: is always cached ($$ is implicit/optional syntax).

Unlike an $ABSTRACT:, a $CONST: must appear in the host code (e.g., const int ... or #define ...) because it represents a "Ground Truth" that the machine needs to know at runtime.

5. The Abstract Constant Paradox ($ABSTRACT CONST:)

This is a specific edge case used for Inlined Immutability.

The Logic: It combines the "Zero Mass" of ABSTRACT with the "Immutable Truth" of CONST.

The Effect: It tells the AI: "This value is X and it will NEVER change. Do not create a variable for it in the code; instead, inline the value directly wherever it is used."

Use Case: Enforcing limits like MAX_BUFFER_SIZE where you want the AI to write optimized code (like char buf[1024]) instead of using a named constant, while still maintaining the ability to change that limit in one place in your Glupe file.

SumWmary: The "Mass" Rule

Concrete (Standard): Intent -> AI -> Host Code -> Binary Mass.

Abstract: Intent -> AI -> Logic Influence -> No Binary Mass.