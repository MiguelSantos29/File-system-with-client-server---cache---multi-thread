#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GHashTable* dados = NULL; //hash com os dados
cacheTipo cache; //cache
int sizeS = 0;
chavePath* listaS = NULL;  //lista para a query s do servidor
int ketTestes=1;


int comparaInt(gconstpointer a, gconstpointer b) {
  return *((int*)a) - *((int*)b);
}

void coldstart (int tamanho) {
  cache.total = tamanho;
  cache.usado = 0;
  dados = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
  cache.cache = g_queue_new();
}

//Gera uma key aleatoria que nao tenha na hash
/*
int gerachave () {
  int key= rand() % MAXRAND;
  char path[256];
  sprintf(path, "%s%d.txt", DISCO,key);
  while (g_hash_table_contains(dados,&key) || (access(path, F_OK))==0) {
    key = rand() % MAXRAND;
    sprintf(path, "%s%d.txt", DISCO,key);
  }
  return key;
}
*/

int gerachave () {
  char path[256];
  sprintf(path, "%s%d.txt", DISCO,ketTestes);
  while (g_hash_table_contains(dados,&ketTestes) || (access(path, F_OK))==0) {
    ketTestes++;
    sprintf(path, "%s%d.txt", DISCO,ketTestes);
  }
  return ketTestes;
}

int estaEmCache (int key){
  if (g_hash_table_contains(dados,&key)) {
    return 1;
  }
  else return 0;
}

int temEspaco () {
  int espaco = cache.total - cache.usado;
  if (espaco == 0) {
    return 0;
  }
  else return 1;
}

int gestaoCacheLigada () {
  int* removido = NULL;

#if TIPOCACHE == 0
    // LRU: remove o mais antigo
    removido = g_queue_pop_head(cache.cache);

#elif TIPOCACHE == 1
    // MRU: remove o mais recente
    removido = g_queue_pop_tail(cache.cache);

#elif TIPOCACHE == 2
    // RANDOM: remove uma posição aleatória
    int idx = rand() % cache.usado;
    removido = g_queue_pop_nth(cache.cache, idx);

#else
    #error "TIPOCACHE inválido. Usa 0 (LRU), 1 (MRU) ou 2 (RANDOM)"
#endif

    if (removido != NULL) {
        int chave = *removido;
        free(removido);
        cache.usado--;
        return chave;
    }

    return -1;  // erro inesperado
}

void gestaoCache () {
  int removido;
  removido = gestaoCacheLigada();
  if (removido == -1) {
    perror("(CACHE)Erro na gestão da cache");
  }
  else {
    if (g_hash_table_remove(dados, &removido)!=1) {
      perror("(CACHE)Erro a remover da hash");
    }
  }
}

DocInfo* getDoc (int key) {

  DocInfo* doc = g_hash_table_lookup(dados, &key);
  if (doc != NULL) {
    return doc;
  }

  doc = malloc(sizeof(DocInfo));
  if (doc == NULL) {
    perror("(CACHE)Erro ao alocar DocInfo");
    return NULL;
  }

  int tamanhoPath = strlen(DISCO) + MAXKEYLEN + 6;
  char realPath[tamanhoPath];
  sprintf(realPath, "%s%d.txt", DISCO, key);
  if (access(realPath, F_OK) != 0) {
    free(doc);
    return NULL;
  }

  int file = open(realPath, O_RDONLY);
  if (file == -1) {
    free(doc);
    perror("(CACHE)Erro a abrir mem para gerar Doc");
    return NULL;
  }

  int tamanho = 475 + MAXKEYLEN;
  char buffer[tamanho];
  int lidos = read(file, buffer, tamanho);
  if (lidos == -1) {
    free(doc);
    perror("(CACHE)Erro ao ler doc da Mem");
    close(file);
    return NULL;
  }
  buffer[lidos] = '\0';

  char* token = strtok(buffer, "\n");
  if (!token) {
    close(file);
    free(doc);
    return NULL;
  }
  doc->id = atoi(token);

  token = strtok(NULL, "\n");
  if (!token) {
    close(file);
    free(doc);
    return NULL;
  }
  strcpy(doc->titulo, token);

  token = strtok(NULL, "\n");
  if (!token) {
    close(file);
    free(doc);
    return NULL;
  }
  strcpy(doc->autor, token);

  token = strtok(NULL, "\n");
  if (!token) {
    close(file);
    free(doc);
    return NULL;
  }
  strcpy(doc->ano, token);

  token = strtok(NULL, "\n");
  if (!token) {
    close(file);
    free(doc);
    return NULL;
  }
  strcpy(doc->path, token);

  close(file);
  return doc;
}

