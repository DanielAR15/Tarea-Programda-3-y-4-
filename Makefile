# Makefile
#
# Para compilar:  make
# Para limpiar:   make clean
#
# Usamos mpicc (el compilador que viene con OpenMPI) en vez de gcc
# directo, porque mpicc ya sabe donde estan los headers y librerias
# de MPI.

CC = mpicc
CFLAGS = -Wall -g -pthread $(shell pkg-config --cflags gtk+-3.0)
LIBS = $(shell pkg-config --libs gtk+-3.0) -lpthread

SRC_DIR = src
OBJ_DIR = obj
BIN = cliente

# Buscamos todos los archivos .c que tenemos en src/
FUENTES = $(wildcard $(SRC_DIR)/*.c)
OBJETOS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FUENTES))

all: $(BIN)

$(BIN): $(OBJETOS)
	$(CC) $(OBJETOS) -o $(BIN) $(LIBS)

# Regla para convertir cada .c en su .o correspondiente
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN) bitacora.txt

.PHONY: all clean
