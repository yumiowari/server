// bibliotecas

// padrão
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // tipo booleano
#include <limits.h> // INT_MAX
#include <time.h>
//

// rede
#include <arpa/inet.h> // manipulação e conversão de endereços IP
#include <sys/socket.h>
//

// processamento paralelo
#include <signal.h> // manipulação de sinais
#include <pthread.h> // manipulação de threads
#include <unistd.h> // manipulação de processos
#include <sys/wait.h>
//

// estrutura de dados
#include "bib/pile.h"
//

// macros
#define MAX_QUEUE 5 // nº máximo de clientes na fila
#define BUFFER_SIZE 1024 // tamanho do buffer para envio e recebimento de mensagens
//

// variáveis globais
int i = 0; // contador
bool child = false; // é filho?
bool stop = false; // deve parar?
int server_socket; // soquete do servidor
int client_socket; // soquete do cliente
char client_name[16]; // nome de usuário
Pile *clients = NULL; // pilha de clientes
int qtd = 0; // quantidade de clientes conectados a todo tempo
//

// funções

void shutdown_routine(int signal);
// rotina de encerramento do servidor

void handle_sigint(int signal);
// função para lidar com o sinal de interrupção (Ctrl+C)

void handle_sigterm(int signal);
// função para lidar com o sinal de encerramento (fecha o terminal)

void handle_sigchld(int signal);
// função para lidar com o fim da conexão do cliente

void *handle_msg_in(void *arg);
// função para lidar com o recebimento de mensagens dos clientes

void *handle_msg_out(void *arg);
// função para lidar com o envio de mensagens aos clientes

//

int main(int argc, char **argv){
    // configura os sinais
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGCHLD, handle_sigchld);
    //

    int port; // porta
    struct sockaddr_in server_addr; // endereço do servidor
    struct sockaddr_in client_addr; // endereço de cliente
    socklen_t client_addr_len = sizeof(client_addr); // tamanho do endereço de cliente

    // trata os argumentos de execução
    if(argc < 2){
        fprintf(stderr, "Argumentos insuficientes. Uso: %s <porta>\n", argv[0]);

        shutdown_routine(1);
    }else if(argc > 2){
        fprintf(stderr, "Muitos argumentos. Uso: %s <porta>\n", argv[0]);

        shutdown_routine(1);
    }else{
        for(i = 0; i < strlen(argv[1]); i++){
            if(argv[1][i] < '0' || argv[1][i] > '9'){
                fprintf(stderr, "A porta deve ser um inteiro.\n");

                shutdown_routine(1);
            }
        }

        port = atoi(argv[1]);
    }
    //

    // criando o soquete do servidor
    printf("Fazendo o soquete do servidor...\n");
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP e IPv4
    if(server_socket == -1){
        perror("Erro ao criar o soquete do servidor.\n");

        shutdown_routine(1);
    }
    //

    // configurando o endereço do servidor
    printf("Configurando o endereço do servidor...\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // aceita conexão de qualquer fonte
    server_addr.sin_port = htons(port);
    //

    // vinculando o soquete à porta
    printf("Vinculando o soquete à porta %d...\n", port);
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Erro ao vincular o soquete à porta.\n");

        shutdown_routine(1);
    }
    //

    // iniciando o servidor
    if(listen(server_socket, MAX_QUEUE) == -1){
        perror("Erro ao iniciar o servidor.\n");

        shutdown_routine(1);
    }
    //

    printf("\nServidor on-line e escutando na porta %d!\n", port);

    // inicia a pilha de clientes
    clients = create_pile();
    if(clients == NULL){
        perror("Falha na criação da pilha.\n");

        shutdown_routine(1);
    }
    //

    i = 0;

    // loop do servidor
    while(!stop){
        // verifica se o servidor está cheio
        if(qtd >= 5)continue;
        //

        // laço para aceitação de uma nova conexão
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1){
            perror("Erro ao aceitar a conexão do cliente.\n");

            continue; // volta ao inicio do loop para tentar uma nova conexão
        }
        //

        // recebe o nome de usuário
        ssize_t recv_bytes = recv(client_socket, client_name, 16, 0);

        if(recv_bytes <= 0){
            perror("Falha na recepção do nome de usuário do cliente.\n");

            close(client_socket);

            continue;
        }
        //

        qtd++;

        i = (i < (INT_MAX - 1)) ? i + 1 : 1; // impede estouro de inteiro

        // fazendo um processo filho para lidar com o cliente
        pid_t pid = fork();

        if(pid == -1){
            perror("Erro ao criar o processo filho.\n");

            continue;
        }else if(pid == 0){
            // processo filho

            close(server_socket); // não aceita novas conexões no processo filho

            printf("%s se juntou ao chat! [%d/5]\n", client_name, qtd);

            child = true; // indica que é um processo filho

            pthread_t tid_in, tid_out; // "thread" ID

            // lógica de comunicação com o cliente
            if(pthread_create(&tid_in, NULL, handle_msg_in, NULL) != 0){
                perror("Erro ao criar thread para escutar o cliente.\n");

                shutdown_routine(1);
            }

            if(pthread_create(&tid_out, NULL, handle_msg_out, NULL) != 0){
                perror("Erro ao criar thread para falar ao cliente.\n");

                shutdown_routine(1);
            }
            //

            // espera o cliente parar de responder
            if(pthread_join(tid_in, NULL) != 0){
                perror("Erro ao aguardar a thread.\n");

                shutdown_routine(1);
            }
            //

            shutdown_routine(0);

            //
        }else{
            // processo pai

            close(client_socket); // somente o processo filho lida com o cliente

            //
        }
        //
    }
    //

    shutdown_routine(0);
}

