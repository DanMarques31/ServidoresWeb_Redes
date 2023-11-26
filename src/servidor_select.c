#include "servers_func.h"

int main() {

    int socket_server, socket_cliente[MAX_CLIENTES];
    fd_set conjunto_sockets;
    int max_descritor, descritor_atual;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAMANHO_BUFFER];

    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Falha na criação do socket");
        exit(EXIT_FAILURE);
    }

    // Função para reutilizar a porta e não dar o erro "address already in use"
    reutilizaPorta(socket_server);

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

    for (int i = 0; i < MAX_CLIENTES; i++) {
        socket_cliente[i] = 0;
    }

    while (1) {
        FD_ZERO(&conjunto_sockets);
        FD_SET(socket_server, &conjunto_sockets);
        max_descritor = socket_server;

        for (int i = 0; i < MAX_CLIENTES; i++) {
            descritor_atual = socket_cliente[i];

            if (descritor_atual > 0) {
                FD_SET(descritor_atual, &conjunto_sockets);
            }

            if (descritor_atual > max_descritor) {
                max_descritor = descritor_atual;
            }
        }

        int atividade = select(max_descritor + 1, &conjunto_sockets, NULL, NULL, NULL);

        if ((atividade < 0) && (errno != EINTR)) {
            perror("Erro no select");
        }

        if (FD_ISSET(socket_server, &conjunto_sockets)) {
            if ((socket_cliente[MAX_CLIENTES-1] = accept(socket_server, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("Erro ao aceitar conexão");
                exit(EXIT_FAILURE);
            }

            printf("Nova conexão , socket cliente fd é %d , ip é : %s , porta : %d\n" , socket_cliente[MAX_CLIENTES-1] , inet_ntoa(address.sin_addr) , ntohs (address.sin_port));

            for (int i = 0; i < MAX_CLIENTES; i++) {
                if (socket_cliente[i] == 0) {
                    socket_cliente[i] = socket_cliente[MAX_CLIENTES-1];
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTES; i++) {
            descritor_atual = socket_cliente[i];

            if (FD_ISSET(descritor_atual, &conjunto_sockets)) {
                int bytes_lidos = read(descritor_atual, buffer, sizeof(buffer));

                if (bytes_lidos <= 0) {
                    getpeername(descritor_atual, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Host desconectado , ip %s , porta %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    close(descritor_atual);
                    socket_cliente[i] = 0;
                } else {
                    buffer[bytes_lidos] = '\0';

                    char caminho[TAMANHO_BUFFER];
                    if (sscanf(buffer, "GET %s", caminho) == 1) {
                        // Imprime o cabeçalho HTTP da requisição
                        printf("Requisição recebida: %s %s\n", "GET", caminho);

                        // Lida com a requisição com base no caminho
                        lidar_com_requisicao(descritor_atual, caminho + 1);  // Ignora a barra inicial no caminho
                    } else {
                        // Requisição inválida
                        const char *mensagem = "400 Bad Request";
                        enviar_resposta(descritor_atual, mensagem, strlen(mensagem), "text/plain");
                        close(descritor_atual);
                        socket_cliente[i] = 0;
                    }
                }
            }
        }
    }

    return 0;
}