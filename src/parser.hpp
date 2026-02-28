#pragma once
#include "utils.hpp"
#include "languages.hpp"
#include "cache.hpp"

// [NEW] Pre-processor to extract glupe syntax from comments
inline string decommentGlupeSyntax(const string& code) {
    string processedCode = code;
    size_t pos = 0;

    // --- Handle block comments: /* ... */ ---
    pos = 0;
    while ((pos = processedCode.find("/*", pos)) != string::npos) {
        size_t endPos = processedCode.find("*/", pos + 2);
        if (endPos == string::npos) break; // Unclosed comment, stop processing

        string commentContent = processedCode.substr(pos + 2, endPos - (pos + 2));
        
        string trimmedContent = commentContent;
        size_t first = trimmedContent.find_first_not_of(" \t\r\n");
        if (first == string::npos) { // Empty or whitespace-only comment
            pos = endPos + 2;
            continue;
        }
        trimmedContent.erase(0, first);
        size_t last = trimmedContent.find_last_not_of(" \t\r\n");
        if (last != string::npos) trimmedContent.erase(last + 1);

        bool isBlockContainer = (trimmedContent.rfind("$$", 0) == 0 && trimmedContent.rfind("$$") == trimmedContent.length() - 2);
        bool isInlineContainer = (trimmedContent.rfind("$", 0) == 0 && trimmedContent.rfind("$") == trimmedContent.length() - 1);

        if (isBlockContainer || isInlineContainer) {
            processedCode.replace(pos, (endPos + 2) - pos, commentContent);
            pos += commentContent.length();
        } else {
            pos = endPos + 2;
        }
    }

    // --- Handle line comments: // ... ---
    pos = 0;
    while ((pos = processedCode.find("//", pos)) != string::npos) {
        size_t endOfLine = processedCode.find('\n', pos);
        if (endOfLine == string::npos) endOfLine = processedCode.length();

        string commentContent = processedCode.substr(pos + 2, endOfLine - (pos + 2));
        
        string trimmedContent = commentContent;
        size_t first = trimmedContent.find_first_not_of(" \t\r\n");
        if (first == string::npos) { // Empty or whitespace-only comment
            pos = endOfLine;
            if (pos >= processedCode.length()) break;
            continue;
        }
        trimmedContent.erase(0, first);
        size_t last = trimmedContent.find_last_not_of(" \t\r\n");
        if (last != string::npos) trimmedContent.erase(last + 1);

        bool isInlineContainer = (trimmedContent.rfind("$", 0) == 0 && trimmedContent.rfind("$") == trimmedContent.length() - 1);

        if (isInlineContainer) {
            processedCode.replace(pos, endOfLine - pos, commentContent);
            pos += commentContent.length();
        } else {
            pos = endOfLine;
            if (pos >= processedCode.length()) break;
        }
    }
    return processedCode;
}

