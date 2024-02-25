#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // isdigit()
#include <signal.h> // signal()
#include <unistd.h> // fork(), pipe(), etc.
#include <arpa/inet.h> // manipulação e conversão de endereços IP

#define MAX 10 // nº máximo de clientes
#define BUFFER_SIZE 1024 // buffer para envio e recebimento de mensagens

int stop = 0;

void handle_sigint(int signum){
    stop = 1;
}
// função de tratamento do SIGINT (Ctrl+C)

int main(int argc, char **argv){
    int i = 0; // contador
    int port; // porta
    int server_socket; // soquete do servidor
    int client_socket; // soquete do cliente
    struct sockaddr_in server_addr; // endereço do servidor
    struct sockaddr_in client_addr; // endereço do cliente
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // trata os argumentos de execução
    if(argc < 2){
        fprintf(stderr, "Argumentos insuficientes. Uso: %s <porta>\n", argv[0]);

        exit(EXIT_FAILURE);
    }else if(argc > 2){
        fprintf(stderr, "Muitos argumentos. Uso: %s <porta>\n", argv[0]);

        exit(EXIT_FAILURE);
    }else{
        for(i = 0; i < strlen(argv[1]); i++){
            if(!isdigit(argv[1][i])){
                fprintf(stderr, "A porta deve ser um inteiro.\n");

                exit(EXIT_FAILURE);
            }
        }

        port = atoi(argv[1]);
    }
    //

    signal(SIGINT, handle_sigint); // atribui o sinal de interrupção à função assíncrona

    // criando o soquete do servidor
    printf("Criando o soquete do servidor...\n");
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("Erro ao criar o soquete do servidor.\n");

        exit(EXIT_FAILURE);
    }
    //

    // configurando o endereço do servidor
    printf("Configurando o endereço do servidor...\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    //

    // vinculando o soquete à porta
    printf("Vinculando o soquete à porta %d...\n", port);
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Erro ao vincular o soquete à porta.\n");

        exit(EXIT_FAILURE);
    }
    //

    // inicia o servidor
    if(listen(server_socket, MAX) == -1){
        perror("Erro ao iniciar o servidor.\n");

        exit(EXIT_FAILURE);
    }
    //

    printf("\nO servidor on-line e escutando na porta %d!\n", port);

    // loop do servidor
    while(1){
        if(stop == 1)break;

        // aceita uma nova conexão
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1){
            perror("Erro ao aceitar a conexão do cliente.\n");

            continue; // volta ao inicio do loop para tentar uma nova conexão
        }

        printf("Novo cliente conectado!\n");
        //

        // lógica de comunicação com o cliente
        while(1){
            if(stop == 1){
                strcpy(buffer, "!exit\n");

                if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1)perror("Erro ao enviar mensagem ao cliente.\n");

                break;
            }

            // recebe mensagem do cliente
            ssize_t recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);

            if(recv_bytes <= 0){
                if(recv_bytes == 0){
                    printf("Conexão perdida com o cliente.\n");
                }else{
                    perror("Erro na recepção de dados do cliente.\n");
                }

                close(client_socket);

                break; // termina a lógica de comunicação
            }

            buffer[recv_bytes] = '\0'; // certifica que a string termina

            printf("Cliente: %s\n", buffer);
            //

            memset(buffer, 0, sizeof(buffer)); // limpa o buffer

            // envia resposta ao cliente
            strcpy(buffer, "Ok!\n");
            if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
                perror("Erro ao enviar mensagem ao cliente.\n");

                break;
            }
            //
        }

        printf("Cliente desconectado.\n");
        //
    }
    //

    printf("Encerrando servidor...\n");

    close(server_socket);

    exit(EXIT_SUCCESS);
}