# GLUPE_OUT: output path. Default: glupe (local). In devcontainer: build/glupe
OUT ?= $(or $(GLUPE_OUT),glupe)

glupe:
	g++ glupec.cpp -o $(OUT) -std=c++17 -lstdc++fs -static-libgcc -static-libstdc++
