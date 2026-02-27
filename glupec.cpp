/* GLUPE COMPILER ( formerly yori.exe) - v5.8*/

// build with this: g++ glupec.cpp -o glupe -std=c++17 -lstdc++fs -static-libgcc -static-libstdc++
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <array>
#include <cstdio>  
#include <cstdlib> 
#include <thread>
#include <chrono>
#include <map>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <ctime>
#include <functional>
#include <set>
#include <memory>
#include <limits>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

#ifndef _WIN32
#define _popen popen
#define _pclose pclose
#endif

#include "json.hpp" 

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;


// --- CONFIGURATION ---
string PROVIDER = "local"; 
string PROTOCOL = "ollama"; // 'google', 'openai', 'ollama'
string API_KEY = "";
string MODEL_ID = ""; 
string API_URL = "";
int MAX_RETRIES = 15;
bool VERBOSE_MODE = false;

const string CURRENT_VERSION = "5.8.1";
const string CURRENT_VERSION = "6.0.0";

enum class GenMode { CODE, MODEL_3D, IMAGE };
GenMode CURRENT_MODE = GenMode::CODE;

// --- LOGGER SYSTEM ---
ofstream logFile;

void initLogger() {
    logFile.open("glupe.log", ios::app); 
    if (logFile.is_open()) {
        auto t = time(nullptr);
        auto tm = *localtime(&t);
        logFile << "\n--- SESSION START (v" << CURRENT_VERSION << "): " << put_time(&tm, "%Y-%m-%d %H:%M:%S") << " ---\n";
    }
}

void log(string level, string message) {
    if (logFile.is_open()) {
        auto t = time(nullptr);
        auto tm = *localtime(&t);
        logFile << "[" << put_time(&tm, "%H:%M:%S") << "] [" << level << "] " << message << endl;
    }
    if (VERBOSE_MODE) cout << "   [" << level << "] " << message << endl;
}

// --- UTILS DECLARATION ---
struct CmdResult { string output; int exitCode; };

CmdResult execCmd(string cmd) {
    array<char, 128> buffer; string result; 
    string full_cmd = cmd + " 2>&1"; 
    
    FILE* pipe = _popen(full_cmd.c_str(), "r");
    if (!pipe) return {"EXEC_FAIL", -1};
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) result += buffer.data();
    int code = _pclose(pipe);
    return {result, code};
}

string stripExt(string fname) {
    size_t lastindex = fname.find_last_of("."); 
    return (lastindex == string::npos) ? fname : fname.substr(0, lastindex); 
}

string getExt(string fname) {
    size_t lastindex = fname.find_last_of("."); 
    return (lastindex == string::npos) ? "" : fname.substr(lastindex); 
}

// [UPDATED v5.1] Enhanced error detection for lazy transpilation
bool isFatalError(const string& errMsg) {
    string lowerErr = errMsg;
    transform(lowerErr.begin(), lowerErr.end(), lowerErr.begin(), ::tolower);
    if (lowerErr.find("fatal error") != string::npos) return true;
    if (lowerErr.find("no such file") != string::npos) return true;
    if (lowerErr.find("file not found") != string::npos) return true;
    if (lowerErr.find("cannot open source file") != string::npos) return true;
    if (lowerErr.find("module not found") != string::npos) return true;
    if (lowerErr.find("importerror") != string::npos) return true; 
    if (lowerErr.find("undefined reference") != string::npos && lowerErr.find("main") != string::npos) return true; 
    // AI Laziness Detection
    if (lowerErr.find("python.h") != string::npos) return true;
    if (lowerErr.find("jni.h") != string::npos) return true;
    if (lowerErr.find("node.h") != string::npos) return true;
    return false;
}

// [NEW] Heuristic to detect spaghetti/legacy code
bool detectIfCodeIsSpaghetti(const string& code) {
    if (code.find("goto ") != string::npos) return true;
    if (code.find("setjmp") != string::npos) return true;
    
    int maxIndent = 0;
    int currentIndent = 0;
    for (char c : code) {
        if (c == '{') currentIndent++;
        else if (c == '}') currentIndent--;
        if (currentIndent > 6) return true; 
    }

    string lowerCode = code;
    transform(lowerCode.begin(), lowerCode.end(), lowerCode.begin(), ::tolower);
    if (lowerCode.find("spaghetti") != string::npos) return true;
    if (lowerCode.find("legacy") != string::npos) return true;
    if (lowerCode.find("don't touch") != string::npos) return true;
    if (lowerCode.find("do not touch") != string::npos) return true;
    if (lowerCode.find("hack") != string::npos) return true;
    if (lowerCode.find("fixme") != string::npos) return true;

    return false;
}

// [NEW] Helper for ETA formatting
string formatDuration(long long seconds) {
    if (seconds < 60) return to_string(seconds) + "s";
    long long min = seconds / 60;
    long long sec = seconds % 60;
    return to_string(min) + "m " + to_string(sec) + "s";
}

// --- LANGUAGE SYSTEM ---
struct LangProfile {
    string id; string name; string extension;  
    string versionCmd; string buildCmd; bool producesBinary;
    string checkCmd; 
};

map<string, LangProfile> LANG_DB = {
    {"cpp",  {"cpp", "C++", ".cpp", "g++ --version", "g++ -std=gnu++17", true}},
    {"cc",   {"cpp", "C++", ".cc",  "g++ --version", "g++ -std=gnu++17", true}},
    {"cxx",  {"cpp", "C++", ".cxx", "g++ --version", "g++ -std=gnu++17", true}},
    {"hpp",  {"cpp", "C++", ".hpp", "g++ --version", "g++ -std=gnu++17", true}},
    {"hh",   {"cpp", "C++", ".hh",  "g++ --version", "g++ -std=gnu++17", true}},
    {"c",    {"c",   "C",   ".c",   "gcc --version", "gcc", true}},
    {"h",    {"c",   "C",   ".h",   "gcc --version", "gcc", true}},
    {"rust", {"rust","Rust",".rs",  "rustc --version", "rustc", true}}, 
    {"go",   {"go",  "Go",  ".go",  "go version", "go build", true}},
    {"py",   {"py",  "Python", ".py", "python --version", "python -m py_compile", false, "python"}},
    {"js",   {"js",  "JavaScript", ".js", "node --version", "node -c", false}},
    {"ts",   {"ts",  "TypeScript", ".ts", "tsc --version", "tsc --noEmit", false}},
    {"cs",   {"cs",  "C#",  ".cs",  "dotnet --version", "dotnet build", true}},
    {"java", {"java","Java",".java","javac -version", "javac", false}},
    {"php",  {"php", "PHP", ".php", "php -v", "php -l", false}},
    {"rb",   {"rb",  "Ruby", ".rb", "ruby -v", "ruby -c", false}},
    {"lua",  {"lua", "Lua", ".lua", "lua -v", "luac -p", false}},
    {"pl",   {"pl",  "Perl", ".pl", "perl -v", "perl -c", false}},
    {"sh",   {"sh",  "Bash", ".sh", "bash --version", "bash -n", false}},
    {"swift",{"swift","Swift",".swift","swift --version", "swiftc", true}},
    {"kt",   {"kt",  "Kotlin", ".kt", "kotlinc -version", "kotlinc", false}},
    {"scala",{"scala","Scala",".scala","scala -version", "scalac", false}},
    {"hs",   {"hs",  "Haskell", ".hs", "ghc --version", "ghc", true}},
    {"jl",   {"jl",  "Julia", ".jl", "julia --version", "julia", false}},
    {"dart", {"dart","Dart",".dart","dart --version", "dart compile exe", true}},
    {"zig",  {"zig", "Zig", ".zig", "zig version", "zig build-exe", true}},
    {"nim",  {"nim", "Nim", ".nim", "nim --version", "nim c", true}},
    {"r",    {"r",   "R",   ".r",   "R --version", "Rscript", false}},
    {"html", {"html", "HTML", ".html", "", "", false}},
    {"css",  {"css",  "CSS",  ".css",  "", "", false}},
    {"sql",  {"sql",  "SQL",  ".sql",  "", "", false}},
    {"xml",  {"xml",  "XML",  ".xml",  "", "", false}},
    {"yaml", {"yaml", "YAML", ".yaml", "", "", false}},
    {"json", {"json", "JSON", ".json", "", "", false}},
    {"md",   {"md",   "Markdown", ".md", "", "", false}},
    {"ps1",  {"ps1", "PowerShell", ".ps1", "pwsh -v", "pwsh -c", false}},
    {"bat",  {"bat", "Batch", ".bat", "cmd /?", "cmd /c", false}},
    {"vue",  {"vue", "Vue", ".vue", "", "", false}},
    {"jsx",  {"jsx", "React (JSX)", ".jsx", "", "", false}},
    {"tsx",  {"tsx", "React (TSX)", ".tsx", "", "", false}},
    {"htm",  {"htm", "HTML", ".html", "", "", false}},
    {"clj",  {"clj", "Clojure", ".clj", "clojure -h", "", false}},
    {"ex",   {"ex",  "Elixir",  ".ex",  "elixir -v", "elixirc", false}},
    {"erl",  {"erl", "Erlang",  ".erl", "erl -version", "erlc", true}},
    {"fs",   {"fs",  "F#",      ".fs",  "dotnet --version", "dotnet build", true}},
    {"vb",   {"vb",  "VB.NET",  ".vb",  "dotnet --version", "dotnet build", true}},
    {"groovy",{"groovy","Groovy",".groovy","groovy -v", "groovyc", false}},
    {"tex",  {"tex", "LaTeX",   ".tex", "pdflatex --version", "pdflatex -interaction=nonstopmode", false}},
    {"acn",  {"acn", "Acorn",   ".acn", "", "", false}},
    {"arduino", {"arduino", "Arduino (AVR)", ".ino", "arduino-cli version", "arduino-cli compile --fqbn arduino:avr:uno", true}},
    {"esp32",   {"esp32",   "ESP32",          ".ino", "arduino-cli version", "arduino-cli compile --fqbn esp32:esp32:esp32", true}},
    {"glp",   {"glp", "glupe",    ".glp",   "", "", false}},
    {"glupe", {"glp", "glupe",    ".glupe", "", "", false}}
};

map<string, LangProfile> MODEL_DB = {
    {"obj",  {"obj",  "Wavefront OBJ", ".obj",  "", "", false}},
    {"stl",  {"stl",  "STL (ASCII)",   ".stl",  "", "", false}},
    {"ply",  {"ply",  "PLY (ASCII)",   ".ply",  "", "", false}},
    {"gltf", {"gltf", "glTF (JSON)",   ".gltf", "", "", false}}
};

map<string, LangProfile> IMAGE_DB = {
    {"svg",  {"svg",  "SVG (Vector)",  ".svg",  "", "", false}},
    {"eps",  {"eps",  "PostScript",    ".eps",  "", "", false}}
};

LangProfile CURRENT_LANG; 

// --- CONFIG & TOOLCHAIN OVERRIDES ---
bool loadConfig(string mode) {
    string configPath = "config.json";
    ifstream f(configPath);
    if (!f.is_open()) {
        if(mode == "local") {
            API_URL = "http://localhost:11434/api/generate";
            PROTOCOL = "ollama";
        } else {
            PROTOCOL = "google"; 
        }
        return true; 
    }

    try {
        json j = json::parse(f);
        if (j.contains("max_retries")) {
            MAX_RETRIES = j["max_retries"];
        }
        if (j.contains(mode)) {
            json profile = j[mode];
            PROVIDER = mode;
            
            MODEL_ID = profile.value("model_id", "gemini-pro");
            
            if (profile.contains("protocol")) PROTOCOL = profile["protocol"];
            else PROTOCOL = (mode == "cloud") ? "google" : "ollama"; 

            if (profile.contains("api_url")) {
                API_URL = profile["api_url"];
            } else {
                if (PROTOCOL == "google") API_URL = "https://generativelanguage.googleapis.com/v1beta/models/" + MODEL_ID + ":generateContent";
                else if (PROTOCOL == "openai") API_URL = "https://api.openai.com/v1/chat/completions";
                else if (mode == "local") API_URL = "http://localhost:11434/api/generate";
            }

            if (mode == "cloud") API_KEY = profile.value("api_key", "");
        }
        if (j.contains("toolchains")) {
            for (auto& [key, val] : j["toolchains"].items()) {
                if (LANG_DB.count(key)) {
                    if (val.contains("build_cmd")) LANG_DB[key].buildCmd = val["build_cmd"];
                    if (val.contains("version_cmd")) LANG_DB[key].versionCmd = val["version_cmd"];
                }
            }
        }
        return true;
    } catch (...) { return false; }
}

// --- AI CORE ---
string callAI(string prompt) {
    string response;
    string url = API_URL;
    
    json body;
    string extraHeaders = "";

    if (PROTOCOL == "google") {
        body["contents"][0]["parts"][0]["text"] = prompt;
        if (url.find("?key=") == string::npos) url += "?key=" + API_KEY;
    } 
    else if (PROTOCOL == "openai") {
        body["model"] = MODEL_ID;
        
        // [FIX] Handle APIFreeLLM divergence 
        if (API_URL.find("apifreellm.com") != string::npos) {
            body["message"] = prompt; 
        } else {
            body["messages"][0]["role"] = "user";
            body["messages"][0]["content"] = prompt;
        }
        
        extraHeaders = " -H \"Authorization: Bearer " + API_KEY + "\"";
    }
    else { 
        body["model"] = MODEL_ID;
        body["prompt"] = prompt;
        body["stream"] = false; 
    }

    for(int i=0; i<3; i++) {
        ofstream file("request_temp.json"); 
        file << body.dump(-1, ' ', false, json::error_handler_t::replace); 
        file.close();
        
        string verbosity = VERBOSE_MODE ? " -v" : " -s";
        string cmd = "curl" + verbosity + " -X POST -H \"Content-Type: application/json\"" + extraHeaders + " -d @request_temp.json \"" + url + "\"";
        
        CmdResult res = execCmd(cmd);
        response = res.output;
        remove("request_temp.json");
        
        if (VERBOSE_MODE) cout << "\n[DEBUG] Raw Response: " << response << endl;

        if (response.find("401 Unauthorized") != string::npos) return "ERROR: 401 Unauthorized (Check API Key)";
        if (response.find("404 Not Found") != string::npos) return "ERROR: 404 Not Found (Check URL)";

        if (response.find("Missing required parameter") != string::npos) {
             cout << "\n[DEBUG] API rejected payload. Sending: " << body.dump() << endl;
        }

        if (PROTOCOL == "google" && response.find("429") != string::npos) { 
             log("WARN", "API 429 Rate Limit. Backoff...");
             this_thread::sleep_for(chrono::seconds(5 * (i+1)));
             continue; 
        }
        break;
    }
    return response;
}

