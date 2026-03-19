
// test_graph.cpp
struct DatabaseConfig {
    const char* url;
    int port;
    bool use_ssl;
};

// ... imagine 500 lines of other code here ...

bool connectToDatabase(DatabaseConfig config) {
    if (config.use_ssl) {
        return true;
    }
    return false;
}