/* GLUPE COMPILER ( formerly yori.exe) - v6.0.0*/

// build with this: g++ glupec.cpp -o glupe -std=c++17 -lstdc++fs -static-libgcc -static-libstdc++
/* GLUPE COMPILER ( formerly yori.exe) - v6.0.0*/

// build with this: g++ glupec.cpp -o glupe -std=c++17 -lstdc++fs -static-libgcc -static-libstdc++

//REFACTORED V 5.9
#include "common.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "languages.hpp"
#include "ai.hpp"
#include "cache.hpp"
#include "parser.hpp"
#include "processor.hpp"
#include "hub.hpp"

void dummyFunc(){
    cout << "DEBUG: This is a dummy function to ensure the file is not empty after refactor." << endl;
}

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
    cout << "  info <file.glp>         : Show file metadata.\n";
    cout << "  insert-metadata <path>  : Insert metadata template.\n\n";
    
    cout << "Examples:\n";
    cout << "  glupe main.glp -o app.exe -cpp\n";
    cout << "  glupe idea.txt -make -series\n";
    cout << "  glupe legacy.c -refine\n";
    cout << "  glupe fix bug.py \"fix index out of range\"\n";
}

int main(int argc, char* argv[]) {
    dummyFunc(); // Prevent empty file after refactor
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