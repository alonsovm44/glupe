#pragma once
#include "common.hpp"

// --- LOGGER SYSTEM ---
inline ofstream logFile;

inline void initLogger() {
    logFile.open("glupe.log", ios::app);
    if (logFile.is_open()) {
        auto t = time(nullptr);
        auto tm = *localtime(&t);
        logFile << "\n--- SESSION START (v" << CURRENT_VERSION << "): " << put_time(&tm, "%Y-%m-%d %H:%M:%S") << " ---\n";
    }
}

inline void log(string level, string message) {
    if (logFile.is_open()) {
        auto t = time(nullptr);
        auto tm = *localtime(&t);
        logFile << "[" << put_time(&tm, "%H:%M:%S") << "] [" << level << "] " << message << endl;
    }
    if (VERBOSE_MODE) cout << "   [" << level << "] " << message << endl;
}

// --- SYSTEM UTILS ---

inline CmdResult execCmd(string cmd) {
    array<char, 128> buffer;
    string result;
    string full_cmd = cmd + " 2>&1";
    
    FILE* pipe = _popen(full_cmd.c_str(), "r");
    if (!pipe) return {"EXEC_FAIL", -1};
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) result += buffer.data();
    int code = _pclose(pipe);
    return {result, code};
}

inline string stripExt(string fname) {
    size_t lastindex = fname.find_last_of("."); 
    return (lastindex == string::npos) ? fname : fname.substr(0, lastindex); 
}

inline string getExt(string fname) {
    size_t lastindex = fname.find_last_of("."); 
    return (lastindex == string::npos) ? "" : fname.substr(lastindex); 
}

inline string formatDuration(long long seconds) {
    if (seconds < 60) return to_string(seconds) + "s";
    long long min = seconds / 60;
    long long sec = seconds % 60;
    return to_string(min) + "m " + to_string(sec) + "s";
}

// --- HEURISTICS ---

// Enhanced error detection for lazy transpilation
inline bool isFatalError(const string& errMsg) {
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

// Heuristic to detect spaghetti/legacy code
inline bool detectIfCodeIsSpaghetti(const string& code) {
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

// Execution Timer for -crono flag
struct ExecutionTimer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    bool enabled = false;
    ExecutionTimer() : start(std::chrono::high_resolution_clock::now()) {}
};