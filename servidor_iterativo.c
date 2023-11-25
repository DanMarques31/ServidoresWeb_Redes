#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

void send_file(int socket, char *path, char *content_type) {
    // Abre o arquivo
    FILE *file = fopen(path, "r");

    if (file != NULL) {
        // Lê o conteúdo do arquivo
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *file_content = (char *)malloc(file_size + 1);
        fread(file_content, 1, file_size, file);
        fclose(file);

        // Responde com o conteúdo do arquivo
        char header[1024];
        sprintf(header, "HTTP/1.1 200 OK\nContent-Type: %s\n\n", content_type);
        write(socket, header, strlen(header));
        write(socket, file_content, file_size);

        free(file_content);
    } else {
        // Responde com um erro 404 se o arquivo não for encontrado
        write(socket, "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><body>Arquivo não encontrado</body></html>", 88);
    }
}

void handle_client(int socket) {
    char buffer[1024];
    int bytes_read;

    // Lê a requisição do cliente
    bytes_read = read(socket, buffer, sizeof(buffer));

    if (bytes_read < 0) {
        perror("read");
        exit(1);
    }

    // Verifica se a leitura do socket foi bem-sucedida
    if (bytes_read == 0) {
        return;
    }

    // Imprime a requisição do cliente
    printf("Requisição recebida:\n%s\n", buffer);

    // Verifica o caminho da requisição
    char *method = strtok(buffer, " ");
    char *path = strtok(NULL, " ");

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0) {
            send_file(socket, "site/teste.html", "text/html");
        } else if (strcmp(path, "/site/appa cabelin.webp") == 0) {
            send_file(socket, "site/appa cabelin.webp", "image/webp");
        } else {
            // Responde com um erro 404 para qualquer outro caminho
            write(socket, "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><body>Página não encontrada</body></html>", 68);
        }
    }

    // Fecha o socket
    close(socket);
}

int main() {
    
    int my_socket, new_socket;
    struct sockaddr_in address;

    // Cria o socket
    my_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (my_socket < 0) {
        perror("socket");
        exit(1);
    }

    // Configura o socket para permitir o reuso da porta
    int opt = 1;
    if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(1);
    }

    // Associa o socket a uma porta
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(my_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }

    // Coloca o socket em modo de escuta
    if (listen(my_socket, 10) < 0) {
        perror("listen");
        exit(1);
    }

    // Loop principal
    while (1) {
        // Aceita uma nova conexão
        new_socket = accept(my_socket, NULL, NULL);

        if (new_socket < 0) {
            perror("accept");
            exit(1);
        }

        // Encaminha a conexão para a função handle_client
        handle_client(new_socket);
    }

    return 0;
}
