#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

bool checkArgs(int argc, char **argv);
// checa se os parâmetros da função main são válidos

int main(int argc, char **argv){
    int i; // contador
    unsigned short int port; // porta (0 - 65535)

    if(checkArgs(argc, argv)){
        printf("Verificação de parâmetros de inicialização bem-sucedida.\n");

        port = atoi(argv[1]);
        printf("%d\n", port);
    }else{
        fprintf(stderr, "Verificação de parâmetros de inicialização retornou falha.\n");

        return 1;
    }

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