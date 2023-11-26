#include "servers_func.h"

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

        char diretorio[TAMANHO_BUFFER];
        if (sscanf(buffer, "GET %s", diretorio) == 1) {

            // Imprime o cabeçalho HTTP da requisição
            printf("Requisição recebida: %s %s\n", "GET", diretorio);

            // Lida com a requisição com base no diretório
            lidar_com_requisicao(socket_cliente, diretorio + 1);  // Ignora a barra inicial no diretório
        } 
        
        else {
            // Requisição inválida ai retorna número de erro do cliente 
            const char *mensagem = "400 Bad Request";
            enviar_resposta(socket_cliente, mensagem, strlen(mensagem), "text/plain");
        }

        close(socket_cliente);
    }

    return 0;
}