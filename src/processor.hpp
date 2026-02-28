#pragma once
#include "ai.hpp"
#include "parser.hpp"
#include "cache.hpp"
#include "config.hpp"
#include "languages.hpp"

// [NEW] Pre-process input to handle containers and caching
inline string processInputWithCache(const string& code, bool useCache, const vector<string>& updateTargets, bool fillMode) {
    // [FUTURE v6.0] AST INTEGRATION POINT
    // 1. Normalize: Replace $$...$$ with valid placeholders (e.g. comments or void calls)
    // 2. Parse: auto tree = parser.parse_string(normalized_code);
    // 3. Traverse: Find placeholder nodes in the AST.
    // 4. Context: For each node, walk up to find enclosing function/class for prompt context.
    // 5. Generate: Call LLM with AST-derived context.
    // 6. Verify: Parse result, ensure tree structure outside container is identical.
    // 7. Unparse: Convert modified AST back to string.
    
    string result;
    size_t pos = 0;
    
    while (pos < code.length()) {
        size_t start = code.find("$", pos);
        if (start == string::npos) {
            result += code.substr(pos);
            break;
        }

        // [v6.0] Semantic Parsing
        bool isBlock = false;
        bool isInline = false;
        bool isVarPersistent = false;
        bool isVarEphemeral = false;
        bool isConstant = false;
        size_t scan = 0;

        if (start + 1 < code.length() && code[start+1] == '$') {
            if (start + 2 < code.length() && code[start+2] == ':') {
                // $$: Persistent Variable
                isVarPersistent = true;
                scan = start + 3;
            } else {
                // $$ Block Container
                isBlock = true;
                scan = start + 2;
            }
        } else {
            if (start + 1 < code.length() && code[start+1] == ':') {
                isVarEphemeral = true;
                scan = start + 2;
            } else if (start + 7 <= code.length() && code.compare(start, 7, "$CONST:") == 0) {
                isConstant = true;
                scan = start + 7;
            } else {
            // Inline container check: $ ... { ... } ... $ on same line
            size_t lineEnd = code.find('\n', start);
            if (lineEnd == string::npos) lineEnd = code.length();
            
            size_t openB = code.find('{', start);
            if (openB != string::npos && openB < lineEnd) {
                // Look for } followed by $
                size_t searchPos = openB + 1;
                while (searchPos < lineEnd) {
                    size_t closeB = code.find('}', searchPos);
                    if (closeB == string::npos || closeB >= lineEnd) break;
                    size_t check = closeB + 1;
                    while (check < lineEnd && isspace(code[check])) check++;
                    if (check < lineEnd && code[check] == '$') {
                        isInline = true;
                        scan = start + 1;
                        break;
                    }
                    searchPos = closeB + 1;
                }
            }
            }
        }

        // Handle Variables and Constants
        if (isVarPersistent || isVarEphemeral || isConstant) {
            // Parse Variable Declaration
            // Format: $...: ID -> VALUE \n
            
            // 1. Parse ID
            while(scan < code.length() && isspace(code[scan])) scan++;
            size_t idStart = scan;
            while(scan < code.length() && (isalnum(code[scan]) || code[scan] == '_')) scan++;
            string id = code.substr(idStart, scan - idStart);
            
            // 2. Parse Arrow ->
            while(scan < code.length() && isspace(code[scan])) scan++;
            if (scan + 1 < code.length() && code[scan] == '-' && code[scan+1] == '>') {
                scan += 2;
            } 

            // 3. Parse Value (rest of line)
            while(scan < code.length() && isspace(code[scan]) && code[scan] != '\n') scan++;
            size_t valStart = scan;
            size_t lineEnd = code.find('\n', scan);
            if (lineEnd == string::npos) lineEnd = code.length();
            
            string value = code.substr(valStart, lineEnd - valStart);
            // Trim trailing whitespace
            size_t last = value.find_last_not_of(" \t\r");
            if (last != string::npos) value = value.substr(0, last + 1);
            else value = "";

            // Create SemanticNode (Placeholder for Phase 2)
            SemanticNode node;
            if (isVarPersistent) node.type = NodeType::VAR_PERSISTENT;
            else if (isVarEphemeral) node.type = NodeType::VAR_EPHEMERAL;
            else node.type = NodeType::CONSTANT;
            
            node.id = id;
            node.content = value;
            node.hash = getContainerHash(value);
            
            SYMBOL_TABLE[id] = node; // [NEW] Store in symbol table
            
            if (VERBOSE_MODE) cout << "   [VAR] Detected " << (isConstant ? "CONST" : "VAR") << ": " << id << " = " << value << endl;

            result += code.substr(pos, start - pos);
            pos = lineEnd; 
            continue;
        }

        if (!isBlock && !isInline) {
            result += code.substr(pos, start - pos + 1);
            pos = start + 1;
            continue;
        }

        // Check named $$ "id" {
        bool isNamed = false;
        bool isAbstract = false; // [NEW] Track abstract status
        string id;
        vector<string> paramIds;  // [NEW] Context Injection params
        vector<string> parentIds; // [NEW] Parent IDs for inheritance
        size_t contentStart = 0;
        
        while(scan < code.length() && isspace(code[scan])) scan++;
        
        // [NEW] Check for ABSTRACT keyword
        if (scan + 8 <= code.length() && code.compare(scan, 8, "ABSTRACT") == 0 && (scan + 8 == code.length() || isspace(code[scan+8]))) {
            isAbstract = true;
            scan += 8;
            while(scan < code.length() && isspace(code[scan])) scan++;
        }

        if (scan < code.length() && code[scan] != '{') {
            size_t idStart = scan;
            // [UPDATED] Stop at '(' for params
            while(scan < code.length() && !isspace(code[scan]) && code[scan] != '{' && code[scan] != '(' && !(code[scan] == '-' && scan+1 < code.length() && code[scan+1] == '>')) {
                scan++;
            }
            
            if (scan > idStart) {
                id = code.substr(idStart, scan - idStart);
                
                // [NEW] Parse Parameters (Context Injection)
                if (scan < code.length() && code[scan] == '(') {
                    size_t pStart = scan + 1;
                    size_t pEnd = code.find(')', pStart);
                    if (pEnd != string::npos) {
                        string paramStr = code.substr(pStart, pEnd - pStart);
                        stringstream ss(paramStr);
                        string segment;
                        while(getline(ss, segment, ',')) {
                            segment.erase(0, segment.find_first_not_of(" \t"));
                            segment.erase(segment.find_last_not_of(" \t") + 1);
                            if(!segment.empty()) paramIds.push_back(segment);
                        }
                        scan = pEnd + 1;
                    }
                }

                size_t brace = scan;
                
                // [NEW] Check for inheritance ->
                while(brace < code.length() && isspace(code[brace])) brace++;
                if (brace + 1 < code.length() && code[brace] == '-' && code[brace+1] == '>') {
                     size_t pScan = brace + 2;
                     while (pScan < code.length()) {
                         while(pScan < code.length() && isspace(code[pScan])) pScan++;
                         if (pScan >= code.length() || code[pScan] == '{') break;
                         
                         size_t pStart = pScan;
                         while(pScan < code.length() && !isspace(code[pScan]) && code[pScan] != ',' && code[pScan] != '{') {
                             pScan++;
                         }
                         if (pScan > pStart) {
                             parentIds.push_back(code.substr(pStart, pScan - pStart));
                         }
                         
                         while(pScan < code.length() && isspace(code[pScan])) pScan++;
                         if (pScan < code.length() && code[pScan] == ',') pScan++;
                         else if (code[pScan] == '{') { brace = pScan; break; }
                     }
                     brace = pScan;
                }

                while(brace < code.length() && isspace(code[brace])) brace++;
                if (brace < code.length() && code[brace] == '{') {
                    isNamed = true;
                    contentStart = brace + 1;
                }
            }
        }

        if (isNamed) {
            // Find end of container
            size_t end = string::npos;
            size_t nextPos = 0;

            if (isBlock) {
                end = code.find("}$$", contentStart);
                if (end != string::npos) nextPos = end + 3;
            } else {
                // Inline end finding
                size_t lineEnd = code.find('\n', start);
                if (lineEnd == string::npos) lineEnd = code.length();
                size_t searchPos = contentStart;
                while (searchPos < lineEnd) {
                    size_t closeB = code.find('}', searchPos);
                    if (closeB == string::npos || closeB >= lineEnd) break;
                    size_t check = closeB + 1;
                    while (check < lineEnd && isspace(code[check])) check++;
                    if (check < lineEnd && code[check] == '$') {
                        end = closeB;
                        nextPos = check + 1;
                        break;
                    }
                    searchPos = closeB + 1;
                }
            }

            if (end == string::npos) {
                // Malformed, just append and continue
                result += code.substr(pos, (isBlock ? 2 : 1));
                pos = start + (isBlock ? 2 : 1);
                continue;
            }

            string prompt = code.substr(contentStart, end - contentStart);
            
            // [NEW] Logic Inheritance
            string contextStr = "";
            if (!parentIds.empty()) {
                for(const auto& pid : parentIds) {
                    if (SYMBOL_TABLE.count(pid)) {
                        contextStr += "\n--- INHERITED FROM " + pid + " ---\n" + SYMBOL_TABLE[pid].content + "\n";
                        cout << "   [INHERIT] Container '" << id << "' inherits from '" << pid << "'" << endl;
                    } else {
                        cout << "   [WARN] Parent container '" << pid << "' not found (must be defined before use)." << endl;
                    }
                }
            }

            // [NEW] Context Injection (Params)
            if (!paramIds.empty()) {
                for(const auto& pid : paramIds) {
                    if (SYMBOL_TABLE.count(pid)) {
                        contextStr += "\n--- INJECTED CONTEXT (" + pid + ") ---\n" + SYMBOL_TABLE[pid].content + "\n";
                        cout << "   [INJECT] Context '" << pid << "' injected into '" << id << "'" << endl;
                    } else {
                        // If not in symbol table, treat as a raw parameter name for the AI
                        contextStr += "\n--- PARAMETER: " + pid + " ---\n";
                    }
                }
            }

            if (!contextStr.empty()) {
                string childLogic = prompt;
                prompt = "CONTEXT:\n" + contextStr + "\nRESOLUTION RULES:\n1. Child logic overrides parent logic.\n2. Use injected context as data/functions.\n\n--- CHILD LOGIC (" + id + ") ---\n" + childLogic;
            }

            // Store resolved prompt in symbol table for future children
            SemanticNode containerNode;
            containerNode.type = NodeType::CONTAINER;
            containerNode.id = id;
            containerNode.content = prompt;
            containerNode.parents = parentIds;
            containerNode.params = paramIds;
            SYMBOL_TABLE[id] = containerNode;

            // [NEW] Abstract Container Logic
            if (isAbstract) {
                cout << "   [ABSTRACT] Defined container: " << id << endl;
                result += code.substr(pos, start - pos); // Append text before container
                result += "// [ABSTRACT: " + id + "]\n"; // Placeholder comment (no code generation)
                pos = nextPos;
                continue;
            }

            string currentHash = getContainerHash(prompt);
            
            result += code.substr(pos, start - pos); // Append text before container

            bool cacheHit = false;
            
            // Check if we should skip this container (Selective Update)
            bool skipUpdate = false;
            if (useCache && !updateTargets.empty()) {
                bool isTarget = false;
                for(const auto& t : updateTargets) if(t == id) isTarget = true;
                if (!isTarget) skipUpdate = true;
            }

            if (useCache && (skipUpdate || LOCK_DATA["containers"].contains(id))) {
                // If skipping, ignore hash check and try to load cache immediately
                if (skipUpdate) {
                    string content = getCachedContent(id);
                    if (!content.empty()) {
                        cout << "   [SKIP] Keeping container: " << id << endl;
                        result += "\n// GLUPE_BLOCK_START: " + id + "\n";
                        result += content; 
                        result += "\n// GLUPE_BLOCK_END: " + id + "\n";
                        cacheHit = true;
                    } else {
                        cout << "   [WARN] Cache missing for skipped container: " << id << ". Regenerating." << endl;
                    }
                }
                // Standard check: hash comparison
                else if (LOCK_DATA["containers"].contains(id)) {
                string storedHash = LOCK_DATA["containers"][id]["hash"];
                if (storedHash == currentHash) {
                    string content = getCachedContent(id);
                    if (!content.empty()) {
                        cout << "   [CACHE] Using cached container: " << id << endl;
                        // [FIX] Wrap cached content in markers so AI preserves it
                        result += "\n// GLUPE_BLOCK_START: " + id + "\n";
                        result += content; 
                        result += "\n// GLUPE_BLOCK_END: " + id + "\n";
                        cacheHit = true;
                    }
                    }
                }
            }

            if (!cacheHit) {
                if (fillMode) {
                    // [FILL MODE] Generate immediately to preserve surrounding code
                    cout << "   [FILL] Generating container: " << id << "..." << endl;
                    
                    // Construct context from what we have processed so far + what remains
                    // This gives the AI the full file view without being able to touch it
                    string currentContext = result + code.substr(pos);
                    
                    stringstream aiPrompt;
                    aiPrompt << "ROLE: Code Generator.\n";
                    aiPrompt << "TASK: Implement the code for the container '" << id << "'.\n";
                    aiPrompt << "LANGUAGE: " << CURRENT_LANG.name << "\n";
                    aiPrompt << "CONTEXT:\n" << currentContext << "\n";
                    aiPrompt << "CONTAINER PROMPT:\n" << prompt << "\n";
                    aiPrompt << "OUTPUT: Only the code implementation. No markdown. No explanations.\n";

                    string generated = callAI(aiPrompt.str());
                    string cleanGenerated = extractCode(generated);
                    
                    result += "\n// GLUPE_BLOCK_START: " + id + "\n";
                    result += cleanGenerated;
                    result += "\n// GLUPE_BLOCK_END: " + id + "\n";
                    
                    // Update cache immediately
                    setCachedContent(id, cleanGenerated);
                    LOCK_DATA["containers"][id]["hash"] = currentHash;
                    LOCK_DATA["containers"][id]["last_run"] = time(nullptr);
                    saveCache();
                } else {
                    // [STANDARD MODE] Wrap in markers for global pass
                    result += "\n// GLUPE_BLOCK_START: " + id + "\n";
                    result += prompt; // The prompt for the AI
                    result += "\n// GLUPE_BLOCK_END: " + id + "\n";
                    
                    // Update lock data (will be saved after successful generation)
                    LOCK_DATA["containers"][id]["hash"] = currentHash;
                    LOCK_DATA["containers"][id]["last_run"] = time(nullptr);
                }
            }

            pos = nextPos; 
        } else {
            // Anonymous or malformed, keep as is (or handle anonymous logic)
            result += code.substr(pos, start - pos + (isBlock ? 2 : 1));
            pos = start + (isBlock ? 2 : 1);
        }
    }
    return result;
}

