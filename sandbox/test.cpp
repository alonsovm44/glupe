#include <iostream>

int main() {
    // The old parser would trip over the $$ in this string!
    std::cout << "The total cost is $$500." << std::endl;
    
    $${ 
        print "AST parsing is working perfectly!" 
    }$$
    
    return 0;
}