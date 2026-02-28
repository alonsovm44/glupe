# refactoring plan
Here is a plan to refactor glupec.cpp into modular .hpp files. This approach uses C++17 inline variables to allow global state to exist in headers without linking errors, keeping the architecture simple while separating concerns.

## Proposed File Structure
File	Responsibility	Dependencies
src/common.hpp	Standard includes, json alias, CmdResult struct.	None
src/utils.hpp	Logging, execCmd, string helpers, error detection.	common.hpp
src/config.hpp	Global config (API_KEY, PROVIDER), loadConfig.	utils.hpp
src/languages.hpp	LangProfile struct, language databases (LANG_DB).	common.hpp
src/ai.hpp	callAI, extractCode, provider logic.	config.hpp
src/cache.hpp	SYMBOL_TABLE, .glupe.lock logic, cache I/O.	common.hpp
src/parser.hpp	resolveImports, validateContainers, stripTemplates.	utils.hpp, cache.hpp
src/processor.hpp	processInputWithCache, performTreeShaking.	ai.hpp, parser.hpp
src/hub.hpp	Login/Signup, startInteractiveHub.	config.hpp
src/glupec.cpp	Main entry point, CLI argument parsing.	All above
Detailed Breakdown

1. src/common.hpp
Includes: iostream, vector, string, filesystem, nlohmann/json.hpp.
Typedefs: using json = nlohmann::json;, namespace fs = std::filesystem;.
Structs: CmdResult.

2. src/utils.hpp
Logging: initLogger(), log().
System: execCmd().
Helpers: stripExt(), getExt(), formatDuration().
Heuristics: isFatalError(), detectIfCodeIsSpaghetti().
Timer: ExecutionTimer struct.

3. src/config.hpp
Globals (inline): PROVIDER, PROTOCOL, API_KEY, MODEL_ID, MAX_RETRIES, VERBOSE_MODE.
Functions: loadConfig(), updateConfigFile(), showConfig().

4. src/languages.hpp
Structs: LangProfile.
Globals (inline): LANG_DB, MODEL_DB, IMAGE_DB, CURRENT_LANG, CURRENT_MODE.
Functions: selectTarget().

5. src/ai.hpp
Functions: callAI(), extractCode(), explainFatalError(), selectOllamaModel(), openApiKeyPage().

6. src/cache.hpp
Structs: SemanticNode, NodeType.
Globals (inline): SYMBOL_TABLE, LOCK_DATA, CACHE_DIR.
Functions: initCache(), saveCache(), getContainerHash(), getCachedContent(), setCachedContent().

7. src/parser.hpp
Functions: decommentGlupeSyntax(), resolveImports(), stripTemplates(), validateContainers(), processExports(), extractDependencies(), preFlightCheck(), splitSourceCode(), extractSignatures(), stripMetadata(), showMetadata(), sanitize_container_syntax().

8. src/processor.hpp
Functions:
processInputWithCache(): The complex logic parsing $$ blocks, checking cache, and calling AI.
performTreeShaking(): AI-based code optimization.
updateCacheFromOutput(): Parsing AI response to update lockfile.
9. src/hub.hpp
Functions: getSession(), saveSession(), checkLogin(), startInteractiveHub().

10. src/glupec.cpp (Main)
Includes: All the above headers.
Logic:
CLI argument parsing loop.
Command dispatch (fix, diff, hub, etc.).
Main compilation loop (Make/Series/Standard modes).
Toolchain verification and execution.
Implementation Strategy
Create src/common.hpp first to establish the base.
Move Utilities: Extract utils.hpp and logger logic.
Move Config & Langs: Extract config.hpp and languages.hpp.
Move AI: Extract ai.hpp.
Move Core Logic: Extract cache.hpp, parser.hpp, and processor.hpp.
Move Hub: Extract hub.hpp.
Clean Main: Replace code in glupec.cpp with includes and ensure the main logic flow remains intact.
