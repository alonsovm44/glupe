Below is a comprehensive, structured Markdown documentation for the provided Bison grammar. It focuses on purpose, behavior, and usage patterns rather than restating syntax that is already obvious from the code.

---

# Glupe Language Parser  
### Bison Grammar Documentation

This document describes the behavior, intent, and structure of the Bison grammar used to parse the Glupe language. The grammar constructs an abstract syntax tree (AST) composed of `ProgramNode`, `RawCodeNode`, `VariableNode`, and `ContainerNode` instances defined in `ast.hpp`.

The parser is responsible for transforming a token stream (produced by Flex) into a structured semantic representation of a Glupe source file.

---

## Overview

The grammar supports three major syntactic categories:

1. **Raw code passthrough** — arbitrary target-language code blocks.
2. **Variable declarations** — persistent, constant, and ephemeral variables.
3. **Semantic containers** — block or inline constructs with optional parameters, parents, and intent bodies.

The parser builds a `ProgramNode` as the root of the AST and appends each parsed element to it.

---

# 1. Program Structure

## `program`
The `program` rule accumulates top-level elements into a `ProgramNode`.  
It initializes the root node on the first reduction and appends subsequent elements.

### Behavior
- Empty input still produces a valid `ProgramNode`.
- Elements that reduce to `nullptr` (e.g., blank lines) are ignored.
- Ownership of AST nodes is transferred into `ProgramNode` via `unique_ptr`.

---

# 2. Elements

## `element`
Represents any top-level construct:

| Construct | Resulting Node |
|----------|-----------------|
| `T_RAW_CODE` | `RawCodeNode` |
| Variable declaration | `VariableNode` |
| Container declaration | `ContainerNode` |
| `T_NEWLINE` | ignored |

### Behavior
- Raw code tokens are wrapped in `RawCodeNode` without modification.
- Newlines do not produce AST nodes, allowing flexible formatting.

---

# 3. Variable Declarations

## Supported variable types
The grammar recognizes three variable prefixes:

| Token | Meaning | AST Type |
|-------|----------|----------|
| `T_PERSISTENT_VAR_START` | persistent variable | `VarType::PERSISTENT` |
| `T_CONST_VAR_START` | constant variable | `VarType::CONSTANT` |
| `T_EPHEMERAL_VAR_START` | ephemeral variable | `VarType::EPHEMERAL` |

## `var_decl`
A variable declaration consists of:

```
<var-type> <identifier> <optional value> NEWLINE
```

### Behavior
- The value is optional; missing values become empty strings.
- The parser constructs a `VariableNode(id, value, type)`.

### Example
```
$: myVar = "hello"
```
→ `VariableNode("myVar", "\"hello\"", VarType::PERSISTENT)`

---

# 4. Containers

Containers are the core semantic units of Glupe. They may be:

- **Block containers** (`T_BLOCK_START ... T_BLOCK_END`)
- **Inline containers** (`T_INLINE_START ... T_INLINE_END`)
- **Named or anonymous**
- **Abstract or concrete**
- **With or without parameters**
- **With or without parent containers**
- **With or without intent bodies**

## 4.1 Container Components

### `optional_abstract`
Allows containers to be marked abstract:

- Empty → `false`
- `T_ABSTRACT` → `true`

### `optional_params`
Parameter list enclosed in parentheses:

```
(name1, name2, ...)
```

- Empty → empty vector
- Values are algebraic expressions (see below)

### `optional_parents`
Parent list introduced by `T_ARROW`:

```
-> Parent1, Parent2
```

- Empty → empty vector

### `intent_body`
A concatenation of `T_INTENT_TEXT` tokens.  
This allows multi-token intent content to be merged into a single string.

---

## 4.2 Container Declaration Rules

### Named Block Container
```
T_BLOCK_START optional_abstract string_or_id optional_params optional_parents
T_LBRACE intent_body T_BLOCK_END
```

### Anonymous Block Container
```
T_BLOCK_START optional_abstract
T_LBRACE intent_body T_BLOCK_END
```

### Named Inline Container
```
T_INLINE_START optional_abstract string_or_id optional_params optional_parents
T_LBRACE intent_body T_INLINE_END
```

### Anonymous Inline Container
```
T_INLINE_START optional_abstract
T_LBRACE intent_body T_INLINE_END
```

### Behavior
- Creates a `ContainerNode(id, intent, isBlock, isAbstract)`.
- Parameter and parent lists are copied into the node.
- Anonymous containers receive an empty string as their ID.
- Inline containers set `isBlock = false`.

---

# 5. Expressions

The grammar supports simple algebraic expressions used primarily for parameters and parent lists.

## 5.1 `expr_list`
A comma-separated list of algebraic expressions.

### Behavior
- Produces a `vector<string>` where each entry is the fully formatted expression.

---

## 5.2 Algebraic Expressions

### `algebra_expr`
Supports left-associative binary operations:

- `+`
- `-`
- `*`
- `/`
- `^`

Expressions are stored as strings, not evaluated.

### `algebra_term`
A term may be:

| Token | Resulting String |
|--------|------------------|
| `T_IDENTIFIER` | prefixed with `$` |
| `T_STRING_LITERAL` | wrapped in quotes |
| `T_NUMBER` | unchanged |

### Example
Input:
```
(a + b) * 3
```

Stored as:
```
"$a + $b * 3"
```

The grammar does not enforce precedence; it preserves the user’s written order.

---

# 6. Raw Code

## `T_RAW_CODE`
Represents uninterpreted target-language code.

### Behavior
- Stored verbatim in a `RawCodeNode`.
- The parser does not attempt to interpret or validate it.
- Useful for embedding host-language snippets inside Glupe files.

---

# 7. Error Handling

The parser uses:

```
void yyerror(ProgramNode** root, const char* s)
```

### Behavior
- Reports the line number using `yylineno`.
- Does not attempt recovery; Bison’s default recovery applies.

---

# 8. Memory Management

The grammar uses `%destructor` to automatically free:

- `std::string*`
- `ASTNode*`
- `std::vector<std::string>*`

This ensures no memory leaks occur when rules are discarded during parsing.

Ownership of AST nodes is transferred to `ProgramNode` via `unique_ptr`.

---

# Summary

This Bison grammar defines a structured, extensible parser for the Glupe language. It supports:

- Rich container semantics
- Multiple variable types
- Algebraic expressions for parameters and parents
- Embedded raw code
- Clean AST construction with safe memory handling

The resulting AST provides a clean, uniform representation suitable for further semantic analysis, code generation, or interpretation.