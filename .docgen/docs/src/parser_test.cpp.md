Below is a comprehensive, structured Markdown documentation for the provided code. It focuses on purpose, behavior, and usage patterns rather than restating trivial syntax.

---

# Documentation: Glupe Parser Test Harness

This document describes the purpose, behavior, and usage of the C++ driver program that integrates with a Flex/Bison–based parser to construct an Abstract Syntax Tree (AST) for the Glupe language. It also explains how the program interacts with the AST classes defined in `ast.hpp`.

---

## Overview

The program serves as a self‑contained test harness for the Glupe lexer and parser. It demonstrates how to:

- Feed source code directly from a string into the Flex lexer.
- Invoke the Bison parser to build a `ProgramNode` AST.
- Display the resulting AST using the node‑specific `print()` methods.
- Clean up all allocated parsing resources.

This file is not the compiler or interpreter itself; it is a diagnostic tool for validating the correctness of the grammar and AST construction.

---

## Key Responsibilities

### 1. Providing Input to the Lexer

Instead of reading from a file, the program embeds a multi‑line string (`test_code`) containing a mixture of:

- Normal C++ code
- Glupe variable definitions
- Glupe semantic containers (inline and block forms)

The lexer is instructed to read from this string using:

```cpp
YY_BUFFER_STATE buffer = yy_scan_string(test_code.c_str());
```

This allows rapid iteration during grammar development without needing external files.

### 2. Invoking the Parser

The parser entry point is:

```cpp
extern int yyparse(ProgramNode** root);
```

The parser allocates and populates a `ProgramNode` instance representing the root of the AST. The driver passes a pointer to a pointer so the parser can return ownership of the constructed tree.

Parsing is initiated with:

```cpp
ProgramNode* root = nullptr;
int result = yyparse(&root);
```

A successful parse sets `root` to a fully constructed AST.

### 3. Displaying the AST

If parsing succeeds, the program prints the AST structure:

```cpp
root->print();
```

Each node type implements its own `print()` method, allowing the output to reflect the hierarchical structure of:

- Raw code segments (`RawCodeNode`)
- Variable definitions (`VariableNode`)
- Semantic containers (`ContainerNode`)
- The overall program (`ProgramNode`)

This output is essential for debugging grammar rules and verifying that the parser correctly interprets Glupe constructs.

### 4. Cleaning Up

The program ensures that all temporary lexer buffers and AST memory are released:

```cpp
yy_delete_buffer(buffer);
delete root;
```

This prevents memory leaks during repeated test runs.

---

## Structure of the Test Input

The embedded `test_code` string includes examples of all major Glupe constructs:

### Variable Definition

```
$$: version -> "1.0"
```

This should produce a `VariableNode` with:

- `id = "version"`
- `value = "1.0"`
- `varType = VarType::PERSISTENT` (depending on grammar rules)

### Block Container

```
$$ my_container -> parent (param1, param2) {
    print "hello world"
}$$
```

This becomes a `ContainerNode` with:

- Identifier: `my_container`
- Parent list: `["parent"]`
- Parameter list: `["param1", "param2"]`
- Intent: the block contents
- `isBlock = true`

### Inline Container

```
$ inline_container { return 0; }$
```

This produces a `ContainerNode` with:

- `isBlock = false`
- Inline intent content

These examples ensure that the parser is exercised across all major syntactic forms.

---

## Program Flow Summary

1. Print the test header and input code.
2. Create a Flex buffer from the input string.
3. Call `yyparse()` to build the AST.
4. If successful, print the AST structure.
5. Destroy the lexer buffer and free the AST.

This flow mirrors how a real compiler front‑end would operate, but in a simplified, self‑contained form.

---

## When to Use This Program

This driver is useful when:

- Developing or debugging the Glupe grammar.
- Verifying that AST nodes are constructed correctly.
- Testing new language features before integrating them into a full compiler pipeline.
- Demonstrating how Glupe constructs interleave with host‑language code.

It is intentionally minimal and avoids any runtime semantics or code generation.

---

## Extending the Test Harness

To expand its usefulness, you might:

- Replace the hard‑coded string with file input.
- Add command‑line options for different test cases.
- Integrate error reporting hooks from Bison.
- Serialize the AST for downstream tools.

The current version is optimized for rapid iteration during grammar development.

---

If you'd like, I can also generate documentation for the AST classes themselves, or produce a diagram of the AST structure created by the sample input.