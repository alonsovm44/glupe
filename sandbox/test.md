// PROJECT: LogScan - A High Performance Log Analyzer
// TARGET: C++ (Native Performance)
````cpp
EXPORT: "logscan.cpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// --- CONTAINER 1: THE PARSER ---
// We name this "parser-core". 
// If we change the log format later, we ONLY update this container.
 $$ "parser-core" {
    Define a struct 'LogEntry' containing timestamp (int), level (string), and message (string).
    Implement a function 'parse_line(string line)' that splits a log line by the ' | ' delimiter.
    Handle malformed lines gracefully by returning a LogEntry with level "ERROR".
    implement a function that returns the square of an input number
}$$ 
// --- CONTAINER 2: THE AGGREGATOR ---
// We name this "stats-engine".
// This is pure logic. We might want to swap this for a Python version later 
// to compare performance, without touching the parser.
 $$ "stats-engine" {
    Implement a class 'StatsCollector'.
    It should store counts of each log level (INFO, WARN, ERROR).
    Implement a method 'process(const LogEntry& entry)' that updates counts.
    Implement a method 'summary()' that returns a formatted string of the counts.
    PRINT("DEBUG: PRINTED MESSAGE")
}$$ 
// --- CONTAINER 3: THE FILE READER ---
// We name this "io-handler".
// This is performance critical. We might want to optimize this later 
// without breaking the parsing logic.
 $$ "io-handler" {
    Implement a function 'read_fast(string filename)' that uses memory-mapped files 
    (or standard efficient buffering) to read the file line by line.
    It should return a vector of strings.
    Make a function that returns a list of 10 numbers from 0 to 9
}$$ 
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
EXPORT: END
```