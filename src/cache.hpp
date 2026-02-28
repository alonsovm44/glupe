#pragma once
#include "common.hpp"

// [v6.0] Semantic Node Structure
enum class NodeType {
    CONTAINER,       // $...$ or $$...$$
    VAR_EPHEMERAL,   // $:
    VAR_PERSISTENT,  // $$:
    CONSTANT         // $CONST:
};

struct SemanticNode {
    NodeType type;
    string id;
    string content; // Prompt or Value
    vector<string> parents;
    vector<string> params; // [NEW] Parameters for context injection
    bool isBlock = false;
    bool isAbstract = false;
    bool isCached = false;
    string hash; // [NEW] Hash for caching
};

// [NEW] Global Symbol Table
inline map<string, SemanticNode> SYMBOL_TABLE;

// [NEW] Cache System Constants
inline const string CACHE_DIR = "glupe_cache";
inline const string LOCK_FILE = ".glupe.lock";

inline json LOCK_DATA;

inline void initCache() {
    if (!fs::exists(CACHE_DIR)) fs::create_directory(CACHE_DIR);
    if (fs::exists(LOCK_FILE)) {
        try {
            ifstream f(LOCK_FILE);
            LOCK_DATA = json::parse(f);
        } catch(...) { LOCK_DATA = json::object(); }
    } else {
        LOCK_DATA = json::object();
        LOCK_DATA["containers"] = json::object();
        LOCK_DATA["variables"] = json::object();
    }

    // [NEW] Load persistent variables from lockfile
    if (LOCK_DATA.contains("variables")) {
        for (auto& [key, val] : LOCK_DATA["variables"].items()) {
            SemanticNode node;
            node.id = key;
            node.type = NodeType::VAR_PERSISTENT;
            node.content = val.value("content", "");
            node.hash = val.value("hash", "");
            node.isCached = true;
            SYMBOL_TABLE[key] = node;
        }
    }
}

inline void saveCache() {
    // [NEW] Save persistent variables
    json vars = json::object();
    for (const auto& [key, node] : SYMBOL_TABLE) {
        if (node.type == NodeType::VAR_PERSISTENT) {
            vars[key] = { {"content", node.content}, {"hash", node.hash} };
        }
    }
    LOCK_DATA["variables"] = vars;

    ofstream f(LOCK_FILE);
    f << LOCK_DATA.dump(4);
}

inline string getContainerHash(const string& prompt) {
    hash<string> hasher;
    return to_string(hasher(prompt));
}

inline string getCachedContent(const string& id) {
    string path = CACHE_DIR + "/" + id + ".txt";
    if (fs::exists(path)) {
        ifstream f(path);
        return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    }
    return "";
}

inline void setCachedContent(const string& id, const string& content) {
    string path = CACHE_DIR + "/" + id + ".txt";
    ofstream f(path);
    f << content;
}