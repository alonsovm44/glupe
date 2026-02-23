/* 
   LEGACY FILE: DataProcessor.cpp 
   WRITTEN: Circa 2006
   AUTHOR: Unknown (Left the company in 2008)
   STATUS: "It works, don't touch it."
*/

#include <iostream>
#include <fstream>
#include <string.h> // Old style header
#include <stdlib.h>

using namespace std;

// No namespace, no class, just global misery
char* buffer; // Global raw pointer, dangerous
int size = 0; 

void loadFile(const char* filename) {
    FILE* f = fopen(filename, "r"); // C-style file I/O
    if (!f) {
        cout << "ERROR: Cant open file!" << endl;
        return;
    }
    
    // Get size
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Manual memory allocation, no check for null
    buffer = new char[size + 1];
    
    // Read loop, inefficient
    int i = 0;
    while (!feof(f)) {
        char c = fgetc(f);
        if (c != -1) buffer[i] = c;
        i++;
    }
    buffer[i] = '\0';
    
    fclose(f);
}

void processData() {
    if (size == 0) return;
    
    // Spaghetti Logic: Count lines, words, and find specific pattern
    int lines = 0;
    int words = 0;
    int patternCount = 0;
    char pattern[] = "ERROR"; // Hardcoded pattern
    
    bool inWord = false;
    bool foundPattern = false;
    
    // One giant loop for everything
    for (int i = 0; i < size; i++) {
        // Count lines
        if (buffer[i] == '\n') {
            lines++;
        }
        
        // Count words (nested logic)
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {
            if (inWord) {
                words++;
                inWord = false;
            }
        } else {
            inWord = true;
        }
        
        // Search for pattern (inefficient O(N*M) check inside loop)
        if (buffer[i] == pattern[0]) {
            bool match = true;
            for (int j = 1; j < strlen(pattern); j++) {
                if (buffer[i+j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                patternCount++;
            }
        }
    }
    
    // Output mixed with logic
    cout << "---------------- REPORT ----------------" << endl;
    cout << "LINES: " << lines << endl;
    cout << "WORDS: " << words << endl;
    cout << "ERRORS FOUND: " << patternCount << endl;
    cout << "----------------------------------------" << endl;
    
    // "Debug" code left in production
    // system("pause"); 
}

void cleanup() {
    // Easy to forget, but we have it here
    if (buffer) {
        delete[] buffer; 
        buffer = NULL; // NULL instead of nullptr
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: processor <file>" << endl;
        return 1;
    }

    // God function calls
    loadFile(argv[1]);
    processData();
    cleanup();
    
    return 0;
}