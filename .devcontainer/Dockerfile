# Glupe Dev Container
# Dependencies: g++ (C++17), make, curl
# Ollama runs on the host; container connects via host.docker.internal
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# Install build-essential (g++, make), curl
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Glupe is built in postCreateCommand (workspace is mounted later)
USER vscode
