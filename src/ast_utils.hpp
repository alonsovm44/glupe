#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

// Tree-sitter core API
#include <tree_sitter/api.h>

// Forward declare the C++ grammar function. 
// When linking against libtree-sitter-cpp, this resolves automatically.
extern "C" const TSLanguage* tree_sitter_cpp();

using namespace std;

// --- RAII WRAPPERS FOR TREE-SITTER ---

// Lightweight wrapper for a Node. Nodes are value types in tree-sitter, 
// so they don't require manual memory management.
class TSASTNode {
private:
    TSNode node;
public:
    TSASTNode(TSNode n) : node(n) {}

    string type() const { 
        return ts_node_type(node); 
    }
    
    uint32_t start_byte() const { return ts_node_start_byte(node); }
    uint32_t end_byte() const { return ts_node_end_byte(node); }
    bool is_null() const { return ts_node_is_null(node); }

    // Get the exact string content of this node from the original source
    string get_text(const string& source) const {
        uint32_t start = start_byte();
        uint32_t end = end_byte();
        if (start < end && end <= source.length()) {
            return source.substr(start, end - start);
        }
        return "";
    }

    uint32_t child_count() const { return ts_node_child_count(node); }
    
    TSASTNode child(uint32_t index) const { 
        return TSASTNode(ts_node_child(node, index)); 
    }

    TSNode get_raw() const { return node; }
};

// RAII Wrapper for TSTree
class TSASTTree {
private:
    TSTree* tree;
public:
    TSASTTree(TSTree* t) : tree(t) {}
    ~TSASTTree() { if (tree) ts_tree_delete(tree); }
    
    // Prevent copying to avoid double free
    TSASTTree(const TSASTTree&) = delete;
    TSASTTree& operator=(const TSASTTree&) = delete;
    
    // Allow moving
    TSASTTree(TSASTTree&& other) noexcept : tree(other.tree) { other.tree = nullptr; }
    TSASTTree& operator=(TSASTTree&& other) noexcept {
        if (this != &other) {
            if (tree) ts_tree_delete(tree);
            tree = other.tree;
            other.tree = nullptr;
        }
        return *this;
    }

    TSASTNode root_node() const { return TSASTNode(ts_tree_root_node(tree)); }
};

// RAII Wrapper for TSParser
class TSASTParser {
private:
    TSParser* parser;
public:
    TSASTParser() { parser = ts_parser_new(); }
    ~TSASTParser() { if (parser) ts_parser_delete(parser); }

    bool set_language(const string& lang_id) {
        if (lang_id == "cpp" || lang_id == "c" || lang_id == "hpp" || lang_id == "h") {
            return ts_parser_set_language(parser, tree_sitter_cpp());
        }
        // Add Python, JS, etc., later
        return false;
    }

    TSASTTree parse_string(const string& source) {
        TSTree* raw_tree = ts_parser_parse_string(parser, nullptr, source.c_str(), source.length());
        if (!raw_tree) {
            throw runtime_error("[AST ERROR] Tree-sitter failed to parse the source code.");
        }
        return TSASTTree(raw_tree);
    }
};