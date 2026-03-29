#!/usr/bin/env bash
# Glupe installer (improved portability)
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/alonsovm44/glupe/master/install.sh | bash
set -euo pipefail

REPO_RAW="https://raw.githubusercontent.com/alonsovm44/glupe/master"
REPO_URL="https://github.com/alonsovm44/glupe"
JSON_URL="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
INSTALL_DIR="${HOME:-$PWD}/.glupe"
EXE_PATH="$INSTALL_DIR/glupe"

# colors (safe if terminal doesn't support)
CYAN='\033[0;36m'; GREEN='\033[0;32m'; YELLOW='\033[0;33m'; RED='\033[0;31m'; NC='\033[0m'

echo -e "${CYAN}--- Glupe Installer ---${NC}"
echo "Install dir: $INSTALL_DIR"

# Ensure bash exists (Alpine's /bin/sh is ash by default)
if ! command -v bash >/dev/null 2>&1; then
  echo -e "${YELLOW}[WARN] bash not found. On Alpine, run: apk add --no-cache bash${NC}"
  echo "You can continue if you run this script with bash explicitly (bash ./install.sh)."
fi

# Detect package manager
PM=""
SUDO=""
if command -v sudo >/dev/null 2>&1; then SUDO="sudo"; fi
if command -v apk >/dev/null 2>&1; then PM="apk"
elif command -v apt-get >/dev/null 2>&1; then PM="apt"
elif command -v dnf >/dev/null 2>&1; then PM="dnf"
elif command -v pacman >/dev/null 2>&1; then PM="pacman"
elif command -v brew >/dev/null 2>&1; then PM="brew"
fi

install_packages() {
  case "$PM" in
    apk)
      echo "[INFO] Installing build deps with apk..."
      $SUDO apk add --no-cache build-base bison flex cmake make curl tar git bash
      ;;
    apt)
      echo "[INFO] Installing build deps with apt..."
      $SUDO apt-get update
      $SUDO apt-get install -y build-essential bison flex cmake make curl tar git
      ;;
    dnf)
      echo "[INFO] Installing build deps with dnf..."
      $SUDO dnf install -y @development-tools bison flex cmake make curl tar git
      ;;
    pacman)
      echo "[INFO] Installing build deps with pacman..."
      $SUDO pacman -Sy --noconfirm base-devel bison flex cmake make curl tar git
      ;;
    brew)
      echo "[INFO] Installing build deps with brew..."
      brew install bison flex cmake make curl git
      ;;
    *)
      echo -e "${YELLOW}[WARN] Could not detect supported package manager. Please install: gcc/g++, make, bison, flex, cmake, curl, tar, git${NC}"
      return 1
      ;;
  esac
  return 0
}

# 1) Check compiler
COMPILER=""
if command -v g++ >/dev/null 2>&1; then COMPILER="g++"
elif command -v clang++ >/dev/null 2>&1; then COMPILER="clang++"
fi

if [ -z "$COMPILER" ]; then
  echo -e "${RED}[ERROR] No C++ compiler found.${NC}"
  if [ -n "$PM" ]; then
    if [ "${AUTO_YES:-}" = "1" ] || [ "${CI:-}" = "true" ]; then ans="Y"; else read -r -p "Install build dependencies now using $PM? [Y/n] " ans; fi
    ans=${ans:-Y}
    if [[ "$ans" =~ ^[Yy]$ ]]; then
      install_packages || true
      # re-check
      if command -v g++ >/dev/null 2>&1; then COMPILER="g++"
elif command -v clang++ >/dev/null 2>&1; then COMPILER="clang++"
      fi
    fi
  fi
  if [ -z "$COMPILER" ]; then
    echo "Please install a C++17 compiler (g++ or clang++) and re-run installer."
    exit 1
  fi
fi
echo -e "${GREEN}[OK] Compiler: $COMPILER${NC}"

# 1.5) Check for Ollama
if command -v ollama >/dev/null 2>&1; then
  echo -e "${GREEN}[OK] Ollama found.${NC}"
