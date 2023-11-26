#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include <sys/select.h>

// Variáveis fixas
#define PORTA 8080
#define TAMANHO_BUFFER 1024
#define NUM_THREADS 4
#define MAX_CLIENTES 5000

// Assinaturas das funções
void enviar_resposta(int socket_cliente, const char *conteudo, size_t tamanho, const char *tipo);
const char *obter_tipo_mime(const char *caminho);
void lidar_com_requisicao(int socket_cliente, const char *caminho);
void *lidar_com_requisicaoTHREADS(void *arg);
void reutilizaPorta(int socket_server);

// Estrutura para representar uma tarefa
typedef struct {
    
    int socket_cliente;
    char diretorio[TAMANHO_BUFFER];

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