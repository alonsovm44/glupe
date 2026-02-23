#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <pqxx/pqxx>
#include <jwt-cpp/jwt.h>
#include <crow.h>
#include <crow/middlewares/cors.h>

namespace fs = std::filesystem;

// Constants
const std::string DATABASE_URL = std::getenv("DATABASE_URL");
const std::string STORAGE_DIR = std::getenv("STORAGE_DIR") ? std::getenv("STORAGE_DIR") : "storage";
const std::string SECRET_KEY = std::getenv("SECRET_KEY") ? std::getenv("SECRET_KEY") : "your_secret_key";

// Global Variables
crow::SimpleApp app;

// Storage Setup
void storage_setup() {
    if (!fs::exists(STORAGE_DIR)) {
        fs::create_directories(STORAGE_DIR);
    }
}

// Database Connection
pqxx::connection get_db_connection() {
    return pqxx::connection(DATABASE_URL);
}

// Database Operation
void db_operation(std::function<void(pqxx::connection&)> operation) {
    auto conn = get_db_connection();
    operation(conn);
}

// Initialize Database
void init_db() {
    db_operation([](pqxx::connection& conn) {
        pqxx::work txn(conn);
        txn.exec("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password_hash TEXT)");
        txn.exec("CREATE TABLE IF NOT EXISTS tags (file_id SERIAL PRIMARY KEY, tag TEXT, username TEXT)");
        txn.commit();
    });
}

// Initialize Application
void initialize_app() {
    init_db();
    std::cout << "Application initialized." << std::endl;
}

// Hash Password
std::string hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// Verify Password
bool verify_password(const std::string& password, const std::string& hash) {
    return hash_password(password) == hash;
}

// Generate JWT Token
std::string generate_token(const std::string& username) {
    auto now = std::chrono::system_clock::now();
    auto token = jwt::create()
        .set_issuer("auth0")
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::hours(24))
        .set_payload_claim("username", jwt::claim(username))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
    return token;
}

// Verify JWT Token
std::string verify_token(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY});
        verifier.verify(decoded);
        return decoded.get_payload_claim("username").as_string();
    } catch (...) {
        return "";
    }
}

// Register User
void register_user(const std::string& username, const std::string& password) {
    db_operation([&](pqxx::connection& conn) {
        pqxx::work txn(conn);
        txn.exec_params("INSERT INTO users (username, password_hash) VALUES ($1, $2)", username, hash_password(password));
        txn.commit();
    });
    fs::create_directories(STORAGE_DIR + "/" + username);
}

// Login User
std::string login_user(const std::string& username, const std::string& password) {
    std::string stored_hash;
    db_operation([&](pqxx::connection& conn) {
        pqxx::work txn(conn);
        auto result = txn.exec_params("SELECT password_hash FROM users WHERE username = $1", username);
        if (!result.empty()) {
            stored_hash = result[0][0].as<std::string>();
        }
    });
    if (!stored_hash.empty() && verify_password(password, stored_hash)) {
        return generate_token(username);
    }
    return "";
}

// Push File
void push_file(const std::string& username, const std::string& file_path, const std::vector<std::string>& tags) {
    fs::copy(file_path, STORAGE_DIR + "/" + username + "/" + fs::path(file_path).filename().string());
    db_operation([&](pqxx::connection& conn) {
        pqxx::work txn(conn);
        for (const auto& tag : tags) {
            txn.exec_params("INSERT INTO tags (tag, username) VALUES ($1, $2)", tag, username);
        }
        txn.commit();
    });
}

// Pull File
std::string pull_file(const std::string& username, const std::string& file_name) {
    auto file_path = STORAGE_DIR + "/" + username + "/" + file_name;
    if (fs::exists(file_path)) {
        return file_path;
    }
    return "";
}

// Search Users
std::vector<std::string> search_users(const std::string& query) {
    std::vector<std::string> results;
    db_operation([&](pqxx::connection& conn) {
        pqxx::work txn(conn);
        auto result = txn.exec_params("SELECT username FROM users WHERE username ILIKE $1", "%" + query + "%");
        for (const auto& row : result) {
            results.push_back(row[0].as<std::string>());
        }
    });
    return results;
}

// Search Files
std::vector<std::string> search_files(const std::string& query, const std::string& username) {
    std::vector<std::string> results;
    db_operation([&](pqxx::connection& conn) {
        pqxx::work txn(conn);
        auto result = txn.exec_params("SELECT file_id FROM tags WHERE tag ILIKE $1 AND username = $2", "%" + query + "%", username);
        for (const auto& row : result) {
            results.push_back(row[0].as<std::string>());
        }
    });
    return results;
}

// Get File Metadata
std::map<std::string, std::string> get_meta(const std::string& username, const std::string& file_name) {
    auto file_path = STORAGE_DIR + "/" + username + "/" + file_name;
    std::map<std::string, std::string> metadata;
    if (fs::exists(file_path)) {
        metadata["size"] = std::to_string(fs::file_size(file_path));
        metadata["last_modified"] = std::to_string(fs::last_write_time(file_path));
    }
    return metadata;
}

// Crow Routes
app.route_dynamic("/register")
([](const crow::request& req) {
    auto username = req.get_body_param("username");
    auto password = req.get_body_param("password");
    try {
        register_user(username, password);
        return crow::response(200);
    } catch (...) {
        return crow::response(500);
    }
});

app.route_dynamic("/login")
([](const crow::request& req) {
    auto username = req.get_body_param("username");
    auto password = req.get_body_param("password");
    auto token = login_user(username, password);
    if (!token.empty()) {
        return crow::response(token);
    }
    return crow::response(401);
});

app.route_dynamic("/push")
([](const crow::request& req) {
    auto token = req.get_header_value("Authorization");
    auto username = verify_token(token);
    if (!username.empty()) {
        auto file = req.get_multipart("file");
        if (file) {
            push_file(username, file.path(), {});
            return crow::response(200);
        }
    }
    return crow::response(401);
});

app.route_dynamic("/pull")
([](const crow::request& req) {
    auto token = req.get_header_value("Authorization");
    auto username = verify_token(token);
    if (!username.empty()) {
        auto file_name = req.url_params.get("file_name");
        auto file_path = pull_file(username, file_name);
        if (!file_path.empty()) {
            return crow::response(file_path);
        }
    }
    return crow::response(404);
});

int main() {
    initialize_app();
    storage_setup();
    app.port(5000).multithreaded().run();
    return 0;
}
