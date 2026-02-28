// Standard Includes
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

// Platform Specifics
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

// JSON Library
#include "json.hpp"

// Namespaces and Aliases
using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

// Common Structures
struct CmdResult {
    string output;
    int exitCode;
};

// Global Flags (Inline for C++17 header-only support)
inline bool VERBOSE_MODE = false;
inline const string CURRENT_VERSION = "6.0.0";