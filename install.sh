#!/bin/bash
# Glupe Installer for Linux/macOS
# Usage: curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash

set -e

REPO_URL="https://raw.githubusercontent.com/alonsovm44/glupe/master"
JSON_URL="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
INSTALL_DIR="$HOME/.glupe"
EXE_PATH="$INSTALL_DIR/glupe"

# Colors
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${CYAN}--- Glupe Installer ---${NC}"

# 1. Check for C++ Compiler
COMPILER=""
if command -v g++ >/dev/null 2>&1; then
    COMPILER="g++"
    echo -e "${GREEN}[OK] g++ found.${NC}"
elif command -v clang++ >/dev/null 2>&1; then
    COMPILER="clang++"
    echo -e "${GREEN}[OK] clang++ found.${NC}"
else
    echo -e "${RED}[ERROR] C++ compiler not found!${NC}"
    echo "Glupe requires a C++17 compatible compiler."
    echo "  Ubuntu/Debian: sudo apt install g++"
    echo "  Fedora: sudo dnf install gcc-c++"
    echo "  Arch: sudo pacman -S gcc"
    echo "  macOS: xcode-select --install"
    exit 1
fi

# 2. Check for Ollama
if command -v ollama >/dev/null 2>&1; then
    echo -e "${GREEN}[OK] Ollama found.${NC}"
else
    echo -e "${YELLOW}[INFO] Ollama (Local AI) not found.${NC}"
    read -p "Do you want to install Ollama? [Y/n] " ans
    if [[ -z "$ans" || "$ans" =~ ^[Yy]$ ]]; then
        echo "Downloading Ollama installer..."
        # We allow this to fail without crashing the whole script
        set +e
        curl -fsSL https://ollama.com/install.sh | sh
        set -e
        echo -e "${GREEN}[OK] Ollama installed.${NC}"
    fi
fi

# 3. Create Directory
mkdir -p "$INSTALL_DIR"
echo -e "${GREEN}[OK] Created installation directory: $INSTALL_DIR${NC}"

# 4. Download Source
echo "[INFO] Downloading source code..."
if ! curl -fsSL "$REPO_URL/glupec.cpp" -o "$INSTALL_DIR/glupec.cpp"; then
    echo -e "${RED}[ERROR] Failed to download glupec.cpp${NC}"; exit 1
fi
if ! curl -fsSL "$JSON_URL" -o "$INSTALL_DIR/json.hpp"; then
    echo -e "${RED}[ERROR] Failed to download json.hpp${NC}"; exit 1
fi

# 5. Compile
echo "[INFO] Compiling Glupe..."
COMPILE_CMD="$COMPILER \"$INSTALL_DIR/glupec.cpp\" -o \"$EXE_PATH\" -std=c++17 -O3 -pthread"

# Handle Filesystem linking
# GCC on Linux usually needs -lstdc++fs for versions < 9, and it doesn't hurt to add it for newer versions.
# Clang (macOS/BSD) usually doesn't need it or handles it internally.
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    COMPILE_CMD="$COMPILE_CMD -lstdc++fs"
fi

eval $COMPILE_CMD

if [ ! -f "$EXE_PATH" ]; then
    echo -e "${RED}[ERROR] Compilation failed.${NC}"
    exit 1
fi
echo -e "${GREEN}[OK] Compilation successful.${NC}"

# 6. Create Config
CONFIG_PATH="$INSTALL_DIR/config.json"
if [ ! -f "$CONFIG_PATH" ]; then
    cat <<EOF > "$CONFIG_PATH"
{
    "local": {
        "model_id": "qwen2.5-coder:latest",
        "api_url": "http://localhost:11434/api/generate"
    },
    "cloud": {
        "protocol": "openai",
        "api_key": "",
        "model_id": "gpt-4o",
        "api_url": "https://api.openai.com/v1/chat/completions"
    },
    "max_retries": 15
}
EOF
    echo -e "${GREEN}[OK] Created default config.json${NC}"
fi

# 7. Add to PATH
SHELL_RC=""
case "$SHELL" in
    */zsh) SHELL_RC="$HOME/.zshrc" ;;
    */bash) SHELL_RC="$HOME/.bashrc" ;;
    *) SHELL_RC="$HOME/.profile" ;;
esac

if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo "[INFO] Adding Glupe to PATH..."
    if [ -f "$SHELL_RC" ]; then
        echo "" >> "$SHELL_RC"
        echo "# Glupe CLI" >> "$SHELL_RC"
        echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$SHELL_RC"
        echo -e "${YELLOW}[OK] Added to PATH. Please restart your terminal or run: source $SHELL_RC${NC}"
    else
        echo -e "${YELLOW}[WARN] Could not determine shell config file. Please add $INSTALL_DIR to your PATH manually.${NC}"
    fi
else
    echo -e "${GREEN}[OK] Already in PATH.${NC}"
fi

# 8. Cleanup
# [FIX] Removed the deletion of source files. They should stay for transparency and manual rebuilding.

echo -e "\n${CYAN}[SUCCESS] Glupe installed successfully!${NC}"
echo "Run 'glupe --help' to get started."