string extractCode(string jsonResponse) {
    if (jsonResponse.empty()) return "ERROR: Empty response from API";
    if (jsonResponse.find("ERROR:") == 0) return jsonResponse;

    try {
        json j = json::parse(jsonResponse);
        string raw = "";
        
        if (j.contains("error")) {
            if (j["error"].is_object() && j["error"].contains("message")) 
                return "ERROR: API Error - " + j["error"]["message"].get<string>();
            return "ERROR: " + j["error"].dump();
        }
        
        if (j.contains("choices") && j["choices"].size() > 0) {
            if (j["choices"][0].contains("message"))
                raw = j["choices"][0]["message"]["content"];
            else if (j["choices"][0].contains("text"))
                raw = j["choices"][0]["text"];
        }
        else if (j.contains("candidates") && j["candidates"].size() > 0) {
             raw = j["candidates"][0]["content"]["parts"][0]["text"];
        }
        else if (j.contains("response")) {
            raw = j["response"];
        } 
        else {
            return "ERROR: UNKNOWN_RESPONSE_FORMAT: " + jsonResponse.substr(0, 100);
        }
        
        size_t start = raw.find("```"); 
        if (start == string::npos) return raw; 
        size_t end_line = raw.find('\n', start); 
        size_t end_block = raw.rfind("```");
        if (end_line != string::npos && end_block != string::npos && end_block > end_line) {
            return raw.substr(end_line + 1, end_block - end_line - 1);
        }
        return raw;
    } catch (const exception& e) { 
        string safeMsg = jsonResponse;
        if (safeMsg.length() > 200) safeMsg = safeMsg.substr(0, 200) + "...";
        replace(safeMsg.begin(), safeMsg.end(), '\n', ' ');
        return "ERROR: JSON Parsing Failed. Response was: " + safeMsg; 
    }
}

void explainFatalError(const string& errorMsg) {
    cout << "\n[GLUPE ASSISTANT] ANALYZING fatal error..." << endl;
    string prompt = "ROLE: Helpful Tech Support.\nTASK: Fix missing file error.\nERROR: " + errorMsg.substr(0, 500) + "\nOUTPUT: Short advice.";
    string advice = callAI(prompt);
    
    try {
        json j = json::parse(advice);
        if (j.contains("candidates")) advice = j["candidates"][0]["content"]["parts"][0]["text"];
        else if (j.contains("response")) advice = j["response"];
        else if (j.contains("choices")) advice = j["choices"][0]["message"]["content"];
    } catch(...) {}
    cout << "> Proposed solution: " << advice << endl;
}

// --- CONFIG COMMANDS ---
void updateConfigFile(string key, string value) {
    string configPath = "config.json";
    json j;
    
    if (fs::exists(configPath)) {
        ifstream i(configPath);
        if (i.is_open()) { try { i >> j; } catch(...) {} }
    }
    
    if (!j.contains("cloud")) j["cloud"] = json::object();
    if (!j.contains("local")) j["local"] = json::object();

    if (key == "api-key") {
        j["cloud"]["api_key"] = value;
        cout << "[CONFIG] Updated cloud.api_key." << endl;
    } else if (key == "model-cloud") {
         j["cloud"]["model_id"] = value;
         cout << "[CONFIG] Updated cloud.model_id to " << value << endl;
    } else if (key == "model-local") {
         j["local"]["model_id"] = value;
         cout << "[CONFIG] Updated local.model_id to " << value << endl;
    } else if (key == "url-local") {
         j["local"]["api_url"] = value;
         cout << "[CONFIG] Updated local.api_url to " << value << endl;
    } else if (key == "url-cloud") { 
         j["cloud"]["api_url"] = value;
         cout << "[CONFIG] Updated cloud.api_url to " << value << endl;
    } else if (key == "cloud-protocol") { 
         if (value != "google" && value != "openai") {
             cout << "[ERROR] Protocol must be 'google' or 'openai'." << endl; return;
         }
         j["cloud"]["protocol"] = value;
         cout << "[CONFIG] Updated cloud.protocol to " << value << endl;
    } else if (key == "max-retries") {
         try {
             int v = stoi(value);
             if (v > 0) {
                 j["max_retries"] = v;
                 cout << "[CONFIG] Updated max_retries to " << v << endl;
             } else {
                 cout << "[ERROR] max-retries must be > 0." << endl; return;
             }
         } catch (...) { cout << "[ERROR] Invalid number." << endl; return; }
    } else {
        cout << "[ERROR] Unknown config key." << endl;
        return;
    }

    ofstream o(configPath);
    o << j.dump(4);
    cout << "[SUCCESS] Saved to " << configPath << endl;
}

void showConfig() {
    string configPath = "config.json";
    if (!fs::exists(configPath)) {
        cout << "[WARN] No config.json found." << endl;
        return;
    }
    try {
        ifstream i(configPath); json j; i >> j;
        cout << "\n--- GLUPE CONFIGURATION ---\n";
        
        if (j.contains("max_retries")) cout << "  Max Retries: " << j["max_retries"] << endl;
        else cout << "  Max Retries: 15 (Default)" << endl;
        
        if (j.contains("cloud")) {
            cout << "[CLOUD]\n";
            auto& c = j["cloud"];
            if (c.contains("protocol")) cout << "  Protocol : " << c["protocol"].get<string>() << endl;
            if (c.contains("model_id")) cout << "  Model    : " << c["model_id"].get<string>() << endl;
            if (c.contains("api_url"))  cout << "  URL      : " << c["api_url"].get<string>() << endl;
            if (c.contains("api_key")) {
                string k = c["api_key"].get<string>();
                if (k.length() > 6) k = k.substr(0, 3) + "..." + k.substr(k.length()-3);
                else if (!k.empty()) k = "***";
                cout << "  API Key  : " << k << endl;
            }
        }
        
        if (j.contains("local")) {
            cout << "\n[LOCAL]\n";
            auto& l = j["local"];
            if (l.contains("model_id")) cout << "  Model    : " << l["model_id"].get<string>() << endl;
            if (l.contains("api_url"))  cout << "  URL      : " << l["api_url"].get<string>() << endl;
        }
        cout << "--------------------------\n";
    } catch (...) { cout << "[ERROR] Corrupt or invalid config file." << endl; }
}

void selectOllamaModel() {
    cout << "[INFO] Scanning for local Ollama models..." << endl;
    string url = "http://localhost:11434/api/tags";
    
    string configPath = "config.json";
    if (fs::exists(configPath)) {
        try {
            ifstream i(configPath); json j; i >> j;
            if (j.contains("local") && j["local"].contains("api_url")) {
                string currentUrl = j["local"]["api_url"];
                size_t pos = currentUrl.find("/api/");
                if (pos != string::npos) url = currentUrl.substr(0, pos) + "/api/tags";
            }
        } catch(...) {}
    }

    string cmd = "curl -s \"" + url + "\"";
    CmdResult res = execCmd(cmd);

    if (res.exitCode != 0 || res.output.empty()) {
        cout << "[ERROR] Could not connect to Ollama at " << url << endl;
        cout << "Make sure Ollama is running." << endl;
        return;
    }

    try {
        json j = json::parse(res.output);
        if (!j.contains("models")) { cout << "[ERROR] Unexpected response format." << endl; return; }

        vector<string> models;
        cout << "\n--- Installed Local Models ---\n";
        int idx = 1;
        for (auto& m : j["models"]) {
            string name = m["name"];
            models.push_back(name);
            cout << idx++ << ". " << name << endl;
        }

        if (models.empty()) { cout << "[WARN] No models found." << endl; return; }

        cout << "\nSelect model number (or 0 to cancel): ";
        int choice;
        if (cin >> choice && choice > 0 && choice <= models.size()) {
            updateConfigFile("model-local", models[choice-1]);
        } 
    } catch (...) { cout << "[ERROR] JSON Parsing failed." << endl; }
}

void openApiKeyPage() {
    cout << "[INFO] Opening ApiFreeLlm.com..." << endl;
    #ifdef _WIN32
    system("start https://apifreellm.com/en/api-access");
    #else
    system("xdg-open https://apifreellm.com/en/api-access");
    #endif
}

