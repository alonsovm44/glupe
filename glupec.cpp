#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

int main() {
    std::cout << "glupe build successful. Running minimal C++ program." << std::endl;

        // Implement some meaningful logic: list current directory contents
    std::cout << "Listing current directory:" << std::endl;
    for (const auto &entry : std::filesystem::directory_iterator(".")) {
        std::cout << " - " << entry.path().string() << std::endl;
    }

    return 0;
}
