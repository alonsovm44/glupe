# Here is a deep dive into Intent Algebra.

When you treat prompts as first-class data types, you are no longer just sending text to an AI. You are building a mathematical system where meaning can be calculated, combined, and simplified by the compiler before any code is generated.

In traditional algebra, you manipulate numbers and variables (x, y) using operators (+, -, *, /) to find a result.
In Glupe's Intent Algebra, you manipulate Semantic Nodes (Containers, Variables, Abstracts) using Structural Operators (->, (), $:) to calculate a Final State of Intent.

Here are the core operations of Intent Algebra:

1. Addition (Contextual Merging)
Operation: Combining two or more distinct intents into a unified semantic space.
Glupe Operator: Multiple Inheritance (-> A, B)

When a container inherits from multiple parents, the compiler adds their contexts together.

Let A = `$ABSTRACT { Use REST API } $`

Let B = `$ABSTRACT { Use OAuth 2.0 } $`

Equation: C = A + B

Glupe Syntax: `$$C -> A, B { build the login endpoint }$$`

The Algebraic Result: The AI does not receive three separate prompts. The Glupe compiler mathematically merges the constraints into a single, unified prompt vector: "Build the login endpoint strictly using a REST API and OAuth 2.0."

2. Subtraction (Constraint Overriding)
Operation: Negating or overriding an inherited rule.
Glupe Operator: Right-to-Left Precedence in -> or explicit contradiction in the child block.

In algebra, if you add a negative number, it subtracts. In Glupe, if a child explicitly contradicts a parent, it subtracts that constraint from the final intent.

Let A = `$$ABSTRACT { Log all user data for analytics. }$$`

Let B = `$$ privacy_module -> A { Do not log passwords or emails. }$$`

Equation: Intent = A - (passwords + emails)

The Algebraic Result: The compiler resolves this conflict before transpilation. The LLM understands that the generic "all user data" rule has been mathematically bounded by the child's explicit subtraction.

3. Multiplication (Functional Amplification)
Operation: Passing an intent as a variable into a functional container that scales, loops, or applies that intent across a system.
Glupe Operator: Context Injection ()

This is where treating intents as data types becomes incredibly powerful. You can take a single "unit of meaning" and multiply its effect.

Let x = `$: format_date -> $ { convert to ISO 8601 } $`

Let F(x) = `$$process_database(x) { apply x to every timestamp column in the database }$$`

The Algebraic Result: x is just a tiny, isolated thought. But when passed into process_database, the AI multiplies the effect of x across the entire schema. If you later change x to convert to UNIX epoch, the compiler recalculates the entire system because the multiplier (the function) is acting on a new base value.

4. The Identity Matrix (Abstract Variables)
Operation: A node that shapes other nodes without possessing its own executable mass.
Glupe Operator: `$ABSTRACT: x`

In math, multiplying by 1 changes nothing's value, but applying an Identity Matrix routes vectors cleanly. Abstract variables are the Identity Matrices of Glupe.

Let I = $ABSTRACT: MAX_LATENCY -> 50ms

Equation: `$: fetch_data -> MAX_LATENCY`

The Algebraic Result: MAX_LATENCY generates exactly zero bytes of C++ code on its own. It has no physical mass. But it forces the AI to route the implementation of fetch_data toward high-performance asynchronous execution instead of a blocking call. It acts purely as a mathematical limit.

## The Compiler's Role as the "Calculator"
Because Glupe uses Intent Algebra, the Glupe Compiler's job is to "simplify the equation" before handing it to the AI.

If you write:

```
$: logic -> $ A { print "Hello" } $
$$ target -> B, C(logic) { execute logic } $$
```
The compiler does not just blindly send all this text to the LLM. It calculates the Abstract Syntax Tree (AST):

- Resolve dependencies: Evaluate A.

- Calculate bounds: Merge constraints of B and C.

- Substitute variables: Inject logic into target.

- Simplify: Create the final, optimized deterministic prompt block.

- Evaluate: Hash the simplified block. Check .glupe.lock. If it exists, return the cached C++ code. If not, send the simplified equation to the AI transpiler.

## Summarizing
Because prompts are now data types, you can perform mathematical/logical operations on them:

1. Addition (Composition): You can pass prompt A into prompt B. $ b(x) { do x three times } $. You are multiplying intents.

2. Subtraction/Override (Inheritance): If Parent A says "Use JSON" and Child B says "Use XML", the -> operator resolves the collision deterministically before the AI ever sees it.

3. Storage (Caching): Because it's a formal data type, your compiler can hash it and store its native C++ resolution in a lockfile.

- State = Meaning, not Memory

In C++, if I write `int x = 5`, the data type int tells the compiler to allocate 4 bytes of memory and store the binary value `00000101`.

In Glupe, if you write `$$: x -> $ { calculate the optimal buffer size for the current OS } $`, the data type tells the compiler: "Store this reasoning. When transpiled, figure out the actual bytes, but for now, hold the concept."

Lisp's big breakthrough was Code as Data (homoiconicity).
Glupe's breakthrough is Intention as Data.

### Why this is a Breakthrough
Until now, interacting with LLMs was like talking to someone in a chaotic room. You just threw words at them and hoped they understood the context.

With Intent Algebra, we have imposed strict mathematical topology on natural language. We are proving that human intent—when structured properly through operators like -> and ()—can be evaluated, cached, and compiled just as predictably as a binary tree.