// [NEW] Tree Shaking Logic
string performTreeShaking(const string& code, const string& language) {
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

// --- PREPROCESSOR ---
// [NEW] Pre-processor to extract glupe syntax from comments
string decommentGlupeSyntax(const string& code) {
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
string resolveImports(string code, fs::path basePath, vector<string>& stack) {
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

// [NEW] Cache System Constants
const string CACHE_DIR = "glupe_cache";
const string LOCK_FILE = ".glupe.lock";

struct Container {
    string id;
    string prompt;
    string hash;
    bool isCached = false;
};

json LOCK_DATA;

void initCache() {
    if (!fs::exists(CACHE_DIR)) fs::create_directory(CACHE_DIR);
    if (fs::exists(LOCK_FILE)) {
        try {
            ifstream f(LOCK_FILE);
            LOCK_DATA = json::parse(f);
        } catch(...) { LOCK_DATA = json::object(); }
    } else {
        LOCK_DATA = json::object();
        LOCK_DATA["containers"] = json::object();
    }
}

void saveCache() {
    ofstream f(LOCK_FILE);
    f << LOCK_DATA.dump(4);
}

string getContainerHash(const string& prompt) {
    hash<string> hasher;
    return to_string(hasher(prompt));
}

string getCachedContent(const string& id) {
    string path = CACHE_DIR + "/" + id + ".txt";
    if (fs::exists(path)) {
        ifstream f(path);
        return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    }
    return "";
}

void setCachedContent(const string& id, const string& content) {
    string path = CACHE_DIR + "/" + id + ".txt";
    ofstream f(path);
    f << content;
}

// [NEW] Pre-process input to handle containers and caching
string processInputWithCache(const string& code, bool useCache, const vector<string>& updateTargets, bool fillMode) {
    // [FUTURE v6.0] AST INTEGRATION POINT
    // 1. Normalize: Replace $$...$$ with valid placeholders (e.g. comments or void calls)
    // 2. Parse: auto tree = parser.parse_string(normalized_code);
    // 3. Traverse: Find placeholder nodes in the AST.
    // 4. Context: For each node, walk up to find enclosing function/class for prompt context.
    // 5. Generate: Call LLM with AST-derived context.
    // 6. Verify: Parse result, ensure tree structure outside container is identical.
    // 7. Unparse: Convert modified AST back to string.
    
    string result;
    map<string, string> containerPrompts; // [NEW] Store prompts for inheritance
    size_t pos = 0;
    
    while (pos < code.length()) {
        size_t start = code.find("$", pos);
        if (start == string::npos) {
            result += code.substr(pos);
            break;
        }

        bool isBlock = false;
        bool isInline = false;
        size_t scan = 0;

        if (start + 1 < code.length() && code[start+1] == '$') {
            isBlock = true;
            scan = start + 2;
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

        if (!isBlock && !isInline) {
            result += code.substr(pos, start - pos + 1);
            pos = start + 1;
            continue;
        }

        // Check named $$ "id" {
        bool isNamed = false;
        bool isAbstract = false; // [NEW] Track abstract status
        string id;
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
            while(scan < code.length() && !isspace(code[scan]) && code[scan] != '{' && !(code[scan] == '-' && scan+1 < code.length() && code[scan+1] == '>')) {
                scan++;
            }
            
            if (scan > idStart) {
                id = code.substr(idStart, scan - idStart);
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
            if (!parentIds.empty()) {
                string combinedParents;
                for(const auto& pid : parentIds) {
                    if (containerPrompts.count(pid)) {
                        combinedParents += "\n--- INHERITED FROM " + pid + " ---\n" + containerPrompts[pid] + "\n";
                        cout << "   [INHERIT] Container '" << id << "' inherits from '" << pid << "'" << endl;
                    } else {
                        cout << "   [WARN] Parent container '" << pid << "' not found (must be defined before use)." << endl;
                    }
                }
                if (!combinedParents.empty()) {
                    string childLogic = prompt;
                    prompt = "CONTEXT: The following logic is inherited from parent containers.\nRESOLUTION RULES:\n1. Child logic overrides parent logic.\n2. If parents contradict, the last listed parent takes precedence.\n" + combinedParents + "\n--- CHILD LOGIC (" + id + ") ---\n" + childLogic;
                }
            }
            containerPrompts[id] = prompt; // Store resolved prompt

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

// [NEW] Post-process AI output to update cache
string updateCacheFromOutput(string code) {
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

// [NEW] Helper to strip AI templates ($${...}$$) from code lines
string stripTemplates(const string& line, bool& insideTemplate) {
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

// [NEW] Series Mode: Parse blueprint for sequential generation
struct BlueprintEntry {
    string filename;
    string content;
};

vector<BlueprintEntry> parseBlueprint(const string& fullContext) {
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

// [NEW] Validate container names and detect collisions
bool validateContainers(const string& code, bool* outHasActive = nullptr) {
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
string processExports(const string& code, const fs::path& basePath) {
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

set<string> extractDependencies(const string& code) {
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

bool preFlightCheck(const set<string>& deps) {
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
        if (isFatalError(res.output)) {
             cout << "   [?] Analyze fatal error with AI? [y/N]: ";
             char ans; cin >> ans;
             if (ans == 'y' || ans == 'Y') explainFatalError(res.output);
        } else {
             cout << "   [ERROR LOG]:\n" << res.output.substr(0, 300) << endl;
        }
        return false;
    }
    cout << "   [OK] Dependencies verified." << endl;
    return true;
}

void selectTarget() {
    string label = "Language";
    if (CURRENT_MODE == GenMode::MODEL_3D) label = "3D Format";
    if (CURRENT_MODE == GenMode::IMAGE) label = "Image Format";

    cout << "\n[?] Ambiguous target. Select " << label << ":\n";
    int i = 1; vector<string> keys;
    
    const auto* db = &LANG_DB;
    if (CURRENT_MODE == GenMode::MODEL_3D) db = &MODEL_DB;
    else if (CURRENT_MODE == GenMode::IMAGE) db = &IMAGE_DB;

    for(auto const& [key, val] : *db) keys.push_back(key);
    for(const auto& k : keys) { cout << i++ << ". " << db->at(k).name << " "; if(i%4==0) cout<<endl;}
    cout << "\n> "; int c; cin >> c; 
    if(c>=1 && c<=keys.size()) CURRENT_LANG = db->at(keys[c-1]);
    else CURRENT_LANG = (CURRENT_MODE == GenMode::CODE) ? LANG_DB["cpp"] : (CURRENT_MODE == GenMode::MODEL_3D ? MODEL_DB["obj"] : IMAGE_DB["svg"]);
}

// [NEW] Helper to split code into semantic chunks for Refine Mode
vector<string> splitSourceCode(const string& code, int targetLines = 500) {
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
string extractSignatures(const string& code) {
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
string stripMetadata(const string& code) {
    size_t start = code.find("META_START");
    size_t end = code.find("META_END");
    if (start != string::npos && end != string::npos && end > start) {
        return code.substr(0, start) + code.substr(end + 8);
    }
    return code;
}

void showMetadata(const string& filename) {
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

// --- AUTH HELPERS ---
const string SESSION_FILE = ".glupe_session";

pair<string, string> getSession() {
    if (!fs::exists(SESSION_FILE)) return {"", ""};
    try {
        ifstream f(SESSION_FILE);
        json j = json::parse(f);
        return {j.value("token", ""), j.value("username", "")};
    } catch (...) {
        return {"", ""};
    }
}

void saveSession(string token, string username) {
    json j;
    j["token"] = token;
    j["username"] = username;
    ofstream f(SESSION_FILE);
    if (f.is_open()) {
        f << j.dump(4);
        f.close();
        cout << "[SUCCESS] Logged in as " << username << endl;
    } else {
        cout << "[ERROR] Could not save session file." << endl;
    }
}

string getSessionToken() {
    return getSession().first;
}

string getSessionUser() {
    return getSession().second;
}

bool checkLogin() {
    if (getSessionToken().empty()) {
        cout << "Error: Not logged in. Run 'glupe login' first." << endl;
        return false;
    }
    return true;
}

void startInteractiveHub() {
    string hub_url = "https://glupehub.up.railway.app";
    cout << "GlupeHub v-alpha-1.0 MVPType 'help' for commands." << endl;

    while (true) {
        cout << "hub> ";
        string line;
        getline(cin, line);

        if (cin.eof() || line == "exit") {
            break;
        }
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "help") {
            cout << "Available commands:\n";
            cout << "  search <query>   : Search for files by name.\n";
            cout << "  tag <tagname>    : Search for files by tag.\n";
            cout << "  show <username>  : Show files for a user.\n";
            cout << "  view <file_id>   : View file metadata (e.g., user/file.glp).\n";
            cout << "  pull <file_id>   : Download a file.\n";
            cout << "  delete <path>    : Delete a file (e.g. file.glp).\n";
            cout << "  rename <old> <new>: Rename a file.\n";
            cout << "  exit             : Exit interactive mode.\n";
        } else if (command == "search") {
            string query;
            getline(ss, query);
            query.erase(0, query.find_first_not_of(" \t"));
            if (query.empty()) { cout << "Usage: search <query>" << endl; continue; }
            string curlCmd = "curl -sS -G --data-urlencode \"q=" + query + "\" \"" + hub_url + "/search/files\"";
            cout << execCmd(curlCmd).output << endl;
        } else if (command == "tag") {
            string tag;
            ss >> tag;
            if (tag.empty()) { cout << "Usage: tag <tagname>" << endl; continue; }
            string curlCmd = "curl -sS -G --data-urlencode \"tag=" + tag + "\" \"" + hub_url + "/search/files\"";
            cout << execCmd(curlCmd).output << endl;
        } else if (command == "show") {
            string target;
            ss >> target;
            if (target.empty()) { cout << "Usage: show <username>" << endl; continue; }
            string curlCmd = "curl -sS -G --data-urlencode \"author=" + target + "\" \"" + hub_url + "/search/files\"";
            string response = execCmd(curlCmd).output;

            try {
                json j = json::parse(response);
                const json* files_to_process = nullptr;

                if (j.is_object() && j.contains("files") && j["files"].is_array()) {
                    files_to_process = &j["files"];
                } else if (j.is_array()) {
                    files_to_process = &j; // For backward compatibility
                }

                if (files_to_process) {
                    cout << "\n Directory of " << target << "\n\n";
                    cout << left << setw(30) << "File Name" 
                         << right << setw(12) << "Size (KB)" 
                         << "   " << left << "Last Modified" << endl;
                    cout << string(65, '-') << endl;

                    for (const auto& item : *files_to_process) {
                        string name = item.value("filename", "unknown");
                        long long bytes = item.value<long long>("size", 0);
                        string date = item.value("created_at", "unknown");
                        if (date.length() > 10) date = date.substr(0, 10);

                        double kb = static_cast<double>(bytes) / 1024.0;
                        
                        cout << left << setw(30) << name 
                             << right << setw(12) << fixed << setprecision(2) << kb 
                             << "   " << left << date << endl;
                    }
                    cout << "\n              " << files_to_process->size() << " File(s)\n";
                } else {
                    cout << response << endl;
                }
            } catch (...) {
                cout << response << endl;
            }
        } else if (command == "view") {
            string file_id;
            ss >> file_id;
            if (file_id.empty()) { cout << "Usage: view <file_id>" << endl; continue; }
            string curlCmd = "curl -sS \"" + hub_url + "/meta/" + file_id + "\"";
            string response = execCmd(curlCmd).output;

            try {
                json j_resp = json::parse(response);
                if (j_resp.contains("content_preview")) {
                    string content = j_resp["content_preview"];
                    size_t start = content.find("META_START");
                    size_t end = content.find("META_END");

                    if (start != string::npos && end != string::npos && end > start) {
                        string metaJsonStr = content.substr(start + 10, end - (start + 10));
                        try {
                            json meta = json::parse(metaJsonStr);
                            
                            size_t maxKeyLen = 5; 
                            for (auto& el : meta.items()) {
                                if (el.key().length() > maxKeyLen) maxKeyLen = el.key().length();
                            }
                            size_t maxValueLen = 60;
                            
                            string border = "+" + string(maxKeyLen + 2, '-') + "+" + string(maxValueLen + 2, '-') + "+";

                            cout << "\n Metadata for " << file_id << "\n";
                            cout << border << endl;
                            cout << "| " << left << setw(maxKeyLen) << "Field" << " | " << setw(maxValueLen) << "Value" << " |" << endl;
                            cout << border << endl;

                            for (auto& el : meta.items()) {
                                string val;
                                if (el.value().is_string()) val = el.value().get<string>();
                                else val = el.value().dump();

                                if (val.length() > maxValueLen) {
                                    val = val.substr(0, maxValueLen - 3) + "...";
                                }

                                cout << "| " << left << setw(maxKeyLen) << el.key() << " | " << setw(maxValueLen) << val << " |" << endl;
                            }
                            cout << border << endl;

                        } catch (exception& e) {
                            cout << "[ERROR] Corrupt metadata block: " << e.what() << endl;
                        }
                    } else {
                        cout << "[INFO] No metadata block found." << endl;
                    }
                } else if (j_resp.contains("error")) {
                    cout << "[ERROR] " << j_resp["error"].get<string>() << endl;
                } else {
                    cout << response << endl;
                }
            } catch (...) {
                cout << response << endl;
            }
        } else if (command == "pull") {
            string file_id;
            ss >> file_id;
            if (file_id.empty()) { cout << "Usage: pull <file_id>" << endl; continue; }
            size_t last_slash_pos = file_id.find_last_of('/');
            string filename = (last_slash_pos == string::npos) ? file_id : file_id.substr(last_slash_pos + 1);
            string curlCmd = "curl -sS -L -o \"" + filename + "\" \"" + hub_url + "/pull/" + file_id + "\"";
            CmdResult res = execCmd(curlCmd);
            if (res.exitCode == 0) { cout << "[SUCCESS] Saved " << filename << endl; } 
            else { cout << "[ERROR] Download failed. Server response: " << res.output << endl; }
        } else if (command == "delete") {
            string path;
            ss >> path;
            if (path.empty()) { cout << "Usage: delete <path>" << endl; continue; }

            auto session = getSession();
            if (session.first.empty()) { cout << "[ERROR] Not logged in." << endl; continue; }

            // If path doesn't contain '/', prepend username
            if (path.find('/') == string::npos) {
                path = session.second + "/" + path;
            }

            cout << "Are you sure you want to delete " << path << "? [y/N]: ";
            char c;
            if (cin >> c && (c == 'y' || c == 'Y')) {
                cin.ignore((numeric_limits<streamsize>::max)(), '\n');
                string curlCmd = "curl -sS -X DELETE -H \"Authorization: Bearer " + session.first + "\" \"" + hub_url + "/delete/" + path + "\"";
                CmdResult res = execCmd(curlCmd);
                try {
                    json j = json::parse(res.output);
                    if (j.contains("error")) cout << "[ERROR] " << j["error"].get<string>() << endl;
                    else if (j.contains("message")) cout << "[SUCCESS] " << j["message"].get<string>() << endl;
                    else cout << res.output << endl;
                } catch (...) {
                    cout << res.output << endl;
                }
            } else {
                cin.ignore((numeric_limits<streamsize>::max)(), '\n');
                cout << "Cancelled." << endl;
            }
        } else if (command == "rename") {
            string old_path, new_path;
            ss >> old_path >> new_path;
            if (old_path.empty() || new_path.empty()) { cout << "Usage: rename <old_path> <new_path>" << endl; continue; }

            auto session = getSession();
            if (session.first.empty()) { cout << "[ERROR] Not logged in." << endl; continue; }

            if (old_path.find('/') == string::npos) old_path = session.second + "/" + old_path;
            if (new_path.find('/') == string::npos) new_path = session.second + "/" + new_path;

            json body;
            body["old_path"] = old_path;
            body["new_path"] = new_path;

            ofstream f("rename_temp.json"); f << body.dump(); f.close();
            string curlCmd = "curl -sS -X POST -H \"Content-Type: application/json\" -H \"Authorization: Bearer " + session.first + "\" -d @rename_temp.json \"" + hub_url + "/rename\"";
            CmdResult res = execCmd(curlCmd);
            remove("rename_temp.json");

            try {
                json j = json::parse(res.output);
                if (j.contains("error")) cout << "[ERROR] " << j["error"].get<string>() << endl;
                else if (j.contains("message")) cout << "[SUCCESS] " << j["message"].get<string>() << endl;
                else cout << res.output << endl;
            } catch (...) {
                cout << res.output << endl;
            }
        } else {
            cout << "Unknown command: '" << command << "'. Type 'help' for commands." << endl;
        }
    }
}

/*
$$ test_func{
function testfunc(x){
    return x*x    
}
// [NEW] Execution Timer for -crono flag
struct ExecutionTimer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    bool enabled = false;

}$$
    ExecutionTimer() : start(std::chrono::high_resolution_clock::now()) {}

*/
    ~ExecutionTimer() {
        if (enabled) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            cout << "\n[CRONO] Total execution time: " << fixed << setprecision(3) << elapsed.count() << "s" << endl;
        }
    }
};


void showHelp() {
    cout << "GLUPE v" << CURRENT_VERSION << " - The Semantic Compiler\n";
    cout << "Usage: glupe [files...] [options] [\"*instructions\"]\n\n";
    
    cout << "Core Options:\n";
    cout << "  -o <file>        : Specify output filename.\n";
    cout << "  -cloud           : Use cloud AI provider (configured in config.json).\n";
    cout << "  -local           : Use local AI provider (Ollama).\n";
    cout << "  -u, --update     : Update mode (edits existing file instead of overwriting).\n";
    cout << "  -make            : Architect mode (generates multi-file projects from blueprints).\n";
    cout << "  -series          : Series mode (generates files sequentially).\n";
    cout << "  -refine          : Refine mode (reverse engineer code to .glp blueprint).\n";
    cout << "  -t, --transpile  : Transpile only (do not compile binary).\n";
    cout << "  -run             : Run the output binary after compilation.\n";
    cout << "  -crono           : Measure execution time.\n";
    cout << "  -fill            : Fill containers in-place (preserves manual code).\n";
    cout << "  -dry-run         : Show prompt/context without calling AI.\n";
    cout << "  -verbose         : Enable verbose logging.\n";
    cout << "  -3d              : 3D model generation mode.\n";
    cout << "  -img             : Image generation mode.\n";
    cout << "  --clean          : Remove temporary build files.\n";
    cout << "  --init           : Initialize project (hello.glp, config.json).\n\n";

    cout << "Commands:\n";
    cout << "  config <key> <val>      : Update configuration.\n";
    cout << "  config model-local      : Interactive local model selection.\n";
    cout << "  clean cache             : Clear semantic cache.\n";
    cout << "  edit <file> --container <name> \"prompt\" : Edit a container's prompt.\n";
    cout << "  check <file>            : Validate syntax of a .glp file.\n";
    cout << "  fix <file> \"instr\"      : AI-powered code repair.\n";
    cout << "  explain <file> [lang]   : Generate documentation.\n";
    cout << "  diff <f1> <f2> [lang]   : Semantic diff report.\n";
    cout << "  sos [lang] \"query\"      : Ask AI for help.\n";
    cout << "  update                  : Check for and apply updates to glupe.\n";
    cout << "  hub                     : Enter interactive GlupeHub mode.\n";
    cout << "  login / signup / logout : GlupeHub authentication.\n";
    cout << "  push <file> [tags]      : Upload to GlupeHub.\n";
    cout << "  pull <file> <user>      : Download from GlupeHub.\n";
    cout << "  info <file>             : Show file metadata.\n\n";
    
    cout << "Examples:\n";
    cout << "  glupe main.glp -o app.exe -cpp\n";
    cout << "  glupe idea.txt -make -series\n";
    cout << "  glupe legacy.c -refine\n";
    cout << "  glupe fix bug.py \"fix index out of range\"";
    cout << "  glupe fix bug.py \"fix index out of range\"\n";
}

// [v6.0] Helper to get Tree-sitter query for refinement
string get_refine_query(const string& lang_id) {
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

// [NEW] Helper to sanitize container syntax after refinement
string sanitize_container_syntax(const string& code) {
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

// --- MAIN ---
int main(int argc, char* argv[]) {
    int testx=0;
    int testy=4;
    /*$$ test -> test_func {
    testx = testy
     print(testfunc(tesx))
    }$$
    */
   //ABSTACT a {all printed messages must end with " glupe!"}
   // $ test2 -> a{ print("it works")}$
    ExecutionTimer cronoTimer;
    auto startTime = std::chrono::high_resolution_clock::now();
    initLogger(); 

    if (argc < 2) {
        cout << "GLUPE v" << CURRENT_VERSION << " (Multi-File)\nUsage: glupe file1 ... [-o output] [-cloud/-local] [-3d/-img] [-u] \"*Custom Instructions\"" << endl;
        cout << "Commands:\n  config <key> <val> : Update config.json\n  config model-local : Detect installed Ollama models\n";
        cout << "  edit <f> --cont <n> \"p\" : Edit a container's prompt\n";
        cout << "  clean cache        : Clear semantic cache\n";
        cout << "  fix <file> \"desc\"  : AI-powered code repair\n";
        cout << "  explain <file> [lg] : Generate commented documentation\n";
        cout << "  diff <f1> <f2> [lg] : Generate semantic diff report\n";
        cout << "  sos [lang] \"error\" : Ask AI for help on error/problem (no file needed)\n";
        cout << "  update             : Check for and apply updates to glupe.\n";
        cout << "  info <file.glp>    : Show metadata for GlupeHub\n";
        cout << "  insert-metadata <path> : Insert metadata template\n";
        cout << "  login [url]        : Authenticate with GlupeHub\n";
        cout << "  signup [url]       : Create a new account\n";
        cout << "  logout             : Log out from GlupeHub\n";
        cout << "  whoami             : Show current user\n";
        cout << "  push <file> [tags] : Upload file to GlupeHub (requires login)\n";
        cout << "  hub                : Enter interactive hub mode\n";
        cout << "  pull <file> <user> : Download file from GlupeHub\n";
        return 0;
    }

    // --- COMMAND MODE HANDLING ---
    string cmd = argv[1];
    
    // CONFIG COMMAND
    if (cmd == "config") {
        if (argc < 3) {
            cout << "Usage: glupe config <key> <value>\n";
            cout << "       glupe config see\n";
            cout << "       glupe config model-local (Interactive selection)\n\n";
            cout << "Keys:\n";
            cout << "  api-key         : Set Cloud API Key\n";
            cout << "  max-retries     : Set Max Retries (Default: 15)\n";
            cout << "  cloud-protocol  : Set protocol ('openai', 'google', 'ollama')\n";
            cout << "  model-cloud     : Set Cloud Model ID\n";
            cout << "  url-cloud       : Set Cloud API URL\n";
            cout << "  model-local     : Set Local Model ID\n";
            cout << "  url-local       : Set Local API URL\n";
            return 1;
        }
        string key = argv[2];
        if (key == "see") {
            showConfig();
            return 0;
        }
        if (key == "model-local" && argc == 3) {
            selectOllamaModel();
            return 0;
        }
        if (argc < 4) {
             cout << "[ERROR] Missing value for key: " << key << endl;
             return 1;
        }
        updateConfigFile(key, argv[3]);
        return 0;
    }

    // CLEAN COMMAND
    if (cmd == "clean") {
        if (argc >= 3 && string(argv[2]) == "cache") {
             cout << "[CLEAN] Removing cache directory (" << CACHE_DIR << ")..." << endl;
             try {
                 if (fs::exists(CACHE_DIR)) fs::remove_all(CACHE_DIR);
                 if (fs::exists(LOCK_FILE)) fs::remove(LOCK_FILE);
                 cout << "[SUCCESS] Cache cleaned." << endl;
             } catch (const fs::filesystem_error& e) {
                 cerr << "[ERROR] Failed to clean cache: " << e.what() << endl;
                 return 1;
             }
             return 0;
        }
        cout << "Usage: glupe clean cache" << endl;
        return 1;
    }
    
    // UTILS COMMANDS
    if (cmd == "get-key" || cmd == "new-key") {
        openApiKeyPage();
        return 0;
    }

    // FIX COMMAND
    if (cmd == "fix") {
        if (argc < 4) {
            cout << "Usage: glupe fix <file> \"instruction\" [-cloud/-local]" << endl;
            return 1;
        }
        string targetFile = argv[2];
        string instruction = argv[3];
        string mode = "local"; 

        for(int i=4; i<argc; i++) {
            string arg = argv[i];
            if (arg == "-cloud") mode = "cloud";
            else if (arg == "-local") mode = "local";
        }

        if (!loadConfig(mode)) return 1;

        if (!fs::exists(targetFile)) {
            cout << "[ERROR] File not found: " << targetFile << endl;
            return 1;
        }

        cout << "[FIX] Reading " << targetFile << "..." << endl;
        ifstream f(targetFile);
        string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        f.close();

        string ext = getExt(targetFile);
        string langName = "Code";
        for(auto const& [key, val] : LANG_DB) {
            if(val.extension == ext) { langName = val.name; break; }
        }

        cout << "[AI] Applying fix (" << mode << ")..." << endl;
        stringstream prompt;
        prompt << "ROLE: Expert " << langName << " developer.\n";
        prompt << "TASK: Fix the code based on the instruction.\n";
        prompt << "INSTRUCTION: " << instruction << "\n";
        prompt << "CODE:\n" << content << "\n";
        prompt << "OUTPUT: Return ONLY the fixed code. No markdown. No explanations.";

        string response = callAI(prompt.str());
        string fixedCode = extractCode(response);

        if (fixedCode.find("ERROR:") == 0) {
            cout << "   [!] API Error: " << fixedCode.substr(6) << endl;
            return 1;
        }

        ofstream out(targetFile);
        out << fixedCode;
        out.close();
        
        cout << "[SUCCESS] File updated: " << targetFile << endl;
        return 0;
    }

    // SOS COMMAND (NEW)
    if (cmd == "sos") {
        if (argc < 3) {
            cout << "Usage: glupe sos [language] [-cloud/-local] \"error or problem description\"" << endl;
            return 1;
        }

        string language = "General Programming";
        string query = "";
        string mode = "local";

        // Parse arguments flexibly
        for (int i = 2; i < argc; i++) {
            string arg = argv[i];
            if (arg == "-cloud") mode = "cloud";
            else if (arg == "-local") mode = "local";
            else if (i == argc - 1) query = arg; // Assume last arg is query if not flag
            else language = arg; // Assume intermediate arg is language
        }

        if (query.empty()) {
            cout << "[ERROR] Please provide a problem description or error message." << endl;
            return 1;
        }

        if (!loadConfig(mode)) return 1;

        cout << "[SOS] Consulting AI (" << mode << ") about " << language << "..." << endl;
        
        stringstream prompt;
        prompt << "ROLE: Senior Software Architect & Technical Lead.\n";
        prompt << "TASK: Provide a clear, concise, and accurate solution for the user's problem.\n";
        prompt << "CONTEXT/LANGUAGE: " << language << "\n";
        prompt << "USER PROBLEM: " << query << "\n";
        prompt << "OUTPUT: Markdown formatted response. Be helpful and direct. Provide code snippets if necessary.";

        string response = callAI(prompt.str());
        
        // Manual JSON parsing to get full text (not just code blocks)
        string answer = response;
        try {
            json j = json::parse(response);
            if (j.contains("choices") && !j["choices"].empty()) {
                 if (j["choices"][0].contains("message")) answer = j["choices"][0]["message"]["content"];
                 else if (j["choices"][0].contains("text")) answer = j["choices"][0]["text"];
            }
            else if (j.contains("candidates") && !j["candidates"].empty()) {
                 answer = j["candidates"][0]["content"]["parts"][0]["text"];
            }
            else if (j.contains("response")) answer = j["response"];
        } catch(...) {
            // If parsing fails, use raw response (might be raw text from some endpoints)
        }

        cout << "\n--- GLUPE SOS REPLY ---\n";
        cout << answer << endl;
        cout << "----------------------\n";
        return 0;
    }

    // CHECK COMMAND
    if (cmd == "check") {
        if (argc < 3) {
            cout << "Usage: glupe check <file>" << endl;
            return 1;
        }
        string targetFile = argv[2];
        if (!fs::exists(targetFile)) {
            cout << "[ERROR] File not found: " << targetFile << endl;
            return 1;
        }
        
        ifstream f(targetFile);
        string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        f.close();

        cout << "[CHECK] Validating syntax for " << targetFile << "..." << endl;
        if (validateContainers(content)) {
            cout << "[SUCCESS] Syntax OK. Containers are valid." << endl;
            return 0;
        } else {
            cout << "[FAIL] Syntax errors detected." << endl;
            return 1;
        }
    }

    // UPDATE COMMAND
    if (cmd == "update") {
        cout << "[UPDATE] Checking for new version..." << endl;
        
        #ifdef _WIN32
        string updateUrl = "https://raw.githubusercontent.com/M-MACHINE/glupe/main/scripts/update.ps1";
        string command = "powershell -Command \"irm " + updateUrl + " | iex\"";
        #else
        string updateUrl = "https://raw.githubusercontent.com/M-MACHINE/glupe/main/scripts/update.sh";
        string command = "curl -sSL " + updateUrl + " | bash";
        #endif

        cout << "   -> Executing update script from repository..." << endl;
        if (VERBOSE_MODE) {
            cout << "   [CMD] " << command << endl;
        }

        int result = system(command.c_str());

        if (result != 0) {
            cerr << "[ERROR] Update script failed with exit code: " << result << endl;
            cerr << "   Please try updating manually from the repository." << endl;
            return 1;
        }
        
        cout << "[SUCCESS] Glupe has been updated. Please restart your terminal." << endl;
        return 0;
    }

    // EXPLAIN COMMAND (Modified with Language Support)
    if (cmd == "explain") {
        if (argc < 3) {
            cout << "Usage: glupe explain <file> [-cloud/-local] [language]" << endl;
            return 1;
        }
        string targetFile = argv[2];
        string mode = "local"; 
        string language = "English"; // Default

        for(int i=3; i<argc; i++) {
            string arg = argv[i];
            if (arg == "-cloud") mode = "cloud";
            else if (arg == "-local") mode = "local";
            else language = arg;
        }

        if (!loadConfig(mode)) return 1;

        if (!fs::exists(targetFile)) {
            cout << "[ERROR] File not found: " << targetFile << endl;
            return 1;
        }

        cout << "[EXPLAIN] Reading " << targetFile << "..." << endl;
        ifstream f(targetFile);
        string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        f.close();

        cout << "[AI] Generating documentation in " << language << " (" << mode << ")..." << endl;
        stringstream prompt;
        prompt << "TASK: Add high-quality technical documentation comments to the provided code in " << language << ".\n";
        prompt << "STRICT RULES:\n";
        prompt << "1. RETURN THE FULL SOURCE CODE exactly as provided but with added comments.\n";
        prompt << "2. DO NOT simplify the code. DO NOT replace it with examples like 'Hello World'.\n";
        prompt << "3. Use the language's standard comment syntax.\n";
        prompt << "4. Document functions, logic blocks, and variables.\n";
        prompt << "5. Ensure all comments are written in " << language << ".\n";
        prompt << "6. Return ONLY the code in a markdown block.\n\n";
        prompt << "CODE TO DOCUMENT:\n" << content;

        string response = callAI(prompt.str());
        string docCode = extractCode(response);

        if (docCode.find("ERROR:") == 0) {
            cout << "   [!] API Error: " << docCode.substr(6) << endl;
            return 1;
        }

        string ext = getExt(targetFile);
        string docFile = stripExt(targetFile) + "_doc" + ext;
        
        ofstream out(docFile);
        out << docCode;
        out.close();
        
        cout << "[SUCCESS] Documentation generated: " << docFile << endl;
        return 0;
    }

    // DIFF COMMAND
    if (cmd == "diff") {
        if (argc < 4) {
            cout << "Usage: glupe diff <fileA> <fileB> [-cloud/-local] [language]" << endl;
            return 1;
        }
        string fileA = argv[2];
        string fileB = argv[3];
        string mode = "local";
        string language = "English";

        for(int i=4; i<argc; i++) {
            string arg = argv[i];
            if (arg == "-cloud") mode = "cloud";
            else if (arg == "-local") mode = "local";
            else language = arg;
        }

        if (!loadConfig(mode)) return 1;

        if (!fs::exists(fileA) || !fs::exists(fileB)) {
            cout << "[ERROR] One or both files not found." << endl;
            return 1;
        }

        cout << "[DIFF] Comparing " << fileA << " vs " << fileB << "..." << endl;
        
        ifstream fa(fileA), fb(fileB);
        string contentA((istreambuf_iterator<char>(fa)), istreambuf_iterator<char>());
        string contentB((istreambuf_iterator<char>(fb)), istreambuf_iterator<char>());
        fa.close(); fb.close();

        stringstream prompt;
        prompt << "ROLE: Expert Software Auditor.\n";
        prompt << "TASK: Compare two source files and generate a semantic diff report in " << language << ".\n";
        prompt << "REPORT FORMAT:\n";
        prompt << "1. Brief Summary of changes.\n";
        prompt << "2. Changed Functions (What changed and where).\n";
        prompt << "3. Additional Observations (Potential bugs, improvements).\n";
        prompt << "RETURN: Only the report in Markdown format.\n\n";
        prompt << "--- FILE A (" << fileA << ") ---\n" << contentA << "\n";
        prompt << "\n--- FILE B (" << fileB << ") ---\n" << contentB << "\n";

        cout << "[AI] Analyzing changes (" << mode << ")..." << endl;
        string res = callAI(prompt.str());
        // For diff, we don't strict extract code blocks as the output IS the report (text)
        string report = res;
        try {
            json j = json::parse(res);
            if (j.contains("choices")) report = j["choices"][0]["message"]["content"];
            else if (j.contains("candidates")) report = j["candidates"][0]["content"]["parts"][0]["text"];
            else if (j.contains("response")) report = j["response"];
        } catch(...) {}

        string outName = fs::path(fileA).stem().string() + "_" + fs::path(fileB).stem().string() + "_diff_report.md";
        ofstream out(outName);
        out << report;
        out.close();

        cout << "[SUCCESS] Report generated: " << outName << endl;
        return 0;
    }

    // INFO COMMAND
    if (cmd == "info") {
        if (argc < 3) {
            cout << "Usage: glupe info <file.glp>" << endl;
            return 1;
        }
        showMetadata(argv[2]);
        return 0;
    }

    // INSERT-METADATA COMMAND
    if (cmd == "insert-metadata") {
        if (argc < 3) {
            cout << "Usage: glupe insert-metadata <path>" << endl;
            return 1;
        }
        string targetFile = argv[2];
        string stem = fs::path(targetFile).stem().string();
        
        stringstream meta;
        meta << "META_START\n{\n";
        meta << "    \"name\": \"" << stem << "\",\n";
        meta << "    \"version\": \"1.0.0\",\n";
        meta << "    \"author\": \"user\",\n";
        meta << "    \"intent\": \"Description...\",\n";
        meta << "    \"tags\": [],\n";
        meta << "    \"license\": \"MIT\",\n";
        meta << "    \"documentation\": []\n";
        meta << "}\nMETA_END\n\n";

        if (fs::exists(targetFile)) {
            ifstream f(targetFile);
            string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
            f.close();
            
            if (content.find("META_START") != string::npos) {
                cout << "[WARN] Metadata block already exists in " << targetFile << endl;
                return 0;
            }

            ofstream out(targetFile);
            out << meta.str() << content;
            out.close();
            cout << "[SUCCESS] Metadata prepended to " << targetFile << endl;
        } else {
            fs::path p(targetFile);
            if (p.has_parent_path() && !fs::exists(p.parent_path())) {
                fs::create_directories(p.parent_path());
            }
            ofstream out(targetFile);
            out << meta.str();
            out.close();
            cout << "[SUCCESS] Created " << targetFile << " with metadata template." << endl;
        }
        return 0;
    }

    // SIGNUP COMMAND
    if (cmd == "signup") {
        string url = (argc >= 3) ? argv[2] : "https://glupehub.up.railway.app";
        string username, password, confirm_pass;

        cout << "--- Glupe Sign Up ---" << endl;
        
        // 1. Username
        while (true) {
            cout << "Username: "; cin >> username;
            string checkCmd = "curl -s \"" + url + "/auth/check_username?q=" + username + "\"";
            CmdResult res = execCmd(checkCmd);
            try {
                json j = json::parse(res.output);
                if (j.value("available", false)) break;
                cout << "[ERROR] Username taken. Try another." << endl;
            } catch (...) {
                cout << "[ERROR] Could not verify username availability." << endl;
                return 1;
            }
        }

        // 2. Password
        while (true) {
            cout << "Password: "; cin >> password;
            cout << "Confirm Password: "; cin >> confirm_pass;
            if (password == confirm_pass) break;
            cout << "[ERROR] Passwords do not match." << endl;
        }

        // Send Signup Request
        json body;
        body["username"] = username;
        body["password"] = password;

        ofstream f("signup_temp.json"); f << body.dump(); f.close();
        string curlCmd = "curl -s -X POST -H \"Content-Type: application/json\" -d @signup_temp.json \"" + url + "/auth/signup\"";
        CmdResult res = execCmd(curlCmd);
        remove("signup_temp.json");

        try {
            json j = json::parse(res.output);
            if (j.contains("error")) {
                cout << "[ERROR] " << j["error"].get<string>() << endl;
                return 1;
            }
            if (j.value("status", "") == "success") {
                cout << "[SUCCESS] Account created! You can now login." << endl;
            } else {
                cout << "[ERROR] Signup failed: " << res.output << endl;
                return 1;
            }
        } catch (...) {
            cout << "[ERROR] Server error: Failed to parse server response." << endl;
            if (!res.output.empty()) {
                cout << "       Raw Response: " << res.output << endl;
            } else {
                cout << "       Raw Response: <empty>" << endl;
            }
            cout << "cURL exit code: " << res.exitCode << endl;
            cout << "(A non-zero cURL exit code suggests a network issue or that the server could not be reached.)" << endl;
            return 1;
        }
        return 0;
    }

    // LOGIN COMMAND
    if (cmd == "login") {
        string url = (argc >= 3) ? argv[2] : "https://glupehub.up.railway.app";
        string username, password;
        
        cout << "Username: "; cin >> username;
        cout << "Password: "; cin >> password;

        json body;
        body["username"] = username;
        body["password"] = password;

        ofstream f("login_temp.json"); f << body.dump(); f.close();
        string curlCmd = "curl -s -X POST -H \"Content-Type: application/json\" -d @login_temp.json \"" + url + "/login\"";
        CmdResult res = execCmd(curlCmd);
        remove("login_temp.json");

        try {
            json response = json::parse(res.output);
            if (response.contains("token")) saveSession(response["token"], username);
            else if (response.value("status", "") == "success") saveSession("dummy_token", username); // Fallback
            else cout << "[ERROR] Login failed: " << (response.contains("error") ? response["error"].get<string>() : res.output) << endl;
        } catch (...) { cout << "[ERROR] Invalid response: " << res.output << endl; }
        return 0;
    }

    // LOGOUT COMMAND
    if (cmd == "logout") {
        if (fs::exists(SESSION_FILE)) {
            fs::remove(SESSION_FILE);
            cout << "[SUCCESS] Logged out." << endl;
        } else {
            cout << "[INFO] You are not logged in." << endl;
        }
        return 0;
    }

    // WHOAMI COMMAND
    if (cmd == "whoami") {
        auto session = getSession();
        if (session.second.empty()) {
            cout << "Not logged in." << endl;
        } else {
            cout << "Logged in as: " << session.second << endl;
        }
        return 0;
    }

    // HUB COMMAND
    if (cmd == "hub") {
        if (!checkLogin()) {
            return 1;
        }
        // Clear cin buffer in case of leftover newlines from other commands
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        startInteractiveHub();
        return 0;
    }

    // PUSH COMMAND
    if (cmd == "push") {
        if (argc < 3) {
            cout << "Usage: glupe push <file> [tags] [url]" << endl;
            return 1;
        }
        string filename = argv[2];
        string tags = "";
        string url = "https://glupehub.up.railway.app";

        if (argc >= 4) {
            string arg3 = argv[3];
            if (arg3.rfind("http", 0) != 0) {
                tags = arg3;
                if (argc >= 5) {
                    url = argv[4];
                }
            } else {
                url = arg3;
            }
        }

        if (!fs::exists(filename)) { cout << "[ERROR] File not found: " << filename << endl; return 1; }

        string ext = getExt(filename);
        if (ext != ".txt" && ext != ".glp" && ext != ".glupe" && ext != ".md") {
            cout << "[ERROR] Invalid file type. Only .txt, .glp, .glupe, and .md files are allowed in the hub." << endl;
            return 1;
        }

        auto session = getSession();
        if (session.first.empty()) { cout << "Error: Not logged in. Run 'glupe login' first." << endl; return 1; }

        cout << "[PUSH] Uploading " << filename << " as " << session.second << (tags.empty() ? "" : " with tags...") << endl;
        string curlCmd = "curl -sS -X POST -H \"Authorization: Bearer " + session.first + "\" -F \"file=@" + filename + "\" -F \"author=" + session.second + "\"" + (tags.empty() ? "" : " -F \"tags=" + tags + "\"") + " \"" + url + "/push\"";
        
        CmdResult res = execCmd(curlCmd);
        cout << "Server Response: " << res.output << endl;
        
        try {
            json j = json::parse(res.output);
            if (j.contains("error")) {
                cout << "[ERROR] Hub: " << j["error"].get<string>() << endl;
                return 1;
            }
            if (j.contains("message")) cout << "[SUCCESS] " << j["message"].get<string>() << endl;
        } catch (...) {
            cout << "[RESPONSE] " << res.output << endl;
        }
        return 0;
    }

    // PULL COMMAND
    if (cmd == "pull") {
        if (argc < 4) {
            cout << "Usage: glupe pull <file> <username> [url]" << endl;
            return 1;
        }
        string filename = argv[2];
        string username = argv[3];
        string url = (argc >= 5) ? argv[4] : "https://glupehub.up.railway.app/";

        string targetUrl = url + "/pull/" + username + "/" + filename;
        cout << "[PULL] Downloading " << filename << " from " << username << "..." << endl;

        string curlCmd = "curl -sS -L -w \"%{http_code}\" -o \"" + filename + "\" \"" + targetUrl + "\"";
        CmdResult res = execCmd(curlCmd);

        if (res.exitCode != 0) {
            cout << "[ERROR] Download failed: " << res.output << endl;
            return 1;
        }

        int httpCode = 0;
        try { httpCode = stoi(res.output); } catch (...) {}

        if (httpCode >= 400) {
            cout << "[ERROR] Server returned HTTP " << httpCode << endl;
            if (fs::exists(filename)) {
                ifstream f(filename);
                string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
                cout << "Server Response: " << content << endl;
                f.close();
                fs::remove(filename);
            }
            return 1;
        }
        
        cout << "[SUCCESS] Saved " << filename << endl;
        return 0;
    }

    // EDIT COMMAND
    if (cmd == "edit") {
        if (argc < 6) {
            cout << "Usage: glupe edit <file> --container <name> \"<new prompt>\"" << endl;
            return 1;
        }
        string targetFile = argv[2];
        string containerFlag = argv[3];
        string containerName = argv[4];
        string newPrompt = argv[5];

        if (containerFlag != "--container") {
            cout << "Usage: glupe edit <file> --container <name> \"<new prompt>\"" << endl;
            return 1;
        }

        if (!fs::exists(targetFile)) {
            cout << "[ERROR] File not found: " << targetFile << endl;
            return 1;
        }

        ifstream f(targetFile);
        string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
        f.close();

        string newContent;
        size_t pos = 0;
        bool found = false;
        while ((pos = content.find("$$", pos)) != string::npos) {
            size_t scan = pos + 2;
            while (scan < content.length() && isspace(content[scan])) scan++;

            if (content.substr(scan, 8) == "ABSTRACT") {
                scan += 8;
                while (scan < content.length() && isspace(content[scan])) scan++;
            }

            string currentId;
            if (content[scan] == '"') {
                size_t idStart = scan + 1;
                size_t idEnd = content.find('"', idStart);
                if (idEnd != string::npos) {
                    currentId = content.substr(idStart, idEnd - idStart);
                    scan = idEnd + 1;
                }
            } else {
                size_t idStart = scan;
                while (scan < content.length() && !isspace(content[scan]) && content[scan] != '{' && content.substr(scan, 2) != "->") {
                    scan++;
                }
                currentId = content.substr(idStart, scan - idStart);
            }

            if (currentId == containerName) {
                while (scan < content.length() && isspace(content[scan])) scan++;
                if (content.substr(scan, 2) == "->") {
                    scan += 2;
                    while (scan < content.length() && content[scan] != '{') scan++;
                }
                while (scan < content.length() && isspace(content[scan])) scan++;

                if (content[scan] == '{') {
                    size_t contentStart = scan + 1;
                    size_t contentEnd = content.find("}$$", contentStart);
                    if (contentEnd != string::npos) {
                        string before = content.substr(0, contentStart);
                        string after = content.substr(contentEnd);
                        string originalPrompt = content.substr(contentStart, contentEnd - contentStart);
                        size_t firstChar = originalPrompt.find_first_not_of(" \t\r\n");
                        string padding = (firstChar != string::npos) ? originalPrompt.substr(0, firstChar) : "\n    ";
                        newContent = before + padding + newPrompt + padding + after;
                        found = true;
                        break;
                    }
                }
            }
            pos++;
        }

        if (!found) {
            cout << "[ERROR] Container '" << containerName << "' not found in " << targetFile << endl;
            return 1;
        }

        ofstream out(targetFile);
        out << newContent;
        out.close();

        cout << "[SUCCESS] Container '" << containerName << "' in " << targetFile << " updated." << endl;
        return 0;
    }

    // --- STANDARD COMPILATION LOGIC ---
    vector<string> positionalArgs;
    vector<string> inputFiles;
    vector<string> updateTargets;
    string outputName = "";
    string mode = "local"; 
    string customBuildCmd = ""; // [NEW] Custom build command override
    string customInstructions = ""; 
    bool explicitLang = false;
    bool dryRun = false;
    bool updateMode = false; 
    bool runOutput = false;
    bool keepSource = false;
    bool transpileMode = false;
    bool makeMode = false;
    bool seriesMode = false;
    bool refineMode = false;
    bool blindMode = false;
    bool fillMode = false;

    for(int i=1; i<argc; i++) {
        string arg = argv[i];
        if (arg == "-o" && i+1 < argc) { outputName = argv[i+1]; i++; }
        else if (arg == "-cloud") mode = "cloud";
        else if (arg == "-local") mode = "local";
        else if (arg == "-dry-run") dryRun = true;
        else if (arg == "-verbose") VERBOSE_MODE = true;
        else if (arg == "-build" && i+1 < argc) { customBuildCmd = argv[i+1]; i++; }
        else if (arg == "-u" || arg == "--update") updateMode = true;
        else if (arg == "-run" || arg == "--run") runOutput = true;
        else if (arg == "-k" || arg == "--keep") keepSource = true;
        else if (arg == "-t" || arg == "--transpile") transpileMode = true;
        else if (arg == "-make") makeMode = true;
        else if (arg == "-series") seriesMode = true;
        else if (arg == "-refine") refineMode = true;
        else if (arg == "-fill") fillMode = true;
        else if (arg == "-crono") cronoTimer.enabled = true;
        else if (arg == "-3d") CURRENT_MODE = GenMode::MODEL_3D;
        else if (arg == "-img") CURRENT_MODE = GenMode::IMAGE;
        else if (arg == "-code") CURRENT_MODE = GenMode::CODE;
        else if (arg == "--version") { cout << "Glupe Compiler v" << CURRENT_VERSION << endl; return 0; }
        else if (arg == "--clean") {
            cout << "[CLEAN] Removing temporary build files..." << endl;
            try {
                if (fs::exists(".glupe_build.cache")) fs::remove(".glupe_build.cache");
                for (const auto& entry : fs::directory_iterator(fs::current_path())) {
                    if (entry.is_regular_file()) {
                        string fname = entry.path().filename().string();
                        if (fname.find("temp_build") == 0) fs::remove(entry.path());
                    }
                }
            } catch (...) {}
            return 0;
        }
        else if (arg == "--init") {
            cout << "[INIT] Creating project template..." << endl;
            if (!fs::exists("hello.glp")) {
                ofstream f("hello.glp");
                f << "// Welcome to Glupe!\nPRINT(\"Hello, World!\")\n";
                f.close();
            }
            if (!fs::exists("config.json")) {
                ofstream f("config.json");
                f << "{\n    \"local\": {\n        \"model_id\": \"qwen2.5-coder:3b\",\n        \"api_url\": \"http://localhost:11434/api/generate\"\n    },\n    \"cloud\": {\n        \"protocol\": \"openai\",\n        \"api_key\": \"YOUR_KEY\",\n        \"model_id\": \"llama3-70b-8192\",\n        \"api_url\": \"https://api.groq.com/openai/v1/chat/completions\"\n    }\n}\n";
                f.close();
            }
            return 0;
        }
        else if (arg == "--help" || arg == "-h") {
            showHelp();
            return 0;
        }
        else if (arg[0] == '-') {
            string langKey = arg.substr(1);
            if (LANG_DB.count(langKey)) { CURRENT_LANG = LANG_DB[langKey]; explicitLang = true; }
            else if (MODEL_DB.count(langKey)) { CURRENT_LANG = MODEL_DB[langKey]; explicitLang = true; CURRENT_MODE = GenMode::MODEL_3D; }
            else if (IMAGE_DB.count(langKey)) { CURRENT_LANG = IMAGE_DB[langKey]; explicitLang = true; CURRENT_MODE = GenMode::IMAGE; }
        }
        else {
            // Instruction Detection
            if (arg.size() > 0 && arg[0] == '*') {
                customInstructions = arg.substr(1);
                cout << "[INFO] Custom instructions detected." << endl;
            } else {
                positionalArgs.push_back(arg); 
            }
        }
    }

    // [FIX] Separate input files from update targets
    for(const auto& arg : positionalArgs) {
        if (fs::exists(arg)) {
            inputFiles.push_back(arg);
        } else if (updateMode) {
            updateTargets.push_back(arg);
        } else {
            inputFiles.push_back(arg); // Let validation fail later
        }
    }

    if (inputFiles.empty()) { cerr << "No input files." << endl; return 1; }
    if (!loadConfig(mode)) return 1;

    // [NEW] Refine Mode: Semantic Compression
    if (refineMode) {
        for (const auto& file : inputFiles) {
            cout << "[REFINE] Processing " << file << "..." << endl;
            if (!fs::exists(file)) {
                cout << "[ERROR] File not found: " << file << endl;
                continue;
            }
            
            ifstream f(file);
            string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
            f.close();

            // [UPDATED] Sliding Window Logic
            vector<string> chunks = splitSourceCode(content);
            
            string fullRefinedCode = "";
            string previousContext = "";

            cout << "[AI] Semantic compression (" << mode << ") - " << chunks.size() << " chunks..." << endl;

            for (size_t i = 0; i < chunks.size(); ++i) {
               cout << "   -> Processing chunk " << (i + 1) << "/" << chunks.size() << "..." << endl;
                cout << "   -> Processing chunk " << (i + 1) << "/" << chunks.size() << "..." << endl;
                
                bool isSpaghetti = detectIfCodeIsSpaghetti(chunks[i]);

            bool isSpaghetti = detectIfCodeIsSpaghetti(chunks[i]);
                stringstream prompt;

                // 1. Define contextual variables upfront to avoid messy branching
                string roleDesc = isSpaghetti ? "Expert Legacy Code Refactorer" : "Senior Systems Architect";
                string taskDesc = isSpaghetti ? "Refactor MESSY/LEGACY code into a clean semantic blueprint (.glp) DO NOT OVERSIMPLIFY";
                                            : "Transpile the source code into a high-fidelity semantic blueprint (.glp)";
                string logicRule = isSpaghetti ? "- Untangle patterns (goto/nesting). Preserve BUSINESS INTENT.";
                                            : "- 1:1 Functional mapping. DO NOT omit any logic.";

                if (isSpaghetti) {
                    cout << "      [!] Spaghetti Code Detected. Enabling Refactoring Mode." << endl;
                    prompt << "ROLE: Expert Legacy Code Refactorer & Systems Architect.\n";
                    prompt << "TASK: Refactor the following MESSY/LEGACY code into a clean, modern semantic blueprint (.glp).\n";
                    prompt << "GOAL: Untangle logic, remove redundancy, and produce a professional structure while PRESERVING FUNCTIONALITY.\n";
                } else {
                    prompt << "ROLE: Senior Systems Engineer & Logic Architect.\n";
                    prompt << "TASK: Transpile the source code into a high fidelity semantic blueprint (.glp).\n";
                    prompt << "GOAL: Destill the implementation into functional blocks '$$ name -> parent { logic } $$' .\n";
                }

                // 2. Build the optimized, token-efficient prompt
                prompt << "ROLE: " << roleDesc << "\n";
                prompt << "TASK: " << taskDesc << "\n\n";
                prompt << "[SYNTAX_RULES]\n";
                prompt << "1. Block: $$ name -> parent1, parent2, ... { logic } $$ or if no parents $$ name { logic }$$\n";
                prompt << "1.1 includes have their own blocks, GOOD: \"standard map lib\", BAD: #include <map> \n";
                prompt << "1.2 Globals and constants have their own blocks\n";
                prompt << "2. Abstract: $$ABSTRACT name -> parent { logic }$$\n. Abstract blocks do not generate code, only influence on other blocks";
                prompt << "3. Inline: GOOD: $ name -> parent { logic }$, BAD: $ name { logic\\nlogic }$\n";
                prompt << "4. STRICT: NO NESTING BLOCKS, NO $${$${}$$}$$ or $${${}$}$$.\n";
                prompt << "5. DO NOT OVER SIMPLIFY. DO NOT OMIT LOGIC. DO NOT REPLACE WITH EXAMPLES or STUBS. every line must have its semantic representation\n";
                prompt << "Use inheritance (->) for relationships/calls.\n\n";
                prompt << "[LOGIC_RULES]\n";
                prompt << "- Format: Numbered algorithmic steps (1, 1.1, 1.2).\n";
                prompt << "- Style: Imperative verbs (Get, Set, Check). No prose.\n";
                prompt << logicRule << "\n";
                prompt << "- Intent Focus: Name blocks by Goal (e.g., 'FilterData') rather than syntax (e.g., 'Loop1').\n";
                prompt << "- Detail: Rewrite logic inside blocks using technical steps. Do not over-summarize.\n";
                prompt << "- Includes/Globals: Must have their own independent semantic blocks.\n";
                    if (i > 0) {
                        prompt << "\n[EXTERNAL_CONTEXT_FROM_PREVIOUS_PARTS]\n";
                        // Extraemos solo firmas y globales del contexto previo para no saturar la memoria
                        prompt << "Existing Signatures/Globals: " << extractSignatures(previousContext) << "\n";
                        prompt << "Maintain strict compatibility with these definitions.\n";
                    }

                if (i > 0) {
                    prompt << "\n[CONTEXT_SYNC]\n";
                    prompt << "Existing Signatures/Globals: " << extractSignatures(previousContext) << "\n";
                    prompt << "Maintain STRICT compatibility with these definitions.\n";
                }
                    prompt << "\n[STRICT_RULES]\n";
                    if (isSpaghetti) {
                        prompt << "0. REFACTOR BAD PATTERNS. Replace 'goto' with loops/control structures. Flatten deep nesting. Use meaningful names.\n";
                    }

                prompt << "\n[OUTPUT_FORMAT]\n";
                    prompt << "RETURN ONLY the .glp fragment. NO conversation. NO markdown code blocks.\n\n";
                    prompt << "[SOURCE_CODE_PART_" << (i + 1) << "]\n";
                    prompt << "When refining code into intent, use a numbered algorithmic format. Use standard indentation for nested logic (1, 1.1, 1.2). Do not use prose. Use imperative verbs (Get, Set, Check, Return).";
                    prompt << "Semantic blocks should represent functions, classes, and logical groupings of code. They should not be arbitrary line groupings.\n";
                    prompt << "Do not nest semantic blocks: BAD: $$ block1 { logic $$ block2 { logic } $$ }$$. GOOD: $$ block1 { logic }$$ $$ block2 { logic }$$\n";
                    prompt << "Semantic blocks support inheritence throguh this syntax $$ child -> parent { logic }$$. Use it to express function calls, class inheritance";
                    prompt << "Blocks can be abstract, express them though '$$ABSTRACT name -> parent {logic}$$, abstract blocks do not produce code, only influence other blockss\n";
                    prompt << "For single-line logic, use inline containers: $ name -> parent { logic }. These behave like standard containers but must be on a single line.\n";
                    
                    if (!isSpaghetti) prompt << "1. DO NOT OMIT ANY LOGIC. Every line of code must have a representation in the semantic blueprint.\n";
                    else prompt << "1. DO NOT OMIT BUSINESS LOGIC. Preserve all functionality, but restructure the implementation details to be clean.\n";
                    
                    prompt << "2. DO NOT OVER SUMMARIZE. Rewrite the logic inside blocks using technical steps.\n";
                    prompt << "3. PRIORITIZE LOGIC OVER SYNTAX. If code contains nested loops or unclear structure, do NOT just list the variables. Instead, determine the Goal of the loop (e.g., 'Count Items', 'Filter Data') and create a block named after that goal. Ignore individual variable declarations if they are part of a larger algorithm.";
                    prompt << "   BAD: $$ init { Set up the system } $$ <- too vague\n";
                    prompt << "   GOOD: $$ init { 1. Open database at DB_URL, 2. verify 'users' table exists, 3. and initialize session_map } $$\n";
                    
                    prompt << "4. Represent each function with a semantic block $$ block_name { ... }$$.\n";
                    prompt << "5. PRESERVE all #include, constants, and global variable declarations in their own semantic blocks\n";
                    prompt << "6. Return ONLY the .glp fragment for this part. No conversation. No markdown code blocks.\n";

                    prompt << "\n[SOURCE_CODE_PART_" << (i+1) << "]\n";
                    prompt << chunks[i] << "\n";

                string refinedChunk;
                bool success = false;
                int retries = 0;

                while (retries < MAX_ING BLOCKS, "NO $${$${}$$}$$ or $${${}$}$$.\n"; RETRIES) {
                while (retries < MAX_RETRIES) {
                    string response = callAI(prompt.str());
                    refinedChunk = extractCode(response);

                    if (refinedChunk.find("ERROR:") == 0) {
                        cout << "   [!] API Error on chunk " << (i+1) << " (Attempt " << (retries + 1) << "/" << MAX_RETRIES << "): " << refinedChunk.substr(6) << endl;
                        string errorMsg = refinedChunk.substr(6);
                        cout << "   [!] API Error on chunk " << (i+1) << " (Attempt " << (retries + 1) << "/" << MAX_RETRIES << "): " << errorMsg << endl;
                        
                        int waitTime = (1 << retries) * 2; // Exponential backoff

                        // [FIX] Smart wait: Parse "wait X seconds" from error message
                        size_t waitPos = errorMsg.find("wait ");
                        if (waitPos != string::npos) {
                            try {
                                int parsedWait = stoi(errorMsg.substr(waitPos + 5));
                                if (parsedWait > 0) waitTime = parsedWait + 2; // +2s buffer
                            } catch(...) {}
                        }

                        cout << "       -> Retrying in " << waitTime << "s..." << endl;
                        std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                        retries++;
                    } else {
                        success = true;
                        break;
                    }
                }

                if (!success) {
                    cout << "   [FATAL] Failed to refine chunk " << (i+1) << " after " << MAX_RETRIES << " attempts. Aborting operation." << endl;
                    return 1;
                }

                fullRefinedCode += refinedChunk + "\n";
                previousContext = refinedChunk; // Update context for next iteration
            }

            // [NEW] Sanitize syntax before saving
            fullRefinedCode = sanitize_container_syntax(fullRefinedCode);

            string outputFile = file + ".glp";
            ofstream out(outputFile);
            out << fullRefinedCode;
            out.close();

            cout << "[SUCCESS] Semantic file generated: " << outputFile << endl;
        }
        return 0;
    }

    if (!explicitLang) {
        if (outputName.empty()) {
             if (makeMode) CURRENT_LANG = LANG_DB["glp"];
             else CURRENT_LANG = (CURRENT_MODE == GenMode::CODE) ? LANG_DB["cpp"] : (CURRENT_MODE == GenMode::MODEL_3D ? MODEL_DB["obj"] : IMAGE_DB["svg"]); 
        } else {
            string ext = getExt(outputName);
            bool found = false;
            for (auto const& [key, val] : LANG_DB) {
                if (val.extension == ext) { CURRENT_LANG = val; explicitLang=true; found=true; CURRENT_MODE = GenMode::CODE; break; }
            }
            if (!found) {
                for (auto const& [key, val] : MODEL_DB) {
                    if (val.extension == ext) { CURRENT_LANG = val; explicitLang=true; found=true; CURRENT_MODE = GenMode::MODEL_3D; break; }
                }
            }
            if (!found) {
                for (auto const& [key, val] : IMAGE_DB) {
                    if (val.extension == ext) { CURRENT_LANG = val; explicitLang=true; found=true; CURRENT_MODE = GenMode::IMAGE; break; }
                }
            }
            if (!explicitLang) selectTarget();
        }
    }

    // [FIX] Smart default output name:
    // If language produces binary and we are NOT in transpile-only mode, default to executable extension.
    if (outputName.empty()) {
        string baseName = stripExt(inputFiles[0]);
        if (CURRENT_LANG.producesBinary && !transpileMode) {
            #ifdef _WIN32
            outputName = baseName + ".exe";
            #else
            outputName = baseName;
            #endif
        } else {
            outputName = baseName + CURRENT_LANG.extension;
        }
    }
    
    if (CURRENT_MODE == GenMode::CODE) {
        if (!makeMode || explicitLang) {
            cout << "[CHECK] Toolchain for " << CURRENT_LANG.name << "..." << endl;
            if (CURRENT_LANG.versionCmd.empty()) {
                cout << "   [INFO] No toolchain required." << endl;
            } else if (execCmd(CURRENT_LANG.versionCmd).exitCode != 0) {
                cout << "   [!] Toolchain not found (" << CURRENT_LANG.versionCmd << "). Blind Mode." << endl;
                blindMode = true;
            } else cout << "   [OK] Ready." << endl;
        }
    } else if (CURRENT_MODE == GenMode::MODEL_3D) {
        cout << "[MODE] 3D Generation (" << CURRENT_LANG.name << ")" << endl;
    } else {
        cout << "[MODE] Image Generation (" << CURRENT_LANG.name << ")" << endl;
    }

    string aggregatedContext = "";
    vector<string> stack;
    
    // [FIX] Store processed files to export them only after validation
    struct InputData {
        string content;
        fs::path path;
    };
    vector<InputData> loadedInputs;

    for (const auto& file : inputFiles) {
        fs::path p(file);
        if (fs::exists(p)) {
            ifstream f(p);
            string raw((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
            
            f.close();

            string decommented = decommentGlupeSyntax(raw);

            string cleanRaw = stripMetadata(decommented);
            string resolved = resolveImports(cleanRaw, p.parent_path(), stack);
            
            // [AUTO-DETECT] Enable makeMode if EXPORT is detected
            if (resolved.find("EXPORT:") != string::npos && !makeMode && !seriesMode) {
                cout << "[INFO] 'EXPORT:' directive detected. Auto-enabling Architect Mode (-make)." << endl;
                makeMode = true;
            }

            // [MOVED] processExports call moved after validation
            loadedInputs.push_back({resolved, p.parent_path()});

            aggregatedContext += "\n// --- START FILE: " + file + " ---\n";
            aggregatedContext += resolved;
            aggregatedContext += "\n// --- END FILE: " + file + " ---\n";
        } else {
            cerr << "Error: File not found: " << file << endl;
            return 1;
        }
    }

    // [NEW] Validate containers globally before processing
    bool hasActiveContainers = false;
    if (!validateContainers(aggregatedContext, &hasActiveContainers)) return 1;

    // [FIX] Now it is safe to write initial exports (if any)
    for (const auto& data : loadedInputs) {
        processExports(data.content, data.path);
    }

    // [NEW] Initialize Cache
    initCache();

    size_t currentHash = hash<string>{}(aggregatedContext + CURRENT_LANG.id + MODEL_ID + (updateMode ? "u" : "n") + customInstructions);
    string cacheFile = ".glupe_build.cache"; 

    if (!updateMode && !dryRun && fs::exists(cacheFile) && fs::exists(outputName)) {
        ifstream cFile(cacheFile);
        size_t storedHash;
        if (cFile >> storedHash && storedHash == currentHash) {
            cout << "[CACHE] No changes detected. Using existing build." << endl;
            if (runOutput) {
                #ifdef _WIN32
                system(outputName.c_str());
                #else
                string cmd = "./" + outputName; system(cmd.c_str());
                #endif
            }
            return 0;
        }
    }

    set<string> potentialDeps = extractDependencies(aggregatedContext);
    if (!preFlightCheck(potentialDeps)) return 1;

    // [NEW] Process Containers (Cache Check & Injection)
    // If updateMode is true, we try to use cache.
    aggregatedContext = processInputWithCache(aggregatedContext, updateMode, updateTargets, fillMode);

    // [SERIES MODE] Sequential Generation
    if (seriesMode) {
        cout << "[SERIES] Parsing blueprint for sequential generation..." << endl;
        auto blueprint = parseBlueprint(aggregatedContext);
        
        if (blueprint.empty()) {
            cout << "[WARN] No EXPORT blocks found for series mode." << endl;
        } else {
            string projectContext = "";
            int currentItem = 0;
            int totalItems = blueprint.size();
            auto seriesStart = std::chrono::high_resolution_clock::now();

            for (const auto& item : blueprint) {
                currentItem++;
                cout << "   [" << currentItem << "/" << totalItems << "] Generating " << item.filename << "..." << endl;
                
                stringstream prompt;
                prompt << "ROLE: " << (CURRENT_MODE == GenMode::CODE ? "Software Architect" : "Asset Generator") << ".\n";
                prompt << "TASK: Implement the file '" << item.filename << "'.\n";
                prompt << "CONTEXT:\n" << projectContext << "\n";
                prompt << "FILE INSTRUCTIONS:\n" << item.content << "\n";
                prompt << "RULES:\n";
                prompt << "1. Implement the full logic. No placeholders.\n";
                prompt << "2. IMPORTANT: If you see '// GLUPE_BLOCK_START: id', IMPLEMENT the logic between it and '// GLUPE_BLOCK_END: id'. PRESERVE these markers exactly in the output so they can be cached.\n";
                prompt << "OUTPUT: Return ONLY the valid code/content for " << item.filename << ". No markdown blocks if possible.";
                
                string code;
                bool success = false;
                int retries = 0;

                while (retries < MAX_RETRIES) {
                    string response = callAI(prompt.str());
                    code = extractCode(response);
                    
                    if (code.find("ERROR:") == 0) {
                        cout << "   [!] API Error (Attempt " << (retries + 1) << "/" << MAX_RETRIES << "): " << code.substr(6) << endl;
                        
                        int waitTime = 5 * (retries + 1);
                        if (code.find("Rate limit") != string::npos || code.find("429") != string::npos) {
                            cout << "       -> Rate limit detected. Waiting " << waitTime << "s..." << endl;
                        } else {
                            cout << "       -> Retrying in " << waitTime << "s..." << endl;
                        }
                        std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                        retries++;
                    } else {
                        success = true;
                        break;
                    }
                }

                if (!success) {
                    cout << "[FATAL] Failed to generate " << item.filename << " after " << MAX_RETRIES << " attempts. Aborting series." << endl;
                    return 1;
                }

                // [NEW] Update Cache from AI Output (Series Mode)
                code = updateCacheFromOutput(code);

                ofstream out(item.filename); out << code; out.close();
                
                // [NEW] Calculate and display ETA
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - seriesStart).count();
                double avg = (double)elapsed / currentItem;
                long long eta = (long long)(avg * (totalItems - currentItem));
                
                cout << "      -> Saved. (ETA: " << formatDuration(eta) << ")" << endl;
                projectContext += "\n// --- FILE: " + item.filename + " ---\n" + code + "\n";
            }
            cout << "[SERIES] All tasks completed." << endl;
            return 0;
        }
    }

    string existingCode = "";
    if (updateMode) {
        string srcPath = stripExt(outputName) + CURRENT_LANG.extension;
        if (fs::exists(srcPath)) {
            ifstream old(srcPath);
            existingCode.assign((istreambuf_iterator<char>(old)), istreambuf_iterator<char>());
            cout << "   [UPDATE] Found existing source: " << srcPath << endl;
        }
    }

    if (dryRun) { cout << "--- CONTEXT PREVIEW ---\n" << aggregatedContext << endl; return 0; }

    string tempSrc = "temp_build" + CURRENT_LANG.extension;
    string tempBin = "temp_build.exe"; 
    string errorHistory = ""; 

    // [FILL MODE] Skip global generation loop
    if (fillMode) {
        cout << "[FILL] Containers processed. Skipping global generation." << endl;
        ofstream out(tempSrc); out << aggregatedContext; out.close();
    } else {

    // [OPTIMIZATION] Direct Compilation for matching source files
    bool canDirectCompile = false;
    if (CURRENT_MODE == GenMode::CODE && CURRENT_LANG.producesBinary && 
        customInstructions.empty() && !updateMode && !transpileMode && !makeMode && !hasActiveContainers) {
        
        canDirectCompile = true;
        for (const auto& file : inputFiles) {
            if (getExt(file) != CURRENT_LANG.extension) {
                canDirectCompile = false;
                break;
            }
        }
    }

    if (canDirectCompile) {
        cout << "[DIRECT] Attempting direct compilation..." << endl;
        string fileList = "";
        for (const auto& file : inputFiles) fileList += "\"" + file + "\" ";
        if (!fileList.empty()) fileList.pop_back();

        string cmd = CURRENT_LANG.buildCmd + " " + fileList + " -o \"" + tempBin + "\"";
        if (VERBOSE_MODE) cout << "[CMD] " << cmd << endl;
        
        CmdResult build = execCmd(cmd);
        
        if (build.exitCode == 0) {
            cout << "[SUCCESS] Direct compilation succeeded." << endl;
            
            bool saveSuccess = false;
            for(int i=0; i<5; i++) {
                try {
                    if (fs::exists(outputName)) fs::remove(outputName);
                    fs::copy_file(tempBin, outputName, fs::copy_options::overwrite_existing);
                    saveSuccess = true;
                    break;
                } catch (...) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
            }
            
            if (fs::exists(tempBin)) fs::remove(tempBin);
            
            if (!saveSuccess) {
                 cerr << "[ERROR] Failed to save final output. File may be locked." << endl;
                 return 1;
            }
            
            if (runOutput) {
                cout << "\n[RUN] Executing..." << endl;
                string runCmd = outputName;
                #ifndef _WIN32
                if (runCmd.find('/') == string::npos) runCmd = "./" + runCmd;
                std::error_code ec;
                fs::permissions(outputName, fs::perms::owner_exec, fs::perm_options::add, ec);
                #endif
                string sysCmd = "\"" + runCmd + "\"";
                system(sysCmd.c_str());
            }
            return 0;
        } else {
            cout << "[WARN] Direct compilation failed. Falling back to AI repair..." << endl;
            errorHistory = "PREVIOUS COMPILATION ATTEMPT FAILED:\n" + build.output;
        }
    }
    
    int passes = MAX_RETRIES;
    
    for(int gen=1; gen<=passes; gen++) {
        if (makeMode) cout << "   [Pass " << gen << "] Architecting Project..." << endl;
        else cout << "   [Pass " << gen << "] Generating " << CURRENT_LANG.name << "..." << endl;
        
        stringstream prompt;
        
        if (CURRENT_MODE == GenMode::CODE) {
            if (makeMode) {
                prompt << "ROLE: Software Architect.\n";
                if (explicitLang) {
                    prompt << "TASK: Structure and implement the project files for a " << CURRENT_LANG.name << " project.\n";
                } else {
                    prompt << "TASK: Structure and implement the project files based on the provided instructions.\n";
                }
                prompt << "RULES:\n";
                prompt << "1. Use 'EXPORT: \"filename.ext\"' ... 'EXPORT: END' for every file.\n";
                prompt << "2. The language for each file is determined by its extension (e.g., '.py' for Python, '.c' for C). You MUST generate valid code for that specific language inside its EXPORT block.\n";
                prompt << "3. Implement the full logic/content. No placeholders.\n";
                prompt << "4. Process '$${ instructions }$$' templates by implementing the logic inside them.\n";
                prompt << "5. IMPORTANT: If you see '// GLUPE_BLOCK_START: id', IMPLEMENT the logic between it and '// GLUPE_BLOCK_END: id'. PRESERVE these markers exactly in the output so they can be cached.\n";
                prompt << "6. Output ONLY the EXPORT blocks. No conversation or other text.\n";
                prompt << "7. Do NOT perform web searches. Rely solely on your internal knowledge.\n";
            } else {
                // [UPDATED v5.1] STRONGER ROLE DEFINITION AND GUARDRAILS
                prompt << "ROLE: Semantic Transpiler.\n";
                prompt << "TASK: Convert input logic to a single valid " << CURRENT_LANG.name << " file.\n";
                prompt << "RULES:\n";
                prompt << "1. NO wrappers (e.g. calling other languages via system()). Re-implement logic natively in " << CURRENT_LANG.name << ".\n";
                prompt << "2. Use standard libraries/modules native to " << CURRENT_LANG.name << ".\n";
                prompt << "3. Output must be self-contained and runnable.\n";
                if (CURRENT_LANG.id == "arduino" || CURRENT_LANG.id == "esp32") {
                    prompt << "4. Use 'setup()' and 'loop()' entry points. Do NOT include 'main()'.\n";
                } else if (CURRENT_LANG.producesBinary) {
                    prompt << "4. Include a 'main' entry point.\n";
                }
                prompt << "5. IMPORTANT: If you see '// GLUPE_BLOCK_START: id', IMPLEMENT the logic between it and '// GLUPE_BLOCK_END: id'. PRESERVE these markers exactly in the output.\n";
                prompt << "6. No external language headers/imports unless standard.\n";
                prompt << "7. Preferably use training knowledge on " << CURRENT_LANG.name << "\n";
            }
        } else if (CURRENT_MODE == GenMode::MODEL_3D) {
            prompt << "ROLE: Expert 3D Technical Artist & Modeler.\n";
            prompt << "TASK: Generate a valid " << CURRENT_LANG.name << " file based on the description provided in the input files.\n";
            prompt << "CONSTRAINTS: Ensure valid syntax for " << CURRENT_LANG.extension << ". Output ONLY the file content.\n";
        } else {
            prompt << "ROLE: Expert Vector Graphics Artist & Technical Illustrator.\n";
            prompt << "TASK: Generate a valid " << CURRENT_LANG.name << " file based on the visual description.\n";
            prompt << "CONSTRAINTS: Ensure valid syntax for " << CURRENT_LANG.extension << ". Output ONLY the file content (e.g. <svg>...</svg>).\n";
        }
        
        if (!customInstructions.empty()) {
            prompt << "\n[USER INSTRUCTIONS - HIGHEST PRIORITY]:\n" << customInstructions << "\n";
        }

        if (updateMode && !existingCode.empty()) {
            prompt << "TASK: UPDATE existing code.\n";
            prompt << "\n--- [OLD CODE] ---\n" << existingCode << "\n--- [END OLD CODE] ---\n";
            prompt << "\n--- [NEW INPUTS] ---\n" << aggregatedContext << "\n--- [END NEW INPUTS] ---\n";
        } else {
            prompt << "TASK: Create SINGLE " << CURRENT_LANG.name << " file.\n";
            prompt << "\n--- INPUT SOURCES ---\n" << aggregatedContext << "\n--- END SOURCES ---\n";
        }
        if (!errorHistory.empty()) prompt << "\n[!] PREVIOUS ERRORS:\n" << errorHistory << "\n";
        prompt << "\nOUTPUT: Only code.";

        string code;
        bool apiSuccess = false;
        int apiRetries = 0;

        while (apiRetries < MAX_RETRIES) {
            string response = callAI(prompt.str());
            code = extractCode(response);
        
            if (code.find("ERROR:") == 0) { 
                cout << "   [!] API Error (Attempt " << (apiRetries + 1) << "/" << MAX_RETRIES << "): " << code.substr(6) << endl; 
                log("API_FAIL", code); 
                if (code.find("JSON Parsing Failed") != string::npos) {
                     cout << "       (Hint: Check 'glupe config cloud-protocol'. Current: " << PROTOCOL << ", Provider URL: " << API_URL << ")" << endl;
                }
                
                int waitTime = 5 * (apiRetries + 1);
                if (code.find("Rate limit") != string::npos || code.find("429") != string::npos) {
                    cout << "       -> Rate limit detected. Waiting " << waitTime << "s..." << endl;
                } else {
                    cout << "       -> Retrying in " << waitTime << "s..." << endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                apiRetries++;
            } else {
                apiSuccess = true;
                break;
            }
        }

        if (!apiSuccess) {
            cout << "   [FATAL] API failed after " << MAX_RETRIES << " attempts. Aborting." << endl;
            return 1;
        }

        // [NEW] Update Cache from AI Output
        code = updateCacheFromOutput(code);

        // [NEW] Tree Shaking (Post-Cache, Pre-Export)
        if (CURRENT_MODE == GenMode::CODE) {
            code = performTreeShaking(code, CURRENT_LANG.name);
        }

        // [MAKE 2.0] Process exports in AI output (Generate files dynamically)
        code = processExports(code, fs::current_path());

        if (makeMode) {
            // Check for content outside exports
            bool hasContent = false;
            for (char c : code) { if (!isspace(c)) { hasContent = true; break; } }

            if (hasContent) {
                cout << "[MAKE] Content detected outside EXPORT blocks." << endl;
                if (!explicitLang) {
                    selectTarget();
                    // Update output filename extension if it was defaulted
                    if (outputName.find(stripExt(inputFiles[0])) != string::npos) {
                         string base = stripExt(outputName);
                         if (CURRENT_LANG.producesBinary && !transpileMode) {
                             #ifdef _WIN32
                             outputName = base + ".exe";
                             #else
                             outputName = base;
                             #endif
                         } else {
                             outputName = base + CURRENT_LANG.extension;
                         }
                    }
                    // Update tempSrc extension
                    tempSrc = "temp_build" + CURRENT_LANG.extension;
                }
            } else {
                cout << "[MAKE] Generation complete. Files exported." << endl;

                // [INTELLIGENT BUILD] Auto-detect and run generated build scripts
                bool buildSuccess = false;
                if (fs::exists("Makefile")) {
                    cout << "[MAKE] Makefile detected. Executing 'make'..." << endl;
                    if (system("make") == 0) buildSuccess = true;
                } else if (fs::exists("CMakeLists.txt")) {
                    cout << "[MAKE] CMakeLists.txt detected. Configuring and building..." << endl;
                    if (!fs::exists("build")) fs::create_directory("build");
                    if (system("cd build && cmake .. && cmake --build .") == 0) buildSuccess = true;
                } else if (fs::exists("build.sh")) {
                    cout << "[MAKE] build.sh detected. Executing..." << endl;
                    #ifndef _WIN32
                    if (system("chmod +x build.sh && ./build.sh") == 0) buildSuccess = true;
                    #else
                    if (system("bash build.sh") == 0) buildSuccess = true;
                    #endif
                } else if (fs::exists("build.bat")) {
                    cout << "[MAKE] build.bat detected. Executing..." << endl;
                    if (system("build.bat") == 0) buildSuccess = true;
                } else {
                    cout << "[MAKE] No build script found. Skipping build step." << endl;
                }

                if (runOutput) {
                    if (buildSuccess) {
                        if (fs::exists(outputName)) {
                            cout << "\n[RUN] Executing " << outputName << "..." << endl;
                            string cmd = outputName;
                            #ifndef _WIN32
                            if (cmd.find('/') == string::npos) cmd = "./" + cmd;
                            std::error_code ec;
                            fs::permissions(outputName, fs::perms::owner_exec, fs::perm_options::add, ec);
                            #endif
                            string sysCmd = "\"" + cmd + "\"";
                            system(sysCmd.c_str());
                        } else {
                            cout << "[WARN] Output binary '" << outputName << "' not found." << endl;
                            cout << "       (Hint: Use -o <filename> to specify the expected binary name)" << endl;
                        }
                    } else {
                        cout << "[WARN] Build failed or missing. Skipping execution." << endl;
                    }
                }
                return 0;
            }
        }

        ofstream out(tempSrc); out << code; out.close();

        if (CURRENT_MODE == GenMode::MODEL_3D || CURRENT_MODE == GenMode::IMAGE) {
            cout << "[SUCCESS] Asset generated: " << outputName << endl;
            bool saved = false;
            for(int i=0; i<5; i++) {
                try {
                    if (fs::exists(outputName)) fs::remove(outputName);
                    fs::copy_file(tempSrc, outputName, fs::copy_options::overwrite_existing);
                    saved = true; break;
                } catch (...) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
            }
            if (!saved) {
                cerr << "[ERROR] Could not save " << outputName << ". File might be locked." << endl;
                return 1;
            }
            std::error_code ec; fs::remove(tempSrc, ec);
            return 0;
        }

        cout << "   Verifying..." << endl;
        
        // [FIX] Eliminar binario previo para evitar errores de bloqueo/permisos en Windows
        if (CURRENT_LANG.producesBinary && fs::exists(tempBin)) {
            try { fs::remove(tempBin); } catch(...) {
                // Si está bloqueado, esperar un poco y reintentar
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                try { fs::remove(tempBin); } catch(...) {}
            }
        }

        CmdResult build;
        if (blindMode) {
            cout << "   [WARN] Blind Mode: Skipping verification." << endl;
            build.exitCode = 0;
        } else if (!customBuildCmd.empty()) {
            // [NEW] Custom build command execution
            string cmd = customBuildCmd;
            // Replace placeholders
            size_t fPos = cmd.find("%FILE%");
            if (fPos != string::npos) cmd.replace(fPos, 6, tempSrc);
            size_t oPos = cmd.find("%OUT%");
            if (oPos != string::npos) cmd.replace(oPos, 5, tempBin);
            build = execCmd(cmd);
        } else if (CURRENT_LANG.buildCmd.empty()) {
            build.exitCode = 0;
        } else {
            string valCmd = CURRENT_LANG.buildCmd + " \"" + tempSrc + "\"";
            if (CURRENT_LANG.producesBinary) valCmd += " -o \"" + tempBin + "\""; 
            build = execCmd(valCmd);
        }
        
        if (build.exitCode == 0) {
            cout << "\nBUILD SUCCESSFUL: " << outputName << endl;
            std::error_code ec;
            bool saveSuccess = false;

            // Smart Output Logic
            bool saveAsSource = (getExt(outputName) == CURRENT_LANG.extension) || transpileMode || blindMode;

            for(int i=0; i<5; i++) {
                try {
                    if (fs::exists(outputName)) fs::remove(outputName);
                    
                    if (CURRENT_LANG.producesBinary && !saveAsSource) {
                        fs::copy_file(tempBin, outputName, fs::copy_options::overwrite_existing);
                        cout << "   [Binary]: " << outputName << endl;
                        
                        if (keepSource) {
                            string sName = stripExt(outputName) + CURRENT_LANG.extension;
                            fs::copy_file(tempSrc, sName, fs::copy_options::overwrite_existing);
                            cout << "   [Source Kept]: " << sName << endl;
                        }
                        fs::remove(tempBin);
                    } else {
                        fs::copy_file(tempSrc, outputName, fs::copy_options::overwrite_existing);
                        cout << "   [Source]: " << outputName << endl;
                        if (CURRENT_LANG.producesBinary && fs::exists(tempBin)) fs::remove(tempBin);
                    }
                    saveSuccess = true;
                    break;
                } catch (const fs::filesystem_error& e) {
                    if (i < 4) std::this_thread::sleep_for(std::chrono::milliseconds(250 * (i + 1)));
                    else ec = e.code();
                }
            }

            if (!saveSuccess) {
                cerr << "[ERROR] Failed to save final output. File may be locked." << endl;
                cout << "   Your build is preserved at: " << (CURRENT_LANG.producesBinary ? tempBin : tempSrc) << endl;
                return 1;
            }
            
            if (fs::exists(tempSrc) && !keepSource) fs::remove(tempSrc, ec);
            ofstream cFile(cacheFile); cFile << currentHash;

            if (runOutput) {
                if (blindMode) {
                    cout << "[WARN] Cannot run in Blind Mode." << endl;
                } else {
                cout << "\n[RUN] Executing..." << endl;
                string cmd = outputName;
                if (!CURRENT_LANG.producesBinary) {
                    string interpreter = CURRENT_LANG.buildCmd.substr(0, CURRENT_LANG.buildCmd.find(' '));
                    cmd = interpreter + " \"" + outputName + "\"";
                } else {
                    #ifndef _WIN32
                    if (outputName.find('/') == string::npos) cmd = "./" + outputName;
                    fs::permissions(outputName, fs::perms::owner_exec, fs::perm_options::add, ec);
                    #endif
                    cmd = "\"" + cmd + "\"";
                }
                system(cmd.c_str());
                }
            }

            return 0;
        } else {
            string err = build.output;
            cout << "   [!] Error (Line " << gen << "): " << err.substr(0, 300) << "..." << endl;
            log("FAIL", "Pass " + to_string(gen) + " failed.");

            // [UPDATED v5.1] Catch literal translation attempts
            // for 6.0 make this more robust by not hardcoding python -> c++ cases
            if (err.find("python.h") != string::npos || err.find("Python.h") != string::npos) {
                 errorHistory = "FATAL: You are trying to include Python.h. STOP. Rewrite the code using native C++ std:: libraries only.\n";
            } else if (err.find("print(") != string::npos || err.find("import ") != string::npos || err.find("def ") != string::npos) {
                 errorHistory = "FATAL: It seems you wrote Python code instead of C++. STOP. Return ONLY valid C++ code.\n";
            } else {
                 errorHistory = "--- Error Pass " + to_string(gen) + " ---\n" + err;
            }

            if (isFatalError(err) && gen > 3) {
                cerr << "\n[FATAL ERROR] Missing dependency/file detected. Aborting." << endl;
                cout << "   [?] Analyze fatal error with AI? [y/N]: ";
                char ans; cin >> ans;
                if (ans == 'y' || ans == 'Y') explainFatalError(err);
                break; 
            }
        }
    }
    
    } // End of !fillMode block

    cerr << "Failed to build after " << passes << " attempts." << endl;
    return 1;
}