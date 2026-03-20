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

// [NEW] Helper for API calls with built-in retry and rate limit handling
inline string safeCallAI(const string& prompt) {
    int retries = 0;
    while (retries < MAX_RETRIES) {
        string response = extractCode(callAI(prompt));
        if (response.find("ERROR:") == 0) {
            string errorMsg = response.substr(6);
            int waitTime = 5 * (retries + 1);
            string lowerErr = errorMsg;
            transform(lowerErr.begin(), lowerErr.end(), lowerErr.begin(), ::tolower);
            
            if (lowerErr.find("rate limit") != string::npos || lowerErr.find("429") != string::npos) {
                size_t waitPos = lowerErr.find("wait ");
                if (waitPos != string::npos) {
                    try {
                        int parsedWait = stoi(lowerErr.substr(waitPos + 5));
                        if (parsedWait > 0) waitTime = parsedWait + 2;
                    } catch(...) {}
                }
                cout << "      [!] Rate limit detected. Waiting " << waitTime << "s..." << endl;
            } else {
                cout << "      [!] API Error: " << errorMsg << ". Retrying in " << waitTime << "s..." << endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(waitTime));
            retries++;
            continue;
        }
        return response;
    }
    return "ERROR: Max retries exceeded";
}

// [NEW] Helper to substitute variables ($VAR) with their content
inline string substituteVariables(const string& content, const string& currentFilePrefix = "global", bool allowContainers = false) {
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
            if (SYMBOL_TABLE.count(prefixedId) && (allowContainers || SYMBOL_TABLE[prefixedId].type != NodeType::CONTAINER)) {
                result += SYMBOL_TABLE[prefixedId].content;
                pos = scan;
            } else if (SYMBOL_TABLE.count(id) && (allowContainers || SYMBOL_TABLE[id].type != NodeType::CONTAINER)) { // Fallback for globals
                result += SYMBOL_TABLE[id].content;
                pos = scan;
            } else {
                result += "$" + id;
                pos = scan;
            }
        } else {
            // [FIX] If no valid variable name follows the $, advance the scanner to prevent an infinite loop
            result += "$";
            pos = scan;
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

    string response = safeCallAI(prompt.str());
    
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

    string response = safeCallAI(prompt.str());
    
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

    string response = safeCallAI(prompt.str());
    
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

    string response = safeCallAI(prompt.str());
    
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

    string response = safeCallAI(prompt.str());
    
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first == string::npos) return "";
    size_t last = response.find_last_not_of(" \t\r\n\"'");
    return response.substr(first, (last - first + 1));
}

// [NEW] Deep Subtraction: 2-Way Semantic Diffing for Inner Container Logic
inline string performDeepSubtraction(const string& expected_intent, const string& actual_intent) {
    stringstream prompt;
    prompt << "ROLE: Expert Software Auditor.\n";
    prompt << "TASK: Perform a deep semantic two-way subtraction between the EXPECTED specification and the ACTUAL implementation of a code container.\n";
    prompt << "EXPECTED SPECIFICATION:\n" << expected_intent << "\n\n";
    prompt << "ACTUAL IMPLEMENTATION:\n" << actual_intent << "\n\n";
    prompt << "LOGIC:\n";
    prompt << "1. EXPECTED - ACTUAL = MISSING (What is explicitly in the spec but missing from the implementation?)\n";
    prompt << "2. ACTUAL - EXPECTED = HALLUCINATED (What is in the implementation but NOT requested/implied in the spec?)\n\n";
    prompt << "OUTPUT FORMAT:\n";
    prompt << "If both are semantically equivalent (meaning no missing and no hallucinated logic), return EXACTLY the word 'NULL'.\n";
    prompt << "Otherwise, list them like this:\n";
    prompt << "[MISSING]\n- ...\n[HALLUCINATED]\n- ...\n";
    prompt << "Do not include empty sections. Return ONLY the report or 'NULL'.";

    string response = safeCallAI(prompt.str());
    
    size_t first = response.find_first_not_of(" \t\r\n\"'");
    if (first != string::npos) {
        size_t last = response.find_last_not_of(" \t\r\n\"'");
        response = response.substr(first, (last - first + 1));
    }
    if (response == "NULL" || response == "'NULL'" || response == "\"NULL\"") return "NULL";
    return response;
}

