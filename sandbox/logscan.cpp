#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

// --- CONTAINER 1: THE PARSER ---
// We name this "parser-core". 
// If we change the log format later, we ONLY update this container.
 

struct LogEntry {
    int timestamp;
    std::string level;
    std::string message;
};

LogEntry parse_line(const std::string& line) {
    LogEntry entry = {0, "ERROR", ""}; // Default values for malformed lines
    size_t pos1 = line.find(" | ");
    if (pos1 != std::string::npos) {
        size_t pos2 = line.find(" | ", pos1 + 3);
        if (pos2 != std::string::npos) {
            std::string timestamp_str = line.substr(0, pos1);
            entry.level = line.substr(pos1 + 3, pos2 - pos1 - 3);
            entry.message = line.substr(pos2 + 3);

            try {
                entry.timestamp = std::stoi(timestamp_str);
            } catch (const std::invalid_argument&) {
                // Handle invalid timestamp
            }
        }
    }
    return entry;
}

int square(int num) {
    return num * num;
}

 
// --- CONTAINER 2: THE AGGREGATOR ---
// We name this "stats-engine".
// This is pure logic. We might want to swap this for a Python version later 
// to compare performance, without touching the parser.
 

class StatsCollector {
private:
    std::unordered_map<std::string, int> counts;

public:
    StatsCollector() {
        counts["INFO"] = 0;
        counts["WARN"] = 0;
        counts["ERROR"] = 0;
    }

    void process(const LogEntry& entry) {
        if (counts.find(entry.level) != counts.end()) {
            counts[entry.level]++;
        } else {
            counts["ERROR"]++; // Unknown levels treated as ERROR
        }
    }

    std::string summary() {
        std::ostringstream out;
        out << "INFO: " << counts["INFO"] << "\n";
        out << "WARN: " << counts["WARN"] << "\n";
        out << "ERROR: " << counts["ERROR"] << "\n";
        // Required debug print
        out << "DEBUG: PRINTED MESSAGE";
        return out.str();
    }
};

 
// --- CONTAINER 3: THE FILE READER ---
// We name this "io-handler".
// This is performance critical. We might want to optimize this later 
// without breaking the parsing logic.
 

std::vector<std::string> read_fast(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return lines;
    }

    lines.reserve(10000); // Pre-allocate some space

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

 
// --- MANUAL ARCHITECTURE (The Glue) ---
// The developer controls the flow. The AI just fills the functions.
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: logscan <logfile>" << std::endl;
        return 1;
    }

    // 1. Read (Uses io-handler)
    auto lines = read_fast(argv[1]);

    // 2. Analyze (Uses stats-engine)
    StatsCollector stats;

    // 3. Process (Uses parser-core)
    for (const auto& line : lines) {
        LogEntry entry = parse_line(line);
        stats.process(entry);
    }

    // 4. Output
    std::cout << stats.summary() << std::endl;

    return 0;
}