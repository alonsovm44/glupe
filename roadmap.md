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

4. nameable containers like this [done]
$${}$$ -> No name, disposable, like a lambda (if other containers are found without id, they do not collide)
$$ "x-1"{}$$ -> container_id = "x-1"
Yori warns when two containters have the same name, collision aborts making process.

    4.1 command idea:
    `yori project.txt -make -u` -> this updates the code outside containers the same way. It does not regenerate all containers, only those whose hash has changed. 

        4.1.1 `yori project.txt -make -u "x-1" "x-2" [DONE]
        this only changes containers with that name, and leaves the rest untouched.
        PROBLEM WITH THIS APPROACH: if I modify container X and I only specify to modify container Y, When I run `-make -u "Y"` only Y updates and now the container and the implementation are out of sync
        SOLUTION: use `-u` to update all 

    4.2 Make a temporary .yori.lock file like this [DONE X ] 
    {
  "containers": {
    "x-1": {
      "hash": "a1b2c3d4...", // Hash of the Prompt text
      "output_file": "utils.cpp",
      "last_run": "2023-10-27T10:00:00Z"
    },
    "main-loop": {
      "hash": "e5f6g7h8...",
      "output_file": "main.cpp"
    }
  }
}
How it works:

User runs yori -make -u.
Yori reads the .yori file.
It calculates the hash of the current prompt in "x-1".
It compares it to the hash in .yori.lock.
Match? Skip AI call. Copy existing code from cache or leave file untouched.
Mismatch? Call AI, generate new code, update lock file.

4.3 Comments in file output telling what container it is `this can be done by educating the user to wrap his containers in comments`
Example:
EXPORT: "main.cpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "--- Start ---" << endl;

    // Container 1: We will keep this one unchanged
    $$ "block_stable" {
        print "Stable Block: Running..."
        simulate a heavy calculation by sleeping for 100ms
    }$$

    // Container 2: We will modify this one later
    $$ "block_dynamic" {
        print "Dynamic Block: Version 1"
    }$$

    cout << "--- End ---" << endl;
    return 0;
}
EXPORT: END

Output:
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "--- Start ---" << endl;

    // cont1{
    cout << "Stable Block: Running..." << endl;
    this_thread::sleep_for(chrono::milliseconds(100));
    //}
    
    // cont2{
        cout << "Dynamic Block: Version 1" << endl;
    //}
    cout << "--- End ---" << endl;
    return 0;
}

4.5 In single file mode, when api errors appear the system skips one pass, we have to fix that so it does not skip a pass

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
- Inheritance 
- Abstracts: Next.
- Then YoriHub.


## For 6.0
1.  AST w tree sitter (not really needed but cool for 6.0 onwards)

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