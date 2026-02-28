#pragma once
#include "config.hpp"

// --- AI CORE ---
inline string callAI(string prompt) {
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

inline string extractCode(string jsonResponse) {
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

inline void explainFatalError(const string& errorMsg) {
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

inline void selectOllamaModel() {
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

inline void openApiKeyPage() {
    cout << "[INFO] Opening ApiFreeLlm.com..." << endl;
    #ifdef _WIN32
    system("start https://apifreellm.com/en/api-access");
    #else
    system("xdg-open https://apifreellm.com/en/api-access");
    #endif
}