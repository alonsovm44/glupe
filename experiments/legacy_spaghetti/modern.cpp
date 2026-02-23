#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

char* buffer = nullptr;
int size = 0;

void loadFile(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file.\n";
        return;
    }

    size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[size + 1];
    file.read(buffer, size);
    buffer[size] = '\0';

    file.close();
}

int count_lines() {
    int lines = 0;
    for (int i = 0; i < size; i++) {
        if (buffer[i] == '\n') {
            lines++;
        }
    }
    return lines;
}

int count_words() {
    int words = 0;
    bool in_word = false;

    for (int i = 0; i < size; i++) {
        if (std::isspace(static_cast<unsigned char>(buffer[i]))) {
            if (in_word) {
                words++;
                in_word = false;
            }
        } else {
            in_word = true;
        }
    }

    if (in_word) words++;
    return words;
}

int find_pattern(const char* pattern) {
    int count = 0;
    int pat_len = std::strlen(pattern);

    for (int i = 0; i <= size - pat_len; i++) {
        if (std::strncmp(&buffer[i], pattern, pat_len) == 0) {
            count++;
        }
    }
    return count;
}

void generate_report() {
    int lines = count_lines();
    int words = count_words();
    int patterns = find_pattern("pattern");

    std::cout << "----- REPORT START -----\n";
    std::cout << "Lines: " << lines << "\n";
    std::cout << "Words: " << words << "\n";
    std::cout << "Pattern occurrences: " << patterns << "\n";
    std::cout << "------ REPORT END ------\n";
}

void processData() {
    if (!buffer) return;

    int lines = count_lines();
    int words = count_words();
    int patterns = find_pattern("pattern");

    generate_report();
}

void cleanup() {
    if (buffer) {
        delete[] buffer;
        buffer = nullptr;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./program <filename>\n";
        return 1;
    }

    loadFile(argv[1]);
    processData();
    cleanup();

    return 0;
}