inline string resolveImports(string code, fs::path basePath, vector<string>& stack) {
    stringstream ss(code);
    string line;
    vector<string> lines;
    while (getline(ss, line)) lines.push_back(line);

    string processed;
    
    for (size_t i = 0; i < lines.size(); ++i) {
        string cleanLine = lines[i];
        size_t first = cleanLine.find_first_not_of(" \t\r\n");
        if (first == string::npos) { processed += lines[i] + "\n"; continue; }
        cleanLine.erase(0, first);
        
        // Ignore orphaned IMPORT: END lines (they should be consumed by blocks)
        if (cleanLine == "IMPORT: END") continue;

        if (cleanLine.rfind("IMPORT:", 0) == 0) {
            string directiveContent = cleanLine.substr(7);
            string fname;
            
            // Robust filename extraction
            size_t q1 = directiveContent.find_first_of("\"'");
            if (q1 != string::npos) {
                char quote = directiveContent[q1];
                size_t q2 = directiveContent.find(quote, q1 + 1);
                if (q2 != string::npos) fname = directiveContent.substr(q1 + 1, q2 - q1 - 1);
                else fname = directiveContent.substr(q1 + 1);
            } else {
                fname = directiveContent;
                size_t start = fname.find_first_not_of(" \t\r\n");
                if (start != string::npos) fname = fname.substr(start);
                size_t end = fname.find_last_not_of(" \t\r\n");
                if (end != string::npos) fname = fname.substr(0, end + 1);
            }

            // Look ahead for block content
            string localModifications;
            int blockEndIndex = -1;
            
            for (size_t j = i + 1; j < lines.size(); ++j) {
                string nextClean = lines[j];
                size_t nf = nextClean.find_first_not_of(" \t\r\n");
                if (nf != string::npos) nextClean.erase(0, nf);
                else nextClean = "";

                if (nextClean == "IMPORT: END") {
                    blockEndIndex = j;
                    break;
                }
                if (nextClean.rfind("IMPORT:", 0) == 0) {
                    break; // Another import started, so previous was single line
                }
            }

            if (blockEndIndex != -1) {
                for (size_t k = i + 1; k < blockEndIndex; ++k) {
                    localModifications += lines[k] + "\n";
                }
                i = blockEndIndex; // Skip consumed lines
            }

            fs::path path = basePath / fname;
            try {
                if (fs::exists(path)) {
                    string absPath = fs::canonical(path).string();
                    bool cycle = false;
                    for(const auto& s : stack) if(s == absPath) cycle = true;
                    if (cycle) {
                        processed += "// [ERROR] CYCLIC IMPORT DETECTED: " + fname + "\n";
                        log("ERROR", "Circular import: " + fname);
                    } else {
                        ifstream imp(path);
                        if (imp.is_open()) {
                            string content((istreambuf_iterator<char>(imp)), istreambuf_iterator<char>());
                            stack.push_back(absPath);
                            string nested = resolveImports(content, path.parent_path(), stack);
                            stack.pop_back();
                            
                            processed += "\n// --- IMPORTED FILE: " + fname + " ---\n";
                            processed += nested;
                            if (!localModifications.empty()) {
                                processed += "// --- LOCAL MODIFICATIONS ---\n";
                                processed += localModifications;
                            }
                            processed += "// --- END IMPORT ---\n";
                            log("INFO", "Imported module: " + fname);
                        }
                    }
                } else {
                    processed += "// [WARN] IMPORT NOT FOUND: " + fname + "\n";
                }
            } catch (...) { processed += "// [ERROR] PATH EXCEPTION\n"; }
        } else {
            processed += lines[i] + "\n";
        }
    }
    return processed;
}

