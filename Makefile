CXX = g++
CXXFLAGS = -std=c++17 -O3 -static -static-libgcc -static-libstdc++
TARGET = glupe
SRC_DIR = src
SRCS = $(SRC_DIR)/glupec.cpp
# Esta línea busca todos los archivos .hpp para que sean dependencias
DEPS = $(wildcard $(SRC_DIR)/*.hpp)

.PHONY: all clean force

all: $(TARGET)

# El ejecutable depende del .cpp Y de todos los .hpp
$(TARGET): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Comando para limpiar y forzar
clean:
	rm -f $(TARGET)

# Si quieres forzar sin borrar, puedes usar 'make force'
force:
	touch $(SRCS)
	$(MAKE) all