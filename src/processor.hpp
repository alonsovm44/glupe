#pragma once
#include "ai.hpp"
#include "parser.hpp"
#include "cache.hpp"
#include "config.hpp"
#include "languages.hpp"
#include <algorithm>
#include <iostream>
#include <cctype>
#include <thread>
#include <chrono>

// [NEW] Helper to substitute variables ($VAR) with their content
inline string substituteVariables(const string& content, const string& currentFilePrefix = "global") {
    string result;
    size_t pos = 0;
    while (pos < content.length()) {
        size_t dollar = content.find('$', pos);
        if (dollar == string::npos) {
            result += content.substr(pos);
            break;
        }
        
        result += content.substr(pos, dollar - pos);
        
        size_t scan = dollar + 1;
        if (scan >= content.length()) { result += "$"; pos = scan; continue; }
        
        char next = content[scan];
        // Ignore special syntax ($$, $:, ${, $ {)
        if (next == '$' || next == ':' || next == '{' || isspace(next)) {
             result += "$";
             pos = scan;
             continue;
        }

        size_t idStart = scan;
        while (scan < content.length() && (isalnum(content[scan]) || content[scan] == '_')) scan++;
        
        string id = content.substr(idStart, scan - idStart);
        
        if (!id.empty()) {
            string prefixedId = currentFilePrefix + "_" + id;
            if (SYMBOL_TABLE.count(prefixedId) && SYMBOL_TABLE[prefixedId].type != NodeType::CONTAINER) {
                result += SYMBOL_TABLE[prefixedId].content;
                pos = scan;
            } else if (SYMBOL_TABLE.count(id) && SYMBOL_TABLE[id].type != NodeType::CONTAINER) { // Fallback for globals
                result += SYMBOL_TABLE[id].content;
                pos = scan;
            } else {
                result += "$" + id;
                pos = scan;
            }
        }
    }
    return result;
}

// [NEW] Helper to check for semantic contradictions (Prompt Arithmetic Validation)
inline bool checkSemanticContradiction(const string& a, const string& b) {
    if (a.empty() || b.empty()) return false;
    if (a == b) return false; // Tautology is not a contradiction

    stringstream prompt;
    prompt << "ROLE: Logic Consistency Checker.\n";
    prompt << "TASK: Analyze the following two instruction sets for logical contradictions.\n";
    prompt << "SET A: " << a << "\n";
    prompt << "SET B: " << b << "\n";
    prompt << "DEFINITION: A contradiction occurs if Set B explicitly forbids what Set A requires, or vice versa (e.g., 'print X' vs 'do not print X').\n";
    prompt << "OUTPUT: Return ONLY the word 'CONTRADICTION' if they conflict, or 'COMPATIBLE' if they can coexist.\n";

    string response = callAI(prompt.str());
    
    string upperRes = response;
    transform(upperRes.begin(), upperRes.end(), upperRes.begin(), ::toupper);
    
    if (upperRes.find("CONTRADICTION") != string::npos) return true;
    return false;
}

// [NEW] Helper to perform semantic subtraction via AI
inline string performSemanticSubtraction(const string& base, const string& subtrahend) {
    if (subtrahend.empty()) return base;

    stringstream prompt;
    prompt << "ROLE: Semantic Logic Engine.\n";
    prompt << "TASK: Perform Prompt Subtraction (Base - Subtrahend).\n";
    prompt << "INPUTS:\n";
    prompt << "  BASE: \"" << (base.empty() ? "" : base) << "\"\n";
    prompt << "  SUBTRAHEND: \"" << subtrahend << "\"\n";
    prompt << "LOGIC:\n";
    prompt << "  1. Subset Removal: If Base contains Subtrahend, remove it.\n";
    prompt << "  2. Negative Remainder: If Subtrahend is NOT in Base, output a constraint forbidding it (e.g., 'Do not " << subtrahend << "').\n";
    prompt << "  3. Mixed: Remove shared logic, forbid extra logic.\n";
    prompt << "OUTPUT: Return ONLY the resulting prompt string. No explanations.";

    string response = callAI(prompt.str());
    
    // Clean up response
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first == string::npos) return "";
    size_t last = response.find_last_not_of(" \t\r\n\"'");
    return response.substr(first, (last - first + 1));
}