void adicionaKey (int key) {
  DocInfo* doc;
  doc = getDoc(key);
  if (doc == NULL) {
    perror("(CACHE)Erro ao gerar Doc para adicionar pos consulta");
    return;
  }
  insereCache(doc->id, doc->titulo, doc->autor, doc->ano, doc->path);
}

void gestaoConsulta (int key) {
  if (g_hash_table_contains(dados, &key)) {
    int* keyTemp = malloc(sizeof(int));
    *keyTemp = key;

    GList* node = g_queue_find_custom(cache.cache, keyTemp, (GCompareFunc)comparaInt);

    if (node != NULL) {
      int* valor = (int*)node->data;
      g_queue_remove(cache.cache, valor);
      free(valor);
      g_queue_push_tail (cache.cache, keyTemp);
    }
    else {
      free(keyTemp);
      perror("(CACHE)Erro na gestão consulta");
    }
  }
  else {
    gestaoCache();
    adicionaKey(key);
  }
}

int apagaLista (int* key) {
  GList* node = g_queue_find_custom(cache.cache, key, (GCompareFunc)comparaInt);
  	if (node != NULL) {
    	int* conteudo = (int*)node->data;
  	  int chaveReal = *key;
      g_queue_remove(cache.cache, conteudo);
      free(conteudo);
      cache.usado--;
      free(key);
      return apagaMemoria(chaveReal);
    }
 else {
      perror("(CACHE)Erro a apagar da Lista");
      return -1;
 }
}

int apagaCache (int key) {
  int* keyTemp = malloc(sizeof(int));
  *keyTemp = key;
  if(g_hash_table_remove(dados, keyTemp)) {
  	return apagaLista(keyTemp);
  }
  else {
    free(keyTemp);
    return apagaMemoria(key);
  }
}

int apagaMemoria (int key) {
  int tamanhoPath;
  tamanhoPath = strlen(DISCO) + MAXKEYLEN + 6;
  char path[tamanhoPath];
  sprintf (path,"%s%i.txt",DISCO,key);
  if (unlink(path)==0) {
    return key;
  }
  else return -1;
}

