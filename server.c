#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h> // rede
#include <unistd.h> // processamento paralelo

bool checkArgs(int argc, char **argv);
// checa se os parâmetros da função main são válidos

int main(int argc, char **argv){
    int i; // contador
    unsigned short int port; // porta (0 - 65535)
    int server_socket; // soquete do servidor
    int client_socket; // soquete de cliente
    struct sockaddr_in server_addr; // endereço do servidor
    struct sockaddr_in client_addr; // endereço de cliente
    socklen_t client_addr_len = sizeof(client_addr); // tamanho do endereço do cliente

    if(checkArgs(argc, argv)){
        printf("Verificação de parâmetros de inicialização bem-sucedida.\n");

        port = atoi(argv[1]);
    }else{
        fprintf(stderr, "Verificação de parâmetros de inicialização retornou falha.\n");

        return 1;
    }

    // criando soquete do servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP e IPv4
    if(server_socket == -1){
        fprintf(stderr, "Falha na criação do soquete de servidor.\n");

        return 2;
    }else printf("Criação do soquete de servidor bem-sucedida.\n");
    //

    // configurando endereço do servidor
    printf("Configurando o endereço do servidor.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // aceita conexão de qualquer IP
    server_addr.sin_port = htons(port);
    //

    // vinculando soquete à porta
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        fprintf(stderr, "Falha na vinculação do soquete à porta.\n");

        return 3;
    }else printf("Vinculação do soquete à porta bem-sucedida.\n");
    //

    // iniciando o servidor
    if(listen(server_socket, 5) == -1){
        fprintf(stderr, "Falha ao iniciar o servidor.\n");
    
        return 4;
    }else printf("\nServidor on-line e ouvindo na porta %d!\n", port);
    //

    // loop do servidor
    while(true){
        // tenta aceitar uma conexão
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1){
            printf("Falha ao aceitar conexão do cliente.\n");

            continue; // pula até a próxima iteração do laço
        }else printf("Conexão estabelecida com o cliente!\n");

        pid_t pid;

        pid = fork();

        if(pid == -1){
            // fork() falhou
            fprintf(stderr, "Conexão terminada com o cliente:\nFalha na criação do processo filho.\n");

            continue;
            //
        }else if(pid == 0){
            // processo filho
            close(server_socket); // não aceita novas conexões no processo filho

            while(true){
                // lógica de comunicação com o cliente
            }
            //
        }else{
            // processo pai
            close(client_socket); // somente o processo filho trata o cliente
            //
        }
        //
    }
    //

    return 0;
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