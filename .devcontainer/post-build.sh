#!/bin/bash
# Post-build setup: init config if needed, set Ollama URL for host.
set -e
cd "$(dirname "$0")/.."

./build/glupe config url-local 'http://host.docker.internal:11434/api/generate'
