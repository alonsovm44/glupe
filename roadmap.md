# TODO LIST
If you could help me getting this donde I would appreciate it much
 
## For version 5.7.2
-Fix readme so it does not sound like vaporware, make readme more concise [done]


# For version 5.8

1. preFlightcheck works for any language not just hardcoded c & c++ (lines 768+) [done for python, pending for the rest of langs][]

2. Treeshaking, at least functional [pending]

3. Estimated remaining time of compilation / production [added ETA]

**IMPORTANT**: Add a yori_cache/ folder to store needed information [DONE]
Move .yori_build.cache to yori_cache/

4.5 In single file mode, when api errors appear the system skips one pass, we have to fix that so it does not skip a pass [DONE]

5. The Determinism Problem (Lines 1014-1037)
```cpp
for(int gen=1; gen<=passes; gen++) {
    string response = callAI(prompt.str());
    string code = extractCode(response);
    // No temperature control, no seeding
}
```
**Issue:** Every build generates different code.
Fix for v5.8:
```cpp
// In callAI(), add to JSON payload:
if (seriesMode || makeModes) {
    body["temperature"] = 0.0; // Deterministic
    body["seed"] = currentHash; // Reproducible (if API supports)
}
```

6. Make semantic guardrails more general, not just hardcoded for Python.h

7. No Incremental Build for Series Mode
for single file mode there is -u mode. 
-make -series mode regenerates EVERYTHING on every run. For a 10-file project, this is slow and expensive.
Solution for v5.8:
```cpp
// Add to BlueprintEntry:
struct BlueprintEntry {
    string filename;
    string content;
    size_t hash;
    vector<string> dependencies; // NEW: Track which files this imports
};

// In -series loop (line 857):
for (const auto& item : blueprint) {
    // Check if file exists AND hash matches AND dependencies unchanged
    if (fs::exists(item.filename)) {
        size_t currentHash = hash<string>{}(item.content + projectContext);
        // Load cached hash from .yori_build/<filename>.hash
        if (cachedHashMatches(item.filename, currentHash)) {
            cout << "   [CACHED] Skipping " << item.filename << endl;
            // Load existing file content into projectContext
            ifstream cached(item.filename);
            projectContext += string((istreambuf_iterator<char>(cached)), istreambuf_iterator<char>());
            continue;
        }
    }
    // Otherwise regenerate as normal...
}
```

8. Wrapper detection for multiple languages, not just c++

9. Container inheritance
4. "Logic Inheritance" (The OOP of Intent)
This is the most exciting engineering possibility.

Scenario: You have a standard approach for logging across your company.
Unlock: You define a "Base Container" logic (mentally or in a template): $$ "logging-base" { Log to JSON format with timestamp }$$.
Implementation: In your specific project, you can have containers that "inherit" this behavior by referencing it or including it.
cpp

$$ "api-error-log" inherits "logging-base" {
   Add error stack trace to the log.
}$$     
Result: You can change the base logging style in one place, and every child container in your entire project updates its implementation.

Named containers: DONE.
- Abstract containers: Next.
- Inheritance 

- Then YoriHub (last).

## For 6.0
1.  AST w tree sitter (Hard Isolation)
    - [ ] Integrate tree-sitter C library
    - [ ] Implement language-agnostic placeholder replacement (normalize `$${}` to `/*ID*/`)
    - [ ] Implement AST diffing algorithm to verify "Host Code" immutability
    - [ ] Implement "Smart Context": Send only relevant scope (Enclosing Class/Function) to LLM

2.  Smart Dependency Management (True Treeshaking)
    - [ ] Use AST to detect symbols used in generated code
    - [ ] Check imports/includes in AST root
    - [ ] Inject missing dependencies automatically

3. Project memory

6. Modularize yoric.cpp into components each managing an aspect of the program, so other find it easier to contribute.
[make only when contributors arrive]

**this requires YoriHub page working**

7. Yori packages.
`yori pull "my:container_id"` downloads "my:containter_id" from YoriHub as a text file in the CWD

    7.1 `yori pull "my:container_id" main.txt` this downloads the container and writes it in the main.txt file at the end.  

8. Yori publish. `yori publish "my_container_id.txt" user_name password` this uploads a container text file 
to the user's profile DB. 

9. FFI integration for imports

10. Namespaces resolution for IMPORT directive
```markdown
IMPORT: "script.py" as spy
FROM spy USE myfunc()
COM=only uses myfunc()

x=3

PRINT(MYFUNC(x))
```