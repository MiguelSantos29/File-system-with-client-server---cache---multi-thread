#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFERSIZE 1024

// Funções de envio de pedidos
void escreve(char* mensagem, int destino);
void imprimeResposta(char* pipePessoal);

void pedido_adicionar(char* pedido, char* titulo, char* autor, char* ano, char* path, int destino);
void pedido_key(char* pedido, char* key, int destino);
void pedido_linhas(char* pedido, char* key, char* palavra, int destino);
void pedido_procura(char* pedido, char* key, char* processos, int destino);
void pedido_desligar(int destino);

int enviaPedido(char *argumentos[], int destino, int tamanho);