// [NEW] Semantic Subtraction: Compare Expected Specification vs Actual Implementation
inline string compareBlueprints(const string& expectedCode, const string& actualCode) {
    auto extractContainers = [](const string& code) -> map<string, string> {
        map<string, string> containers;
        YY_BUFFER_STATE buffer = yy_scan_string(code.c_str());
        yylineno = 1;
        ProgramNode* root = nullptr;
        if (yyparse(&root) == 0 && root != nullptr) {
            for (const auto& el : root->elements) {
                if (auto* contNode = dynamic_cast<ContainerNode*>(el.get())) {
                    if (!contNode->id.empty() && !contNode->isAbstract) {
                        containers[contNode->id] = contNode->intent;
                    }
                }
            }
        }
        if (buffer) yy_delete_buffer(buffer);
        if (root) delete root;
        return containers;
    };

    auto expected = extractContainers(expectedCode);
    auto actual = extractContainers(actualCode);

    cout << "   [AUDIT] Aligning container mappings semantically..." << endl;
    stringstream mapPrompt;
    mapPrompt << "ROLE: Expert Software Auditor.\n";
    mapPrompt << "TASK: Map the EXPECTED containers to their corresponding ACTUAL containers based on intent.\n";
    mapPrompt << "EXPECTED CONTAINERS:\n";
    for (const auto& [id, intent] : expected) mapPrompt << "- " << id << ": " << intent.substr(0, 100) << "...\n";
    mapPrompt << "\nACTUAL CONTAINERS:\n";
    for (const auto& [id, intent] : actual) mapPrompt << "- " << id << ": " << intent.substr(0, 100) << "...\n";
    mapPrompt << "\nRULES:\n";
    mapPrompt << "1. Match them by semantic similarity. They might have different names (e.g. 'main' vs 'MainStep').\n";
    mapPrompt << "2. If an expected container has no logical match in actual, map it to \"\" (empty string).\n";
    mapPrompt << "OUTPUT FORMAT: Return ONLY a valid JSON dictionary where keys are EXPECTED container names and values are ACTUAL container names.\n";
    mapPrompt << "Example: { \"main\": \"MainStep\", \"popup_intro\": \"PopupIntro\", \"missing\": \"\" }";

    string mapResponse = safeCallAI(mapPrompt.str());
    map<string, string> expectedToActual;
    try {
        size_t jsonStart = mapResponse.find('{');
        size_t jsonEnd = mapResponse.rfind('}');
        if (jsonStart != string::npos && jsonEnd != string::npos && jsonEnd > jsonStart) {
            mapResponse = mapResponse.substr(jsonStart, jsonEnd - jsonStart + 1);
            json j = json::parse(mapResponse);
            for (auto& el : j.items()) {
                if (el.value().is_string()) expectedToActual[el.key()] = el.value().get<string>();
            }
        }
    } catch (...) {
        cout << "   [WARN] Failed to parse semantic mapping. Falling back to strict name matching." << endl;
        for (const auto& [id, intent] : expected) {
            if (actual.count(id)) expectedToActual[id] = id;
        }
    }

    stringstream report;
    report << "=== SEMANTIC SUBTRACTION REPORT ===\n\n";

    // 1. Structural Subtraction (Missing Containers)
    vector<string> missing;
    for (const auto& [id, intent] : expected) {
        if (expectedToActual.find(id) == expectedToActual.end() || expectedToActual[id].empty() || actual.find(expectedToActual[id]) == actual.end()) {
            missing.push_back(id);
        }
    }
    if (!missing.empty()) {
        report << "[!] MISSING CONTAINERS (Spec - Code):\n";
        for (const auto& id : missing) report << "  - " << id << "\n";
        report << "\n";
    }

    // 2. Structural Subtraction (Hallucinated Containers)
    set<string> mappedActual;
    for (const auto& [e, a] : expectedToActual) if (!a.empty()) mappedActual.insert(a);
    
    vector<string> undocumented;
    for (const auto& [id, intent] : actual) {
        if (mappedActual.find(id) == mappedActual.end()) undocumented.push_back(id);
    }
    if (!undocumented.empty()) {
        report << "[?] UNDOCUMENTED/HALLUCINATED CONTAINERS (Code - Spec):\n";
        for (const auto& id : undocumented) report << "  - " << id << "\n";
        report << "\n";
    }

    // 3. Deep Subtraction (Intent Diffing)
    for (const auto& [id, expected_intent] : expected) {
        if (expectedToActual.find(id) != expectedToActual.end() && !expectedToActual[id].empty() && actual.find(expectedToActual[id]) != actual.end()) {
            string actual_id = expectedToActual[id];
            cout << "   [AUDIT] Deep Semantic Subtraction on container: " << id << " (mapped to " << actual_id << ")..." << endl;
            string deepDiff = performDeepSubtraction(expected_intent, actual[actual_id]);
            if (deepDiff != "NULL" && !deepDiff.empty() && deepDiff.find("NULL") == string::npos) {
                report << "[~] LOGIC MISMATCH IN CONTAINER: " << id << " (Code: " << actual_id << ")\n" << deepDiff << "\n\n";
            }
        }
    }
    return report.str();
}

