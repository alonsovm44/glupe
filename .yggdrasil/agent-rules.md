## CORE PROTOCOL

<EXTREMELY-IMPORTANT>
This is your operating manual for working in a Yggdrasil-managed repository. Every rule below is mandatory — no skill, plan, workflow, or instruction overrides these requirements.

BEFORE working with ANY source file, you MUST run `yg owner` to check if it is mapped. If mapped, you MUST use the graph before reading source. YOU DO NOT HAVE A CHOICE.

This is not negotiable. This is not optional. You cannot rationalize your way out of this.
</EXTREMELY-IMPORTANT>

Yggdrasil is persistent semantic memory stored in `.yggdrasil/`. It maps the repository and provides deterministic implementation context for every node.

### Quick Start Protocol

```
BEFORE reading, researching, planning, OR modifying ANY mapped file:
  0. Don't know which file or node to start from? Run
     yg select --task "<your goal>" to find relevant nodes via keyword
     matching against graph artifacts. If a semantic search tool is also
     available, use it for richer intent matching. Use the results
     to identify relevant nodes, then proceed to step 1.
  1. yg owner --file <path>
  2. Choose the right graph tool for your task:
     - Understanding how/why it works → yg build-context --node <owner>
     - Assessing what is affected by a change → yg impact --node <owner>
     - Planning modifications → both (build-context first, then impact)
  `yg build-context --node <path>`. Read the YAML map for topology,
  then read artifact files listed in the artifacts section. For quick
  orientation, the map alone is sufficient. For implementation, read
  all artifact files before changing code.
  If the context package seems insufficient — enrich the graph.

AFTER modifying:
  3. Update graph artifacts to reflect changes
  4. yg validate — fix all errors
  5. yg drift-sync --node <owner>

EVERY conversation start:
  yg preflight → act on findings (see Operations)

NEVER: modify code without graph coverage.
NEVER: read mapped source files to understand a component without
       running yg build-context first — the graph captures intent,
       constraints, and relations that source files cannot.
NEVER: assess blast radius of a change without running yg impact first
       — the graph knows the dependency structure that grep cannot infer.
NEVER: invent rationale, business rules, or decisions.
NEVER: auto-resolve drift without asking the user.
WHEN UNSURE: ask the user. Never guess. Never assume.
```

### Five Core Rules

1. **Graph first.** Before reading, researching, planning, or modifying mapped files, run `yg owner` and the appropriate graph tool: `yg build-context` to understand a component, `yg impact` to assess blast radius. The graph is your primary source of architectural understanding. For implementation-level precision (exact behavior, error paths, edge cases) — verify against source code after loading the context package.
2. **The graph is the specification; code implements it.** The graph absorbs knowledge from every source — external docs, conversations, decisions — and must be self-sufficient. If all other sources disappeared, the graph alone must contain enough to understand the system. Do not leave knowledge in external documents and reference them — capture the knowledge in graph artifacts. Update graph artifacts immediately after each file change, while context is fresh — do not batch graph updates to the end of a task. Code and graph move together: code changed → graph updated before moving to the next file. Graph changed → source verified in the same response. When planning work — in any tool, skill, or workflow — graph updates are part of each step's definition of done, never a separate phase.
3. **Never invent why.** The graph captures human intent. If you don't know why something was decided, ask. Never hallucinate rationale.
4. **Always capture why — especially why NOT.** When the user explains a reason, record it in the graph immediately. When a design choice is made, also record rejected alternatives: "Chose X over Y because Z." Rejected alternatives are the highest-value information — invisible in code and irrecoverable once forgotten. Conversation evaporates; graph persists.
5. **Ask before resolving ambiguity.** When multiple valid interpretations exist, stop, list options, ask the user. Never silently choose.

### Recognizing Graph-Required Actions

