/* YORI COMPILER (yori.exe) - v3.2 (Modular Unity Build)
   Usage: yori main.yori [-cloud | -local] [-o output.exe] [-k] [-u]
   Feature: Supports 'IMPORT: filename.yori' for modularity.
*/

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

#include "json.hpp" 

using json = nlohmann::json;
using namespace std;

// --- DYNAMIC CONFIGURATION ---
string PROVIDER = "local"; 
string API_KEY = "";
string MODEL_ID = ""; 
string API_URL = "";
const int MAX_RETRIES = 5;

// --- COMPATIBILITY PATCH ---
#ifdef _WIN32
extern "C" {
    FILE* _popen(const char* command, const char* mode);
    int _pclose(FILE* stream);
}
#else
    #define _popen popen
    #define _pclose pclose
#endif

// --- CONFIG LOADER (Standard) ---
bool loadConfig(string mode) {
    ifstream f("config.json");
    if (!f.is_open()) { cerr << "FATAL: config.json missing." << endl; return false; }
    try {
        json j = json::parse(f);
        if (!j.contains(mode)) { cerr << "FATAL: Profile '" << mode << "' missing." << endl; return false; }
        json profile = j[mode];
        PROVIDER = mode;
        if (mode == "cloud") {
            if (!profile.contains("api_key") || profile["api_key"].get<string>().empty()) return false;
            API_KEY = profile["api_key"];
        }
        MODEL_ID = profile["model_id"];
        if (mode == "local") API_URL = profile.contains("api_url") ? profile["api_url"].get<string>() : "http://localhost:11434/api/generate";
        return true;
    } catch (...) { return false; }
}

// --- SYSTEM CORE ---
string execCmd(string cmd) {
    array<char, 128> buffer;
    string result;
    string full_cmd = cmd + " 2>&1"; 
    FILE* pipe = _popen(full_cmd.c_str(), "r");
    if (!pipe) return "EXEC_FAIL";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) result += buffer.data();
    _pclose(pipe);
    return result;
}

string callAI(string prompt) {
    string response;
    while (true) {
        json body;
        string url;
        if (PROVIDER == "local") {
            body["model"] = MODEL_ID; body["prompt"] = prompt; body["stream"] = false; url = API_URL;
        } else {
            body["contents"][0]["parts"][0]["text"] = prompt;
            url = "https://generativelanguage.googleapis.com/v1beta/models/" + MODEL_ID + ":generateContent?key=" + API_KEY;
        }
        ofstream file("request_temp.json"); file << body.dump(); file.close();
        string cmd = "curl -s -X POST -H \"Content-Type: application/json\" -d @request_temp.json \"" + url + "\"";
        response = execCmd(cmd); remove("request_temp.json");
        if (PROVIDER == "cloud" && (response.find("429") != string::npos)) {
            cout << "\n[RATE LIMIT] Waiting 30s..." << endl; this_thread::sleep_for(chrono::seconds(30)); continue; 
        }
        break;
    }
    return response;
}

string extractCode(string jsonResponse) {
    try {
        json j = json::parse(jsonResponse);
        string raw = "";
        if (j.contains("error")) return "ERROR: API Error";
        if (PROVIDER == "local") { if (j.contains("response")) raw = j["response"]; } 
        else { if (j.contains("candidates")) raw = j["candidates"][0]["content"]["parts"][0]["text"]; }
        size_t start = raw.find("```");
        if (start == string::npos) return raw;
        size_t end_line = raw.find('\n', start);
        size_t end_block = raw.rfind("```");
        if (end_line != string::npos && end_block != string::npos) return raw.substr(end_line + 1, end_block - end_line - 1);
        return raw;
    } catch (...) { return "JSON_PARSE_ERROR"; }
}

string stripExt(string fname) {
    size_t lastindex = fname.find_last_of("."); 
    return (lastindex == string::npos) ? fname : fname.substr(0, lastindex); 
}

