# GLUPE_OUT: output path. Default: glupe (local). In devcontainer: build/glupe
OUT ?= $(or $(GLUPE_OUT),glupe)

# Compiler flags
CXXFLAGS = -std=c++17 -lstdc++fs -static-libgcc -static-libstdc++

glupe:
	g++ $(CXXFLAGS) glupec.cpp -o $(OUT)
	@echo "Glupe v6.0 MVP built successfully as '$(OUT)'"

# [NEW] Verification target for Glupe's self-hosting workflow
verify:
	g++ $(CXXFLAGS) $(SRC) -o $(OUT)
