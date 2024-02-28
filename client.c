#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // isdigit()
#include <signal.h> // signal()
#include <unistd.h> // fork(), pipe(), etc.
#include <arpa/inet.h> // manipulação e conversão de endereços IP
#include <pthread.h> // pthread_create(), pthread_cancel(), etc.

#define SERVER_IP "127.0.0.1" // endereço do servidor
#define BUFFER_SIZE 1024 // buffer para envio e recebimento de mensagens

void *handle_in(void *arg);
// função para lidar com o recebimento de mensagens do servidor

void *handle_out(void *arg);
// função para lidar com o envio de mensagens ao servidor

int main(int argc, char **argv){
    int i = 0; // contador
    int port; // porta
    char nome[16]; // nome de usuário
    int client_socket; // soquete do servidor
    struct sockaddr_in server_addr; // endereço do servidor

    // trata os argumentos de execução
    if(argc < 3){
        fprintf(stderr, "Argumentos insuficientes. Uso: %s <porta> <nome de usuário>\n", argv[0]);

        exit(EXIT_FAILURE);
    }else if(argc > 3){
        fprintf(stderr, "Muitos argumentos. Uso: %s <porta> <nome de usuário>\n", argv[0]);

        exit(EXIT_FAILURE);
    }else{
        for(i = 0; i < strlen(argv[1]); i++){
            if(!isdigit(argv[1][i])){
                fprintf(stderr, "A porta deve ser um inteiro.\n");

                exit(EXIT_FAILURE);
            }
        }

        if(strlen(argv[2]) > 15){
            fprintf(stderr, "O nome de usuario não pode ultrapassar 15 caracteres.\n");

            exit(EXIT_FAILURE);
        }

        port = atoi(argv[1]);

        strncpy(nome, argv[2], strlen(argv[2]));
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
    if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr) == -1){
        perror("Erro ao converter o endereço IPv4.\n");

        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(port);
    //

    // conectando ao servidor
    printf("Estabelecendo conexão...\n");
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Falha ao conectar ao servidor.\n");

        close(client_socket);

        exit(EXIT_FAILURE);
    }
    //

    pthread_t tid_in, tid_out; // "thread" id

    // informa o nome de usuário ao servidor
    if(send(client_socket, nome, 16, 0) == -1){
            perror("Erro ao informar o nome de usuário ao servidor.\n");

            close(client_socket);

            exit(EXIT_FAILURE);
        }
    //

    // lógica de comunicação com o servidor
    if(pthread_create(&tid_in, NULL, handle_in, (void*)&client_socket) != 0){
        perror("Erro ao criar thread para escutar o servidor.\n");

        close(client_socket);

        exit(EXIT_FAILURE);
    }

    if(pthread_create(&tid_out, NULL, handle_out, (void*)&client_socket) != 0){
        perror("Erro ao criar thread para falar ao servidor.\n");

        close(client_socket);

        exit(EXIT_FAILURE);
    }
    //

    // espera o servidor parar de responder, isto é, o fim da thread
    if(pthread_join(tid_in, NULL) != 0){
        perror("Erro ao aguardar a thread.\n");

        close(client_socket);
        
        exit(EXIT_FAILURE);
    }
    //

    printf("Encerrando aplicação...\n");
    
    close(client_socket);

    exit(EXIT_SUCCESS);
}

void *handle_in(void *arg){
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while(1){
        ssize_t recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        if(recv_bytes <= 0){
            if(recv_bytes == 0){
                printf("\nConexão perdida com o servidor.\n");
            }else{
                perror("\nErro na recepção de dados do servidor.\n");
            }

            close(client_socket);

            exit(EXIT_FAILURE);
        }

        buffer[recv_bytes] = '\0'; // certifica que a string termina

        if(strncmp(buffer, "Ok!", 3) != 0){
            printf("\nO servidor terminou a conexão.\n");

            break;
        }

        memset(buffer, 0, sizeof(buffer)); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL); // termina a thread
}

void *handle_out(void *arg){
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while(1){
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if(strncmp(buffer, "!exit", 5) == 0){
            printf("Terminando aplicação.\n");

            close(client_socket);

            exit(EXIT_FAILURE);
        }

        if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
            perror("Erro ao enviar mensagem ao servidor.\n");

            break;
        }

        memset(buffer, 0, sizeof(buffer)); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL);
}