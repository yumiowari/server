#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // isdigit()
#include <signal.h> // signal()
#include <unistd.h> // fork(), pipe(), etc.
#include <arpa/inet.h> // manipulação e conversão de endereços IP

#define BUFFER_SIZE 1024

int main(int argc, char **argv){
    int i = 0; // contador
    int port; // porta
    int client_socket; // soquete do servidor
    struct sockaddr_in server_addr; // endereço do servidor
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

    // criando o soquete de cliente
    printf("Criando o soquete de cliente...\n");
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        perror("Erro ao criar o soquete de cliente.\n");

        exit(EXIT_FAILURE);
    }
    //

    // configurando o endereço do servidor
    printf("Configurando o endereço do servidor...\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    //

    // conectando ao servidor
    printf("Tentando conexão...\n");
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Falha ao conectar ao servidor.\n");

        close(client_socket);

        exit(EXIT_FAILURE);
    }
    //

    // lógica de comunicação com o servidor
    while(1){
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // envia mensagem ao servidor
        if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
            perror("Erro ao enviar mensagem ao servidor.\n");

            break;
        }
        //

        if(strncmp(buffer, "!exit", 5) == 0){
            printf("Terminando conexão com o servidor...\n");

            break;
        }

        memset(buffer, 0, sizeof(buffer)); // limpa o buffer

        // recebe mensagem do servidor
        if(recv(client_socket, buffer, sizeof(buffer), 0) == -1){
            perror("Erro ao receber mensagem do servidor.\n");

            break;
        }

        if(strncmp(buffer, "!exit", 5) == 0){
            printf("O servidor terminou a conexão.\n");

            break;
        }
        //

        printf("Resposta do servidor: %s\n", buffer);
    }
    //

    printf("Encerrando aplicação...\n");
    
    close(client_socket);

    exit(EXIT_SUCCESS);
}