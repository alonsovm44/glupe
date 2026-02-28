#pragma once
#include "utils.hpp"
#include "languages.hpp"

// --- CONFIGURATION ---
inline string PROVIDER = "local"; 
inline string PROTOCOL = "ollama"; // 'google', 'openai', 'ollama'
inline string API_KEY = "";
inline string MODEL_ID = ""; 
inline string API_URL = "";
inline int MAX_RETRIES = 15;

// --- CONFIG & TOOLCHAIN OVERRIDES ---
inline bool loadConfig(string mode) {
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

inline void updateConfigFile(string key, string value) {
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

inline void showConfig() {
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