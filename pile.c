// bibliotecas

// padrão
#include <stdlib.h>
//

// link
#include "pile.h"
//

//

// tipos de dado
struct no{
    struct no *next; // próximo nó
    int id; // id do nó
};

struct pile{
    No *top; // elemento no topo da pilha
    int qtd; // quantidade de elementos na pilha
};
//

// funções
Pile *create_pile(){
    Pile *pile = (Pile*) malloc(sizeof(Pile));
    if(pile == NULL)return NULL;

    pile->qtd = 0;
    pile->top = NULL;

    return pile;
}

int push(Pile *pile, int id){
    No *new_node = (No*) malloc(sizeof(No));
    if(new_node == NULL)return 1;

    new_node->id = id;

    if(pile->qtd == 0){
        new_node->next = NULL;
    }else{
        new_node->next = pile->top;
    }

    pile->top = new_node;
    
    pile->qtd++;

    return 0;
}

int pop(Pile *pile){
    No *aux = pile->top;
    
    if(pile->top == NULL)return 1;

    pile->top = aux->next;

    free(aux);

    pile->qtd--;

    return 0;
}

int jenga(Pile *pile, int id){
    No *aux = pile->top;
    No *prev = aux;

    if(aux->id == id){
        pop(pile);

        return 0;
    }

    while(aux->id != id && aux->next != NULL){
        prev = aux;
        aux = aux->next;
    }

    if(aux->id != id && aux->next == NULL)return 1;

    prev->next = aux->next;

    free(aux);

    pile->qtd--;

    return 0;
}

int pile_size(Pile *pile){
    return pile->qtd;
}

void drop_pile(Pile *pile){
    No *aux = pile->top;
    No *prev = aux;

    while(aux->next != NULL){
        aux = aux->next;

        free(prev);

        prev = aux;
    }

    free(pile);
}
//