// [NEW] Helper to perform semantic multiplication via AI
inline string performSemanticMultiplication(const string& a, const string& b) {
    auto is_number = [](const string& s) {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c){ return ::isdigit(c); });
    };

    if (is_number(a)) {
        return "Repeat the following logic " + a + " times: " + b;
    }
    if (is_number(b)) {
        return "Repeat the following logic " + b + " times: " + a;
    }

    stringstream prompt;
    prompt << "ROLE: Semantic Logic Engine.\n";
    prompt << "TASK: Perform Prompt Multiplication (Action * Iterator).\n";
    prompt << "INPUTS:\n";
    prompt << "  ACTION (p): \"" << a << "\"\n";
    prompt << "  ITERATOR (q): \"" << b << "\"\n";
    prompt << "LOGIC:\n";
    prompt << "  Map the ACTION over the domain of the ITERATOR.\n";
    prompt << "  Example: 'print' * 'list items' -> 'print every item in the list'.\n";
    prompt << "OUTPUT: Return ONLY the resulting merged instruction string. No explanations.";

    string response = callAI(prompt.str());
    
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first == string::npos) return "";
    size_t last = response.find_last_not_of(" \t\r\n\"'");
    return response.substr(first, (last - first + 1));
}

// [NEW] Helper to perform semantic division/inversion via AI
inline string performSemanticDivision(const string& numerator, const string& denominator) {
    stringstream prompt;
    prompt << "ROLE: Semantic Logic Engine.\n";
    
    bool isIdentity = (numerator == "1");

    if (isIdentity) {
        prompt << "TASK: Perform Prompt Inversion (1 / Denominator).\n";
        prompt << "INPUT: \"" << denominator << "\"\n";
        prompt << "LOGIC: Generate the semantic opposite or negation of the input. (e.g., 'connect' -> 'disconnect', 'enable' -> 'disable').\n";
    } else {
        prompt << "TASK: Perform Prompt Division (Numerator / Denominator).\n";
        prompt << "INPUTS:\n";
        prompt << "  NUMERATOR: \"" << numerator << "\"\n";
        prompt << "  DENOMINATOR: \"" << denominator << "\"\n";
        prompt << "LOGIC: This represents 'Numerator' combined with the semantic inverse of 'Denominator'.\n";
    }
    prompt << "OUTPUT: Return ONLY the resulting instruction string. No explanations.";

    string response = callAI(prompt.str());
    
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first == string::npos) return "";
    size_t last = response.find_last_not_of(" \t\r\n\"'");
    return response.substr(first, (last - first + 1));
}

// [NEW] Helper to perform semantic exponentiation via AI
inline string performSemanticExponentiation(const string& base, const string& exponent) {
    auto is_number = [](const string& s) {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c){ return ::isdigit(c); });
    };

    if (is_number(exponent)) {
        return "Perform a recursive loop of " + exponent + " iterations where the output of one iteration becomes the input of the next. Logic: " + base;
    }

    stringstream prompt;
    prompt << "ROLE: Semantic Logic Engine.\n";
    prompt << "TASK: Perform Prompt Exponentiation (Base ^ Exponent).\n";
    prompt << "INPUTS:\n";
    prompt << "  BASE: \"" << base << "\"\n";
    prompt << "  EXPONENT: \"" << exponent << "\"\n";
    prompt << "LOGIC:\n";
    prompt << "  Recursive Composition: Apply the BASE logic recursively, modulated by the EXPONENT.\n";
    prompt << "  Example: 'rewrite' ^ '3 times' -> 'rewrite the text, then rewrite the result, then rewrite that result'.\n";
    prompt << "OUTPUT: Return ONLY the resulting instruction string. No explanations.";

    string response = callAI(prompt.str());
    
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first == string::npos) return "";
    size_t last = response.find_last_not_of(" \t\r\n\"'");
    return response.substr(first, (last - first + 1));
}

