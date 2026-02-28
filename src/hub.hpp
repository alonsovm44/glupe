#pragma once
#include "utils.hpp"

// --- AUTH HELPERS ---
inline const string SESSION_FILE = ".glupe_session";

inline pair<string, string> getSession() {
    if (!fs::exists(SESSION_FILE)) return {"", ""};
    try {
        ifstream f(SESSION_FILE);
        json j = json::parse(f);
        return {j.value("token", ""), j.value("username", "")};
    } catch (...) {
        return {"", ""};
    }
}

inline void saveSession(string token, string username) {
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

inline string getSessionToken() {
    return getSession().first;
}

inline string getSessionUser() {
    return getSession().second;
}

inline bool checkLogin() {
    if (getSessionToken().empty()) {
        cout << "Error: Not logged in. Run 'glupe login' first." << endl;
        return false;
    }
    return true;
}

inline void startInteractiveHub() {
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