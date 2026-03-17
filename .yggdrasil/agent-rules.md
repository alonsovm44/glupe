# Yggdrasil - System Semantic Memory (Operating Manual)

You are working in a repository managed by Yggdrasil.
Yggdrasil is a persistent, structured semantic memory graph stored in `.yggdrasil/`. It maps the repository, dictates system rules, and assembles implementation contexts.

THIS PROMPT IS YOUR ENTIRE OPERATING MANUAL. Read it carefully. Follow it strictly.

---

## 1. CORE PRINCIPLES (NON-NEGOTIABLE)

1. **Graph First, Always:** Before answering a question, modifying code, or planning a feature, you MUST consult the graph.
2. **Context is Sufficient:** If you feel the need to randomly explore source files to understand what a node should do, the graph is incomplete. **Fix the graph** (add decisions, interface details, constraints). Do not bypass the graph by reading raw code.
3. **Graph is Intended Truth:** If the code and graph diverge, the graph is the truth. If a code change is deliberate, update the graph to match.
4. **Exhaustive Coverage:** Every source file MUST belong to exactly one graph node. No orphaned files.
5. **Tools Read, You Write:** The `yg` CLI tools only read, validate, and manage metadata. YOU must create and edit graph directories, `.yaml` files, and `.md` artifacts manually.
6. **English Only for Artifacts:** All graph artifact files (filenames from `config.artifacts`, in the same directory as `node.yaml`) MUST be written in English. Conversation can be in the user's language.
7. **Never Touch Operational Metadata:** NEVER manually edit `.yggdrasil/.drift-state` or `.yggdrasil/.journal.yaml`.

---

## 2. CONVERSATION LIFECYCLE (YOUR HABITS)

You do not need explicit "session" commands. Follow these conversational triggers:

### A. Preflight (First message of the conversation)
Always execute these commands before doing anything else:
1. `yg journal-read` -> If entries exist, consolidate them into the graph, then `yg journal-archive`.
2. `yg drift` -> If divergence is detected, present states (`ok`, `drift`, `missing`, `unmaterialized`). Ask the user: Absorb (update graph) or Reject (re-materialize code from graph)?
3. `yg status` -> Report graph health.
4. `yg validate` -> If W008 stale-knowledge appears, update the knowledge artifacts to reflect current node state.

### B. Wrap-up (User signals closing the topic)
Triggered by phrases like: "kończymy", "wrap up", "to tyle", "gotowe".
**Note: The graph should ALREADY be up to date. Do not wait for wrap-up to update graph artifacts.**
1. If iterative journal mode was used: consolidate notes to the graph, then `yg journal-archive`.
2. `yg drift` -> Check if files changed manually during the conversation.
3. `yg validate` -> Fix any structural errors.
4. Report exactly what nodes and files were changed.

---

## 3. WORKFLOW: MODIFYING OR CREATING FILES

You are NOT ALLOWED to edit or create source code without establishing graph coverage first.

**Step 1: Check coverage** -> Run `yg owner --file <path>`

**Step 2: If Owner FOUND (The Execution Checklist)**
Whenever you write or edit source code, you MUST output this exact checklist in your response to the user, and execute each step BEFORE finishing your turn. This forces you to remember the graph:

