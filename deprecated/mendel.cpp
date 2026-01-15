// engine_m.cpp
// El Motor Evolutivo (Machine M) reescrito en C++ Nativo.
// Compilar con: g++ engine_m.cpp -o engine_m -lcurl

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <thread>
#include <chrono>

// Librerías externas (Asegúrate de tener json.hpp en la misma carpeta)
#include "json.hpp"
#include <curl/curl.h>

using json = nlohmann::json;
using namespace std;

// --- CONFIGURACIÓN ---
const string API_KEY = "TU_API_KEY_AQUI"; // <--- PEGA TU CLAVE
const string MODEL_ID = "gemini-1.5-flash";
const int MAX_GENERATIONS = 10; // O ponlo infinito si prefieres

// --- UTILIDADES DE CURL (RED) ---
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string callGeminiAPI(string prompt) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        string url = "https://generativelanguage.googleapis.com/v1beta/models/" + MODEL_ID + ":generateContent?key=" + API_KEY;
        
        // Estructura JSON para la API REST de Google
        json body;
        body["contents"][0]["parts"][0]["text"] = prompt;
        
        // Configuración de generación (Opcional: forzar JSON si quisieras, aquí pedimos texto/código)
        // body["generationConfig"]["temperature"] = 0.2;

        string jsonStr = body.dump();

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            cerr << "❌ Error de Red (Curl): " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return readBuffer;
}

// --- UTILIDADES DE SISTEMA ---
string execCommand(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "FATAL_EXEC_ERROR";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// --- LÓGICA DE LA FÁBRICA ---

struct LanguageConfig {
    string lang;
    string ext;
    string compile_cmd_base;
};

LanguageConfig detectLanguage(json blueprint) {
    vector<string> stack = blueprint["meta"]["tech_stack"];
    string stack_str = "";
    for(auto& s : stack) stack_str += s + " "; 
    
    // Convertir a minusculas (manual simple)
    for(auto& c : stack_str) c = tolower(c);

    if (stack_str.find("c++") != string::npos || stack_str.find("cpp") != string::npos) {
        return {"C++", ".cpp", "g++ -fsyntax-only "};
    } else {
        return {"Python", ".py", "python3 -m py_compile "};
    }
}

string cleanMarkdown(string raw) {
    // Extracción básica de bloques de código
    size_t start = raw.find("```");
    if (start == string::npos) return raw; // No hay markdown

    size_t end_line = raw.find('\n', start);
    size_t end_block = raw.rfind("```");
    
    if (end_line != string::npos && end_block != string::npos && end_block > end_line) {
        return raw.substr(end_line + 1, end_block - end_line - 1);
    }
    return raw;
}

// --- MAIN LOOP ---
int main() {
    cout << "🏭 [ENGINE M - C++ NATIVE] Iniciando..." << endl;

    // 1. Cargar Blueprint
    ifstream f("app.json");
    if(!f.is_open()) {
        cerr << "❌ Error: No se encuentra app.json" << endl;
        return 1;
    }
    json blueprint = json::parse(f);
    
    LanguageConfig config = detectLanguage(blueprint);
    string appName = blueprint["meta"]["program_name"];
    string filename = appName + config.ext; // Ej: minipaint.cpp
    
    // Convertir appName a minúsculas para el archivo (opcional)
    // ...

    cout << "🔧 Modo detectado: " << config.lang << " (" << filename << ")" << endl;

    string currentError = "";
    
    // 2. Bucle Evolutivo
    for(int gen = 1; gen <= MAX_GENERATIONS; gen++) {
        cout << "\n⚙️  [GEN " << gen << "] Forjando codigo..." << endl;

        // A. Construir Prompt
        string prompt;
        string blueprintStr = blueprint.dump(2);
        
        if (currentError.empty()) {
            prompt = "ROLE: Expert " + config.lang + " Coder. TASK: Write full code for this JSON Blueprint: " + blueprintStr + ". Output ONLY code.";
        } else {
            cout << "🧬 [EVOLUCION] Mutando para arreglar error..." << endl;
            prompt = "ROLE: Expert " + config.lang + " Coder. TASK: Fix this error:\n" + currentError + "\nFor this requirement:\n" + blueprintStr + "\nOutput ONLY corrected code.";
        }

        // B. Llamar a la IA
        string apiResponse = callGeminiAPI(prompt);
        
        // Parsear respuesta de Google
        json responseJson;
        try {
            responseJson = json::parse(apiResponse);
        } catch(...) {
            cerr << "❌ Error parseando respuesta de Google (JSON invalido)." << endl;
            continue;
        }

        if(!responseJson.contains("candidates")) {
            cerr << "⚠️  API Error: " << apiResponse << endl;
            continue; // Reintentar
        }

        string rawCode = responseJson["candidates"][0]["content"]["parts"][0]["text"];
        string cleanCode = cleanMarkdown(rawCode);

        // C. Materializar (Guardar archivo)
        ofstream out(filename);
        out << cleanCode;
        out.close();
        cout << "   💾 Archivo escrito: " << filename << endl;

        // D. Test de Integridad (Compilar)
        cout << "   🛡️  Verificando integridad..." << endl;
        string cmd = config.compile_cmd_base + filename + " 2>&1"; // Redirigir stderr a stdout
        string output = execCommand(cmd.c_str());

        // Análisis de Resultado
        if (output.empty()) {
            // Si g++ o py_compile no dicen nada, suele ser Éxito.
            cout << "\n✨ [EXITO TOTAL] El codigo es valido en Gen " << gen << endl;
            cout << "🚀 Listo para despliegue." << endl;
            break;
        } else {
            // Hubo output, verifiquemos si es error
            // g++ a veces tira warnings que no son errores. Buscamos "error:"
            bool isError = false;
            if (config.lang == "C++" && output.find("error:") != string::npos) isError = true;
            if (config.lang == "C++" && output.find("fatal error:") != string::npos) isError = true;
            if (config.lang == "Python" && output.find("Error") != string::npos) isError = true;

            if (!isError && config.lang == "C++") {
                 cout << "\n✨ [EXITO CON WARNINGS] Codigo valido." << endl;
                 break;
            }

            // --- FRENO DE EMERGENCIA ---
            if (output.find("No such file") != string::npos || output.find("fatal error") != string::npos) {
                cout << "\n🛑 [FRENO DE EMERGENCIA] Error de entorno detectado." << endl;
                cout << "   Causa: " << output.substr(0, 100) << "..." << endl;
                break;
            }

            cout << "   ⚠️ [FALLO] Error detectado. Reintentando..." << endl;
            // cout << output << endl; // Debug
            currentError = output;
        }
    }

    return 0;
}