// --- MODULE PROCESSOR (New Feature) ---
// Scans for "IMPORT: file.yori", loads content, and removes the line from main code.
string processImports(string& mainCode, string& importContext) {
    stringstream ss(mainCode);
    string line;
    string cleanedMain = "";
    
    while (getline(ss, line)) {
        if (line.find("IMPORT:") != string::npos) {
            // Extract filename
            size_t pos = line.find("IMPORT:");
            string fname = line.substr(pos + 7);
            // Trim whitespace
            fname.erase(0, fname.find_first_not_of(" \t\r\n"));
            fname.erase(fname.find_last_not_of(" \t\r\n") + 1);
            
            cout << "   📦 Importing module: " << fname << endl;
            
            ifstream lib(fname);
            if (lib.is_open()) {
                string content((istreambuf_iterator<char>(lib)), istreambuf_iterator<char>());
                importContext += "\n// --- MODULE: " + fname + " ---\n" + content + "\n";
            } else {
                cerr << "   ⚠️  Warning: Could not find module " << fname << endl;
            }
        } else {
            cleanedMain += line + "\n";
        }
    }
    return cleanedMain;
}

// --- MAIN ---
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: yori <main.yori> [-u] [-o output.exe] [-k]" << endl;
        return 0;
    }

    string inputFile = argv[1];
    string outputFile = "a.exe";
    string mode = "local"; 
    bool keepSource = false;
    bool updateMode = false;

    for(int i=2; i<argc; i++) {
        string arg = argv[i];
        if (arg == "-cloud") mode = "cloud";
        if (arg == "-local") mode = "local";
        if (arg == "-o" && i+1 < argc) outputFile = argv[i+1];
        if (arg == "-k") keepSource = true;
        if (arg == "-u") updateMode = true;
    }

    if (!loadConfig(mode)) return 1;

    cout << "[YORI] Mode: " << PROVIDER << " | Modular Build" << endl;

    ifstream f(inputFile);
    if (!f.is_open()) { cerr << "Error: Main file not found." << endl; return 1; }
    string rawCode((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());

    // --- STEP 1: RESOLVE IMPORTS ---
    string libraryContext = "";
    string mainLogic = processImports(rawCode, libraryContext);

    string sourceFile = stripExt(outputFile) + ".cpp";
    string existingCpp = "";
    if (updateMode) {
        ifstream oldSrc(sourceFile);
        if (oldSrc.is_open()) existingCpp = string((istreambuf_iterator<char>(oldSrc)), istreambuf_iterator<char>());
        else updateMode = false;
    }

    string tempCpp = "temp_build.cpp";
    string currentError = "";

    for(int gen=1; gen<=MAX_RETRIES; gen++) {
        cout << "   [Pass " << gen << "] Generating Unity Build..." << endl;
        
        string prompt;
        
        if (currentError.empty()) {
            if (updateMode) {
                 prompt = "ROLE: Expert C++ Maintainer.\nTASK: Update code. KEEP existing logic.\n\n[EXISTING]:\n" + existingCpp + "\n\n[NEW INSTRUCTIONS]:\n" + mainLogic + "\n\nOUTPUT: Updated C++ code.";
            } else {
                // NEW MODULAR PROMPT
                prompt = "ROLE: Expert C++ Architect.\nTASK: Create a SINGLE valid C++ file (Unity Build).\n\n[LIBRARIES / MODULES]:\n" + libraryContext + "\n\n[MAIN ENTRY POINT LOGIC]:\n" + mainLogic + "\n\nINSTRUCTIONS:\n1. Combine all logic into one file.\n2. Only generate one 'main' function based on Entry Point Logic.\n3. Output ONLY C++ code.";
            }
        } else {
            prompt = "ROLE: Expert Debugger.\nTASK: Fix compilation error.\nERROR:\n" + currentError + "\nCONTEXT:\n" + mainLogic + "\nOUTPUT: Only corrected C++ code.";
        }

        string response = callAI(prompt);
        string code = extractCode(response);

        if (code.find("ERROR:") == 0) { cerr << "AI Error: " << code << endl; return 1; }
        
        ofstream out(tempCpp);
        out << code;
        out.close();

        cout << "   Compiling..." << endl;
        string output = execCmd("g++ " + tempCpp + " -o " + outputFile);

        if (output.find("error:") == string::npos) {
            cout << "\nBUILD SUCCESSFUL: " << outputFile << endl;
            string cleanName = stripExt(outputFile) + ".cpp";
            remove(cleanName.c_str());
            rename(tempCpp.c_str(), cleanName.c_str());
            if (keepSource) cout << "   Source saved: " << cleanName << endl;
            return 0;
        }
        currentError = output;
    }
    cerr << "\nBUILD FAILED." << endl;
    return 1;
}