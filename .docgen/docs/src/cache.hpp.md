Below is a comprehensive, purpose‑focused Markdown documentation for the provided code. It avoids restating trivial C++ behavior and instead explains design intent, semantics, and how the components interact.

---

# Semantic Node & Cache System Documentation

This module defines the core data structures and caching mechanisms used by the semantic‑processing engine introduced in version **6.0**. It provides:

- A typed representation of semantic nodes extracted from user prompts.
- A global symbol table for variable and container tracking.
- A persistent caching system for containers and variables.
- Utility helpers for hashing and storing cached content.

The design centers on enabling **semantic reuse**, **persistent variables**, and **deterministic caching** across runs.

---

## 1. `NodeType` — Semantic Node Classification

```cpp
enum class NodeType {
    CONTAINER,
    VAR_EPHEMERAL,
    VAR_PERSISTENT,
    CONSTANT
};
```

### Purpose
`NodeType` categorizes the meaning and lifecycle of parsed semantic elements. It allows the engine to distinguish between:

| Type | Meaning | Lifecycle |
|------|---------|-----------|
| `CONTAINER` | Represents `$...$` or `$$...$$` blocks. Often used for prompt templates or semantic groups. | Cached by hash; may persist across runs. |
| `VAR_EPHEMERAL` | Represents `$:` variables. | Exists only for the current execution. |
| `VAR_PERSISTENT` | Represents `$$:` variables. | Saved to disk and restored on next run. |
| `CONSTANT` | Represents `$CONST:` definitions. | Immutable once defined. |

This classification drives how nodes are stored, cached, and reused.

---

## 2. `SemanticNode` — Core Semantic Representation

```cpp
struct SemanticNode {
    NodeType type;
    string id;
    string content;
    vector<string> parents;
    vector<string> params;
    vector<string> vectorContent;
    bool isVector = false;
    bool isBlock = false;
    bool isAbstract = false;
    bool isCached = false;
    string hash;
};
```

### Purpose
`SemanticNode` is the fundamental unit of semantic processing. Each node represents a parsed entity with metadata describing:

- **Identity** (`id`)  
  Unique key used for symbol‑table lookup and caching.

- **Content** (`content`)  
  Either the raw prompt text or the resolved value.

- **Hierarchy** (`parents`)  
  Tracks which containers or nodes contributed to this one.

- **Parameterization** (`params`)  
  Allows context injection into semantic templates.

- **Vector semantics** (`vectorContent`, `isVector`)  
  Supports multi‑element semantic vectors (e.g., lists of prompts or embeddings).

- **Structural flags**  
  - `isBlock`: Node represents a block‑level construct.  
  - `isAbstract`: Node is a template requiring parameter substitution.  
  - `isCached`: Node was restored from cache rather than freshly computed.

- **Hashing** (`hash`)  
  Used to determine whether cached content is still valid.

This structure enables flexible semantic modeling while supporting caching, inheritance, and template expansion.

---

## 3. Global Symbol Table

```cpp
inline map<string, SemanticNode> SYMBOL_TABLE;
```

### Purpose
A global registry of all semantic nodes encountered during execution.

- Stores ephemeral and persistent variables.
- Allows lookup by identifier.
- Serves as the authoritative source for semantic resolution.

Persistent variables are automatically loaded into this table during cache initialization.

---

## 4. Cache System

### Constants

```cpp
inline const string CACHE_DIR = "glupe_cache";
inline const string LOCK_FILE = ".glupe.lock";
inline json LOCK_DATA;
```

- **`CACHE_DIR`**: Directory where container outputs are stored.
- **`LOCK_FILE`**: JSON file tracking persistent variables and container metadata.
- **`LOCK_DATA`**: In‑memory representation of the lockfile.

---

## 5. `initCache()` — Initialize Cache and Load Persistent Variables

```cpp
inline void initCache();
```

### Purpose
Initializes the caching environment and restores persistent state.

### Behavior
1. Ensures the cache directory exists.
2. Loads `.glupe.lock` if present; otherwise initializes a new structure.
3. Restores all `VAR_PERSISTENT` nodes into the global symbol table:
   - Their content
   - Their cached hash
   - Their `isCached` flag

This allows persistent variables (`$$:`) to survive across program runs.

---

## 6. `saveCache()` — Persist Variables to Disk

```cpp
inline void saveCache();
```

### Purpose
Writes all persistent variables back to `.glupe.lock`.

### Behavior
- Iterates through the symbol table.
- Extracts only `VAR_PERSISTENT` nodes.
- Saves their `content` and `hash`.
- Writes the updated JSON to disk.

This ensures that persistent variables remain consistent across sessions.

---

## 7. Hashing and Cache I/O Utilities

### `getContainerHash()`

```cpp
inline string getContainerHash(const string& prompt);
```

Computes a deterministic hash for a container’s prompt content.  
Used to detect whether cached output is still valid.

---

### `getCachedContent()`

```cpp
inline string getCachedContent(const string& id);
```

Retrieves cached output for a container or variable by ID.

- Returns empty string if no cache file exists.
- Used to avoid recomputation when the hash matches.

---

### `setCachedContent()`

```cpp
inline void setCachedContent(const string& id, const string& content);
```

Writes computed content to a cache file.

- Overwrites existing cache.
- Ensures future runs can reuse the result.

---

# Summary

This module provides the foundational infrastructure for:

- Representing semantic constructs (`SemanticNode`)
- Tracking variables and containers (`SYMBOL_TABLE`)
- Persisting state across runs (`initCache`, `saveCache`)
- Avoiding redundant computation through hashing and caching

It is designed to support a semantic‑driven prompt engine where templates, variables, and containers can be reused efficiently and consistently.