#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

int main() {
    std::ifstream infile("INPUT.DAT");
    if (!infile.is_open()) {
        std::cerr << "Unable to open INPUT.DAT\n";
        return 1;
    }

    std::string line;
    bool eof = false;
    double ws_total = 0.0;
    int ws_count = 0;

        while (true) {
        if (!std::getline(infile, line)) {
            eof = true;
            break;
        }

        if (line.size() < 10 + 20 + 7) {
            // Not enough data for a valid record; skip or break
            continue;
        }

        std::string customer_id   = line.substr(0, 10);
        std::string customer_name = line.substr(10, 20);
        std::string balance_str   = line.substr(30, 7);

        // COBOL PIC 9(5)V99 means 5 digits, implied decimal before last 2 digits.
        // Convert by inserting decimal point.
        std::string bal_conv = balance_str.substr(0, 5) + "." + balance_str.substr(5, 2);
        double balance = 0.0;
        try {
            balance = std::stod(bal_conv);
        } catch (...) {
            balance = 0.0;
        }

        ws_total += balance;
        ws_count += 1;
    }
    
    std::cout << "TOTAL CUSTOMERS: " << ws_count << "\n";
    std::cout << "TOTAL BALANCE: " << std::fixed << std::setprecision(2) << ws_total << "\n";

    return 0;
}