// [NEW] Helper to strip AI templates ($${...}$$) from code lines
inline string stripTemplates(const string& line, bool& insideTemplate) {
    string result;
    size_t pos = 0;

    if (insideTemplate) {
        size_t end = line.find("}$$");
        if (end != string::npos) {
            pos = end + 3;
            insideTemplate = false;
        } else {
            return "";
        }
    }

    while (pos < line.length()) {
        size_t start = line.find("$", pos);
        if (start == string::npos) {
            result += line.substr(pos);
            break;
        }
        
        bool isBlock = false;
        bool isInline = false;
        size_t scan = 0;

        if (start + 1 < line.length() && line[start+1] == '$') {
            isBlock = true;
            scan = start + 2;
        } else {
            // Inline check
            size_t openB = line.find('{', start);
            if (openB != string::npos) {
                size_t searchPos = openB + 1;
                while (searchPos < line.length()) {
                    size_t closeB = line.find('}', searchPos);
                    if (closeB == string::npos) break;
                    size_t check = closeB + 1;
                    while (check < line.length() && isspace(line[check])) check++;
                    if (check < line.length() && line[check] == '$') {
                        isInline = true;
                        scan = start + 1;
                        break;
                    }
                    searchPos = closeB + 1;
                }
            }
        }

        if (!isBlock && !isInline) {
            result += line.substr(pos, start - pos + 1);
            pos = start + 1;
            continue;
        }

        bool isContainer = false;
        size_t contentStart = 0;

        // Check anonymous $${
        if (start + 2 < line.length() && line[start+2] == '{') {
            isContainer = true;
            contentStart = start + 3;
        } 
        // Check named $$ "id" {
        else {
            while(scan < line.length() && isspace(line[scan])) scan++;
            
            // [NEW] Skip ABSTRACT keyword in stripper
            if (scan + 8 <= line.length() && line.compare(scan, 8, "ABSTRACT") == 0 && (scan + 8 == line.length() || isspace(line[scan+8]))) {
                scan += 8;
                while(scan < line.length() && isspace(line[scan])) scan++;
            }

            if (scan < line.length() && line[scan] != '{') {
                while(scan < line.length() && !isspace(line[scan]) && line[scan] != '{' && !(line[scan] == '-' && scan+1 < line.length() && line[scan+1] == '>')) {
                    scan++;
                }
                
                size_t brace = scan;
                while(brace < line.length() && isspace(line[brace])) brace++;
                
                // [NEW] Skip inheritance syntax in stripper
                if (brace + 1 < line.length() && line[brace] == '-' && line[brace+1] == '>') {
                     size_t pScan = brace + 2;
                     while(pScan < line.length() && line[pScan] != '{') pScan++; // Skip until {
                     brace = pScan;
                }

                while(brace < line.length() && isspace(line[brace])) brace++;
                if (brace < line.length() && line[brace] == '{') {
                    isContainer = true;
                    contentStart = brace + 1;
                }
            }
        }

        if (isContainer) {
            if (start > pos) {
                result += line.substr(pos, start - pos);
            }
            size_t end = string::npos;
            size_t nextPos = 0;
            if (isBlock) {
                end = line.find("}$$", contentStart);
                if (end != string::npos) nextPos = end + 3;
            } else {
                size_t searchPos = contentStart;
                while (searchPos < line.length()) {
                    size_t closeB = line.find('}', searchPos);
                    if (closeB == string::npos) break;
                    size_t check = closeB + 1;
                    while (check < line.length() && isspace(line[check])) check++;
                    if (check < line.length() && line[check] == '$') {
                        end = closeB;
                        nextPos = check + 1;
                        break;
                    }
                    searchPos = closeB + 1;
                }
            }

            if (end == string::npos) {
                insideTemplate = true;
                break;
            }
            pos = nextPos;
        } else {
            // Not a container, keep $$
            result += line.substr(pos, start - pos + (isBlock ? 2 : 1));
            pos = start + (isBlock ? 2 : 1);
        }
    }
    return result;
}

