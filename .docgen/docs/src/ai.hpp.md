# AI Core Module Documentation

## Overview

The AI Core module provides functionality to interact with various AI models and services, including local and cloud-based providers. It handles API calls, response parsing, and error handling, enabling seamless integration with different AI protocols.

## Functions

### `callAI(string prompt)`

**Purpose:** Sends a prompt to the configured AI provider and returns the raw response.

**Behavior:**
- Constructs the API request based on the current `PROTOCOL` (Google, OpenAI, or Ollama).
- Handles API-specific headers, authentication, and request body formatting.
- Retries up to 3 times if the API returns a rate limit error (429) for Google.
- Logs raw responses in verbose mode and checks for common error codes (401, 404).
- Uses `curl` to send the HTTP request and `execCmd` to execute the command.

**Returns:** The raw JSON response from the AI provider or an error message.

---

### `extractCode(string jsonResponse)`

**Purpose:** Extracts and cleans the code or text from the AI's JSON response.

**Behavior:**
- Parses the JSON response to locate the relevant content.
- Handles different response formats based on the provider (Google, OpenAI, Ollama).
- Extracts code blocks enclosed in triple backticks (` ``` `) if present.
- Returns error messages if the response is empty, malformed, or contains API errors.

**Returns:** The extracted code or text, or an error message.

---

### `explainFatalError(const string& errorMsg)`

**Purpose:** Analyzes fatal errors and provides AI-generated advice for resolution.

**Behavior:**
- Constructs a prompt with the error message and sends it to the AI.
- Parses the AI's response to extract the proposed solution.
- Displays the solution in the console.

**Usage:** Called when a fatal error occurs to assist with debugging.

---

### `selectOllamaModel()`

**Purpose:** Allows the user to select and configure a local Ollama model.

**Behavior:**
- Queries the Ollama API (`/api/tags`) to retrieve installed models.
- Displays the list of models and prompts the user to select one.
- Updates the `config.json` file with the selected model under the `local` profile.

**Usage:** Useful for switching between local models without manually editing the config.

---

### `openApiKeyPage()`

**Purpose:** Opens the ApiFreeLlm.com API access page in the default browser.

**Behavior:**
- Uses platform-specific commands (`start` on Windows, `xdg-open` on Unix-like systems) to open the URL.

**Usage:** Helps users obtain API keys for cloud-based services.

## Configuration Dependencies

- Relies on global variables (`PROVIDER`, `PROTOCOL`, `API_KEY`, `MODEL_ID`, `API_URL`, `MAX_RETRIES`) defined in `config.hpp`.
- Uses `loadConfig`, `updateConfigFile`, and `showConfig` functions from `config.hpp` for managing settings.
- Assumes the presence of `utils.hpp` for `execCmd` and `CmdResult` functionality.

## Error Handling

- Returns descriptive error messages for API issues (e.g., unauthorized access, rate limits).
- Catches JSON parsing exceptions and provides truncated responses for debugging.
- Logs warnings and errors to the console for user visibility.

## Usage Notes

- Ensure the `config.json` file is correctly configured before using AI-related functions.
- Use `VERBOSE_MODE` to enable detailed logging for debugging purposes.
- The module is designed to work with both local and cloud AI providers, with automatic protocol detection and request formatting.