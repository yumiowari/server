#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // isdigit()
#include <signal.h> // signal()
#include <unistd.h> // fork(), pipe(), etc.
#include <stdbool.h> // true/false
#include <arpa/inet.h> // manipulação e conversão de endereços IP
#include <pthread.h> // pthread_create(), pthread_cancel(), etc.
#include <limits.h> // INT_MAX

#define MAX 10 // nº máximo de clientes
#define BUFFER_SIZE 1024 // buffer para envio e recebimento de mensagens

int i = 0; // contador
int filho = false;
int stop = false;
int server_socket; // soquete do servidor
int client_socket; // soquete do cliente

void handle_sigint(int signum){
    if(filho){
        printf("Encerrando processo filho...\n");
    }else{
        printf("\nEncerrando servidor...\n");
    }

    close(server_socket);
    close(client_socket);

    stop = true;

    exit(EXIT_SUCCESS);
}

void *handle_in(void *arg);
// função para lidar com o recebimento de mensagens dos clientes

void *handle_out(void *arg);
// função para lidar com o envio de mensagens aos clientes

int main(int argc, char **argv){
    int port; // porta
    struct sockaddr_in server_addr; // endereço do servidor
    struct sockaddr_in client_addr; // endereço do cliente
    socklen_t client_addr_len = sizeof(client_addr);

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

    signal(SIGINT, handle_sigint);

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

    // iniciando o servidor
    if(listen(server_socket, MAX) == -1){
        perror("Erro ao iniciar o servidor.\n");

        exit(EXIT_FAILURE);
    }
    //

    printf("\nO servidor on-line e escutando na porta %d!\n", port);

    i = 0; // agora o contador indica o id do cliente, isto é, congruente à órdem de login

    // loop do servidor
    while(!stop){
        // laço de aceitação de uma nova conexão
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1){
            perror("Erro ao aceitar a conexão do cliente.\n");

            continue; // volta ao inicio do loop para tentar uma nova conexão
        }
        //

        if(i < (INT_MAX - 1)){
            i++; // o primeiro id é 1
        }else i = 1; // reinicia (impede estouro de inteiro)
        

        // fazendo um processo filho para lidar com o cliente
        pid_t pid = fork();

        if(pid == -1){
            perror("Erro ao criar o processo filho.\n");

            continue;
        }else if(pid == 0){
            // processo filho
            close(server_socket); // não aceita novas conexões no processo filho

            printf("Novo cliente conectado!\n");

            filho = true; // indica que é um processo filho

            pthread_t tid_in, tid_out; // "thread" id

            // lógica de comunicação com o cliente
            if(pthread_create(&tid_in, NULL, handle_in, (void*)&client_socket) != 0){
                perror("Erro ao criar thread para escutar o cliente.\n");

                close(client_socket);

                exit(EXIT_FAILURE);
            }

            if(pthread_create(&tid_out, NULL, handle_out, (void*)&client_socket) != 0){
                perror("Erro ao criar thread para falar ao cliente.\n");

                close(client_socket);

                exit(EXIT_FAILURE);
            }
            //

            // espera o cliente parar de responder, isto é, o fim da thread
            if(pthread_join(tid_in, NULL) != 0){
                perror("Erro ao aguardar a thread.\n");

                close(client_socket);
                
                exit(EXIT_FAILURE);
            }
            //

            close(client_socket);

            exit(EXIT_SUCCESS);
            
            //
        }else{
            // processo pai
            close(client_socket); // o soquete do cliente agora está SOMENTE no processo filho
            //
        }
        //
    }
    //

    close(server_socket);

    exit(EXIT_SUCCESS);
}

void *handle_in(void *arg){
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while(!stop){
        ssize_t recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        printf("%ld\n", recv_bytes);

        if(recv_bytes <= 0){
            if(recv_bytes == 0){
                printf("Conexão perdida com o cliente.\n");
            }else{
                perror("Erro na recepção de dados do cliente.\n");
            }

            break; // termina a lógica de comunicação
        }

        buffer[recv_bytes] = '\0'; // certifica que a string termina

        if(buffer[0] != '\0'){ // impede de imprimir "lixo" quando o cliente encerra indevidamente
            printf("Cliente %d: %s\n", i, buffer);
        }

        memset(buffer, 0, sizeof(buffer)); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL); // termina a thread
}

void *handle_out(void *arg){
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while(!stop){
        strcpy(buffer, "Ok!\n");
        if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
            perror("Erro ao enviar mensagem ao cliente.\n");

            break;
        }

        memset(buffer, 0, sizeof(buffer)); // limpa o buffer

        sleep(1); // espera 1 segundo antes de enviar a próxima mensagem
    }

    close(client_socket);

    pthread_exit(NULL);
}