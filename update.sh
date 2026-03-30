#!/usr/bin/env bash
# Safe updater for glupe (Linux/macOS)
set -euo pipefail

REPO_RAW="https://raw.githubusercontent.com/alonsovm44/glupe/master"
JSON_URL="https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"

echo "--- Glupe Update Script (Linux/macOS) ---"

# locate installed glupe
CURRENT_GLUPE_PATH="$(command -v glupe || true)"
if [ -z "$CURRENT_GLUPE_PATH" ]; then
  echo "Error: 'glupe' not found in PATH. Re-run installer or add to PATH."
  exit 1
fi
GLUPE_DIR="$(dirname "$CURRENT_GLUPE_PATH")"
echo "Current glupe path: $CURRENT_GLUPE_PATH"
SRC_DIR="$GLUPE_DIR/src"
mkdir -p "$SRC_DIR"

VENDOR_DIR="$GLUPE_DIR/vendor"
if [ ! -d "$VENDOR_DIR" ] && [ -d "$HOME/.glupe/vendor" ]; then
  VENDOR_DIR="$HOME/.glupe/vendor"
fi

# download sources (best-effort)
SOURCE_FILES="glupec.cpp common.hpp utils.hpp config.hpp languages.hpp ai.hpp cache.hpp parser.hpp processor.hpp hub.hpp ast.hpp ast_utils.hpp glupe.l glupe.y"
for f in $SOURCE_FILES; do
  url="$REPO_RAW/src/$f"
  if ! curl -fsSL "$url" -o "$SRC_DIR/$f"; then
    echo "Warning: failed to download $f from $url (file may be missing). Continuing..."
  else
    echo "Fetched $f"
  fi
done
curl -fsSL "$JSON_URL" -o "$SRC_DIR/json.hpp" || echo "Warning: could not fetch json.hpp"

# parser/lexer gen or fallback
GEN_OK=0
if command -v bison >/dev/null 2>&1 && command -v flex >/dev/null 2>&1; then
  if bison -d -o "$SRC_DIR/glupe.tab.c" "$SRC_DIR/glupe.y" && flex -o "$SRC_DIR/lex.yy.c" "$SRC_DIR/glupe.l"; then
    GEN_OK=1
    echo "Generated parser and lexer"
  else
    echo "Warning: bison/flex ran but generation failed"
  fi
fi
if [ "$GEN_OK" -eq 0 ]; then
  echo "Attempting to download pre-generated parser/lexer..."
  curl -fsSL "$REPO_RAW/src/glupe.tab.c" -o "$SRC_DIR/glupe.tab.c" || true
  curl -fsSL "$REPO_RAW/src/lex.yy.c" -o "$SRC_DIR/lex.yy.c" || true
fi

# choose compiler
COMPILER=""
if command -v g++ >/dev/null 2>&1; then COMPILER="g++"
elif command -v clang++ >/dev/null 2>&1; then COMPILER="clang++"
fi
if [ -z "$COMPILER" ]; then
  echo "No C++ compiler found. Please install g++ or clang++ and re-run."
  exit 1
fi
echo "Using compiler: $COMPILER"

CC_COMPILER="gcc"
if ! command -v gcc >/dev/null 2>&1; then
  if command -v clang >/dev/null 2>&1; then CC_COMPILER="clang"
  else CC_COMPILER="$COMPILER"; fi
fi

