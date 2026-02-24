# Glupe Dev Container

This devcontainer builds Glupe and connects to **Ollama running on your host machine** (no Ollama inside the container).

## Host setup (required for `-local` mode)

Ollama must listen on all interfaces so the container can reach it:

**macOS:**
```bash
launchctl setenv OLLAMA_HOST "0.0.0.0:11434"
```
Then restart the Ollama app.

**Linux:**
```bash
sudo systemctl edit ollama.service
```
Add under `[Service]`:
```ini
Environment="OLLAMA_HOST=0.0.0.0:11434"
```
Then:
```bash
sudo systemctl daemon-reload && sudo systemctl restart ollama
```

**Windows:** Set `OLLAMA_HOST=0.0.0.0:11434` in Environment Variables, then restart Ollama.

## Usage

1. Start Ollama on the host and pull a model: `ollama pull qwen2.5-coder:3b`
2. Reopen the project in the devcontainer
3. Run: `glupe hello.glp -o hello.exe -cpp -local`

Glupe is built to `build/glupe` (keeps workspace root clean on the host volume). To rebuild: run task **Rebuild Glupe (devcontainer)** or `.devcontainer/build.sh`.