// [NEW] Validate container names and detect collisions
inline bool validateContainers(const string& code, bool* outHasActive = nullptr) {
    set<string> ids;
    size_t pos = 0;
    while ((pos = code.find("$", pos)) != string::npos) {
        bool isBlock = false;
        bool isInline = false;
        size_t scan = 0;

        if (pos + 1 < code.length() && code[pos+1] == '$') {
            isBlock = true;
            scan = pos + 2;
        } else {
            size_t lineEnd = code.find('\n', pos);
            if (lineEnd == string::npos) lineEnd = code.length();
            size_t openB = code.find('{', pos);
            if (openB != string::npos && openB < lineEnd) {
                size_t searchPos = openB + 1;
                while (searchPos < lineEnd) {
                    size_t closeB = code.find('}', searchPos);
                    if (closeB == string::npos || closeB >= lineEnd) break;
                    size_t check = closeB + 1;
                    while (check < lineEnd && isspace(code[check])) check++;
                    if (check < lineEnd && code[check] == '$') {
                        isInline = true;
                        scan = pos + 1;
                        break;
                    }
                    searchPos = closeB + 1;
                }
            }
        }

        if (!isBlock && !isInline) { 
            // [NEW] Check for malformed inline container (multiline)
            size_t lineEnd = code.find('\n', pos);
            if (lineEnd == string::npos) lineEnd = code.length();
            
            size_t check = pos + 1;
            while(check < lineEnd && isspace(code[check])) check++;
            
            bool isHeader = false;
            size_t bracePos = string::npos;

            // Case: $ {
            if (check < lineEnd && code[check] == '{') {
                isHeader = true;
                bracePos = check;
            } 
            // Case: $ ID ... or $ -> ...
            else {
                size_t idStart = check;
                while(check < lineEnd && (isalnum(static_cast<unsigned char>(code[check])) || code[check] == '_')) check++;
                
                // If we advanced (found ID) or didn't (maybe just ->), check next
                while(check < lineEnd && isspace(code[check])) check++;
                
                if (check < lineEnd && code[check] == '{') {
                    isHeader = true; // $ ID {
                    bracePos = check;
                } else if (check + 1 < lineEnd && code[check] == '-' && code[check+1] == '>') {
                    // $ ... -> ...
                    check += 2;
                    while(check < lineEnd && code[check] != '{') check++; // Skip parents
                    if (check < lineEnd && code[check] == '{') {
                        isHeader = true;
                        bracePos = check;
                    }
                }
            }

            if (isHeader) {
                // If it looks like a container start but !isInline, check if it lacks closing brace on same line
                size_t closeB = code.find('}', bracePos);
                bool hasClosingBrace = (closeB != string::npos && closeB < lineEnd);
                
                if (!hasClosingBrace) {
                     int lineNum = 1;
                     for(size_t i=0; i<pos; ++i) if(code[i] == '\n') lineNum++;
                     cerr << "[ERROR] Malformed inline container at line " << lineNum << ".\n        Inline containers ($ ... $) must be closed on the same line.\n        Use block containers ($$ ... $$) for multi-line logic.\n        Context: " << code.substr(pos, min((size_t)50, lineEnd - pos)) << "..." << endl;
                     return false;
                }
            }
            pos++; continue; 
        }

        // Check anonymous $${ (skip)
        if (scan < code.length() && code[scan] == '{') {
            if (outHasActive) *outHasActive = true;
            pos = scan + 1; continue;
        }
        // Check named $$ "id" {
        while(scan < code.length() && isspace(code[scan])) scan++;
        
        // [NEW] Skip ABSTRACT keyword in validator
        bool isAbstract = false;
        if (scan + 8 <= code.length() && code.compare(scan, 8, "ABSTRACT") == 0 && (scan + 8 == code.length() || isspace(code[scan+8]))) {
            isAbstract = true;
            scan += 8;
            while(scan < code.length() && isspace(code[scan])) scan++;
        }

        if (scan < code.length() && code[scan] != '{') {
            size_t idStart = scan;
            while(scan < code.length() && !isspace(code[scan]) && code[scan] != '{' && !(code[scan] == '-' && scan+1 < code.length() && code[scan+1] == '>')) {
                scan++;
            }
            
            if (scan > idStart) {
                string id = code.substr(idStart, scan - idStart);
                size_t brace = scan;
                
                // [NEW] Skip inheritance syntax in validator
                while(brace < code.length() && isspace(code[brace])) brace++;
                if (brace + 1 < code.length() && code[brace] == '-' && code[brace+1] == '>') {
                     size_t pScan = brace + 2;
                     while(pScan < code.length() && code[pScan] != '{') pScan++; // Skip until {
                     brace = pScan;
                }

                while(brace < code.length() && isspace(code[brace])) brace++;
                if (brace < code.length() && code[brace] == '{') {
                    if (ids.count(id)) {
                        cerr << "[ERROR] Duplicate container ID found: \"" << id << "\"" << endl;
                        return false;
                    }
                    ids.insert(id);
                    if (outHasActive && !isAbstract) *outHasActive = true;
                    
                    // [FIX] Check for closing tag and skip content to avoid false positives inside prompts
                    size_t end = string::npos;
                    size_t nextPos = 0;
                    if (isBlock) {
                        end = code.find("}$$", brace + 1);
                        if (end != string::npos) nextPos = end + 3;
                    } else {
                        size_t searchPos = brace + 1;
                        while (searchPos < code.length()) {
                            size_t closeB = code.find('}', searchPos);
                            if (closeB == string::npos) break;
                            size_t check = closeB + 1;
                            while (check < code.length() && isspace(code[check])) check++;
                            if (check < code.length() && code[check] == '$') {
                                end = closeB; nextPos = check + 1; break;
                            }
                            searchPos = closeB + 1;
                        }
                    }

                    if (end == string::npos) {
                        cerr << "[ERROR] Unclosed container: \"" << id << "\"" << endl;
                        return false;
                    }
                    pos = nextPos;
                    continue;
                }
            }
        }
        pos = scan;
    }
    return true;
}

