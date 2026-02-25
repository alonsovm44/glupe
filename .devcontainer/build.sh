#!/bin/bash
# Build Glupe to build/ (devcontainer only). Keeps workspace root clean on host.
# GLUPE_OUT is set in devcontainer.json so make outputs to build/glupe.
set -e
cd "$(dirname "$0")/.."
mkdir -p build
sudo chown vscode:vscode build
make

# Make glupe available globally via ~/.local/bin
mkdir -p ~/.local/bin
cp "$(pwd)/build/glupe" ~/.local/bin/glupe
grep -q '\.local/bin' ~/.bashrc || echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