void shutdown_routine(int signal){
    if(child){
        printf("Encerrando processo filho...\n");
    }else{
        while(waitpid(-1, NULL, WNOHANG) > 0); // espera por todos os processos filhos
        
        printf("\nEncerrando servidor...\n");
    }

    server_socket != -1 ? close(server_socket) : false;
    client_socket != -1 ? close(client_socket) : false;

    signal == 0 ? exit(EXIT_SUCCESS) : exit(EXIT_FAILURE);
}

void handle_sigint(int signal){
    if(signal == SIGINT){ // se for Ctrl+C
        stop = true;

        shutdown_routine(0);
    }
}

void handle_sigterm(int signal){
    if(signal == SIGTERM){ // se fechar pelo X
        stop = true;

        shutdown_routine(0);
    }
}

void handle_sigchld(int signal){
    qtd = signal == SIGCHLD ? qtd - 1 : qtd;
}

void *handle_msg_in(void *arg){
    char buffer[BUFFER_SIZE]; // buffer para a mensagem
    ssize_t recv_bytes; // qtd de bytes recebidos
    char time_buffer[20];
    time_t rawtime;
    struct tm *timeinfo;

    while(!stop){
        recv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if(recv_bytes <= 0){
            recv_bytes == 0 ? printf("Conexão perdida com o cliente.\n") : perror("Erro na recepção de dados do cliente.\n");

            break; // termina a lógica de comunicação
        }

        buffer[recv_bytes] = '\0'; // certifica que a string termina

        time(&rawtime);

        timeinfo = localtime(&rawtime);

        strftime(time_buffer, sizeof(time_buffer), "%H:%M", timeinfo);

        buffer[0] != '\0' ? printf("[%s] %s (%d): %s\n", time_buffer, client_name, i, buffer) : false;

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL);
}

void *handle_msg_out(void *arg){
    char buffer[BUFFER_SIZE]; // buffer para a mensagem

    while(!stop){
        strcpy(buffer, "Ok!\n");
        if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
            perror("Erro ao enviar mensagem ao cliente.\n");

            break;
        }

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer

        sleep(1); // espera 1 segundo antes de enviar a próxima mensagem
    }

    close(client_socket);

    pthread_exit(NULL);
}