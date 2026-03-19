CC = gcc
CXX = g++
CFLAGS = -O3 -std=c11 -Ivendor/tree-sitter/lib/include -Ivendor/tree-sitter/lib/src
CXXFLAGS = -std=c++17 -O3 -Ivendor/tree-sitter/lib/include -Ivendor/tree-sitter/lib/src
LDFLAGS = -static -static-libgcc -static-libstdc++
TARGET = glupe
SRC_DIR = src
VENDOR_DIR = vendor
SRCS = $(SRC_DIR)/glupec.cpp

TS_OBJS = $(VENDOR_DIR)/tree-sitter/lib/src/lib.o \
          $(VENDOR_DIR)/tree-sitter-cpp/src/parser.o \
          $(VENDOR_DIR)/tree-sitter-cpp/src/scanner.o

# This line looks at all dependencies .hpp 
DEPS = $(wildcard $(SRC_DIR)/*.hpp)

LEX = flex
BISON = bison
LFILE = $(SRC_DIR)/glupe.l
YFILE = $(SRC_DIR)/glupe.y
C_LEX = $(SRC_DIR)/lex.yy.c
C_BISON = $(SRC_DIR)/glupe.tab.c
H_BISON = $(SRC_DIR)/glupe.tab.h

.PHONY: all clean force

all: $(TARGET)

$(C_BISON) $(H_BISON): $(YFILE)
	$(BISON) -d -o $(C_BISON) $(YFILE)

$(C_LEX): $(LFILE) $(H_BISON)
	$(LEX) -o $(C_LEX) $(LFILE)

$(VENDOR_DIR)/tree-sitter/lib/src/lib.o: $(VENDOR_DIR)/tree-sitter/lib/src/lib.c
	$(CC) $(CFLAGS) -c $< -o $@

$(VENDOR_DIR)/tree-sitter-cpp/src/parser.o: $(VENDOR_DIR)/tree-sitter-cpp/src/parser.c
	$(CC) $(CFLAGS) -c $< -o $@

# CAMBIO AQUÍ: Usar $(CC) en lugar de $(CXX)
$(VENDOR_DIR)/tree-sitter-cpp/src/scanner.o: $(VENDOR_DIR)/tree-sitter-cpp/src/scanner.c
	$(CC) $(CFLAGS) -c $< -o $@

# El ejecutable depende del .cpp Y de todos los .hpp
$(TARGET): $(SRCS) $(DEPS) $(C_LEX) $(C_BISON) $(TS_OBJS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(C_LEX) $(C_BISON) $(TS_OBJS) $(LDFLAGS) -o $(TARGET)

# Comando para limpiar y forzar
clean:
	rm -f $(TARGET) $(C_LEX) $(C_BISON) $(H_BISON) $(TS_OBJS)

# Si quieres forzar sin borrar, puedes usar 'make force'
force:
	touch $(SRCS)
	$(MAKE) all