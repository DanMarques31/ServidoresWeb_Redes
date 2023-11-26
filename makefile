CC = gcc
CFLAGS = -Wall
OBJ_DIR = object

all: servidor_iterativo

servidor_iterativo: $(OBJ_DIR)/servidor_iterativo.o $(OBJ_DIR)/servers_func.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/servidor_iterativo.o: servidor_iterativo.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/servers_func.o: servers_func.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) servidor_iterativo