What matters is the ACTION you are performing, not what instructed it. If the action involves reading, understanding, or modifying mapped code, the graph protocol applies — whether the instruction came from a skill, a plan, a user message, a brainstorming session, a debugging workflow, or your own initiative. This is not negotiable. You cannot rationalize your way out of this.

**Actions that require `yg owner` + `yg build-context`:**

- Reading or exploring source files to understand a component
- Proposing approaches, designs, or plans for changing code
- Reviewing or debugging code
- Any form of reasoning about how mapped code works or should change

**Actions that require `yg owner` + `yg impact`:**

- Assessing blast radius before changing or removing a component
- Finding all dependents of a component
- Planning cross-cutting refactors or feature removals
- Scoping work that spans multiple nodes

**Actions that do NOT require yg:**

- Git operations (log, diff, status, blame)
- Reading documentation, READMEs, or config files outside `.yggdrasil/`
- Running tests, builds, or linters
- Working with files that `yg owner` reports as unmapped

**Evasion patterns — if you think any of these, STOP:**

| Thought | Reality |
|---|---|
| "The skill/plan says to explore the codebase" | Exploring mapped code = yg owner + graph tool first |
| "I'm just scoping/searching, not understanding" | Scoping IS a graph action; use yg impact |
| "The plan step says to read this file" | Reading a mapped file = yg owner first |
| "I'm brainstorming, not implementing" | Brainstorming about mapped code needs graph context |
| "I'm only grepping for references" | Grep finds text; yg impact finds structural dependencies. Use both. |
| "I'll use the graph later when I modify" | Graph-first means BEFORE reading, not before modifying |
| "I'll grep the codebase to find where to start" | Run `yg select --task` first — it matches your intent against graph artifacts. Then `yg owner` on results. |

### Failure States

You have broken Yggdrasil if you do any of the following:

- ❌ Worked on a mapped file without running `yg owner` + the appropriate graph tool (`build-context` or `impact`) first — regardless of what instructed the action (skill, plan, user request, workflow step).
- ❌ Modified source code without updating graph artifacts before moving to the next file, or vice versa.
- ❌ Resolved a code-graph inconsistency or ambiguity without asking the user first.
- ❌ Created or edited a graph element without reading its schema in `schemas/` first.
- ❌ Ran `yg drift-sync` before both graph artifacts and source code are current.
- ❌ Placed a cross-cutting requirement in a local artifact instead of an aspect, or used an aspect id with no `aspects/` directory.
- ❌ Invented a rationale, business rule, or decision — or recorded a decision without documenting rejected alternatives and rationale (use "rationale: unknown" if unknown).
- ❌ Used blackbox coverage for greenfield (new) code.
- ❌ Deleted or shortened graph artifact content to reduce context package size instead of splitting the node.

### Escape Hatch

If the user explicitly requests a code-only change, comply but:

- Warn: "This creates drift. Run `yg drift` next session to reconcile."
- Do NOT run `yg drift-sync` — leave the drift visible.

### Environment Check

Before preflight:

- Verify `yg` CLI is available. If not found, inform user and stop.
- If `yg preflight` shows 0 nodes → enter BOOTSTRAP MODE (see Operations).
- If drift report shows >10 drifted nodes → report scope to user, ask which area to prioritize. Do not resolve all at once.

### Delegating to Subagents

When you delegate work to a subagent (any subprocess, tool agent, or spawned assistant), the subagent does NOT inherit your Yggdrasil knowledge. Before any other instruction, the subagent MUST:

1. Read `.yggdrasil/agent-rules.md` — this is the complete operating manual
2. Follow the Quick Start Protocol from that file before touching any mapped code

Include this as the FIRST instruction in every subagent prompt:

```
BEFORE doing anything else: read .yggdrasil/agent-rules.md and follow its protocol.
DELIVERABLES — all required, incomplete work will be rejected:
  1. Working source code
  2. Graph nodes with artifacts for every new/modified source file
  3. `yg validate` passing
```