// --- EXPORT SYSTEM ---
inline string processExports(const string& code, const fs::path& basePath) {
    stringstream ss(code);
    string line;
    unique_ptr<ofstream> outFile;
    string remaining;
    bool exportError = false;
    bool insideTemplate = false; // [FIX] Track template blocks
    
    while (getline(ss, line)) {
        string cleanLine = line;
        size_t first = cleanLine.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            if (outFile && outFile->is_open()) *outFile << "\n";
            else if (!exportError) remaining += line + "\n";
            continue; 
        }
        cleanLine.erase(0, first);
        
        if (cleanLine.rfind("EXPORT:", 0) == 0) {
            outFile.reset(); // Cerrar archivo anterior siempre
            exportError = false; // Resetear estado de error
            insideTemplate = false; // [FIX] Reset template state
            
            string rawArgs = cleanLine.substr(7);
            string fname;
            string sameLineCode;

            // [FIX] Robust filename parsing: Find FIRST pair of quotes, not last
            size_t q1 = rawArgs.find_first_of("\"'");
            if (q1 != string::npos) {
                char quote = rawArgs[q1];
                size_t q2 = rawArgs.find(quote, q1 + 1); 
                if (q2 != string::npos) {
                    fname = rawArgs.substr(q1 + 1, q2 - q1 - 1);
                    // Capture content after the filename (e.g. code generated on same line)
                    if (q2 + 1 < rawArgs.length()) {
                        sameLineCode = rawArgs.substr(q2 + 1);
                    }
                } else {
                    fname = rawArgs.substr(q1 + 1); // Unmatched quote fallback
                }
            } else {
                // No quotes, take first word
                stringstream fss(rawArgs);
                fss >> fname;
                // Capture remainder
                size_t fPos = rawArgs.find(fname);
                if (fPos != string::npos) {
                    size_t afterFname = fPos + fname.length();
                    if (afterFname < rawArgs.length()) sameLineCode = rawArgs.substr(afterFname);
                }
            }
            
            // Trim fname
            fname.erase(0, fname.find_first_not_of(" \t\r\n"));
            size_t last = fname.find_last_not_of(" \t\r\n");
            if (last != string::npos) fname.erase(last + 1);

            if (fname.empty() || fname == "END") {
                continue;
            }

            fs::path path = basePath / fname;
            try {
                if (path.has_parent_path()) {
                    if (!fs::exists(path.parent_path())) cout << "[EXPORT] Creating directory: " << path.parent_path().string() << endl;
                    fs::create_directories(path.parent_path());
                }
                outFile = make_unique<ofstream>(path);
                if (outFile->is_open()) {
                    cout << "[EXPORT] Writing to " << fname << "..." << endl;
                    // Write content found on the same line (if any non-whitespace)
                    if (!sameLineCode.empty() && sameLineCode.find_first_not_of(" \t\r\n") != string::npos) {
                        *outFile << sameLineCode << "\n";
                    }
                } else {
                    cerr << "[ERROR] Could not open " << fname << " for writing." << endl;
                    outFile.reset();
                    exportError = true; // Activar modo "sumidero"
                }
            } catch (const fs::filesystem_error& e) {
                cerr << "[ERROR] Filesystem error: " << e.what() << endl;
                outFile.reset();
                exportError = true; // Activar modo "sumidero"
            }
        } else {
            if (outFile && outFile->is_open()) {
                // [FIX] Robust template handling via helper
                string cleanContent = stripTemplates(line, insideTemplate);
                if (!cleanContent.empty()) {
                    *outFile << cleanContent << "\n";
                }
            } else if (!exportError) {
                remaining += line + "\n";
            }
        }
    }
    return remaining;
}

inline set<string> extractDependencies(const string& code) {
    set<string> deps;
    stringstream ss(code);
    string line;
    while(getline(ss, line)) {
        size_t warnPos = line.find("// [WARN] IMPORT NOT FOUND: ");
        if (warnPos != string::npos) {
            deps.insert(line.substr(warnPos + 28)); 
        }
        size_t incPos = line.find("#include");
        if (incPos != string::npos) {
            size_t startQuote = line.find_first_of("\"<", incPos);
            size_t endQuote = line.find_first_of("\">", startQuote + 1);
            if (startQuote != string::npos && endQuote != string::npos) {
                deps.insert(line.substr(startQuote + 1, endQuote - startQuote - 1));
            }
        }
    }
    return deps;
}