else
  echo -e "${YELLOW}[INFO] Ollama not found. Skipping automatic install on minimal distros.${NC}"
  if [ "${AUTO_YES:-}" = "1" ] || [ "${CI:-}" = "true" ]; then ans="N"; else read -r -p "Attempt to run Ollama installer? (not recommended on Alpine) [y/N] " ans; fi
  ans=${ans:-N}
  if [[ "$ans" =~ ^[Yy]$ ]]; then
    echo "Running Ollama installer..."
    set +e
    curl -fsSL https://ollama.com/install.sh | sh || echo -e "${YELLOW}[WARN] Ollama install may have failed.${NC}"
    set -e
  fi
fi

# 2) Create directories
mkdir -p "$INSTALL_DIR/src" "$INSTALL_DIR/vendor"
echo -e "${GREEN}[OK] Created $INSTALL_DIR${NC}"

# 4) Download source files
echo "[INFO] Downloading source files..."
SOURCE_FILES="glupec.cpp common.hpp utils.hpp config.hpp languages.hpp ai.hpp cache.hpp parser.hpp processor.hpp hub.hpp ast.hpp ast_utils.hpp glupe.l glupe.y"
for f in $SOURCE_FILES; do
  url="$REPO_RAW/src/$f"
  if ! curl -fsSL "$url" -o "$INSTALL_DIR/src/$f"; then
    echo -e "${YELLOW}[WARN] Could not download $f from $url. Continuing; build may fail.${NC}"
  else
    echo -e "${GREEN}[OK] Fetched $f${NC}"
  fi
done

# download json.hpp
if ! curl -fsSL "$JSON_URL" -o "$INSTALL_DIR/src/json.hpp"; then
  echo -e "${YELLOW}[WARN] failed to download json.hpp${NC}"
else
  echo -e "${GREEN}[OK] Fetched json.hpp${NC}"
fi

# 4.5 Generate parser & lexer
echo "[INFO] Generating parser and lexer with bison/flex (if available)..."
GEN_OK=0
if command -v bison >/dev/null 2>&1 && command -v flex >/dev/null 2>&1; then
  if bison -d -o "$INSTALL_DIR/src/glupe.tab.c" "$INSTALL_DIR/src/glupe.y" && \
     flex -o "$INSTALL_DIR/src/lex.yy.c" "$INSTALL_DIR/src/glupe.l"; then
    GEN_OK=1
    echo -e "${GREEN}[OK] Generated glupe.tab.c and lex.yy.c${NC}"
  else
    echo -e "${YELLOW}[WARN] bison/flex ran but generation failed.${NC}"
  fi
fi

# If generation failed, try to download pre-generated C files (if available)
if [ "$GEN_OK" -eq 0 ]; then
  echo "[INFO] Attempting to fetch pre-generated parser/lexer from repo..."
  if curl -fsSL "$REPO_RAW/src/glupe.tab.c" -o "$INSTALL_DIR/src/glupe.tab.c" && \
     curl -fsSL "$REPO_RAW/src/lex.yy.c" -o "$INSTALL_DIR/src/lex.yy.c"; then
    echo -e "${GREEN}[OK] downloaded pre-generated parser/lexer${NC}"
  else
    echo -e "${YELLOW}[WARN] Could not generate or download parser/lexer. Compilation may fail.${NC}"
  fi
fi

# 4.8 Download tree-sitter sources (if missing)
if [ ! -d "$INSTALL_DIR/vendor/tree-sitter" ]; then
  echo "[INFO] Downloading tree-sitter sources..."
  curl -fsSL "https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.tar.gz" | tar -xz -C "$INSTALL_DIR/vendor" || true
  mv "$INSTALL_DIR/vendor/tree-sitter-0.22.6" "$INSTALL_DIR/vendor/tree-sitter" 2>/dev/null || true
fi
if [ ! -d "$INSTALL_DIR/vendor/tree-sitter-cpp" ]; then
  curl -fsSL "https://github.com/tree-sitter/tree-sitter-cpp/archive/refs/tags/v0.22.0.tar.gz" | tar -xz -C "$INSTALL_DIR/vendor" || true
  mv "$INSTALL_DIR/vendor/tree-sitter-cpp-0.22.0" "$INSTALL_DIR/vendor/tree-sitter-cpp" 2>/dev/null || true
fi

C_COMPILER="gcc"
if [ "$COMPILER" = "clang++" ]; then C_COMPILER="clang"; fi

