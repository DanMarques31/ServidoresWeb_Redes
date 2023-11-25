#include <stdio.h> // Biblioteca padrão in and out
#include <stdlib.h> // Utilidade geral 
#include <string.h> // Manipulações de string
#include <sys/socket.h> // Programação de sockets
#include <arpa/inet.h> // Funções para manipulação de endereços IP
#include <unistd.h> // Fornece acesso ao SO para leitura e escrita de sockets
#include <fcntl.h> // Bibliotecas para manipulação, obtenção e descritores de arquivos
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

    write(socket_cliente, resposta, strlen(resposta));
    write(socket_cliente, conteudo, tamanho);
}

// Função para lidar com uma requisição HTTP
void lidar_com_requisicao(int socket_cliente, const char *caminho) {
    
    char caminho_completo[TAMANHO_BUFFER];
    snprintf(caminho_completo, TAMANHO_BUFFER, "./site/%s", caminho);

    int fd = open(caminho_completo, O_RDONLY);
    
    if (fd == -1) {
        perror("Erro ao abrir o arquivo");
        const char *mensagem = "404 Not Found";
        enviar_resposta(socket_cliente, mensagem, strlen(mensagem), "text/plain");
        return;
    }

    struct stat stat_buffer;
    fstat(fd, &stat_buffer);

    // Determina o tipo de conteúdo com base na extensão do arquivo
    const char *tipo;
    if (strstr(caminho_completo, ".html")) {
        tipo = "text/html";
    } 
    
    else if (strstr(caminho_completo, ".webp")) {
        tipo = "image/webp";
    } 
    
    else {
        tipo = "text/plain";
    }

    enviar_resposta(socket_cliente, NULL, stat_buffer.st_size, tipo);

    // Envia o conteúdo do arquivo para o cliente
    char buffer[TAMANHO_BUFFER];
    ssize_t lidos, enviados;
    
    while ((lidos = read(fd, buffer, sizeof(buffer))) > 0) {
        
        enviados = write(socket_cliente, buffer, lidos);
        
        if (enviados != lidos) {
            perror("Erro ao enviar dados");
            exit(EXIT_FAILURE);
        }
    }

    // Fecha o arquivo
    close(fd);
}

int main() {

    int socket_server, socket_cliente;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAMANHO_BUFFER];

    // Criação do socket do servidor
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Falha na criação do socket");
        exit(EXIT_FAILURE);
    }

    // Configuração do socket para permitir o reuso da porta
    int opcao = 1;
    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &opcao, sizeof(opcao))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configuração do endereço do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORTA);

    // Vinculação do socket à porta 8080
    if (bind(socket_server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Falha na vinculação do socket à porta");
        exit(EXIT_FAILURE);
    }

    // Aguarda conexões
    if (listen(socket_server, 1) < 0) {
        perror("Erro ao aguardar conexões");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexões...\n");

    while (1) {
        // Aceita a conexão do cliente
        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erro ao aceitar conexão");
            exit(EXIT_FAILURE);
        }

        // Lê a requisição do cliente
        int bytes_lidos = read(socket_cliente, buffer, sizeof(buffer));

        // Verifica se a leitura foi bem-sucedida
        if (bytes_lidos <= 0) {
            // Erro ou conexão fechada
            close(socket_cliente);
            printf("Conexão fechada\n");
            continue;
        }

        // Adiciona um caractere nulo no final da string para garantir que seja uma string válida
        buffer[bytes_lidos] = '\0';

        // Extrai o caminho da requisição
        char caminho[TAMANHO_BUFFER];
        if (sscanf(buffer, "GET %s", caminho) == 1) {
            // Lida com a requisição com base no caminho
            lidar_com_requisicao(socket_cliente, caminho + 1);  // Ignora a barra inicial no caminho
        } 
        
        else {
            // Requisição inválida
            const char *mensagem = "400 Bad Request";
            enviar_resposta(socket_cliente, mensagem, strlen(mensagem), "text/plain");
        }

        // Fecha o socket do cliente após enviar a resposta
        close(socket_cliente);
        printf("Conexão fechada\n");
    }

    return 0;
}