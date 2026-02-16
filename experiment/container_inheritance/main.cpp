#include <iostream>
#include <string>

using namespace std;

// --- PARENT CONTAINER ---
// This container defines a reusable logic block.
// It doesn't have to be valid C++ if it's just instructions, 
// but here we make it generate a helper function.


void log_msg(string s) {
    cout << "[LOG]: " << s << endl;
}



int main() {
    // --- CHILD CONTAINER ---
    // This container inherits from "logger_base".
    // The AI will receive the parent's prompt + the child's prompt.
    // It should generate the log_msg function AND the main logic.
    // $$ "app_logic" -> "logger_base" {
    //     Call log_msg("System starting...").
    //     Print "Doing work...".
    //     Call log_msg("System finished.").
    // }$$

    log_msg("System starting...");
    cout << "Doing work..." << endl;
    log_msg("System finished.");
    
    return 0;
}