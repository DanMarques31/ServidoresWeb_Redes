#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORTA 8080
#define TAMANHO_BUFFER 1024

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

int main() {

    int socket_server, socket_cliente;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAMANHO_BUFFER];

    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Falha na criação do socket");
        exit(EXIT_FAILURE);
    }

    int opcao = 1;
    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &opcao, sizeof(opcao))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORTA);

    if (bind(socket_server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Falha na vinculação do socket à porta");
        exit(EXIT_FAILURE);
    }

    if (listen(socket_server, 1) < 0) {
        perror("Erro ao aguardar conexões");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando requisições...\n");

    while (1) {

        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erro ao aceitar conexão");
            exit(EXIT_FAILURE);
        }

        int bytes_lidos = read(socket_cliente, buffer, sizeof(buffer));

        if (bytes_lidos <= 0) {
            close(socket_cliente);
            printf("Conexão fechada\n");
            continue;
        }

        buffer[bytes_lidos] = '\0';

        char caminho[TAMANHO_BUFFER];
        if (sscanf(buffer, "GET %s", caminho) == 1) {

            // Imprime o cabeçalho HTTP da requisição
            printf("Requisição recebida: %s %s\n", "GET", caminho);

            // Lida com a requisição com base no caminho
            lidar_com_requisicao(socket_cliente, caminho + 1);  // Ignora a barra inicial no caminho
        } 
        
        else {
            // Requisição inválida
            const char *mensagem = "400 Bad Request";
            enviar_resposta(socket_cliente, mensagem, strlen(mensagem), "text/plain");
        }

        close(socket_cliente);
    }

    return 0;
}