void geraFicheiro (int key, char* titulo, char* autor, char* ano, char * path) {
  int tamanho=0,escrito=0;
  tamanho =strlen(DISCO) + MAXKEYLEN + 6;
  char realPath[tamanho];
  sprintf(realPath, "%s%d.txt",DISCO,key);
  int ficheiro = open(realPath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (ficheiro == -1) {
    perror("(CACHE)Erro na creacao do ficheiro de DISCO");
    return;
  }
  char conteudo[475+MAXKEYLEN];
  sprintf(conteudo, "%d\n%s\n%s\n%s\n%s\n", key,titulo,autor,ano,path);
  escrito = write(ficheiro, conteudo, strlen(conteudo));
  if (escrito == -1) {
    perror("(CACHE)Erro na escrever o ficheiro de DISCO");
  }
  close(ficheiro);
}

int insereCache (int key, char* titulo, char* autor, char* ano, char * path) {
  if (temEspaco()) {
    DocInfo* doc = malloc(sizeof(DocInfo));
    int* keyHash = malloc(sizeof(int));
    int* keyQueue = malloc(sizeof(int));
    *keyHash = key;
    *keyQueue = key;
    doc->id = key;
    if (!titulo || !autor || !ano || !path) return -1;
    strcpy(doc->titulo, titulo);
    strcpy(doc->autor, autor);
    strcpy(doc->ano, ano);
    strcpy(doc->path, path);
	  g_hash_table_insert (dados, keyHash, doc);
    g_queue_push_tail (cache.cache, keyQueue);
    cache.usado++;
    geraFicheiro(key,titulo,autor, ano, path);
    return key;
  }
  else {
    gestaoCache();
    return insereCache(key, titulo, autor, ano, path);

  }
}

void shutdown() {
    //Free da lista ligada
    if (cache.cache != NULL) {
        g_queue_foreach(cache.cache, (GFunc)free, NULL);  // free das keys
        g_queue_free(cache.cache);                        // free da estrutura
    }

    //Libertar a hash
    if (dados != NULL) {
        g_hash_table_destroy(dados); //free da hash
    }

    //Libertar listaS
    if (listaS != NULL) {
        free(listaS);
    }
}

int contaMem() {
  int ficheiros = 0;
  DIR* dir = NULL;
  dir = opendir(DISCO);
  if (dir == NULL) {
    perror("(CACHE)Erro ao abrir diretório DISCO");
    return 0;
  }
  struct dirent* file;
  while ((file = readdir(dir)) != NULL) {
    if (strstr(file->d_name, ".txt") != NULL) {
      ficheiros++;
    }
  }
  closedir(dir);
  return ficheiros;
}

int geraListaS () {
  if (listaS != NULL) {
    free(listaS);
    listaS = NULL;
  }

  sizeS = contaMem();
  listaS = malloc(sizeof(chavePath) * sizeS);
  int i = 0;
  DocInfo* doc = NULL;
  DIR* dir = NULL;
  dir = opendir(DISCO);
  if (dir == NULL) {
    perror("(CACHE)Erro ao abrir diretório DISCO");
    free(listaS);
    listaS = NULL;
    sizeS = 0;
    return 0;
  }
  struct dirent* file = NULL;
  while ((file = readdir(dir)) != NULL) {
    if (strstr(file->d_name, ".txt") != NULL) {
      char copiaFilePath[64];
      strcpy(copiaFilePath,file->d_name);
      char* textoKey = strtok(copiaFilePath, ".");
      int key = atoi(textoKey);
        doc = getDoc(key);
        if (doc != NULL) {
          listaS[i].chave = doc->id;
          strcpy(listaS[i].path, doc->path);
          i++;
          if (!estaEmCache(key)) {
            free(doc);
          }
        }
    }
  }

  closedir(dir);
  return i;
}



void imprimeCache() {
  printf("Na hash temos %d entradas ||||| na queue temos %d entradas\n",g_hash_table_size(dados),cache.usado);
  printf ("Tamanho total - (%d) Tamanho usado (%d)\n", cache.total, cache.usado);
  printf("Cabeça - ");
  for (GList* node = cache.cache->head; node != NULL; node = node->next) {
    int* ptr = NULL;
    ptr = (int*)node->data;
    int valor = *ptr;
    printf("%d", valor);
    if (node->next != NULL) {
      printf(" <- ");
    }
  }
  printf(" Fim\n");
}

void imprimeListaS(int total) {
  printf("=== Conteúdo de listaS ===\n");
  for (int i = 0; i < total; i++) {
    printf("%d -> %s\n", listaS[i].chave, listaS[i].path);
  }
  printf("==========================\n\n");
}

int comparaDoc(DocInfo* doc, char* titulo, char* autor, char* ano, char* path) {
  if (strcmp(doc->titulo, titulo) == 0 &&
      strcmp(doc->autor, autor) == 0 &&
      strcmp(doc->ano, ano) == 0 &&
      strcmp(doc->path, path) == 0) {
    int res =doc->id;
    return res;
      }
  return -1;
}


int existeDoc (char* titulo, char* autor, char* ano, char* path) {
  int encontrou = 0;
  DocInfo* doc = NULL;
  DIR* dir = NULL;
  int res=-1;
  dir = opendir(DISCO);
  if (dir == NULL) {
    perror("(CACHE)Erro ao abrir diretório DISCO");
    return 0;
  }
  struct dirent* file = NULL;
  while ((file = readdir(dir)) != NULL && !encontrou)  {
    if (strstr(file->d_name, ".txt") != NULL) {
      char copiaFilePath[64];
      strcpy(copiaFilePath,file->d_name);
      char* textoKey = strtok(copiaFilePath, ".");
      if (!textoKey) {
        perror("Falhou strtok dos duplicados");
      }
      int key = atoi(textoKey);
      //printf("k-%d\n", key);
      doc = getDoc(key);
      if (doc != NULL) {
        res = comparaDoc(doc, titulo, autor, ano, path);
        if (res > 0) {
          encontrou = res;
          if (!estaEmCache(key)) {
            free(doc);
          }
          break;
        }
        if (!estaEmCache(key)) {
          free(doc);
        }
      }
    }
  }

  closedir(dir);
  return encontrou;
}

void printTCache () {
#if TIPOCACHE == 0
  // LRU
  write (1,"CACHE LRU ATIVA\n",strlen("CACHE LRU ATIVA\n")+1);

#elif TIPOCACHE == 1
  // MRU
  write (1,"CACHE MRU ATIVA\n",strlen("CACHE MRU ATIVA\n")+1);

#elif TIPOCACHE == 2
  // RANDOM
  write (1,"CACHE RANDOM ATIVA\n",strlen("CACHE RANDOM ATIVA\n")+1);
#endif
}