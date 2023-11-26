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

        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Processo filho
            close(socket_server); // Fecha o socket do processo pai

            int bytes_lidos = read(socket_cliente, buffer, sizeof(buffer));

            if (bytes_lidos <= 0) {
                close(socket_cliente);
                printf("Conexão fechada\n");
                exit(EXIT_SUCCESS);
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
            exit(EXIT_SUCCESS);
        } 
        
        else {
            close(socket_cliente); // Processo pai fecha o socket do processo filho
            waitpid(-1, NULL, WNOHANG); // Limpeza de processos filhos
        }
    }

    return 0;
}