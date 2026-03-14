#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <utility>

using namespace std;

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    // Print method for debugging the parsed tree structure
    virtual void print(int indent = 0) const = 0;
};

// Represents raw target-language code (e.g., C++, Python) that is passed through untouched
class RawCodeNode : public ASTNode {
public:
    string code;
    explicit RawCodeNode(string c) : code(std::move(c)) {}
    
    void print(int indent = 0) const override {
        string padding(indent, ' ');
        cout << padding << "[RawCode] length=" << code.length() << "\n";
    }
};

// Variable type enum (aligns with Glupe semantic variable types)
enum class VarType { EPHEMERAL, PERSISTENT, CONSTANT };

// Represents a Glupe variable definition (e.g., $: var = "value")
class VariableNode : public ASTNode {
public:
    string id;
    string value;
    VarType varType;

    VariableNode(string i, string v, VarType t) 
        : id(std::move(i)), value(std::move(v)), varType(t) {}

    void print(int indent = 0) const override {
        string padding(indent, ' ');
        cout << padding << "[Variable] " << (varType == VarType::CONSTANT ? "CONST " : "") << id << " = " << value << "\n";
    }
};

// Represents a Glupe Semantic Container ($$ name -> parent { intent } $$)
class ContainerNode : public ASTNode {
public:
    string id;
    vector<string> parents;
    vector<string> params;
    string intent;
    bool isBlock;
    bool isAbstract;

    ContainerNode(string i, string intent_text, bool block, bool abstr = false)
        : id(std::move(i)), intent(std::move(intent_text)), isBlock(block), isAbstract(abstr) {}

    void print(int indent = 0) const override {
        string padding(indent, ' ');
        cout << padding << "[Container] " << id << (isAbstract ? " (ABSTRACT)" : "") << (isBlock ? " [BLOCK]" : " [INLINE]") << "\n";
    }
};

// Represents the root of the parsed file
class ProgramNode : public ASTNode {
public:
    vector<unique_ptr<ASTNode>> elements;

    void addElement(unique_ptr<ASTNode> node) {
        if (node) elements.push_back(std::move(node));
    }

    void print(int indent = 0) const override {
        string padding(indent, ' ');
        cout << padding << "[ProgramNode]\n";
        for (const auto& el : elements) {
            el->print(indent + 2);
        }
    }
};