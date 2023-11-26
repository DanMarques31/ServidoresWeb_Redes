#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORTA 8080
#define TAMANHO_BUFFER 1024
#define NUM_THREADS 4

void enviar_resposta(int socket_cliente, const char *conteudo, size_t tamanho, const char *tipo);
const char *obter_tipo_mime(const char *caminho);
void lidar_com_requisicao(int socket_cliente, const char *caminho);
void *lidar_com_requisicaoTHREADS(void *arg);

// Estrutura para representar uma tarefa
typedef struct {
    int socket_cliente;
    char caminho[TAMANHO_BUFFER];
} Tarefa;

// Estrutura para a fila de tarefas
typedef struct {
    Tarefa *tarefas;
    int capacidade;
    int inicio;
    int fim;
    pthread_mutex_t mutex;
    pthread_cond_t cheio;
    pthread_cond_t vazio;
} FilaTarefas;