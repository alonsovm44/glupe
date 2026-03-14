# Configuration System Documentation

## Overview

This module provides a configuration system for managing settings related to AI providers, protocols, models, and toolchains. It includes functionality to load, update, and display configuration settings from a `config.json` file. The system supports both local and cloud-based AI providers, with customizable parameters for each.

## Configuration Variables

### Global Variables

- **`PROVIDER`**: Current AI provider (`local` or `cloud`).
- **`PROTOCOL`**: Communication protocol (`ollama`, `google`, `openai`).
- **`API_KEY`**: API key for cloud providers.
- **`MODEL_ID`**: Model identifier for the selected provider.
- **`API_URL`**: Endpoint URL for API requests.
- **`MAX_RETRIES`**: Maximum number of retries for API requests.

## Functions

### `loadConfig(string mode)`

**Purpose**: Loads configuration settings from `config.json` based on the specified mode (`local` or `cloud`). If the file does not exist, default settings are applied.

**Behavior**:
- Parses `config.json` if it exists.
- Sets default values if the file is missing or the mode is not configured.
- Updates global variables (`PROTOCOL`, `API_URL`, `MODEL_ID`, `API_KEY`, `MAX_RETRIES`) based on the configuration.
- Overrides toolchain settings (e.g., `buildCmd`, `versionCmd`) for specific languages if defined in the config.

**Returns**: `true` if configuration is loaded successfully, `false` otherwise.

### `updateConfigFile(string key, string value)`

**Purpose**: Updates specific configuration settings in `config.json`.

**Behavior**:
- Validates the provided key and value.
- Updates the corresponding field in the JSON configuration.
- Saves the updated configuration to `config.json`.
- Provides feedback on the success or failure of the update.

**Supported Keys**:
- `api-key`: Updates the API key for cloud providers.
- `model-cloud`/`model-local`: Updates the model ID for cloud/local providers.
- `url-cloud`/`url-local`: Updates the API URL for cloud/local providers.
- `cloud-protocol`: Updates the protocol for cloud providers (must be `google` or `openai`).
- `max-retries`: Updates the maximum number of retries for API requests.

### `showConfig()`

**Purpose**: Displays the current configuration settings from `config.json`.

**Behavior**:
- Reads `config.json` and prints its contents in a formatted manner.
- Masks sensitive information like API keys.
- Handles cases where `config.json` is missing or corrupted.

## Usage Examples

### Loading Configuration
```cpp
loadConfig("local"); // Load local provider settings
loadConfig("cloud"); // Load cloud provider settings
```

### Updating Configuration
```cpp
updateConfigFile("api-key", "your_api_key_here");
updateConfigFile("model-cloud", "gemini-pro");
updateConfigFile("max-retries", "20");
```

### Displaying Configuration
```cpp
showConfig(); // Display current configuration settings
```

## Error Handling

- **File Errors**: If `config.json` is missing or corrupted, default values are used or errors are logged.
- **Validation Errors**: Invalid keys or values result in error messages without modifying the configuration.

## Dependencies

- **`utils.hpp`**: Provides utility functions and logging.
- **`languages.hpp`**: Defines language profiles and toolchains.
- **JSON Library**: Used for parsing and updating `config.json`.

## Notes

- The configuration system is designed to be flexible, allowing users to override default settings for both local and cloud AI providers.
- Toolchain overrides enable customization of build and version commands for specific programming languages.
- Sensitive information like API keys is masked when displayed to prevent accidental exposure.