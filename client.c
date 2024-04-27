#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"

bool checkArgs(int argc, char **argv);
// checa se os parâmetros da função main são válidos

int main(int argc, char **argv){
    int i; // contador
    unsigned short int port; // porta (0 - 65535)
    char username[16];
    int client_socket; // soquete de cliente
    struct sockaddr_in server_addr; // endereço do servidor

    if(checkArgs(argc, argv)){
        printf("Verificação de parâmetros de inicialização bem-sucedida.\n");

        port = atoi(argv[1]);
        strcpy(username, argv[2]);
        username[15] = '\0';
    }else{
        fprintf(stderr, "Verificação de parâmetros de inicialização retornou falha.\n");

        return 1;
    }

    // criando soquete de cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP e IPv4
    if(client_socket == -1){
        fprintf(stderr, "Falha na criação do soquete de cliente.\n");

        return 2;
    }else printf("Criação do soquete de cliente bem-sucedida.\n");
    //

    // configurando endereço do servidor
    printf("Configurando o endereço do servidor.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(port);
    //

    // conectando ao servidor
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        fprintf(stderr, "Falha ao conectar ao servidor.\n");

        return 3;
    }else printf("\nConexão estabelecida com o servidor!\n");
    //

    while(true){
        // lógica de comunicação com o servidor
    }

    return 0;
}

bool checkArgs(int argc, char **argv){
    int i; // contador
    bool flag = true;

    if(argc < 3){
        fprintf(stderr, "Argumentos insuficientes.\nUso: ./server <porta> <nome de usuário>\n");

        flag = false;
    }else if(argc > 3){
        fprintf(stderr, "Argumentos demais.\nUso: ./server <porta> <nome de usuário>\n");

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

        // verifica se o nome de usuário é inválido
        if(strlen(argv[2]) > 15){
            fprintf(stderr, "O nome de usuário não pode exceder 15 caracteres.\n");

            flag = false;
        }

        for(i = 0; i < strlen(argv[2]); i++){
            if((argv[2][i] < 65) || (argv[2][i] > 90) && (argv[2][i] < 97) || (argv[2][i] > 122) && (argv[2][1] != 95)){
                fprintf(stderr, "O nome de usuário só pode conter caracteres válidos:\nA-Z a-z _\n");

                flag = false;

                break;
            }
        }
        //
    }
    
    return flag;
}