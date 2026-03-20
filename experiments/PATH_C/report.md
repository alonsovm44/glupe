# Experiment Report: Validation of Semantic Subtraction and Audit Guardrails (Path C)
Project: The Semantic Equivalence Verification Loop
Date: March 20, 2026
Researcher: Alonso (Glupe)

## 1. Objective

To test the viability of "Semantic Subtraction" (Spec vs. Implementation) in Glupe. The goal is to prove that by translating both an architectural specification and an AI-generated implementation into Glupe Intermediate Representation (GIR), we can mathematically compute the missing and hallucinated features using Set Theory ($Spec - Impl$).

## 2. Methodology

A multi-step forward and backward generation loop was executed:
1. **Specification**: Created `win_installer.glp`, a high-level intent script requesting a PowerShell installer execution.
2. **Implementation**: Compiled the `.glp` into a C++ program (`wini.cpp`). The AI applied standard C++ abstractions (e.g., helper templates for `system()`).
3. **Refinement (Reverse-Engineering)**: Used `glupe -refine` to extract the semantic blueprint of the generated `wini.cpp`, yielding `wini.cpp.glp`.
4. **Audit**: Executed the newly implemented `glupe audit` command to compare `win_installer.glp` against `wini.cpp.glp` via LLM-powered deep subtraction.

## 3. Results

The audit produced a deterministic failure, which is the mathematically correct and desired outcome acting as a strict guardrail.

### 3.1 Rate Limit Resilience
The utility API calls successfully intercepted `429 Rate Limit` errors, parsed the required backoff times, and resumed operation without crashing or corrupting the semantic loop.

### 3.2 Semantic Drift Alignment
The AI successfully bridged the naming gap between human specifications and refined architecture:
- `done` -> `DoneStep`
- `main` -> `MainStep`
- `popup_intro` -> `PopupIntro`

### 3.3 Architectural Hallucination Detection ($Impl - Spec$)
The structural subtraction successfully identified scaffolding containers that the AI "hallucinated" to make the C++ code professional, which were not explicitly requested in the spec:
- `Constants`
- `EmitPopup`
- `ExecuteSystemCommand`
- `MainFlow`
- `StandardIncludes`

### 3.4 Deep Omission Detection ($Spec - Impl$)
The deep semantic subtraction successfully caught the exact deviation in intent within the mapped containers:
- **Missing**: Direct invocation of `system(...)` as requested.
- **Hallucinated**: The use of the abstracted `ExecuteSystemCommand` template wrapper.

## 4. Conclusion

The experiment successfully validates the Prompt Arithmetic theory of "Prompt Subtraction" in a practical CI/CD context. 

The LLM "Omission Blindspot" has been successfully solved. By reducing both artifacts to the same semantic vector space, Glupe mathematically evaluates the absence of requested logic. The `audit` command successfully acts as a deterministic, automated guardrail to reject generated code that drifts from the master architecture blueprint.