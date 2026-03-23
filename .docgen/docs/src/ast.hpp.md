```markdown
# Abstract Syntax Tree (AST) Node Classes Documentation

## Overview
This documentation describes the structure and behavior of the AST node classes used to represent parsed Glupe language constructs. The classes are designed to facilitate the interpretation and manipulation of Glupe code, providing a clear hierarchy for semantic analysis and code generation.

## Classes

### `ASTNode`
**Purpose**: Base class for all AST nodes, defining a common interface for printing the tree structure.  
**Behavior**:  
- Virtual destructor ensures proper cleanup of derived classes.  
- Pure virtual `print` method enforces implementation in derived classes for debugging purposes.  

### `RawCodeNode`
**Purpose**: Represents raw target-language code (e.g., C++, Python) that is passed through untouched during parsing.  
**Usage**: Used to embed unmodified code snippets within Glupe programs.  
**Behavior**:  
- Stores raw code as a `string`.  
- `print` method displays the node type and the length of the embedded code.  

### `VariableNode`
**Purpose**: Represents a Glupe variable definition, including its type, identifier, and value.  
**Usage**: Models variable declarations such as `$: var = "value"`.  
**Behavior**:  
- Stores variable identifier (`id`), value (`value`), and type (`VarType`).  
- `print` method displays the node type, variable type (e.g., `CONST`), identifier, and value.  

### `ContainerNode`
**Purpose**: Represents a Glupe Semantic Container, defining reusable components with inheritance and intent.  
**Usage**: Models constructs like `$$ name -> parent { intent } $$`.  
**Behavior**:  
- Stores container identifier (`id`), parents, parameters, intent, and flags for block/inline and abstract types.  
- `print` method displays the node type, identifier, abstract status, and block/inline type.  

### `ProgramNode`
**Purpose**: Represents the root of the parsed file, containing all top-level AST elements.  
**Usage**: Serves as the entry point for traversing the entire AST.  
**Behavior**:  
- Stores a vector of unique pointers to `ASTNode` instances.  
- `addElement` method appends non-null nodes to the elements list.  
- `print` method recursively prints the entire tree structure with indentation.  

## Enum: `VarType`
**Purpose**: Defines the semantic types of Glupe variables.  
**Values**:  
- `EPHEMERAL`: Temporary variables.  
- `PERSISTENT`: Variables with persistent state.  
- `CONSTANT`: Immutable variables.  

## Usage Example
```cpp
auto program = make_unique<ProgramNode>();
program->addElement(make_unique<VariableNode>("var1", "value1", VarType::CONSTANT));
program->addElement(make_unique<RawCodeNode>("cout << \"Hello, World!\";"));
program->print();
```

## Notes
- The `print` methods are primarily for debugging and do not affect the semantic interpretation of the AST.  
- `unique_ptr` is used for memory management to ensure proper ownership and cleanup of AST nodes.  
- The hierarchy is designed to be extensible, allowing for additional node types as needed.
```