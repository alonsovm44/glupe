CXX ?= g++
CC ?= gcc
CXXFLAGS = -std=c++17 -O3 -pthread -Isrc -Ivendor/tree-sitter/lib/include
CFLAGS = -O3 -Ivendor/tree-sitter/lib/include
LDFLAGS = 

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lstdc++fs
endif

SRC = src/glupec.cpp src/lex.yy.c src/glupe.tab.c

TS_OBJS = vendor/tree-sitter.o \
          vendor/c_parser.o \
          vendor/cpp_parser.o vendor/cpp_scanner.o \
          vendor/javascript_parser.o vendor/javascript_scanner.o \
          vendor/python_parser.o vendor/python_scanner.o \
          vendor/typescript_parser.o vendor/typescript_scanner.o

glupe: $(SRC) $(TS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

vendor/tree-sitter.o: vendor/tree-sitter/lib/src/lib.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter/lib/src -c $< -o $@

vendor/c_parser.o: vendor/tree-sitter-c/src/parser.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-c/src -c $< -o $@

vendor/cpp_parser.o: vendor/tree-sitter-cpp/src/parser.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-cpp/src -c $< -o $@

vendor/cpp_scanner.o: vendor/tree-sitter-cpp/src/scanner.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-cpp/src -c $< -o $@

vendor/javascript_parser.o: vendor/tree-sitter-javascript/src/parser.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-javascript/src -c $< -o $@

vendor/javascript_scanner.o: vendor/tree-sitter-javascript/src/scanner.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-javascript/src -c $< -o $@

vendor/python_parser.o: vendor/tree-sitter-python/src/parser.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-python/src -c $< -o $@

vendor/python_scanner.o: vendor/tree-sitter-python/src/scanner.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-python/src -c $< -o $@

vendor/typescript_parser.o: vendor/tree-sitter-typescript/typescript/src/parser.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-typescript/typescript/src -c $< -o $@

vendor/typescript_scanner.o: vendor/tree-sitter-typescript/typescript/src/scanner.c
	$(CC) $(CFLAGS) -Ivendor/tree-sitter-typescript/typescript/src -c $< -o $@

clean:
	rm -f glupe vendor/*.o