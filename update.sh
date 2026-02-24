#!/bin/bash

REPO="M-MACHINE/glupe"
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
echo "Fetching latest release information from GitHub..."

# Get the latest release tag
LATEST_RELEASE_TAG=$(curl -s "https://api.github.com/repos/${REPO}/releases/latest" | grep -Po '"tag_name": "\K.*?(?=")')

if [ -z "$LATEST_RELEASE_TAG" ]; then
    echo "Error: Could not fetch latest release tag from GitHub."
    exit 1
fi

echo "Latest release: $LATEST_RELEASE_TAG"
DOWNLOAD_URL="https://github.com/${REPO}/releases/download/${LATEST_RELEASE_TAG}/${GLUPE_BIN_NAME}"

echo "Downloading new glupe binary from $DOWNLOAD_URL to $TEMP_BIN..."
if ! curl -L -o "$TEMP_BIN" "$DOWNLOAD_URL"; then
    echo "Error: Failed to download new glupe binary."
    rm -f "$TEMP_BIN"
    exit 1
fi

chmod +x "$TEMP_BIN"
echo "Downloaded and made executable."

echo "Replacing current glupe executable..."
if mv "$TEMP_BIN" "$CURRENT_GLUPE_PATH"; then
    echo "Successfully updated glupe to $LATEST_RELEASE_TAG!"
    echo "Please restart your terminal or shell to ensure the new version is loaded."
else
    echo "Error: Failed to replace glupe executable."
    echo "You might need to run this script with 'sudo' or manually replace '$CURRENT_GLUPE_PATH' with '$TEMP_BIN'."
    rm -f "$TEMP_BIN"
    exit 1
fi

echo "--- Update Complete ---"