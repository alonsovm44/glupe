#pragma once
#include "common.hpp"

enum class GenMode { CODE, MODEL_3D, IMAGE };
inline GenMode CURRENT_MODE = GenMode::CODE;

struct LangProfile {
    string id; string name; string extension;  
    string versionCmd; string buildCmd; bool producesBinary;
    string checkCmd; 
};

inline map<string, LangProfile> LANG_DB = {
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

inline map<string, LangProfile> MODEL_DB = {
    {"obj",  {"obj",  "Wavefront OBJ", ".obj",  "", "", false}},
    {"stl",  {"stl",  "STL (ASCII)",   ".stl",  "", "", false}},
    {"ply",  {"ply",  "PLY (ASCII)",   ".ply",  "", "", false}},
    {"gltf", {"gltf", "glTF (JSON)",   ".gltf", "", "", false}}
};

inline map<string, LangProfile> IMAGE_DB = {
    {"svg",  {"svg",  "SVG (Vector)",  ".svg",  "", "", false}},
    {"eps",  {"eps",  "PostScript",    ".eps",  "", "", false}}
};

inline LangProfile CURRENT_LANG; 

inline void selectTarget() {
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