- [x] 1. Read Specification (ran `yg build-context`)
- [x] 2. Modify Source Code
- [x] 3. Sync Graph Artifacts (manually edit the node's artifact files — filenames from `config.artifacts` — IMMEDIATELY to match new code behavior)
- [x] 4. Baseline Hash (ran `yg drift-sync` ONLY AFTER updating the graph)

*If you do not print this checklist and check off step 3, you have failed the core directive of Yggdrasil.*

**Step 3: If Owner NOT FOUND (Uncovered Area)**
STOP. Do not modify the code. First determine: **Is this greenfield (empty or new code to be created)?**

*   **If GREENFIELD (empty directory, new project, code not yet written):** Do NOT offer blackbox. Use Option 1 only — create proper nodes (reverse engineering or upfront design) before implementing. Blackbox is forbidden for new code.
*   **If EXISTING CODE (legacy, third-party, shipped-but-unmapped):** Present the user with 3 options and wait for their decision:
    *   **Option 1: Reverse Engineering:** Create/extend standard nodes to map the area fully before modifying.
    *   **Option 2: Blackbox Coverage:** Create a `blackbox: true` node at a user-chosen granularity to establish ownership without deep semantic exploration.
    *   **Option 3: Abort/Change Plan:** Do not touch the file.


---

## 4. WORKFLOW: MODIFYING THE GRAPH & BLAST RADIUS

When adding features or changing architecture, update the graph FIRST.

**DO NOT DEFER GRAPH UPDATES:**
*   **DO NOT wait for the user to confirm if a change is "final".** The graph must evolve continuously with your code edits.
*   **Default Behavior:** If iterative journal mode is OFF, you MUST write structural and semantic changes directly to the graph files (`node.yaml`, artifacts or other files like aspects or flows, etc.) IMMEDIATELY. Suppress your innate safety bias to wait for permission.

1. **Check Blast Radius:** Before modifying a node that others depend on, run `yg impact --node <node_path> --simulate`. Report the impact to the user.
2. **Read Config & Templates:**
    *   Check `.yggdrasil/config.yaml` for allowed `node_types` and `tags`.
    *   **CRITICAL:** ALWAYS read the required schema files in `.yggdrasil/templates/` (e.g., `node.yaml`, `service.yaml`) to know the exact fields and structure before creating or editing any graph file.
3. **Validate & Fix:** Run `yg validate`. You must fix all E-codes (Errors).
4. **Token Economy & W-codes (Warnings):**
    *   If you see `W005 budget-warning` or `W006 budget-error`, the context package is too large. You MUST consider splitting the node or reducing dependencies.
    *   If you see `W008 stale-knowledge`, the semantic memory is outdated compared to the code. Update the knowledge artifacts.
    *   **Smallest Viable Scope:** Prefer `scope: nodes` over `scope: tags`. Prefer tags over `scope: global`. Global scope costs token budget in EVERY context package.

**Journaling (Iterative Mode):**
*   **Default:** Write changes directly to graph files immediately.
*   **Opt-in:** ONLY if the user says "tryb iteracyjny" or "użyj journala", use `yg journal-add --note "..."` to buffer intent during fast ping-pong changes.

---

## 5. PATH CONVENTIONS (CRITICAL)

To avoid broken references (`E004`, `E005`), use correct relative paths:
*   **Node paths** (used in CLI, relations, flow nodes): Relative to `.yggdrasil/model/` (e.g., `orders/order-service`).
*   **File paths** (used in mapping, `yg owner`): Relative to the repository root (e.g., `src/modules/orders/order.service.ts`).
*   **Knowledge paths** (used in node explicit refs): Relative to `.yggdrasil/knowledge/` (e.g., `decisions/001-event-sourcing`).

---

## 6. GRAPH STRUCTURE, CONFIG & TEMPLATES CHEAT SHEET

The graph lives entirely under `.yggdrasil/`. You NEVER guess structure. You MUST ALWAYS read the corresponding schema reference in `.yggdrasil/templates/` before creating or editing any graph file.

*   **`.yggdrasil/config.yaml`**: The ONLY config file. Defines `node_types`, `tags`, `artifacts`, `knowledge_categories`, and quality thresholds. Read this before any graph work.
*   **`.yggdrasil/templates/`**: The SINGLE place for all templates and schemas.
    *   Contains node-type templates (e.g., `service.yaml`, `module.yaml`) with suggested artifacts and guidance.
    *   Contains schema references (`node.yaml`, `aspect.yaml`, `flow.yaml`, `knowledge.yaml`) showing exact file structures.
*   **`.yggdrasil/model/`**: Node tree. Each node is a directory with `node.yaml` and artifact files (filenames from `config.artifacts`; required ones depend on config).
*   **`.yggdrasil/aspects/`**: Cross-cutting rules. Directory contains `aspect.yaml` (binds via `tag: <name>`) and `.md` content.
*   **`.yggdrasil/flows/`**: End-to-end processes. Directory contains `flow.yaml` (lists `nodes: [paths]` and `knowledge: [paths]`) and `.md` content.
*   **`.yggdrasil/knowledge/`**: Repo-wide wisdom (`decisions/`, `patterns/`, `invariants/`). Directory contains `knowledge.yaml` and `.md` content.

---

## 7. CLI TOOLS REFERENCE (`yg`)

Always use these exact commands.

*   `yg owner --file <file_path>` -> Find owning node.
*   `yg build-context --node <node_path>` -> Assemble strict specification.
*   `yg tree [--root <node_path>] [--depth N]` -> Print graph structure.
*   `yg deps --node <node_path> [--type structural|event|all]` -> Show dependencies.
*   `yg impact --node <node_path> --simulate` -> Simulate blast radius.
*   `yg status` -> Graph health metrics.
*   `yg validate [--scope <node_path>|all]` -> Compile/check graph. Run after EVERY graph edit.
*   `yg drift [--scope <node_path>|all]` -> Check code vs graph baseline.
*   `yg drift-sync --node <node_path>` -> Save current file hash as new baseline. Run ONLY after ensuring graph artifacts match the code.

*(Iterative mode only)*
*   `yg journal-read`
*   `yg journal-add --note "<content>" [--target <node_path>]`
*   `yg journal-archive`
