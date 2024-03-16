// bibliotecas

// padrão
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // tipo booleano
//

// rede
#include <arpa/inet.h> // manipulação e conversão de endereços IP
#include <sys/socket.h>
//

// processamento paralelo
#include <signal.h> // manipulação de sinais
#include <pthread.h> // manipulação de threads
#include <unistd.h> // manipulação de processos
//

//

// macros
#define SERVER_IP "127.0.0.1" // endereço do servidor
#define BUFFER_SIZE 1024 // tamanho do buffer para envio e recebimento de mensagens
//

// variáveis globais
int i = 0; // contador
bool stop = false; // deve parar?
int client_socket; // soquete do servidor
char client_name[16]; // nome de usuário
//

// funções

void shutdown_routine(int signal);
// rotina de encerramento do servidor

void handle_sigint(int signal);
// função para lidar com o sinal de interrupção (Ctrl+C)

void handle_sigterm(int signal);
// função para lidar com o sinal de encerramento (fecha o terminal)

void *handle_in(void *arg);
// função para lidar com o recebimento de mensagens do servidor

void *handle_out(void *arg);
// função para lidar com o envio de mensagens ao servidor

//

int main(int argc, char **argv){
    // configura os sinais
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    //

    int port; // porta
    struct sockaddr_in server_addr; // endereço do servidor

    // trata os argumentos de execução
    if(argc < 3){
        fprintf(stderr, "Argumentos insuficientes. Uso: %s <porta> <nome de usuário>\n", argv[0]);

        shutdown_routine(1);
    }else if(argc > 3){
        fprintf(stderr, "Muitos argumentos. Uso: %s <porta> <nome de usuário>\n", argv[0]);

        shutdown_routine(1);
    }else{
        for(i = 0; i < strlen(argv[1]); i++){
            if(argv[1][i] < '0' || argv[1][i] > '9'){
                fprintf(stderr, "A porta deve ser um inteiro.\n");

                shutdown_routine(1);
            }
        }

        if(strlen(argv[2]) > 15){
            fprintf(stderr, "O nome de usuario não pode ultrapassar 15 caracteres.\n");

            shutdown_routine(1);
        }

        for(i = 0; i < strlen(argv[2]); i++){
            if(argv[2][i] < 'A' || argv[2][i] > 'Z'){
                if(argv[2][i] < 'a' || argv[2][i] > 'z'){
                    fprintf(stderr, "Somente os caracteres de 'a' - 'Z' são válidos.\n");

                    shutdown_routine(1);
                }
            }
        }

        port = atoi(argv[1]);

        strncpy(client_name, argv[2], strlen(argv[2]));
    }
    //

    // criando o soquete de cliente
    printf("Fazendo o soquete de cliente...\n");
    client_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP e IPv4
    if(client_socket == -1){
        perror("Erro ao criar o soquete de cliente.\n");

        shutdown_routine(1);
    }
    //

    // configurando o endereço do servidor
    printf("Configurando o endereço do servidor...\n");
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr) == -1){ // conecta somente no endereço IPv4 do servidor
        perror("Erro ao converter o endereço IPv4.\n");

        shutdown_routine(1);
    }
    server_addr.sin_port = htons(port);
    //

    // conectando ao servidor
    printf("Estabelecendo conexão...\n");
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("Falha ao conectar ao servidor.\n");

        shutdown_routine(1);
    }
    //

    pthread_t tid_in, tid_out; // "thread" ID

    // informa o nome de usuário ao servidor
    if(send(client_socket, client_name, 16, 0) == -1){
            perror("Falha ao informar o nome de usuário ao servidor.\n");

            shutdown_routine(1);
        }
    //

    // lógica de comunicação com o servidor
    if(pthread_create(&tid_in, NULL, handle_in, NULL) != 0){
        perror("Erro ao criar thread para escutar o servidor.\n");

        shutdown_routine(1);
    }

    if(pthread_create(&tid_out, NULL, handle_out, NULL) != 0){
        perror("Erro ao criar thread para falar ao servidor.\n");

        shutdown_routine(1);
    }
    //

    // espera o servidor parar de responder
    if(pthread_join(tid_in, NULL) != 0){
        perror("Erro ao aguardar a thread.\n");

        shutdown_routine(1);
    }
    //
    
    shutdown_routine(0);
}

void shutdown_routine(int signal){
    printf("\nEncerrando aplicação...\n");

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

void *handle_in(void *arg){
    char buffer[BUFFER_SIZE]; // buffer para a mensagem
    ssize_t recv_bytes; // qtd de bytes recebidos

    while(!stop){
        recv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if(recv_bytes <= 0){
            recv_bytes == 0 ? printf("\nConexão perdida com o servidor.\n") : perror("\nErro na recepção de dados do servidor.\n");

            shutdown_routine(1);
        }

        buffer[recv_bytes] = '\0'; // certifica que a string termina

        if(strncmp(buffer, "Ok!", 3) != 0){
            printf("\nO servidor terminou a conexão.\n");

            break;
        }

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL); // termina a thread
}

void *handle_out(void *arg){
    char buffer[BUFFER_SIZE]; // buffer para a mensagem

    while(!stop){
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);

        strncmp(buffer, "!exit", 5) != 0 ? true : shutdown_routine(0);

        if(send(client_socket, buffer, BUFFER_SIZE, 0) == -1){
            perror("Erro ao enviar mensagem ao servidor.\n");

            break;
        }

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer
    }

    close(client_socket);

    pthread_exit(NULL);
}