# Download and build tree-sitter objects if missing
if [ ! -f "$VENDOR_DIR/tree-sitter.o" ] || [ ! -f "$VENDOR_DIR/cpp_parser.o" ]; then
  echo "Tree-sitter objects missing. Fetching and building..."
  mkdir -p "$VENDOR_DIR"
  
  if [ ! -d "$VENDOR_DIR/tree-sitter" ]; then
    echo "Downloading tree-sitter (0.22.6)..."
    curl -fsSL "https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.tar.gz" | tar -xz -C "$VENDOR_DIR"
    mv "$VENDOR_DIR/tree-sitter-0.22.6" "$VENDOR_DIR/tree-sitter" 2>/dev/null || true
  fi
  
  if [ -f "$VENDOR_DIR/tree-sitter/lib/src/lib.c" ]; then
    "$CC_COMPILER" -O3 -I"$VENDOR_DIR/tree-sitter/lib/include" -I"$VENDOR_DIR/tree-sitter/lib/src" -c "$VENDOR_DIR/tree-sitter/lib/src/lib.c" -o "$VENDOR_DIR/tree-sitter.o" || echo "Warning: Failed to compile tree-sitter core"
  fi

  for lang in cpp python javascript java go rust; do
    if [ "$lang" = "cpp" ] || [ "$lang" = "java" ]; then ver="0.22.0"; elif [ "$lang" = "python" ]; then ver="0.21.0"; else ver="0.21.2"; fi
    if [ ! -d "$VENDOR_DIR/tree-sitter-$lang" ]; then
      echo "Downloading tree-sitter-$lang ($ver)..."
      curl -fsSL "https://github.com/tree-sitter/tree-sitter-$lang/archive/refs/tags/v${ver}.tar.gz" | tar -xz -C "$VENDOR_DIR"
      mv "$VENDOR_DIR/tree-sitter-${lang}-${ver}" "$VENDOR_DIR/tree-sitter-$lang" 2>/dev/null || true
    fi
    srcDirTS="$VENDOR_DIR/tree-sitter-$lang/src"
    if [ -f "$srcDirTS/parser.c" ]; then
      "$CC_COMPILER" -O3 -I"$srcDirTS" -c "$srcDirTS/parser.c" -o "$VENDOR_DIR/${lang}_parser.o" || echo "Warning: Failed to compile $lang parser"
    fi
    if [ -f "$srcDirTS/scanner.c" ]; then
      "$CC_COMPILER" -O3 -I"$srcDirTS" -c "$srcDirTS/scanner.c" -o "$VENDOR_DIR/${lang}_scanner.o" || echo "Warning: Failed to compile $lang scanner"
    elif [ -f "$srcDirTS/scanner.cc" ]; then
      "$COMPILER" -O3 -I"$srcDirTS" -c "$srcDirTS/scanner.cc" -o "$VENDOR_DIR/${lang}_scanner.o" || echo "Warning: Failed to compile $lang scanner"
    fi
  done
fi

# compile to safe temporary file
TMP_BIN="$(mktemp "/tmp/glupe.XXXXXX")"
chmod 700 "$TMP_BIN"
TS_INCLUDE_DIR="$VENDOR_DIR/tree-sitter/lib/include"
COMPILE_ARGS=( "$SRC_DIR/glupec.cpp" "$SRC_DIR/lex.yy.c" "$SRC_DIR/glupe.tab.c" -o "$TMP_BIN" -std=c++17 -O3 -pthread -I "$SRC_DIR" )

# Add tree-sitter objects if present
if [ -d "$TS_INCLUDE_DIR" ]; then
  COMPILE_ARGS+=( -I "$TS_INCLUDE_DIR" )
fi

if [ -f "$VENDOR_DIR/tree-sitter.o" ]; then COMPILE_ARGS+=( "$VENDOR_DIR/tree-sitter.o" ); fi
for lang in cpp python javascript java go rust ruby c typescript; do
  if [ -f "$VENDOR_DIR/${lang}_parser.o" ]; then COMPILE_ARGS+=( "$VENDOR_DIR/${lang}_parser.o" ); fi
  if [ -f "$VENDOR_DIR/${lang}_scanner.o" ]; then COMPILE_ARGS+=( "$VENDOR_DIR/${lang}_scanner.o" ); fi
done
# older libstdc++ may need -lstdc++fs on Linux
if [ "$(uname -s)" = "Linux" ]; then COMPILE_ARGS+=( -lstdc++fs ); fi

echo "Compiling with: $COMPILER ${COMPILE_ARGS[*]}"
if ! "$COMPILER" "${COMPILE_ARGS[@]}"; then
  echo "Error: compilation failed. Check required dev packages (g++, bison/flex if needed)."
  rm -f "$TMP_BIN"; exit 1
fi
echo "Compilation produced $TMP_BIN"

# backup existing binary
BACKUP="$CURRENT_GLUPE_PATH.$(date +%s).bak"
if ! cp -p "$CURRENT_GLUPE_PATH" "$BACKUP"; then
  echo "Warning: could not create backup. You may need sudo to replace the system binary."
fi

# attempt atomic replace; use sudo if needed
if mv "$TMP_BIN" "$CURRENT_GLUPE_PATH" 2>/dev/null; then
  chmod +x "$CURRENT_GLUPE_PATH" || true
  echo "Successfully updated glupe (replaced in-place). Backup (if created): $BACKUP"
else
  echo "mv failed: trying with sudo..."
  if command -v sudo >/dev/null 2>&1 && sudo mv "$TMP_BIN" "$CURRENT_GLUPE_PATH"; then
    sudo chmod +x "$CURRENT_GLUPE_PATH" || true
    echo "Successfully updated glupe with sudo. Backup (if created): $BACKUP"
  else
    echo "Error: could not move new binary into place. Keep $TMP_BIN and restore manually or re-run with sudo."
    rm -f "$TMP_BIN"
    exit 1
  fi
fi

echo "Update complete. Please restart shells if necessary."