// [NEW] Tree Shaking Logic
inline string performTreeShaking(const string& code, const string& language) {
    cout << "   [OPTIMIZE] Tree shaking (removing unused code)..." << endl;
    stringstream prompt;
    prompt << "ROLE: Senior Code Optimizer.\n";
    prompt << "TASK: Analyze the following " << language << " code and remove UNUSED functions, variables, and imports.\n";
    prompt << "RULES:\n";
    prompt << "1. Keep the 'main' function (or entry point) and everything it uses (transitively).\n";
    prompt << "2. Keep all 'EXPORT:' directives and file structures intact.\n";
    prompt << "3. Remove dead code that is never called or referenced.\n";
    prompt << "4. Do not change logic, only remove unused elements.\n";
    prompt << "5. Return ONLY the cleaned code.\n";
    prompt << "CODE:\n" << code << "\n";
    
    string response = callAI(prompt.str());
    string cleaned = extractCode(response);

    // [SAFETY] Verify that EXPORT directives were not lost during optimization
    if (code.find("EXPORT:") != string::npos && cleaned.find("EXPORT:") == string::npos) {
        cout << "   [WARN] Tree shaking corrupted file structure (lost EXPORTs). Reverting." << endl;
        return code;
    }

    return (cleaned.find("ERROR:") == 0) ? code : cleaned; // Fallback if error
}