A subagent that delivers code without corresponding graph updates has not completed its task. Before accepting subagent output, verify: are there new or modified source files without corresponding graph coverage? If yes, the work is incomplete.

---

## OPERATIONS

### Conversation Lifecycle

```
PREFLIGHT (every conversation, before any work):
  - [ ] 1. yg preflight → read unified report
  - [ ] 2. If drift: resolve per Drift Resolution, then yg drift-sync per node
  - [ ] 3. If validation errors: fix, re-run yg validate
  Exception: read-only requests (explain, analyze) — skip preflight.

UNDERSTANDING mapped code (questions, research, OR planning):
  - [ ] 1. yg owner --file <path>
  - [ ] 2. Owner found → yg build-context --node <path>. Read the YAML map
         for topology, then read artifact files from the artifacts section.
         For quick orientation, the map alone is sufficient. For implementation,
         read all artifact files before changing code.
  - [ ] 3. Owner not found → use file analysis, state it is not graph-backed.
  Never use grep or raw file reads as primary understanding when graph coverage exists.
  Raw reads supplement the context package — they do not replace it.

WRAP-UP (user signals "done", "wrap up", "that's enough"):
  - [ ] 1. yg drift --drifted-only → resolve
  - [ ] 2. yg validate → fix errors
  - [ ] 3. Report: which nodes and files were changed

BEFORE ENDING ANY RESPONSE (self-audit):
  - [ ] Did I interact with mapped code (read, research, or modify)? If yes → did I use a graph tool BEFORE reading source?
  - [ ] Did I modify source code? If yes → did I update graph artifacts before moving to the next file?
  - [ ] If you broke either rule, you have broken the protocol. Do not finish until both are fixed.
```

### Modify Source Code

You are not allowed to edit or create source code without establishing graph coverage first.

**Step 1** — Check coverage: `yg owner --file <path>`

**Step 2a** — Owner found: execute checklist:

- [ ] 1. Read specification: `yg build-context --node <node_path>`
- [ ] 2. Assess blast radius: `yg impact --node <node_path>` — review dependents, descendants, and co-aspect nodes before changing interfaces or shared behavior
- [ ] 3. Modify source code
- [ ] 4. Sync graph artifacts — edit artifact files to reflect the changes (after each file, not batched — context is freshest immediately after the change)
- [ ] 5. Run `yg validate` — fix all errors (if unfixable after 3 attempts → stop, report to user)
- [ ] 6. Run `yg drift-sync --node <node_path>` — only after graph and code are both current

**Step 2b** — Owner not found: establish coverage first. Present options to the user:

*Partially mapped* (file unmapped but inside a mapped module): ask whether to add to existing node or create new one.

*Existing code:*

- Option A — Full node: create node(s), map files, write artifacts from code analysis
- Option B — Blackbox: create a blackbox node at agreed granularity
- Option C — Abort

*Greenfield (new code):* Only Option A. Blackbox is forbidden for new code. Follow the graph-first workflow:

1. Create aspects first (cross-cutting requirements the new code must satisfy)
2. Create flows if the code participates in a business process
3. Create nodes with full artifacts — responsibility, interface, internals
4. Review the context package (`yg build-context`) — it is now the behavioral specification
5. Implement code that satisfies the specification
6. The graph specifies WHAT and WHY; the code implements HOW (framework APIs, library choices)

After the user chooses, return to Step 1 and follow Step 2a.

### Modify Graph

- [ ] 1. Read the relevant schema from `schemas/` before touching any YAML
- [ ] 2. Before changing an aspect or flow, check scope: `yg impact --aspect <id>` or `yg impact --flow <name>` — understand which nodes are affected before modifying shared rules or processes
- [ ] 3. Make changes
- [ ] 4. Run `yg validate` immediately — fix all errors
- [ ] 5. Verify affected source files are consistent — update if needed
- [ ] 6. Run `yg drift-sync` for affected nodes