inline bool preFlightCheck(const set<string>& deps) {
    if (deps.empty()) return true;
    // If no check command is defined and it's not C/C++, skip check
    if (CURRENT_LANG.checkCmd.empty() && CURRENT_LANG.id != "cpp" && CURRENT_LANG.id != "c") return true; 

    cout << "[CHECK] Verifying dependencies locally..." << endl;
    string tempCheck = "temp_dep_check" + CURRENT_LANG.extension;
    ofstream out(tempCheck);
    
    if (CURRENT_LANG.id == "cpp" || CURRENT_LANG.id == "c") {
        for(const auto& d : deps) {
            if (d.find(".h") != string::npos || d.find("/") != string::npos) out << "#include \"" << d << "\"\n";
            else out << "#include <" << d << ">\n"; 
        }
        out << "int main() { return 0; }\n";
    } else if (CURRENT_LANG.id == "py") {
        for(const auto& d : deps) {
            out << "import " << d << "\n";
        }
    } // pending for the rest of languages - for now we only do strict checks for C/C++ and Python
    out.close();
    
    string cmd;
    if (!CURRENT_LANG.checkCmd.empty()) {
        cmd = CURRENT_LANG.checkCmd + " \"" + tempCheck + "\"";
    } else {
        cmd = CURRENT_LANG.buildCmd + " -c \"" + tempCheck + "\""; 
    }
    CmdResult res = execCmd(cmd);
    
    fs::remove(tempCheck);
    if (fs::exists(stripExt(tempCheck) + ".o")) fs::remove(stripExt(tempCheck) + ".o");
    if (fs::exists(stripExt(tempCheck) + ".obj")) fs::remove(stripExt(tempCheck) + ".obj");

    if (res.exitCode != 0) {
        cout << "   [!] Missing Dependency Detected!" << endl;
        for(const auto& d : deps) {
            if (res.output.find(d) != string::npos) {
                cout << "       -> " << d << " not found." << endl;
            }
        }
        // Note: isFatalError and explainFatalError are in utils.hpp and ai.hpp respectively.
        // We can't call explainFatalError here easily without circular dependency if ai.hpp includes parser.hpp.
        // For now, we just log. The main loop handles explainFatalError.
        if (isFatalError(res.output)) {
             cout << "   [ERROR LOG]:\n" << res.output.substr(0, 300) << endl;
        }
        return false;
    }
    cout << "   [OK] Dependencies verified." << endl;
    return true;
}

// [NEW] Helper to split code into semantic chunks for Refine Mode
inline vector<string> splitSourceCode(const string& code, int targetLines = 500) {
    vector<string> chunks;
    stringstream ss(code);
    string line;
    string currentChunk;
    int lineCount = 0;
    int braceBalance = 0;
    
    while (getline(ss, line)) {
        currentChunk += line + "\n";
        lineCount++;
        
        // Simple brace counting (ignoring full line comments for basic robustness)
        string trimmed = line;
        size_t start = trimmed.find_first_not_of(" \t");
        if (start != string::npos) trimmed = trimmed.substr(start);
        
        if (trimmed.find("//") != 0 && trimmed.find("#") != 0) {
            for (char c : line) {
                if (c == '{') braceBalance++;
                else if (c == '}') braceBalance--;
            }
        }
        
        bool safeToSplit = false;
        // Heuristic: Split if balance is 0 (or less) and line is empty or starts at column 0
        if (braceBalance <= 0) {
            if (trimmed.empty()) safeToSplit = true; 
            else if (line.find_first_not_of(" \t") == 0) safeToSplit = true; // Top level
        }

        if ((lineCount >= targetLines && safeToSplit) || lineCount >= targetLines + 200) {
            chunks.push_back(currentChunk);
            currentChunk = "";
            lineCount = 0;
            braceBalance = 0; 
        }
    }
    if (!currentChunk.empty()) chunks.push_back(currentChunk);
    return chunks;
}