// [NEW] Post-process AI output to update cache
inline string updateCacheFromOutput(string code) {
    string cleanCode;
    size_t pos = 0;
    
    while (pos < code.length()) {
        size_t start = code.find("// GLUPE_BLOCK_START: ", pos);
        if (start == string::npos) {
            cleanCode += code.substr(pos);
            break;
        }
        
        cleanCode += code.substr(pos, start - pos); // Keep text before marker
        
        size_t idStart = start + 21;
        size_t idEnd = code.find('\n', idStart);
        if (idEnd == string::npos) break; // Should not happen
        
        string id = code.substr(idStart, idEnd - idStart);
        // Trim id
        id.erase(0, id.find_first_not_of(" \t\r"));
        id.erase(id.find_last_not_of(" \t\r") + 1);

        size_t blockEnd = code.find("// GLUPE_BLOCK_END: " + id, idEnd);
        if (blockEnd == string::npos) {
            // Marker broken by AI, just keep going
            cleanCode += code.substr(start); 
            break;
        }

        // Extract content
        string content = code.substr(idEnd + 1, blockEnd - (idEnd + 1));
        
        // Save to cache
        setCachedContent(id, content);
        cout << "   [CACHE] Updated container: " << id << endl;

        cleanCode += content; // Keep content in final file
        
        // Skip end marker
        size_t markerEnd = code.find('\n', blockEnd);
        pos = (markerEnd == string::npos) ? code.length() : markerEnd + 1;
    }
    
    saveCache();
    return cleanCode;
}