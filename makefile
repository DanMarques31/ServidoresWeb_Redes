CC = gcc
CFLAGS = -Wall
OBJ_DIR = object

all: servidor_iterativo servidor_fork servidor_threads servidor_select

servidor_iterativo: $(OBJ_DIR)/servidor_iterativo.o $(OBJ_DIR)/servers_func.o
	$(CC) $(CFLAGS) $^ -o $@

servidor_fork: $(OBJ_DIR)/servidor_fork.o $(OBJ_DIR)/servers_func.o
	$(CC) $(CFLAGS) $^ -o $@

servidor_threads: $(OBJ_DIR)/servidor_threads.o $(OBJ_DIR)/servers_func.o -lpthread
	$(CC) $(CFLAGS) $^ -o $@

servidor_select: $(OBJ_DIR)/servidor_select.o $(OBJ_DIR)/servers_func.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/servidor_iterativo.o: servidor_iterativo.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/servidor_fork.o: servidor_fork.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/servidor_threads.o: servidor_threads.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/servidor_select.o: servidor_select.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/servers_func.o: servers_func.c servers_func.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) servidor_iterativo servidor_fork servidor_threads servidor_select
