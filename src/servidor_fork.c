#include "servers_func.h"

int main() {

    // Declaração de sockets e estruturação do endereço pro socket
    int socket_server, socket_cliente;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAMANHO_BUFFER]; // Buffer para armazenar os dados recebidos

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

    while (1) {

        // Aceita uma nova conexão
        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erro ao aceitar conexão");
            exit(EXIT_FAILURE);
        }

        pid_t processoID = fork(); // Cria um novo processo

        if (processoID < 0) {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }

        if (processoID == 0) { // Processo filho
            
            close(socket_server); // Fecha o socket do processo pai

            // Lê os dados do client e fecha se não houver dados lidos
            int bytes_lidos = read(socket_cliente, buffer, sizeof(buffer));

            if (bytes_lidos <= 0) {
                close(socket_cliente);
                continue;
            }

            buffer[bytes_lidos] = '\0';

            // Verifica se a requisição é um GET
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
            exit(EXIT_SUCCESS);
        } 
        
        else {
            close(socket_cliente); // Processo pai fecha o socket do processo filho
            waitpid(-1, NULL, WNOHANG); // Limpeza de processos filhos
        }
    }

    return 0;
}