// [UPDATED] Resolve Prompt Arithmetic (Addition & Subtraction)
inline string resolvePromptArithmetic(string expression, const string& currentFilePrefix = "global") {
    vector<string> terms;
    vector<char> ops;
    
    string current;
    bool inQuote = false;
    bool hasOp = false;
    
    for (char c : expression) {
        if (c == '"') inQuote = !inQuote;
        
        if (!inQuote && (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')) {
            terms.push_back(current);
            ops.push_back(c);
            current = "";
            hasOp = true;
        } else {
            current += c;
        }
    }
    terms.push_back(current);

    // If no arithmetic, just substitute and return
    if (!hasOp) return substituteVariables(expression, currentFilePrefix);

    // Process first term
    string accumulatedIntent = substituteVariables(terms[0], currentFilePrefix);
    // Cleanup first term
    if (accumulatedIntent.size() >= 2 && accumulatedIntent.front() == '"' && accumulatedIntent.back() == '"') {
        accumulatedIntent = accumulatedIntent.substr(1, accumulatedIntent.size() - 2);
    }
    {
        size_t first = accumulatedIntent.find_first_not_of(" \t\r\n");
        if (first != string::npos) {
            size_t last = accumulatedIntent.find_last_not_of(" \t\r\n");
            accumulatedIntent = accumulatedIntent.substr(first, (last - first + 1));
        } else {
            accumulatedIntent = "";
        }
    }
    
    // [NEW] Normalize Null State
    if (accumulatedIntent == "0" || accumulatedIntent == "NULL" || accumulatedIntent == "EMPTY") accumulatedIntent = "0";
    
    for (size_t i = 0; i < ops.size(); ++i) {
        char op = ops[i];
        string nextTerm = substituteVariables(terms[i+1], currentFilePrefix);
        
        // Cleanup next term
        if (nextTerm.size() >= 2 && nextTerm.front() == '"' && nextTerm.back() == '"') {
            nextTerm = nextTerm.substr(1, nextTerm.size() - 2);
        }
        {
            size_t first = nextTerm.find_first_not_of(" \t\r\n");
            if (first != string::npos) {
                size_t last = nextTerm.find_last_not_of(" \t\r\n");
                nextTerm = nextTerm.substr(first, (last - first + 1));
            } else {
                nextTerm = "";
            }
        }

        // [NEW] Normalize Null State for next term
        if (nextTerm == "0" || nextTerm == "NULL" || nextTerm == "EMPTY") nextTerm = "0";

        if (nextTerm.empty()) continue;

        if (op == '+') {
            // Identity: x + 0 = x
            if (nextTerm == "0") continue;
            // Identity: 0 + x = x
            if (accumulatedIntent == "0") { accumulatedIntent = nextTerm; continue; }

            cout << "   [ARITHMETIC] Adding: (" << accumulatedIntent.substr(0, 10) << "...) + (" << nextTerm.substr(0, 10) << "...)" << endl;
            if (checkSemanticContradiction(accumulatedIntent, nextTerm)) {
                cerr << "\n[FATAL ERROR] Semantic Contradiction detected in Prompt Arithmetic!" << endl;
                cerr << "   Term 1: " << accumulatedIntent << endl;
                cerr << "   Term 2: " << nextTerm << endl;
                cerr << "   Result: Compile-Time Error (Bottom)" << endl;
                exit(1);
            }
            if (!accumulatedIntent.empty()) accumulatedIntent += "\n";
            accumulatedIntent += nextTerm;
        } else if (op == '-') {
            // Identity: x - 0 = x
            if (nextTerm == "0") continue;
            // 0 - x = -x (Inhibitor)
            if (accumulatedIntent == "0") accumulatedIntent = ""; 
            // Annihilation: x - x = 0
            if (accumulatedIntent == nextTerm) { accumulatedIntent = "0"; continue; }

            cout << "   [ARITHMETIC] Subtracting: (" << accumulatedIntent.substr(0, 10) << "...) - (" << nextTerm.substr(0, 10) << "...)" << endl;
            accumulatedIntent = performSemanticSubtraction(accumulatedIntent, nextTerm);
        } else if (op == '*') {
            // Zero Property: x * 0 = 0
            if (nextTerm == "0" || accumulatedIntent == "0") {
                accumulatedIntent = "0";
                continue; 
            }
            // Identity: x * 1 = x
            if (nextTerm == "1") continue;
            // Identity: 1 * x = x
            if (accumulatedIntent == "1") { accumulatedIntent = nextTerm; continue; }

            cout << "   [ARITHMETIC] Multiplying: (" << accumulatedIntent.substr(0, 10) << "...) * (" << nextTerm.substr(0, 10) << "...)" << endl;
            accumulatedIntent = performSemanticMultiplication(accumulatedIntent, nextTerm);
        } else if (op == '/') {
            // Identity: x / 1 = x
            if (nextTerm == "1") continue;
            // Zero Property: 0 / x = 0
            if (accumulatedIntent == "0") continue;
            // Division by Zero
            if (nextTerm == "0") {
                 cerr << "\n[FATAL ERROR] Division by Zero (Null State) in Prompt Arithmetic." << endl;
                 exit(1);
            }

            cout << "   [ARITHMETIC] Dividing: (" << accumulatedIntent.substr(0, 10) << "...) / (" << nextTerm.substr(0, 10) << "...)" << endl;
            accumulatedIntent = performSemanticDivision(accumulatedIntent, nextTerm);
        } else if (op == '^') {
            // Identity: x ^ 1 = x
            if (nextTerm == "1") continue;
            // Zero Property: x ^ 0 = 1 (Identity Element)
            if (nextTerm == "0") { accumulatedIntent = "1"; continue; }

            cout << "   [ARITHMETIC] Exponentiation: (" << accumulatedIntent.substr(0, 10) << "...) ^ (" << nextTerm.substr(0, 10) << "...)" << endl;
            accumulatedIntent = performSemanticExponentiation(accumulatedIntent, nextTerm);
        }
    }
    return accumulatedIntent;
}

// [NEW] Frontend Pass: Generate Pseudo-Code IR
inline string generateIR(const string& id, const string& intent, const string& context) {
    stringstream irPrompt;
    irPrompt << "ROLE: Semantic Frontend Compiler.\n";
    irPrompt << "TASK: Translate the following human intent into Glupe Intermediate Representation (GIR).\n";
    irPrompt << "RULES:\n";
    irPrompt << "1. Use strict algorithmic steps. Do not write actual target code.\n";
    irPrompt << "2. Infer and explicitly declare generic data types in the @STATE block (e.g., Int, Float, String, Bool, Byte, Any, List<T>, Map<K,V>, Future<T>).\n";
    irPrompt << "3. Restrict logic to formalized operations: ALLOC, SET, CALL, ASYNC CALL, AWAIT, RETURN, ITER, LOOP, BRANCH IF, OR IF, ELSE, TRY, CATCH, FINALLY, THROW, ASSERT.\n";
    irPrompt << "4. Resolve all ambiguity. Generate a fully deterministic pseudo-code.\n";
    irPrompt << "5. Observe the /* [GLUPE_INSERTION_POINT: " << id << "] */ in the CONTEXT. If it is inside an existing function block, set @TYPE to SNIPPET.\n";
    irPrompt << "GIR FORMAT:\n";
    irPrompt << "@UNIT " << id << "\n";
    irPrompt << "@TYPE <FUNCTION | CLASS | SNIPPET | GLOBAL>\n";
    irPrompt << "@SIGNATURE\n  IN: <Type> <name>, ...\n  OUT: <Type>\n";
    irPrompt << "@DEPS\n  <abstract dependencies>\n";
    irPrompt << "@STATE\n  <Type> <var>\n";
    irPrompt << "@LOGIC\n  1. <OPERATION> ...\n";
    irPrompt << "\nCONTEXT:\n" << context << "\n";
    irPrompt << "\nINTENT:\n" << intent << "\n";
    irPrompt << "OUTPUT: Return ONLY the GIR representation. No markdown blocks. No explanations.";

    return extractCode(callAI(irPrompt.str()));
}

// [NEW] Backend Pass: Generate Target Code from IR
inline string generateTargetCodeFromIR(const string& ir, const string& targetLang, const string& context, const string& id) {
    stringstream codePrompt;
    codePrompt << "ROLE: Semantic Backend Compiler.\n";
    codePrompt << "TASK: Translate the following Glupe Intermediate Representation (GIR) into strict " << targetLang << " code.\n";
    codePrompt << "RULES:\n";
    codePrompt << "1. Map the GIR steps 1:1 to " << targetLang << " idioms and best practices.\n";
    codePrompt << "2. Map generic GIR types (List, Map, Future) to native " << targetLang << " types.\n";
    codePrompt << "3. Map GIR operations (ASYNC CALL, TRY/CATCH) to native " << targetLang << " constructs.\n";
    codePrompt << "4. Do not omit any logic from the GIR.\n";
    codePrompt << "5. EXTREMELY IMPORTANT: Output ONLY the code intended to replace the /* [GLUPE_INSERTION_POINT: " << id << "] */ marker in the CONTEXT. If the insertion point is inside a function, output ONLY inner statements, NO wrapper functions, NO #includes.\n";
    codePrompt << "\nCONTEXT:\n" << context << "\n";
    codePrompt << "\nGIR:\n" << ir << "\n";
    codePrompt << "OUTPUT: Return ONLY the raw code implementation. No markdown blocks. No explanations.";

    return extractCode(callAI(codePrompt.str()));
}

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
    string currentFilePrefix = "global";
    size_t nextFileMarker = code.find("// --- START FILE: ");
    
    while (pos < code.length()) {
        size_t start = code.find("$", pos);
        if (start == string::npos) {
            result += code.substr(pos);
            break;
        }

        // Update file prefix if we passed a file marker
        while (nextFileMarker != string::npos && nextFileMarker < start) {
            size_t fileEnd = code.find(" ---", nextFileMarker + 19);
            if (fileEnd != string::npos) {
                currentFilePrefix = code.substr(nextFileMarker + 19, fileEnd - (nextFileMarker + 19));
                std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '/', '_');
                std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '\\', '_');
                std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '.', '_');
            }
            nextFileMarker = code.find("// --- START FILE: ", nextFileMarker + 19);
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
            
            // [NEW] Vector Parsing Logic
            bool isVector = false;
            vector<string> vectorElements;

            if (value.size() >= 2 && value.front() == '{' && value.back() == '}') {
                isVector = true;
                string inner = value.substr(1, value.size() - 2);
                
                string currentElement;
                bool inQuote = false;
                for (char c : inner) {
                    if (c == '"') inQuote = !inQuote;
                    
                    if (c == ',' && !inQuote) {
                        size_t first = currentElement.find_first_not_of(" \t\r\n");
                        if (first != string::npos) {
                            size_t lastEl = currentElement.find_last_not_of(" \t\r\n");
                            string el = currentElement.substr(first, lastEl - first + 1);
                            // Resolve variables in element
                            el = resolvePromptArithmetic(el, currentFilePrefix);
                            vectorElements.push_back(el);
                        }
                        currentElement = "";
                    } else {
                        currentElement += c;
                    }
                }
                size_t first = currentElement.find_first_not_of(" \t\r\n");
                if (first != string::npos) {
                    size_t lastEl = currentElement.find_last_not_of(" \t\r\n");
                    string el = currentElement.substr(first, lastEl - first + 1);
                    el = resolvePromptArithmetic(el, currentFilePrefix);
                    vectorElements.push_back(el);
                }
                
                // [FIX] Reconstruct 'value' from resolved elements so $vector expands to resolved content
                value = "{ ";
                for(size_t i=0; i<vectorElements.size(); ++i) {
                    value += "\"" + vectorElements[i] + "\"";
                    if(i < vectorElements.size() - 1) value += ", ";
                }
                value += " }";
            } else {
                // [NEW] Resolve Prompt Arithmetic (Addition & Contradiction Check)
                value = resolvePromptArithmetic(value, currentFilePrefix);
            }

            // Create SemanticNode (Placeholder for Phase 2)
            SemanticNode node;
            if (isVarPersistent) node.type = NodeType::VAR_PERSISTENT;
            else if (isVarEphemeral) node.type = NodeType::VAR_EPHEMERAL;
            else node.type = NodeType::CONSTANT;
            
            string varKey = currentFilePrefix + "_" + id;
            node.id = varKey;
            node.content = value;
            node.isVector = isVector;
            node.vectorContent = vectorElements;
            node.hash = getContainerHash(value);
            
            SYMBOL_TABLE[varKey] = node; // [NEW] Store in symbol table
            
            if (VERBOSE_MODE) {
                cout << "   [VAR] Detected " << (isConstant ? "CONST" : "VAR") << ": " << id << " = " << value << endl;
                if (isVector) cout << "         Vector with " << vectorElements.size() << " elements." << endl;
            }

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
            
            // [NEW] Variable Substitution ($VAR -> value)
            prompt = substituteVariables(prompt, currentFilePrefix);
            
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
            string cacheKey = currentFilePrefix + "_" + id; // [NEW] Prefix container ID to avoid cache collisions
            
            result += code.substr(pos, start - pos); // Append text before container

            bool cacheHit = false;
            
            // Check if we should skip this container (Selective Update)
            bool skipUpdate = false;
            if (useCache && !updateTargets.empty()) {
                bool isTarget = false;
                for(const auto& t : updateTargets) if(t == id || t == cacheKey) isTarget = true;
                if (!isTarget) skipUpdate = true;
            }

            if (useCache && (skipUpdate || LOCK_DATA["containers"].contains(cacheKey))) {
                // If skipping, ignore hash check and try to load cache immediately
                if (skipUpdate) {
                    string content = getCachedContent(cacheKey);
                    if (!content.empty()) {
                        cout << "   [SKIP] Keeping container: " << cacheKey << endl;
                        result += "\n// GLUPE_BLOCK_START: " + cacheKey + "\n";
                        result += content; 
                        result += "\n// GLUPE_BLOCK_END: " + cacheKey + "\n";
                        cacheHit = true;
                    } else {
                        cout << "   [WARN] Cache missing for skipped container: " << cacheKey << ". Regenerating." << endl;
                    }
                }
                // Standard check: hash comparison
                else if (LOCK_DATA["containers"].contains(cacheKey)) {
                string storedHash = LOCK_DATA["containers"][cacheKey]["hash"];
                if (storedHash == currentHash) {
                    string content = getCachedContent(cacheKey);
                    if (!content.empty()) {
                        cout << "   [CACHE] Using cached container: " << cacheKey << endl;
                        // [FIX] Wrap cached content in markers so AI preserves it
                        result += "\n// GLUPE_BLOCK_START: " + cacheKey + "\n";
                        result += content; 
                        result += "\n// GLUPE_BLOCK_END: " + cacheKey + "\n";
                        cacheHit = true;
                    }
                    }
                }
            }

            if (!cacheHit) {
                if (fillMode) {
                    // [FILL MODE] Generate immediately to preserve surrounding code
                    cout << "   [FILL] Generating container: " << cacheKey << "..." << endl;
                    
                    // Construct context from what we have processed so far + what remains
                    // This gives the AI the full file view without being able to touch it
                    string currentContext = result + "/* [GLUPE_INSERTION_POINT: " + id + "] */\n" + code.substr(pos);
                    
                    cout << "      -> Pass 1: Semantic Frontend (Generating GIR)..." << endl;
                    string generatedIR;
                    int irRetries = 0;
                    while (irRetries < MAX_RETRIES) {
                        generatedIR = generateIR(id, prompt, currentContext);
                        if (generatedIR.find("ERROR:") == 0) {
                            cout << "   [!] API Error on GIR (Attempt " << (irRetries + 1) << "/" << MAX_RETRIES << "): " << generatedIR.substr(6) << endl;
                            int waitTime = 5 * (irRetries + 1);
                            if (generatedIR.find("Rate limit") != string::npos || generatedIR.find("rate limit") != string::npos || generatedIR.find("429") != string::npos) {
                                size_t waitPos = generatedIR.find("wait ");
                                if (waitPos != string::npos) {
                                    try {
                                        int parsedWait = stoi(generatedIR.substr(waitPos + 5));
                                        if (parsedWait > 0) waitTime = parsedWait + 2;
                                    } catch(...) {}
                                }
                                cout << "       -> Rate limit detected. Waiting " << waitTime << "s..." << endl;
                            } else cout << "       -> Retrying in " << waitTime << "s..." << endl;
                            std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                            irRetries++;
                        } else {
                            break;
                        }
                    }
                    
                    if (SYMBOL_TABLE.count(id)) {
                        SYMBOL_TABLE[id].ir_content = generatedIR;
                        SYMBOL_TABLE[id].is_resolved_to_ir = true;
                    }

                    cout << "      -> Pass 2: Semantic Backend (Generating " << CURRENT_LANG.name << ")..." << endl;
                    string cleanGenerated;
                    int codeRetries = 0;
                    while (codeRetries < MAX_RETRIES) {
                        cleanGenerated = generateTargetCodeFromIR(generatedIR, CURRENT_LANG.name, currentContext, id);
                        if (cleanGenerated.find("ERROR:") == 0) {
                            cout << "   [!] API Error on Target Code (Attempt " << (codeRetries + 1) << "/" << MAX_RETRIES << "): " << cleanGenerated.substr(6) << endl;
                            int waitTime = 5 * (codeRetries + 1);
                            if (cleanGenerated.find("Rate limit") != string::npos || cleanGenerated.find("rate limit") != string::npos || cleanGenerated.find("429") != string::npos) {
                                size_t waitPos = cleanGenerated.find("wait ");
                                if (waitPos != string::npos) {
                                    try {
                                        int parsedWait = stoi(cleanGenerated.substr(waitPos + 5));
                                        if (parsedWait > 0) waitTime = parsedWait + 2;
                                    } catch(...) {}
                                }
                                cout << "       -> Rate limit detected. Waiting " << waitTime << "s..." << endl;
                            } else cout << "       -> Retrying in " << waitTime << "s..." << endl;
                            std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                            codeRetries++;
                        } else {
                            break;
                        }
                    }
                    
                    result += "\n// GLUPE_BLOCK_START: " + cacheKey + "\n";
                    result += cleanGenerated;
                    result += "\n// GLUPE_BLOCK_END: " + cacheKey + "\n";
                    
                    // Update cache immediately
                    setCachedContent(cacheKey, cleanGenerated);
                    setCachedContent(cacheKey + "_ir", generatedIR); // Cache the IR
                    LOCK_DATA["containers"][cacheKey]["hash"] = currentHash;
                    LOCK_DATA["containers"][cacheKey]["last_run"] = time(nullptr);
                    saveCache();
                } else {
                    // [STANDARD MODE] Wrap in markers for global pass
                    result += "\n// GLUPE_BLOCK_START: " + cacheKey + "\n";
                    result += prompt; // The prompt for the AI
                    result += "\n// GLUPE_BLOCK_END: " + cacheKey + "\n";
                    
                    // Update lock data (will be saved after successful generation)
                    LOCK_DATA["containers"][cacheKey]["hash"] = currentHash;
                    LOCK_DATA["containers"][cacheKey]["last_run"] = time(nullptr);
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
        
        string cacheKey = code.substr(idStart, idEnd - idStart);
        // Trim cacheKey
        cacheKey.erase(0, cacheKey.find_first_not_of(" \t\r"));
        cacheKey.erase(cacheKey.find_last_not_of(" \t\r") + 1);

        size_t blockEnd = code.find("// GLUPE_BLOCK_END: " + cacheKey, idEnd);
        if (blockEnd == string::npos) {
            // Marker broken by AI, just keep going
            cleanCode += code.substr(start); 
            break;
        }

        // Extract content
        string content = code.substr(idEnd + 1, blockEnd - (idEnd + 1));
        
        // Save to cache
        setCachedContent(cacheKey, content);
        cout << "   [CACHE] Updated container: " << cacheKey << endl;

        cleanCode += content; // Keep content in final file
        
        // Skip end marker
        size_t markerEnd = code.find('\n', blockEnd);
        pos = (markerEnd == string::npos) ? code.length() : markerEnd + 1;
    }
    
    saveCache();
    return cleanCode;
}