// [NEW] Helper to extract signatures for context
inline string extractSignatures(const string& code) {
    stringstream ss(code);
    string line;
    string result;
    while (getline(ss, line)) {
        string trimmed = line;
        size_t first = trimmed.find_first_not_of(" \t");
        if (first == string::npos) continue;
        trimmed = trimmed.substr(first);

        if (trimmed.find("//") == 0) continue; 
        
        if (trimmed[0] == '#' || trimmed.find("using ") == 0 || trimmed.find("typedef ") == 0) {
            result += line + "\n";
            continue;
        }
        
        if (trimmed.find("class ") == 0 || trimmed.find("struct ") == 0 || trimmed.find("enum ") == 0 ||
            (trimmed.find('(') != string::npos && trimmed.find(')') != string::npos)) {
            result += line + "\n";
        }
    }
    return result;
}

// [NEW] Metadata System for GlupeHub
inline string stripMetadata(const string& code) {
    size_t start = code.find("META_START");
    size_t end = code.find("META_END");
    if (start != string::npos && end != string::npos && end > start) {
        return code.substr(0, start) + code.substr(end + 8);
    }
    return code;
}

inline void showMetadata(const string& filename) {
    if (!fs::exists(filename)) {
        cout << "[ERROR] File not found: " << filename << endl;
        return;
    }
    ifstream f(filename);
    string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    f.close();

    size_t start = content.find("META_START");
    size_t end = content.find("META_END");

    json meta;
    bool found = false;

    if (start != string::npos && end != string::npos && end > start) {
        string jsonStr = content.substr(start + 10, end - (start + 10));
        try {
            meta = json::parse(jsonStr);
            found = true;
        } catch (exception& e) {
            cout << "[ERROR] Invalid JSON in META block: " << e.what() << endl;
            return;
        }
    } else {
        meta["name"] = fs::path(filename).stem().string();
        meta["inferred"] = true;
    }

    set<string> deps = extractDependencies(content);
    if (!deps.empty()) meta["dependencies_detected"] = deps;

    cout << "\n--- GLUPE METADATA: " << filename << " ---\n";
    cout << meta.dump(4) << endl;
    cout << "-----------------------------------\n";
}

// [NEW] Helper to sanitize container syntax after refinement
inline string sanitize_container_syntax(const string& code) {
    string out = code;

    // 1. Fix malformed starts: "${" -> "$ {"
    size_t pos = 0;
    while ((pos = out.find("${", pos)) != string::npos) {
        out.replace(pos, 2, "$ {");
        pos += 3;
    }

    // 2. Fix malformed ends: "}$" -> "} $" (if not }$$)
    pos = 0;
    while ((pos = out.find("}$", pos)) != string::npos) {
        if (pos + 2 < out.length() && out[pos+2] == '$') {
            pos += 3; 
        } else {
            out.replace(pos, 2, "} $");
            pos += 3;
        }
    }

    // 3. Flatten multiline inline containers: $ { \n } $ -> $ { } $
    pos = 0;
    while ((pos = out.find("$ {", pos)) != string::npos) {
        if (pos > 0 && out[pos-1] == '$') { // Skip $$ {
            pos += 3; continue; 
        }
        size_t end_pos = out.find("} $", pos);
        if (end_pos != string::npos) {
            for (size_t k = pos; k < end_pos; ++k) {
                if (out[k] == '\n') out[k] = ' ';
            }
            pos = end_pos + 3;
        } else {
            pos += 3;
        }
    }

    // 4. Fix Block Containers ($$ ... $$)
    pos = 0;
    while ((pos = out.find("$$", pos)) != string::npos) {
        if (pos > 0 && out[pos-1] == '}') { // Closing }$$
            pos += 2; continue;
        }
        
        // Opener $$
        // 4.1 Check for {
        size_t brace_pos = out.find('{', pos);
        size_t next_double_dollar = out.find("$$", pos + 2);
        
        if (brace_pos == string::npos || (next_double_dollar != string::npos && brace_pos > next_double_dollar)) {
            // Missing {, add it
            size_t newline_pos = out.find('\n', pos);
            if (newline_pos != string::npos && (next_double_dollar == string::npos || newline_pos < next_double_dollar)) {
                out.insert(newline_pos, " {");
            } else {
                out.insert(pos + 2, " {");
            }
        }
        
        // 4.2 Check for closing }$$
        next_double_dollar = out.find("$$", pos + 2);
        size_t closer = out.find("}$$", pos + 2);
        
        if (closer == string::npos || (next_double_dollar != string::npos && next_double_dollar < closer)) {
            // Missing closer
            if (next_double_dollar != string::npos) {
                size_t insert_point = next_double_dollar;
                // Backtrack to find a good spot (e.g. before newline)
                if (insert_point > 0 && out[insert_point-1] == '\n') insert_point--;
                out.insert(insert_point, "\n}$$");
            } else {
                out += "\n}$$";
            }
        }
        pos += 2;
    }

    // 5. Remove stray single $ (unpaired)
    string final_out = "";
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '$') {
            bool is_double = (i + 1 < out.size() && out[i+1] == '$') || (i > 0 && out[i-1] == '$');
            bool is_inline_start = (i + 2 < out.size() && out[i+1] == ' ' && out[i+2] == '{');
            bool is_inline_end = (i > 1 && out[i-1] == ' ' && out[i-2] == '}');
            
            if (!is_double && !is_inline_start && !is_inline_end) {
                continue; // Skip stray $
            }
        }
        final_out += out[i];
    }
    
    return final_out;
}

