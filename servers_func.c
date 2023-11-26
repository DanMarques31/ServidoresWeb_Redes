#include "servers_func.h"

// Função para enviar uma resposta HTTP ao cliente
void enviar_resposta(int socket_cliente, const char *conteudo, size_t tamanho, const char *tipo) {
    
    char resposta[TAMANHO_BUFFER];
    
    snprintf(resposta, TAMANHO_BUFFER,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            tipo, tamanho);

    // Imprime o cabeçalho HTTP da resposta
    printf("Resposta enviada:\n%s", resposta);

    write(socket_cliente, resposta, strlen(resposta));
    write(socket_cliente, conteudo, tamanho);
}

// Função para obter o tipo MIME com base na extensão do arquivo
const char *obter_tipo_mime(const char *caminho) {
    
    const char *extensao = strrchr(caminho, '.');
    
    if (extensao != NULL) {
        // Incrementa para ignorar o ponto na extensão
        extensao++;

        if (strcmp(extensao, "html") == 0) {
            return "text/html";
        } 
        
        else if (strcmp(extensao, "webp") == 0) {
            return "image/webp";
        }
        // Vou colocar mais arquivos aqui só pra não esquecer
    }

    // Tipo MIME padrão
    return "application/octet-stream";
}

// Função para lidar com uma requisição HTTP
void lidar_com_requisicao(int socket_cliente, const char *caminho) {
    
    char caminho_completo[TAMANHO_BUFFER];
    snprintf(caminho_completo, TAMANHO_BUFFER, "./site/%s", caminho);

    int descritor_arq = open(caminho_completo, O_RDONLY);
    
    if (descritor_arq == -1) {
        perror("Erro ao abrir o arquivo");
        const char *mensagem = "404 Not Found";
        enviar_resposta(socket_cliente, mensagem, strlen(mensagem), "text/plain");
        return;
    }

    struct stat stat_buffer;
    fstat(descritor_arq, &stat_buffer);

    const char *tipo = obter_tipo_mime(caminho_completo);

    enviar_resposta(socket_cliente, NULL, stat_buffer.st_size, tipo);

    char buffer[TAMANHO_BUFFER];
    ssize_t lidos, enviados;
    
    while ((lidos = read (descritor_arq, buffer, sizeof(buffer))) > 0) {
        
        enviados = write(socket_cliente, buffer, lidos);
        
        if (enviados != lidos) {
            perror("Erro ao enviar dados");
            exit(EXIT_FAILURE);
        }
    }

    close(descritor_arq);
}

// Função para inicializar a fila de tarefas
void inicializar_fila(FilaTarefas *fila, int capacidade) {
    fila->tarefas = (Tarefa *)malloc(capacidade * sizeof(Tarefa));
    fila->capacidade = capacidade;
    fila->inicio = 0;
    fila->fim = 0;
    pthread_mutex_init(&fila->mutex, NULL);
    pthread_cond_init(&fila->cheio, NULL);
    pthread_cond_init(&fila->vazio, NULL);
}

// Função para enfileirar uma tarefa
void enfileirar(FilaTarefas *fila, Tarefa tarefa) {
    pthread_mutex_lock(&fila->mutex);

    while ((fila->fim + 1) % fila->capacidade == fila->inicio) {
        pthread_cond_wait(&fila->cheio, &fila->mutex);
    }

    fila->tarefas[fila->fim] = tarefa;
    fila->fim = (fila->fim + 1) % fila->capacidade;

    pthread_cond_signal(&fila->vazio);
    pthread_mutex_unlock(&fila->mutex);
}

// Função para desenfileirar uma tarefa
Tarefa desenfileirar(FilaTarefas *fila) {
    pthread_mutex_lock(&fila->mutex);

    while (fila->inicio == fila->fim) {
        pthread_cond_wait(&fila->vazio, &fila->mutex);
    }

    Tarefa tarefa = fila->tarefas[fila->inicio];
    fila->inicio = (fila->inicio + 1) % fila->capacidade;

    pthread_cond_signal(&fila->cheio);
    pthread_mutex_unlock(&fila->mutex);

    return tarefa;
}


// Função para lidar com uma requisição HTTP
void *lidar_com_requisicaoTHREADS(void *arg) {
    FilaTarefas *fila = (FilaTarefas *)arg;

    while (1) {
        Tarefa tarefa = desenfileirar(fila);

        char caminho_completo[TAMANHO_BUFFER];
        snprintf(caminho_completo, TAMANHO_BUFFER, "./site/%s", tarefa.caminho);

        int descritor_arq = open(caminho_completo, O_RDONLY);
        
        if (descritor_arq == -1) {
            perror("Erro ao abrir o arquivo");
            const char *mensagem = "404 Not Found";
            enviar_resposta(tarefa.socket_cliente, mensagem, strlen(mensagem), "text/plain");
        } else {
            struct stat stat_buffer;
            fstat(descritor_arq, &stat_buffer);

            const char *tipo = obter_tipo_mime(caminho_completo);

            enviar_resposta(tarefa.socket_cliente, NULL, stat_buffer.st_size, tipo);

            char buffer[TAMANHO_BUFFER];
            ssize_t lidos, enviados;
            
            while ((lidos = read (descritor_arq, buffer, sizeof(buffer))) > 0) {
                
                enviados = write(tarefa.socket_cliente, buffer, lidos);
                
                if (enviados != lidos) {
                    perror("Erro ao enviar dados");
                    exit(EXIT_FAILURE);
                }
            }

            close(descritor_arq);
        }

        close(tarefa.socket_cliente);
    }

    return NULL;
}