#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

bool checkArgs(int argc, char **argv);
// checa se os parâmetros da função main são válidos

int main(int argc, char **argv){
    int i; // contador
    unsigned short int port; // porta (0 - 65535)
    char username[16];

    if(checkArgs(argc, argv)){
        printf("Verificação de parâmetros de inicialização bem-sucedida.\n");

        port = atoi(argv[1]);
        printf("%d\n", port);
        strcpy(username, argv[2]);
        printf("%s\n", username);
        username[15] = '\0';
        printf("%s\n", username);
    }else{
        fprintf(stderr, "Verificação de parâmetros de inicialização retornou falha.\n");

        return 1;
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