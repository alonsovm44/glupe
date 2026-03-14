CXX = g++
CXXFLAGS = -std=c++17 -O3 -static -static-libgcc -static-libstdc++
TARGET = glupe
SRC_DIR = src
SRCS = $(SRC_DIR)/glupec.cpp
# Esta línea busca todos los archivos .hpp para que sean dependencias
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

# El ejecutable depende del .cpp Y de todos los .hpp
$(TARGET): $(SRCS) $(DEPS) $(C_LEX) $(C_BISON)
	$(CXX) $(CXXFLAGS) $(SRCS) $(C_LEX) $(C_BISON) -o $(TARGET)

# Comando para limpiar y forzar
clean:
	rm -f $(TARGET) $(C_LEX) $(C_BISON) $(H_BISON)

# Si quieres forzar sin borrar, puedes usar 'make force'
force:
	touch $(SRCS)
	$(MAKE) all