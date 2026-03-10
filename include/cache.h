#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>


#define BUFFERSIZE 1024
#define MAXRAND 1000
#define MAXKEYLEN 4
#define TIPOCACHE 1

// 0 - LRU
// 1 - MRU
// 2 - Random

#define SAVELOCAL "dados_info.txt"
#define DISCO "../indice/"

typedef struct {
    int id;
    char titulo[200];
    char autor[200];
    char ano[5];
    char path[64];
} DocInfo;

typedef struct {
    int total;
    int usado;
    GQueue* cache;
} cacheTipo;

typedef struct {
    int chave;
    char path[64];
} chavePath;

extern chavePath* listaS;
extern int sizeS;


int comparaInt(gconstpointer a, gconstpointer b);
void coldstart(int tamanho);
int gerachave();
int estaEmCache(int key);
int temEspaco();
int gestaoCacheLigada();
void gestaoCache();
void gestaoConsulta(int key);
int insereCache(int key, char* titulo, char* autor, char* ano, char* path);
int apagaCache(int key);
int apagaMemoria(int key);
int apagaLista(int* key);
void adicionaKey(int key);
void shutdown();
int existeDoc (char* titulo, char* autor, char* ano, char* path);
int comparaDoc(DocInfo* doc, char* titulo, char* autor, char* ano, char* path);


void geraFicheiro(int key, char* titulo, char* autor, char* ano, char* path);
DocInfo* getDoc(int key);
int contaMem();
int geraListaS();


//Debug
void imprimeCache();
void imprimeListaS(int total);
void printTCache ();