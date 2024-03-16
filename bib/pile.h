#ifndef PILE_H
#define PILE_H

typedef struct no No;
typedef struct pile Pile;

Pile *create_pile();
// aloca a pilha na memória principal

int push(Pile *pile, int id);
// empilha um elemento

int pop(Pile *pile);
// desempilha o elemento no topo

int jenga(Pile *pile, int id);
// desempilha qualquer elemento

int pile_size(Pile *pile);
// retorna o tamanho da pilha

void drop_pile(Pile *pile);
// desaloca a pilha da memória

#endif // PILE_H