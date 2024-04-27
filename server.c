// BIBLIOTECAS //

// padrão
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
//

// rede
#include <arpa/inet.h>
//

// processamento paralelo
#include <unistd.h>
#include <pthread.h>
//

/////////////////



// MACROS //

#define BUFFER_SIZE 1024

////////////



// FUNÇÕES //

bool checkArgs(int argc, char **argv);
// checa se os parâmetros da função main são válidos

void *handleMsgIn(void *arg);
// lida com o recebimento de mensagens do cliente

void *handleMsgOut(void *arg);
// lida com o envio de mensagens ao cliente

/////////////

int main(int argc, char **argv){
    unsigned short int port; // porta (0 - 65535)
    int server_socket; // soquete do servidor
    int client_socket; // soquete de cliente
    struct sockaddr_in server_addr; // endereço do servidor
    struct sockaddr_in client_addr; // endereço de cliente
    socklen_t client_addr_len = sizeof(client_addr); // tamanho do endereço de cliente
    pid_t pid; // "process id"
    pthread_t tid_in, tid_out; // "thread id"

    if(checkArgs(argc, argv)){
        printf("Verificação de parâmetros de inicialização bem-sucedida.\n");

        port = atoi(argv[1]);
    }else{
        fprintf(stderr, "Verificação de parâmetros de inicialização falhou.\n");

        exit(EXIT_FAILURE);
    }

    // criando soquete do servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP e IPv4
    if(server_socket == -1){
        fprintf(stderr, "Falha na criação do soquete do servidor.\n");

        exit(EXIT_FAILURE);
    }else printf("Criação do soquete do servidor bem-sucedida.\n");
    //

    // configurando endereço do servidor
    printf("Configurando o endereço do servidor...\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // aceita conexão de qualquer IP
    server_addr.sin_port = htons(port);
    //

    // vinculando soquete à porta
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        fprintf(stderr, "Falha ao vincular o soquete à porta.\n");

        exit(EXIT_FAILURE);
    }else printf("Vinculação do soquete à porta bem-sucedida.\n");
    //

    // iniciando o servidor
    if(listen(server_socket, 5) == -1){
        fprintf(stderr, "Falha ao iniciar o servidor.\n");
    
        exit(EXIT_FAILURE);
    }else printf("\nServidor on-line e ouvindo na porta %d!\n", port);
    //

    // loop do servidor
    while(true){
        // tenta aceitar uma nova conexão
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1){
            printf("Falha ao aceitar conexão do cliente.\n");

            continue; // pula até a próxima iteração do laço
        }else printf("Conexão estabelecida com o cliente!\n");

        pid = fork();

        if(pid == -1){
            // fork() falhou
            fprintf(stderr, "Conexão terminada com o cliente.\nCriação de processo filho falhou.\n");

            close(client_socket);

            continue;
            //
        }else if(pid == 0){
            // processo filho
            close(server_socket); // não aceita novas conexões no processo filho

            // lógica de comunicação com o cliente
            if(pthread_create(&tid_in, NULL, handleMsgIn, &client_socket) != 0){
                fprintf(stderr, "Falha ao criar thread para escutar o cliente.\n");

                exit(EXIT_FAILURE);
            }

            if(pthread_create(&tid_out, NULL, handleMsgOut, &client_socket) != 0){
                fprintf(stderr, "Falha ao criar thread para falar ao cliente.\n");

                exit(EXIT_FAILURE);
            }
                
            if(pthread_join(tid_in, NULL) != 0){ // espera o fim da conexão com o cliente
                fprintf(stderr, "Falha ao aguardar a thread handleMsgIn().\n");

                exit(EXIT_FAILURE);
            }
            //

            printf("Conexão com o cliente terminou.\nEncerrando processo filho...\n");

            exit(EXIT_SUCCESS);
            //
        }else{
            // processo pai
            close(client_socket); // somente o processo filho trata o cliente
            //
        }
        //
    }
    //

    printf("Encerrando servidor...\n");

    exit(EXIT_SUCCESS);
}

bool checkArgs(int argc, char **argv){
    int i;
    bool flag = true;

    if(argc < 2){
        fprintf(stderr, "Argumentos insuficientes.\nUso: ./server <porta>\n");

        flag = false;
    }else if(argc > 2){
        fprintf(stderr, "Argumentos demais.\nUso: ./server <porta>\n");

        flag = false;
    }else{
        // verifica se a porta é inválida
        for(i = 0; i < strlen(argv[1]); i++){
            if((argv[1][i] < 48) || (argv[1][i] > 57)){
                flag = false;
            
                break;
            }
        }

        if((atoi(argv[1]) > 65535) || (atoi(argv[1]) < 0))flag = false;

        if(flag == false)fprintf(stderr, "A porta deve ser um número inteiro entre 0 e 65535.\n");
        //
    }

    return flag;
}

void *handleMsgIn(void *arg){
    char buffer[BUFFER_SIZE];
    ssize_t recv_bytes;
    int *client_socket = (int*) arg;

    while(true){
        recv_bytes = recv(*client_socket, buffer, BUFFER_SIZE, 0);
        
        if(recv_bytes <= 0){
            recv_bytes == 0 ? fprintf(stderr, "Conexão com o cliente foi perdida.\n") : fprintf(stderr, "Falha na recepção de dados.\n");

            break;
        }

        buffer[recv_bytes] = '\0';

        printf("Cliente: %s", buffer);

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer
    }

    close(*client_socket);

    pthread_exit(NULL);
}

void *handleMsgOut(void *arg){
    char buffer[BUFFER_SIZE];
    int *client_socket = (int*) arg;

    while(true){
        strcpy(buffer, "Ok!");

        if(send(*client_socket, buffer, BUFFER_SIZE, 0) == -1){
            fprintf(stderr, "Falha ao enviar mensagem ao cliente.\n");

            break;
        }

        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer

        sleep(1); // dá sinal de vida ao cliente a cada segundo
    }

    close(*client_socket);

    pthread_exit(NULL);
}