### Reverse Engineering

**Order:** aspects (cross-cutting patterns) → flows (business processes) → model nodes. Never create nodes before aspects and flows are understood.

Per area checklist:

- [ ] 1. `yg owner --file <path>` — confirm no coverage
- [ ] 2. Determine node granularity — propose to user if unclear
- [ ] 3. Create node directory, read `schemas/yg-node.yaml`, create `yg-node.yaml`
- [ ] 4. Analyze source — for each artifact type in `yg-config.yaml artifacts`: extract content, do not invent
- [ ] 5. Identify relations — add to `yg-node.yaml`
- [ ] 6. Identify cross-cutting requirements — add matching aspects, create if needed
- [ ] 6b. For each aspect on the node: identify 2-5 code anchors (function names, constants) that evidence the pattern → add as `anchors` in the aspect entry in `yg-node.yaml`
- [ ] 7. Identify business process participation — add to flow, ask user if process unclear
- [ ] 8. `yg validate` — fix errors
- [ ] 9. `yg drift-sync --node <path>`

**When to ask:**

- Business process unclear: "This code appears to be part of a larger process. Can you describe what it means from a business perspective?"
- Constraint without rationale: "I see [constraint X]. Do you know why this exists? I want to record the reason, not just the rule."
- Unexplained architectural choice: "I see [approach X]. What was the reason for this choice?"
- Decision without alternatives: "You chose [X]. What alternatives did you consider, and why did you reject them?" Record the answer in the Decisions section of `internals.md`.
- Decision without known rationale: Record the decision in `internals.md` with "rationale: unknown — inferred from code, not confirmed by developer." A recorded decision with unknown rationale is infinitely more valuable than no record at all, and safer than an invented rationale.

### Bootstrap Mode

Trigger: `yg preflight` shows 0 nodes, or no nodes cover the active work area.

- [ ] 1. Identify the active work area (files the user wants to modify)
- [ ] 2. Scan for cross-cutting patterns → create aspects
- [ ] 3. Ask user about business processes → create flows if applicable
- [ ] 4. Propose node structure for the area
- [ ] 5. Create node(s) with initial artifacts, map files
- [ ] 6. `yg validate`, `yg drift-sync`
- [ ] 7. Proceed with user's original request

Constraint: Do NOT map the entire repository. Focus on the active area. Expand incrementally.

### Drift Resolution

Always ask the user before resolving drift. Never auto-resolve.

- **Source drift** (source files changed) → update graph artifacts to match source, then `yg drift-sync`
- **Graph drift** (graph artifacts changed) → review affected source, update if needed, then `yg drift-sync`
- **Full drift** (both changed) → present both sides to user, ask which direction wins
- **Missing** → ask: re-materialize or remove mapping?
- **Unmaterialized** → ask user how to proceed

Threshold: >10 drifted nodes → ask user which area to prioritize. Do not resolve all at once.

**Drift triage:** Prioritize aspects and `internals.md` (highest decay rate), then `responsibility.md` and `interface.md` (most stable).

### Graph Audit

When reviewing graph quality (triggered by user or quality improvement):

**Step 1 — Consistency** (catches WRONG information):

- [ ] 1. `yg build-context --node <path>`
- [ ] 2. Read mapped source files
- [ ] 3. For each claim in graph: verify against source code
- [ ] 4. For each aspect: verify the pattern holds in THIS node. If it deviates, add `exceptions` to the aspect entry in `yg-node.yaml`
- [ ] 5. Report inconsistencies

**Step 2 — Completeness** (catches MISSING information):

- [ ] 1. For each public method: is it in `interface.md`?
- [ ] 2. For each error path: is it in `interface.md` (Failure Modes section)?
- [ ] 3. For each behavioral invariant: is it in the graph?
- [ ] 4. Report omissions separately from inconsistencies

### Error Recovery