// [UPDATED] Resolve Prompt Arithmetic (Addition & Subtraction)
inline string resolvePromptArithmetic(string expression, const string& currentFilePrefix = "global", bool allowContainers = false) {
    vector<string> terms;
    vector<char> ops;
    
    bool inQuote = false;
    bool hasOp = false;
    string current = "";
    
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
    if (!hasOp) return substituteVariables(expression, currentFilePrefix, allowContainers);

    // Process first term
    string accumulatedIntent = substituteVariables(terms[0], currentFilePrefix, allowContainers);
    // Cleanup first term
    if (accumulatedIntent.size() >= 2 && accumulatedIntent.front() == '"' && accumulatedIntent.back() == '"') {
        accumulatedIntent = accumulatedIntent.substr(1, accumulatedIntent.size() - 2);
    }
    size_t first = accumulatedIntent.find_first_not_of(" \t\r\n");
    if (first != string::npos) {
        size_t last = accumulatedIntent.find_last_not_of(" \t\r\n");
        accumulatedIntent = accumulatedIntent.substr(first, (last - first + 1));
    } else {
        accumulatedIntent = "";
    }
    
    // [NEW] Normalize Null State
    if (accumulatedIntent == "0" || accumulatedIntent == "NULL" || accumulatedIntent == "EMPTY") accumulatedIntent = "0";
    
    for (size_t i = 0; i < ops.size(); ++i) {
        char op = ops[i];
        string nextTerm = substituteVariables(terms[i+1], currentFilePrefix, allowContainers);
        
        // Cleanup next term
        if (nextTerm.size() >= 2 && nextTerm.front() == '"' && nextTerm.back() == '"') {
            nextTerm = nextTerm.substr(1, nextTerm.size() - 2);
        }
        size_t n_first = nextTerm.find_first_not_of(" \t\r\n");
        if (n_first != string::npos) {
            size_t n_last = nextTerm.find_last_not_of(" \t\r\n");
            nextTerm = nextTerm.substr(n_first, (n_last - n_first + 1));
        } else {
            nextTerm = "";
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
inline string generateIR(const string& id, const string& intent, const string& context, bool interactiveMode = false) {
    stringstream irPrompt;
    irPrompt << "ROLE: Semantic Frontend Compiler.\n";
    irPrompt << "TASK: Translate the following human intent into Glupe Intermediate Representation (GIR).\n";
    irPrompt << "RULES:\n";
    irPrompt << "1. Use strict algorithmic steps. Do not write actual target code.\n";
    irPrompt << "2. Infer and explicitly declare generic data types in the @STATE block (e.g., Int, Float, String, Bool, Byte, Any, List<T>, Map<K,V>, Future<T>).\n";
    irPrompt << "3. Restrict logic to formalized operations: ALLOC, SET, CALL, ASYNC CALL, AWAIT, RETURN, ITER, LOOP, BRANCH IF, OR IF, ELSE, TRY, CATCH, FINALLY, THROW, ASSERT.\n";
    irPrompt << "4. Resolve all ambiguity. Generate a fully deterministic pseudo-code.\n";
    irPrompt << "5. Observe the /* [GLUPE_INSERTION_POINT: " << id << "] */ in the CONTEXT. If it is inside an existing function block, set @TYPE to SNIPPET.\n";
    if (interactiveMode) {
        irPrompt << "6. INTERACTIVE MODE: If the INTENT is highly ambiguous (e.g., 'sort' but no order given), DO NOT guess. Instead, return ONLY:\nAMBIGUOUS: <Question asking the user for clarification, with numbered options>\n";
    }
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
inline string processInputWithCache(const string& code, bool useCache, const vector<string>& updateTargets, bool fillMode, bool interactiveMode = false, bool ideMode = false, const map<string, string>& interactiveAnswers = {}) {
    // [v6.0] AST-Based Compilation
    YY_BUFFER_STATE buffer = yy_scan_string(code.c_str());
    yylineno = 1; // Reset lexer line counter
    ProgramNode* root = nullptr;
    int parse_res = yyparse(&root);

    if (parse_res != 0 || root == nullptr) {
        cerr << "[FATAL ERROR] Syntax error parsing Glupe structures." << endl;
        if (buffer) yy_delete_buffer(buffer);
        if (root) delete root;
        exit(1);
    }
    
    string result;
    string currentFilePrefix = "global";
    
    for (size_t i = 0; i < root->elements.size(); ++i) {
        auto* node = root->elements[i].get();

        if (auto* rawNode = dynamic_cast<RawCodeNode*>(node)) {
            size_t marker = rawNode->code.find("// --- START FILE: ");
            if (marker != string::npos) {
                size_t fileEnd = rawNode->code.find(" ---", marker + 19);
                if (fileEnd != string::npos) {
                    currentFilePrefix = rawNode->code.substr(marker + 19, fileEnd - (marker + 19));
                    std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '/', '_');
                    std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '\\', '_');
                    std::replace(currentFilePrefix.begin(), currentFilePrefix.end(), '.', '_');
                }
            }
            result += rawNode->code;
        }
        else if (auto* varNode = dynamic_cast<VariableNode*>(node)) {
            bool isVector = false;
            vector<string> vectorElements;
            string value = varNode->value;

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
                
                value = "{ ";
                for(size_t vIdx = 0; vIdx < vectorElements.size(); ++vIdx) {
                    value += "\"" + vectorElements[vIdx] + "\"";
                    if(vIdx < vectorElements.size() - 1) value += ", ";
                }
                value += " }";
            } else {
                value = resolvePromptArithmetic(value, currentFilePrefix);
            }

            SemanticNode snode;
            if (varNode->varType == VarType::PERSISTENT) snode.type = NodeType::VAR_PERSISTENT;
            else if (varNode->varType == VarType::EPHEMERAL) snode.type = NodeType::VAR_EPHEMERAL;
            else snode.type = NodeType::CONSTANT;
            
            string varKey = currentFilePrefix + "_" + varNode->id;
            snode.id = varKey;
            snode.content = value;
            snode.isVector = isVector;
            snode.vectorContent = vectorElements;
            snode.hash = getContainerHash(value);
            
            SYMBOL_TABLE[varKey] = snode; 
            
            if (VERBOSE_MODE) {
                cout << "   [VAR] Detected " << (varNode->varType == VarType::CONSTANT ? "CONST" : "VAR") << ": " << varNode->id << " = " << value << endl;
                if (isVector) cout << "         Vector with " << vectorElements.size() << " elements." << endl;
            }
        }
        else if (auto* contNode = dynamic_cast<ContainerNode*>(node)) {
            string id = contNode->id;
            
            if (id.empty()) {
                if (contNode->isBlock) result += "$$ {\n" + contNode->intent + "\n}$$";
                else result += "$ { " + contNode->intent + " }$";
                continue;
            }

            string prompt = substituteVariables(contNode->intent, currentFilePrefix);
            string contextStr = "";
            
            if (!contNode->parents.empty()) {
                for(const auto& pid_expr : contNode->parents) {
                    string resolved_parent = resolvePromptArithmetic(pid_expr, currentFilePrefix, true);
                    string raw_pid = pid_expr;
                    if (!raw_pid.empty() && raw_pid.front() == '$') raw_pid = raw_pid.substr(1);

                    if (!resolved_parent.empty() && resolved_parent != pid_expr && resolved_parent != "$" + raw_pid) {
                        contextStr += "\n--- INHERITED INTENT ---\n" + resolved_parent + "\n";
                        cout << "   [INHERIT] Evaluated inherited intent from: " << raw_pid << endl;
                    } else {
                        string varKey = currentFilePrefix + "_" + raw_pid;
                        if (SYMBOL_TABLE.count(varKey)) {
                            contextStr += "\n--- INHERITED FROM " + raw_pid + " ---\n" + SYMBOL_TABLE[varKey].content + "\n";
                            cout << "   [INHERIT] Container '" << id << "' inherits from '" << raw_pid << "'" << endl;
                        } else if (SYMBOL_TABLE.count(raw_pid)) {
                            contextStr += "\n--- INHERITED FROM " + raw_pid + " ---\n" + SYMBOL_TABLE[raw_pid].content + "\n";
                            cout << "   [INHERIT] Container '" << id << "' inherits from '" << raw_pid << "'" << endl;
                        } else {
                            cout << "   [WARN] Parent intent '" << raw_pid << "' could not be resolved." << endl;
                        }
                    }
                }
            }

            if (!contNode->params.empty()) {
                for(const auto& param_expr : contNode->params) {
                    string resolved_param = resolvePromptArithmetic(param_expr, currentFilePrefix, true);
                    string raw_param = param_expr;
                    if (!raw_param.empty() && raw_param.front() == '$') raw_param = raw_param.substr(1);

                    if (!resolved_param.empty() && resolved_param != param_expr && resolved_param != "$" + raw_param) {
                        contextStr += "\n--- INJECTED CONTEXT ---\n" + resolved_param + "\n";
                        cout << "   [INJECT] Evaluated context from: " << raw_param << endl;
                    } else {
                        string varKey = currentFilePrefix + "_" + raw_param;
                        if (SYMBOL_TABLE.count(varKey)) {
                            contextStr += "\n--- INJECTED CONTEXT (" + raw_param + ") ---\n" + SYMBOL_TABLE[varKey].content + "\n";
                            cout << "   [INJECT] Context '" << raw_param << "' injected into '" << id << "'" << endl;
                        } else if (SYMBOL_TABLE.count(raw_param)) {
                            contextStr += "\n--- INJECTED CONTEXT (" + raw_param + ") ---\n" + SYMBOL_TABLE[raw_param].content + "\n";
                            cout << "   [INJECT] Context '" << raw_param << "' injected into '" << id << "'" << endl;
                        } else {
                            contextStr += "\n--- PARAMETER: " + raw_param + " ---\n";
                        }
                    }
                }
            }

            if (!contextStr.empty()) {
                string childLogic = prompt;
                prompt = "CONTEXT:\n" + contextStr + "\nRESOLUTION RULES:\n1. Child logic overrides parent logic.\n2. Use injected context as data/functions.\n\n--- CHILD LOGIC (" + id + ") ---\n" + childLogic;
            }

            SemanticNode sNode;
            sNode.type = NodeType::CONTAINER;
            sNode.id = id;
            sNode.content = prompt;
            sNode.parents = contNode->parents;
            sNode.params = contNode->params;
            SYMBOL_TABLE[id] = sNode;

            if (contNode->isAbstract) {
                cout << "   [ABSTRACT] Defined container: " << id << endl;
                result += "// [ABSTRACT: " + id + "]\n"; 
                continue;
            }

            string currentHash = getContainerHash(prompt);
            string cacheKey = currentFilePrefix + "_" + id;
            
            if (interactiveAnswers.count(cacheKey)) {
                prompt += "\n[USER CLARIFICATION]: " + interactiveAnswers.at(cacheKey);
                currentHash = getContainerHash(prompt); // Update hash so it matches new prompt
            }

            bool cacheHit = false;
            bool skipUpdate = false;
            if (useCache && !updateTargets.empty()) {
                bool isTarget = false;
                for(const auto& t : updateTargets) if(t == id || t == cacheKey) isTarget = true;
                if (!isTarget) skipUpdate = true;
            }

            if (useCache && (skipUpdate || LOCK_DATA["containers"].contains(cacheKey))) {
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
                else if (LOCK_DATA["containers"].contains(cacheKey)) {
                    string storedHash = LOCK_DATA["containers"][cacheKey]["hash"];
                    if (storedHash == currentHash) {
                        string content = getCachedContent(cacheKey);
                        if (!content.empty()) {
                            cout << "   [CACHE] Using cached container: " << cacheKey << endl;
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
                    cout << "   [FILL] Generating container: " << cacheKey << "..." << endl;
                    
                    string remainingCode;
                    for (size_t j = i + 1; j < root->elements.size(); ++j) {
                        if (auto* r = dynamic_cast<RawCodeNode*>(root->elements[j].get())) remainingCode += r->code;
                    }
                    string currentContext = result + "/* [GLUPE_INSERTION_POINT: " + id + "] */\n" + remainingCode;
                    
                    cout << "      -> Pass 1: Semantic Frontend (Generating GIR)..." << endl;
                    string generatedIR;
                    int irRetries = 0;
                    while (irRetries < MAX_RETRIES) {
                        generatedIR = generateIR(id, prompt, currentContext, interactiveMode);
                        
                        if (interactiveMode && generatedIR.find("AMBIGUOUS:") == 0) {
                            if (ideMode) {
                                cerr << "\n[IDE_AMBIGUOUS_PROMPT] " << cacheKey << "|" << generatedIR.substr(10) << endl;
                                exit(2);
                            } else {
                                cout << "\n[AI REQUIRES CLARIFICATION FOR '" << id << "']" << endl;
                                cout << generatedIR.substr(10) << "\n> ";
                                string answer;
                                getline(cin, answer);
                                prompt += "\n[USER CLARIFICATION]: " + answer;
                                continue; 
                            }
                        }

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
                    
                    setCachedContent(cacheKey, cleanGenerated);
                    setCachedContent(cacheKey + "_ir", generatedIR);
                    LOCK_DATA["containers"][cacheKey]["hash"] = currentHash;
                    LOCK_DATA["containers"][cacheKey]["last_run"] = time(nullptr);
                    saveCache();
                } else {
                    result += "\n// GLUPE_BLOCK_START: " + cacheKey + "\n";
                    result += prompt;
                    result += "\n// GLUPE_BLOCK_END: " + cacheKey + "\n";
                    
                    LOCK_DATA["containers"][cacheKey]["hash"] = currentHash;
                    LOCK_DATA["containers"][cacheKey]["last_run"] = time(nullptr);
                }
            }
        }
    }

    yy_delete_buffer(buffer);
    delete root;

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
    
    string cleaned = safeCallAI(prompt.str());

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