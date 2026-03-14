#include <iostream>
#include <string>
#include "ast.hpp"

// Forward declare the Flex/Bison interface functions
extern int yyparse(ProgramNode** root);

// Flex buffer state type and functions for string scanning
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

using namespace std;

int main() {
    // Sample code combining target code and Glupe primitives
    string test_code = R"(
#include <iostream>

int main() {
    // This is a normal C++ comment
    $$: version -> "1.0"
    
    $$ my_container -> parent (param1, param2) {
        print "hello world"
    }$$

    $ inline_container { return 0; }$
}
)";

    cout << "--- Testing Lexer & Parser ---\n\n";
    cout << "[Input Code]\n" << test_code << "\n------------------------------\n";

    // 1. Point the Flex lexer to read from our string
    YY_BUFFER_STATE buffer = yy_scan_string(test_code.c_str());

    // 2. Parse the input into our AST
    ProgramNode* root = nullptr;
    int result = yyparse(&root);

    if (result == 0 && root != nullptr) {
        cout << "\n[SUCCESS] AST Constructed:\n";
        root->print();
    } else {
        cout << "\n[ERROR] Parsing failed!\n";
    }

    // 3. Cleanup
    yy_delete_buffer(buffer);
    delete root;

    return 0;
}