- **`yg` not found** → inform user: "yg CLI is not installed or not in PATH." Stop.
- **Unfixable validate errors** → if not resolved after 3 attempts, stop and report to user. Do not loop.
- **Budget warning (W005/W006)** → informational. `yg validate` shows a breakdown (own/hierarchy/aspects/flows/dependencies). Large inherited context means the system is complex — this is not a problem to fix, it is reality to acknowledge. Do not delete knowledge from artifacts. Do not attempt to "reduce" inherited context.
- **Own budget warning (W015)** → own artifacts are large. Consider splitting this node's responsibilities into child nodes. Redistribute knowledge across children so total knowledge is preserved or increased, never reduced.
- **Corrupted `.yggdrasil/` files** → report to user. Do not attempt repair.
- **Incremental sync** → run `yg drift-sync` every 3-5 source files during multi-file tasks. Do not defer to end.

---

## KNOWLEDGE BASE

### Graph Structure

```
.yggdrasil/
  yg-config.yaml     ← version, vocabulary, node types, artifact rules, required aspects
  model/             ← what exists: nodes, hierarchy, relations, file mappings
  aspects/           ← what must: cross-cutting requirements with rationale and guidance
  flows/             ← why and in what process: business processes with node participation
  schemas/           ← YAML schemas — read before creating any graph element
  .drift-state/      ← generated by CLI; never edit manually
```

Key facts:

- **Hierarchy:** nodes nest in `model/`. Children inherit parent context. Do not repeat parent content in children.
- **Aspect id = directory path** under `aspects/`. Each aspect has `yg-aspect.yaml` + content `.md` files. No automatic parent-child — use `implies` explicitly.
- **Flows = business processes.** A flow describes what happens in the world, not code sequences. Flow aspects propagate to all participants.

**Node type guidance:** Each type in `yg-config.yaml node_types` has a `description` that tells you when to use it. Check the project's config for the full list and descriptions. Common types: `module` (business logic), `service` (providing functionality), `library` (shared utilities), `infrastructure` (guards, middleware, interceptors — invisible in call graphs but affect blast radius).

### Artifact Structure

Three artifacts capture node knowledge at three levels:

- **responsibility.md** (always required) — WHAT: identity, boundaries, what the node is NOT responsible for.
- **interface.md** (required when node has consumers) — HOW TO USE: public methods, parameters, return types, contracts, failure modes, exposed data structures. Everything another node needs to interact with this one.
- **internals.md** (optional, highest value for cross-module nodes) — HOW IT WORKS + WHY: algorithms, control flow, business rules, invariants, state machines, lifecycle, and design decisions with rejected alternatives. Use sections within the file: ## Logic, ## Constraints, ## State, ## Decisions (with "Chose X over Y because Z" format).

**Enrichment priority (when adding incrementally):** `interface.md` first (highest cross-module ROI — contracts enable other nodes to reason about interactions), then `responsibility.md` (identity and boundaries), then `internals.md` (depth for complex nodes). A node with only `interface.md` provides more cross-module value than one with only `internals.md`.

Projects can define additional artifact types in `yg-config.yaml` under `artifacts`. Each custom artifact has a `description` (tells you what to write), a `required` condition (`always`, `never`, `when: has_incoming_relations`, `when: has_aspect:<id>`), and an `included_in_relations` flag (if true, included in dependency context packages for structural relations). The three standard artifacts are always present in config. Check `yg-config.yaml` to see all defined artifacts for the project.

### Context Assembly

**Reading context:** `yg build-context --node <path>` returns a YAML map with the node's topology (hierarchy, dependencies, aspects, flows) and an `artifacts` section listing files to read. All artifact paths are relative to `.yggdrasil/` — construct full path as `.yggdrasil/<path>`.

**Default mode (paths-only):** Use for all graph operations. Read the YAML map first to understand topology. Then read artifact files from the `artifacts` section using the Read tool. For quick orientation (scoping, blast radius assessment), the map alone is sufficient. For implementation or modification, read all artifact files before changing code.

