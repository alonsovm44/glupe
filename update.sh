#!/bin/bash

REPO_URL="https://raw.githubusercontent.com/M-MACHINE/glupe/main"
JSON_URL="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
GLUPE_BIN_NAME="glupe"
TEMP_BIN="/tmp/${GLUPE_BIN_NAME}.new"

echo "--- Glupe Update Script (Linux/macOS) ---"

# Get the directory of the currently running glupe executable
CURRENT_GLUPE_PATH=$(command -v glupe)
if [ -z "$CURRENT_GLUPE_PATH" ]; then
    echo "Error: 'glupe' command not found in PATH."
    echo "Please ensure glupe is installed and in your PATH."
    exit 1
fi
GLUPE_DIR=$(dirname "$CURRENT_GLUPE_PATH")

echo "Current glupe path: $CURRENT_GLUPE_PATH"
echo "Downloading source code from $REPO_URL..."

SRC_DIR="$GLUPE_DIR/src"
mkdir -p "$SRC_DIR"

SOURCE_FILES="glupec.cpp common.hpp utils.hpp config.hpp languages.hpp ai.hpp cache.hpp parser.hpp processor.hpp hub.hpp ast.hpp glupe.l glupe.y"
for file in $SOURCE_FILES; do
    if ! curl -fsSL "$REPO_URL/src/$file" -o "$SRC_DIR/$file"; then
        echo "Error: Failed to download $file"; exit 1
    fi
done
if ! curl -fsSL "$JSON_URL" -o "$SRC_DIR/json.hpp"; then
    echo "Error: Failed to download json.hpp"; exit 1
fi

echo "Generating parser and lexer with Bison and Flex..."
if ! bison -d -o "$SRC_DIR/glupe.tab.c" "$SRC_DIR/glupe.y" || ! flex -o "$SRC_DIR/lex.yy.c" "$SRC_DIR/glupe.l"; then
    echo "Warning: Parser/Lexer generation failed. Ensure flex and bison are installed."
fi

echo "Compiling Glupe..."
if ! g++ "$SRC_DIR/glupec.cpp" "$SRC_DIR/lex.yy.c" "$SRC_DIR/glupe.tab.c" -o "$TEMP_BIN" -std=c++17 -O3 -pthread -I "$SRC_DIR"; then
    echo "Error: Compilation failed."
    exit 1
fi

echo "Replacing current glupe executable..."
if mv "$TEMP_BIN" "$CURRENT_GLUPE_PATH"; then
    echo "Successfully updated glupe to the latest version!"
    echo "Please restart your terminal or shell to ensure the new version is loaded."
else
    echo "Error: Failed to replace glupe executable."
    echo "You might need to run this script with 'sudo' or manually replace '$CURRENT_GLUPE_PATH' with '$TEMP_BIN'."
    rm -f "$TEMP_BIN"
    exit 1
fi

echo "--- Update Complete ---"