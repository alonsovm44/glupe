#!/bin/bash
# Build Glupe to build/ (devcontainer only). Keeps workspace root clean on host.
# GLUPE_OUT is set in devcontainer.json so make outputs to build/glupe.
set -e
cd "$(dirname "$0")/.."
mkdir -p build
sudo chown vscode:vscode build

# Ensure dependencies (json.hpp) are present for v5.9
if [ ! -f "src/json.hpp" ]; then
    echo "Downloading json.hpp..."
    curl -fsSL "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" -o "src/json.hpp"
fi

echo "Compiling Glupe v5.9..."
g++ src/glupec.cpp -o build/glupe -std=c++17 -O3 -pthread -lstdc++fs -I src/

# Make glupe available globally via ~/.local/bin
mkdir -p ~/.local/bin
cp "$(pwd)/build/glupe" ~/.local/bin/glupe
grep -q '\.local/bin' ~/.bashrc || echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