**Full mode (`--full`):** Use only when you cannot read files individually — e.g., when pasting context into a prompt, sharing with a user, or when you have no Read tool available.

Artifact paths are stable identifiers within a session. When building context for multiple nodes, skip reading files you have already read — same path means same content.

### Information Routing

When you encounter information, route it to the correct location:

- **Specific to this node** → local node artifact (check `yg-config.yaml artifacts` for available types)
- **Rule for many nodes** → aspect (`aspects/<id>/` with `yg-aspect.yaml` + content `.md` files). If applies to ALL nodes of a type → `node_types.<type>.required_aspects` in `yg-config.yaml`
- **Business process** → flow (`flows/<name>/` with `yg-flow.yaml` + `description.md`). Ask user if process unclear.
- **Shared across a domain** → parent node artifact. Children receive it through hierarchy.
- **Technology stack or standard** → node artifact at the appropriate hierarchy level (e.g., root node's `responsibility.md` for single-stack repos, or deployment unit node for monorepos)
- **Decision (why + why NOT):** one node → Decisions section of `internals.md` with format "Chose X over Y because Z"; category of nodes → aspect content files; tech choice → node artifact at the level where the technology applies. Always include rejected alternatives — they are the highest-value graph content. If the rationale is unknown: record the decision with "rationale: unknown" and note what CAN be observed from the code. Never invent a plausible-sounding rationale.

### Creating Aspects

- [ ] 1. Read `schemas/yg-aspect.yaml`
- [ ] 2. Create `aspects/<id>/` directory
- [ ] 3. Write `yg-aspect.yaml` — name, optional description, optional implies
- [ ] 4. Write content `.md` files: WHAT must be satisfied + WHY (user's words, do not invent)
- [ ] 5. `yg validate`

Test: "Does this requirement apply to more than one node?" Yes → aspect. No → local artifact.

**Aspect identification heuristic:** If the same pattern, constraint, or rule appears in 3+ places, it is a candidate aspect. Aspects fall into natural categories:

- **Domain-specific:** Business rules that cross module boundaries (e.g., timezone handling, booking periods, currency rounding)
- **Architectural:** Structural patterns with rationale (e.g., dual-rollback on provider failure, idempotency via key generation, fire-and-forget dispatch)
- **Concurrency:** Shared concurrency strategies (e.g., pessimistic locking, retry-on-deadlock, optimistic versioning)

When a node follows an aspect's pattern with exceptions, record them in the `exceptions` field of the aspect entry in `yg-node.yaml`. Example: aspect says "fire-and-forget" but this node awaits the publish call — add `exceptions: ["awaits publish call instead of fire-and-forget because..."]`. Exceptions appear in the context package next to the aspect content, preventing abstractions from masking implementation details.

**Aspect lifecycle warning.** Aspects decay CATASTROPHICALLY — a pattern either exists or it doesn't. When a pattern changes, ALL aspect claims become wrong at once. This differs from other artifacts: `interface.md` and `responsibility.md` are most stable (~9-year half-life); `internals.md` has moderate stability (~2.5-year half-life); aspects are least stable (~2.4-year half-life, binary decay). After any significant feature addition, review ALL aspects touching the affected area. Don't wait for drift — aspects can be 100% wrong without any mapped file changing.

**Aspect stability tiers.** If an aspect has a `stability` field in `yg-aspect.yaml`, use it to calibrate review urgency:

- `schema` — enforced by data model; review only when data model changes (most stable)
- `protocol` — contractual pattern; review when contracts or interfaces change
- `implementation` — specific mechanism; review after ANY significant code change (least stable)

When code anchors (`anchors` in an aspect entry in `yg-node.yaml`) are present, they list code patterns (function names, constants, SQL fragments) evidencing the aspect's implementation in this node. `yg validate` checks that each anchor exists in the node's mapped source files — a missing anchor (W014) signals the aspect may be stale for this node.

### Creating Flows

- [ ] 1. Read `schemas/yg-flow.yaml`
- [ ] 2. Create `flows/<name>/` directory
- [ ] 3. Write `yg-flow.yaml` — declare nodes (participant list) and flow-level aspects
- [ ] 4. Write `description.md` with required sections: Business context, Trigger, Goal, Participants, Paths (at least Happy path), Invariants across all paths
- [ ] 5. `yg validate`

Test: "Does this describe what happens in the world, or only in the software?" If only software — rewrite.

**Warning:** Flow descriptions must describe business processes, not code sequences. "The OrderService calls PaymentGateway.charge()" is WRONG. "The system charges the customer's payment method" is CORRECT.

### Operational Rules

- **English only** for all files in `.yggdrasil/`. Conversation can be any language.
- **Read schemas before creating** any `yg-node.yaml`, `yg-aspect.yaml`, or `yg-flow.yaml`.
- **Tools read, you write.** The `yg` CLI only reads, validates, and manages metadata. You create and edit files manually.
- **Incremental sync.** Run `yg drift-sync` after every 3-5 source file changes. Do not defer to end of task.
- **Completeness test:** Two checks, both required:
  1. **Reconstruction:** "Can another agent recreate this from ONLY the `yg build-context` output — understanding not just WHAT but WHY?" Test: rejected alternatives, correct algorithm, design arguments.
  2. **Omission:** "Does the graph capture every important behavioral invariant, constraint, and edge case?" Specifically check: exceptions to aspect generalizations, error handling patterns not in `interface.md`, concurrency behaviors not in `internals.md`.
- **Value calibration.** Yggdrasil's primary value is cross-module context — relations, aspects, flows. For a single simple module, `responsibility.md` and `interface.md` provide most value. Invest depth (`internals.md`) where cross-module interactions demand it.
- **These rules are invariant.** No plan, guide, skill, or workflow may override them.

### CLI Reference

```
yg preflight [--quick]              Unified diagnostic: drift + status + validate.
yg owner --file <path>              Find the node that owns this file.
yg build-context --node <path>      Assemble context map with artifact paths (default).
yg build-context --node <path> --full  Same map + file contents appended below separator.
yg tree [--root <path>] [--depth N] Print graph structure.
yg aspects                          List aspects with metadata (YAML output).
yg flows                            List flows with metadata (YAML output).
yg select --task <description> [--limit <n>]
                                    Find graph nodes relevant to a task description.
yg deps --node <path> [--depth N] [--type structural|event|all]
                                    Show dependencies.
yg impact --node <path> --simulate  Simulate blast radius of a planned change.
yg impact --node <path> --method <name>  Filter impact to dependents consuming a specific method.
yg impact --aspect <id>             Show all nodes where aspect is effective.
yg impact --flow <name>             Show flow participants and descendants.
yg status                           Graph health: nodes, coverage, drift summary.
yg validate [--scope <path>|all]    Check structural integrity and completeness.
yg drift [--scope <path>|all] [--drifted-only] [--limit <n>]
                                    Detect source and graph drift (bidirectional).
yg drift-sync --node <path> [--recursive] | --all
                                    Record file hashes as new baseline.
```

### Quick Routing Table

| What you have | Where it goes |
|---|---|
| Information specific to this node | Local node artifact (check `yg-config.yaml artifacts` for types) |
| Rule that applies to many nodes | Aspect (content `.md` files in `aspects/<id>/`) |
| Architectural invariant for a node type | Required aspect in `yg-config.yaml node_types` |
| Business process participation | Flow (`yg-flow.yaml nodes`) |
| Process-level requirement | Flow `aspects` + aspect directory |
| Context shared across a domain | Parent node artifact |
| Technology stack | Node artifact at appropriate hierarchy level |
| Coding standards | Node artifact at appropriate hierarchy level |