// [NEW] Series Mode: Parse blueprint for sequential generation
struct BlueprintEntry {
    string filename;
    string content;
};

inline vector<BlueprintEntry> parseBlueprint(const string& fullContext) {
    vector<BlueprintEntry> entries;
    stringstream ss(fullContext);
    string line;
    BlueprintEntry current;
    bool inBlock = false;

    while (getline(ss, line)) {
        // Skip internal markers
        if (line.find("// --- START FILE:") == 0 || line.find("// --- END FILE:") == 0) continue;

        string cleanLine = line;
        size_t first = cleanLine.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            if (inBlock) current.content += line + "\n";
            continue;
        }
        cleanLine.erase(0, first);

        if (cleanLine.rfind("EXPORT:", 0) == 0) {
            if (inBlock && !current.filename.empty()) {
                entries.push_back(current);
            }
            current = BlueprintEntry();
            string rawArgs = cleanLine.substr(7);
            
            size_t q1 = rawArgs.find_first_of("\"'");
            if (q1 != string::npos) {
                char quote = rawArgs[q1];
                size_t q2 = rawArgs.find(quote, q1 + 1);
                if (q2 != string::npos) current.filename = rawArgs.substr(q1 + 1, q2 - q1 - 1);
            } else {
                stringstream fss(rawArgs); fss >> current.filename;
            }
            
            inBlock = (current.filename != "END" && !current.filename.empty());
        } else {
            if (inBlock) current.content += line + "\n";
        }
    }
    if (inBlock && !current.filename.empty()) entries.push_back(current);
    return entries;
}

// [v6.0] Helper to get Tree-sitter query for refinement
inline string get_refine_query(const string& lang_id) {
    if (lang_id == "cpp" || lang_id == "c" || lang_id == "hpp" || lang_id == "h" || lang_id == "cc" || lang_id == "cxx") {
        return R"(
            (function_definition) @unit
            (class_specifier) @unit
            (struct_specifier) @unit
            (template_declaration) @unit
            (preproc_include) @unit
            (preproc_def) @unit
            (using_declaration) @unit
            (namespace_definition) @unit
        )";
    }
    if (lang_id == "py") {
        return R"(
            (function_definition) @unit
            (class_definition) @unit
            (import_statement) @unit
            (import_from_statement) @unit
        )";
    }
    if (lang_id == "js" || lang_id == "ts") {
        return R"(
            (function_declaration) @unit
            (class_declaration) @unit
            (variable_declaration) @unit
            (lexical_declaration) @unit
            (import_statement) @unit
            (export_statement) @unit
        )";
    }
    if (lang_id == "java") {
        return R"(
            (class_declaration) @unit
            (interface_declaration) @unit
            (enum_declaration) @unit
            (import_declaration) @unit
            (method_declaration) @unit
        )";
    }
    if (lang_id == "go") {
        return R"(
            (function_declaration) @unit
            (type_declaration) @unit
            (import_declaration) @unit
        )";
    }
    return "";
}