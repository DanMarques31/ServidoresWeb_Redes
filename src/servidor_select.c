#include "servers_func.h"

int main() {

    int socket_server, socket_cliente[MAX_CLIENTES]; // declaração dos sockets
    fd_set conjunto_sockets; // Conjunto de sockets do select
    int fd_max, fd_atual; // variáveis dos descritores de arquivo atual e o maior
    
    // Estrutura de endereço, tamanho de endereço e criação do buffer
    struct sockaddr_in address; 
    int tam_address = sizeof(address);
    char buffer[TAMANHO_BUFFER];

    // Criação do socket do servidor
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Falha na criação do socket");
        exit(EXIT_FAILURE);
    }

    // Função para reutilizar a porta e não dar o erro "address already in use"
    reutilizaPorta(socket_server);

    // Configuração do endereço do socket, endereço de entrada e porta de escuta do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORTA);

    // Anexa o socket a porta
    if (bind(socket_server, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Falha na vinculação do socket à porta");
        exit(EXIT_FAILURE);
    }

    // Escutando conexões
    if (listen(socket_server, 1) < 0) {
        perror("Erro ao aguardar conexões");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando requisições...\n");

    // Inicializa os clientes com 0
    for (int i = 0; i < MAX_CLIENTES; i++) {
        socket_cliente[i] = 0;
    }

    while (1) {

        // Limpa o conjunto e adiciona o socket_server no conjunto
        FD_ZERO(&conjunto_sockets);
        FD_SET(socket_server, &conjunto_sockets);
        fd_max = socket_server; // Inicializa o fd_max com o socket do server

        // Vai colocando os sockets dos clientes ao conjunto
        for (int i = 0; i < MAX_CLIENTES; i++) {
            fd_atual = socket_cliente[i];

            // Coloca o FD atual no conjunto
            if (fd_atual > 0) {
                FD_SET(fd_atual, &conjunto_sockets);
                fd_max = (fd_atual > fd_max) ? fd_atual : fd_max; // Caso fd_atual maior que fd_max então utiliza o atual
            }

        }

        // Espera atividade em algum dos sockets
        int atividade = select(fd_max + 1, &conjunto_sockets, NULL, NULL, NULL);

        if ((atividade < 0) && (errno != EINTR)) {
            perror("Erro no select");
        }
        
        // Se tiver atividade significa uma nova conexão
        if (FD_ISSET(socket_server, &conjunto_sockets)) {
            
            // Aceita essa nova conexão
            int novo_cliente = accept(socket_server, (struct sockaddr *)&address, (socklen_t*)&tam_address);

            if (novo_cliente < 0) {
                perror("Erro ao aceitar conexão");
                exit(EXIT_FAILURE); // sai se der erro na conexão
            }

            else {
                
                // Adiciona o cliente novo atual no array de clients
                for (int i = 0; i < MAX_CLIENTES; i++) {
                    
                    if (socket_cliente[i] == 0) {
                        socket_cliente[i] = novo_cliente;
                        break;
                    }
                }
            }
        }

        // Depois de conectados, caso houver atividade ja lê o input do cliente
        for (int i = 0; i < MAX_CLIENTES; i++) {
            fd_atual = socket_cliente[i];

            if (FD_ISSET(fd_atual, &conjunto_sockets)) {
                int bytes_lidos = read(fd_atual, buffer, sizeof(buffer));

                if (bytes_lidos <= 0) {

                    if (bytes_lidos < 0) {
                        perror("Erro na leitura");
                    }

                    // Pega o endereço do cliente, fecha o socket dele remove o socket do conjunto e remove ele do array de clientes
                    getpeername(fd_atual, (struct sockaddr*)&address, (socklen_t*)&tam_address);
                    close(fd_atual);
                    FD_CLR(fd_atual, &conjunto_sockets);
                    socket_cliente[i] = 0;
                }
                
                else {

                    buffer[bytes_lidos] = '\0'; // Coloca um char nulo no final do que foi lido

                    char diretorio[TAMANHO_BUFFER];
                    if (sscanf(buffer, "GET %s", diretorio) == 1) {
                        // Imprime o cabeçalho HTTP da requisição
                        printf("Requisição recebida: %s %s\n", "GET", diretorio);

                        // Lida com a requisição com base no diretorio
                        lidar_com_requisicao(fd_atual, diretorio + 1);  // Ignora a barra inicial no diretorio
                    } 
                    
                    else {

                        // Requisição inválida
                        const char *mensagem = "400 Bad Request";
                        enviar_resposta(fd_atual, mensagem, strlen(mensagem), "text/plain");
                        close(fd_atual);
                        FD_CLR(fd_atual, &conjunto_sockets);
                        socket_cliente[i] = 0;
                    }
                }
            }
        }
    }

    return 0;
}