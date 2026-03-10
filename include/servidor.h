#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <syslog.h>
#include <stdio.h>


#define DUPLICADOS 1
extern char mainPath[256];
extern int power;

int inserir(char pedido[]);
int removeKey(int key);
void guardarValor(void* chave, void* valor, void* ficheiro_ptr);
void guardarDocPorKey(int key);
void doUpdate (int key);
void sendUpdate (int key);

void pedidoAdicionar(char* destino, char* pedido);
void pedidoRemover(char* destino, char* pedido);
void pedidoConsulta(char* destino, char* pedido);
void pedidoContaLinhas(char* destino, char* pedido);
void pedidoPalavras(char* destino, char* pedido);
void desligaservidor(char* destino);

void contaLinhas(const char* palavra, int key, int resposta);
int palavraExiste(char* palavra, char* path);
void palavraChave(char* palavra, int n, int total, int resposta);

void tratamensagem(char* mensagem);
void cleanup(void);
void info ();

int conta_descritores_abertos();