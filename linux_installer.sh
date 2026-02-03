#!/bin/bash
# Yori Compiler Installer for Linux & macOS
# Fixed version: Now downloads source code from GitHub automatically

set -e  # Exit on error

# --- CONFIGURATION ---
REPO_URL="https://raw.githubusercontent.com/alonsovm44/yori/master"
CPP_FILE="yoric.cpp"
INSTALL_DIR="$HOME/.yori"
BIN_DIR="$INSTALL_DIR/bin"
CONFIG_DIR="$INSTALL_DIR/config"

# --- COLORS ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' 

# --- FUNCTIONS ---
info() { echo -e "${CYAN}ℹ ${1}${NC}"; }
success() { echo -e "${GREEN}✓ ${1}${NC}"; }
warn() { echo -e "${YELLOW}⚠ ${1}${NC}"; }
error() { echo -e "${RED}✗ ${1}${NC}"; }

# Banner
echo -e "${CYAN}"
cat << "EOF"
╔══════════════════════════════════════╗
║      YORI COMPILER INSTALLER         ║
║           Version 5.4 (Fixed)        ║
╚══════════════════════════════════════╝
EOF
echo -e "${NC}"

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     MACHINE=Linux;;
    Darwin*)    MACHINE=Mac;;
    *)          MACHINE="UNKNOWN:${OS}"
esac
info "Detected OS: $MACHINE"

# 1. Create directory structure
info "Setting up installation directories..."
mkdir -p "$BIN_DIR"
mkdir -p "$CONFIG_DIR"

# 2. Download Source Code
# This fixes the issue where the script assumed files were already present
info "Downloading source code from GitHub..."
if ! curl -fsSL "${REPO_URL}/${CPP_FILE}" -o "${INSTALL_DIR}/${CPP_FILE}"; then
    error "Failed to download ${CPP_FILE}. Check your internet or the repo URL."
    exit 1
fi
success "Source code downloaded"

# 3. Check Dependencies
HAS_GCC=$(command -v g++ >/dev/null 2>&1 && echo "yes" || echo "no")

if [ "$HAS_GCC" == "no" ]; then
    warn "g++ not found. Attempting to install..."
    if [ "$MACHINE" == "Linux" ]; then
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get update && sudo apt-get install -y g++ build-essential
        elif command -v dnf >/dev/null 2>&1; then
            sudo dnf install -y gcc-c++
        elif command -v pacman >/dev/null 2>&1; then
            sudo pacman -S --noconfirm gcc
        else
            error "Could not auto-install g++. Please install it manually."
            exit 1
        fi
    elif [ "$MACHINE" == "Mac" ]; then
        if ! command -v brew >/dev/null 2>&1; then
            warn "Homebrew not found. Please install g++ via Xcode Command Line Tools: xcode-select --install"
        else
            brew install gcc
        fi
    fi
fi

# 4. Compile Yori
info "Compiling Yori..."
# We try compiling with -lstdc++fs first, fallback if it fails
g++ -std=c++17 "${INSTALL_DIR}/${CPP_FILE}" -o "${BIN_DIR}/yori" -lstdc++fs 2>/dev/null || \
g++ -std=c++17 "${INSTALL_DIR}/${CPP_FILE}" -o "${BIN_DIR}/yori"

if [ -f "${BIN_DIR}/yori" ]; then
    chmod +x "${BIN_DIR}/yori"
    success "Yori compiled successfully"
else
    error "Compilation failed. Please check your compiler."
    exit 1
fi

# Cleanup source file (optional, keeps system clean)
# rm "${INSTALL_DIR}/${CPP_FILE}"

# 5. Handle Ollama (Interactive)
echo ""
read -p "Do you want to install Ollama for Local AI? [y/N]: " INSTALL_OLLAMA
if [ "$INSTALL_OLLAMA" == "y" ] || [ "$INSTALL_OLLAMA" == "Y" ]; then
    if ! command -v ollama >/dev/null 2>&1; then
        info "Installing Ollama..."
        curl -fsSL https://ollama.com/install.sh | sh
        success "Ollama installed"
    else
        success "Ollama already installed"
    fi
    
    # Optional: Pull model
    read -p "Download default model (qwen2.5-coder:3b ~2GB)? [y/N]: " PULL_MODEL
    if [ "$PULL_MODEL" == "y" ]; then
        ollama pull qwen2.5-coder:3b
    fi
fi

# 6. Create Config
info "Creating config file..."
cat > "${CONFIG_DIR}/config.json" << 'EOF'
{
    "local": {
        "model_id": "qwen2.5-coder:3b",
        "api_url": "http://localhost:11434/api/generate"
    },
    "cloud": {
        "api_key": "YOUR_API_KEY_HERE",
        "model_id": "gemini-1.5-flash"
    },
    "toolchains": {
        "cpp": {
            "build_cmd": "g++ -std=c++17"
        }
    }
}
EOF
success "Config created"

# 7. Add to PATH
# Detect shell profile
if [ -n "$ZSH_VERSION" ]; then
    PROFILE="$HOME/.zshrc"
elif [ -n "$BASH_VERSION" ]; then
    PROFILE="$HOME/.bashrc"
else
    PROFILE="$HOME/.profile"
fi

# Check if already in path
if ! grep -q "yori/bin" "$PROFILE" 2>/dev/null; then
    echo "" >> "$PROFILE"
    echo "# Yori Compiler" >> "$PROFILE"
    echo "export PATH