# try to build optional tree-sitter objects (best-effort)
if [ -f "$INSTALL_DIR/vendor/tree-sitter/lib/src/lib.c" ]; then
  $C_COMPILER -O3 -I"$INSTALL_DIR/vendor/tree-sitter/lib/include" -I"$INSTALL_DIR/vendor/tree-sitter/lib/src" -c "$INSTALL_DIR/vendor/tree-sitter/lib/src/lib.c" -o "$INSTALL_DIR/vendor/tree-sitter.o" || true
fi
if [ -f "$INSTALL_DIR/vendor/tree-sitter-cpp/src/parser.c" ]; then
  $C_COMPILER -O3 -I"$INSTALL_DIR/vendor/tree-sitter-cpp/src" -c "$INSTALL_DIR/vendor/tree-sitter-cpp/src/parser.c" -o "$INSTALL_DIR/vendor/parser.o" || true
fi
if [ -f "$INSTALL_DIR/vendor/tree-sitter-cpp/src/scanner.c" ]; then
  $C_COMPILER -O3 -I"$INSTALL_DIR/vendor/tree-sitter-cpp/src" -c "$INSTALL_DIR/vendor/tree-sitter-cpp/src/scanner.c" -o "$INSTALL_DIR/vendor/scanner.o" || true
fi
if [ -f "$INSTALL_DIR/vendor/tree-sitter-cpp/src/scanner.cc" ]; then
  $COMPILER -O3 -I"$INSTALL_DIR/vendor/tree-sitter-cpp/src" -c "$INSTALL_DIR/vendor/tree-sitter-cpp/src/scanner.cc" -o "$INSTALL_DIR/vendor/scanner.o" || true
fi

# 5) Compile
echo "[INFO] Compiling Glupe..."
COMPILE_CMD=("$COMPILER" "$INSTALL_DIR/src/glupec.cpp" "$INSTALL_DIR/src/lex.yy.c" "$INSTALL_DIR/src/glupe.tab.c" "$INSTALL_DIR/vendor/tree-sitter.o" "$INSTALL_DIR/vendor/parser.o" "$INSTALL_DIR/vendor/scanner.o" -o "$EXE_PATH" -std=c++17 -O3 -pthread -I"$INSTALL_DIR/src" -I"$INSTALL_DIR/vendor/tree-sitter/lib/include")

if [[ "$(uname -s)" == "Linux" ]]; then
  COMPILE_CMD+=("-lstdc++fs")
fi

# run the compile command
echo "+ ${COMPILE_CMD[*]}"
if ! "${COMPILE_CMD[@]}"; then
  echo -e "${RED}[ERROR] Compilation failed. Check required dev packages (bison/flex/g++/make).${NC}"
  exit 1
fi
echo -e "${GREEN}[OK] Compilation successful: $EXE_PATH${NC}"

# 6) Create config
CONFIG_PATH="$INSTALL_DIR/config.json"
if [ ! -f "$CONFIG_PATH" ]; then
  cat > "$CONFIG_PATH" <<'EOF'
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

# 7) Add to PATH
SHELL_RC=""
case "${SHELL:-/bin/sh}" in
  */zsh) SHELL_RC="$HOME/.zshrc" ;; 
  */bash) SHELL_RC="$HOME/.bashrc" ;; 
  *) SHELL_RC="$HOME/.profile" ;;
esac

if ! echo "$PATH" | tr ':' '\n' | grep -qx "$INSTALL_DIR"; then
  if [ -f "$SHELL_RC" ]; then
    printf "\n# Glupe CLI\nexport PATH=\"\$PATH:%s\"\n" "$INSTALL_DIR" >> "$SHELL_RC"
    echo -e "${YELLOW}[OK] Added $INSTALL_DIR to $SHELL_RC. Run: source $SHELL_RC${NC}"
  else
    echo -e "${YELLOW}[WARN] Could not determine shell rc file. Add $INSTALL_DIR to PATH manually.${NC}"
  fi
else
  echo -e "${GREEN}[OK] Already in PATH.${NC}"
fi

echo -e "\n${CYAN}[SUCCESS] Glupe installed successfully!${NC}"
echo "Run: $EXE_PATH --help or glupe --help (after reloading shell)."