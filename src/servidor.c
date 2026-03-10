#include "servidor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "cache.h"


char mainPath[256]; //pasta dos docs
int power=1; //estado do servidor


int inserir (char pedido[]) {
  int key,resultado;
  char *titulo, *autor,*ano, *path;
  titulo = strtok(pedido, ";");
  if (!titulo) {return -1;}
  autor = strtok(NULL, ";");
  if (!autor) {return -1;}
  ano = strtok(NULL, ";");
  if (!ano) {return -1;}
  path = strtok(NULL, ";");
  if (!path) {return -1;}
  if (DUPLICADOS==1) {
    // Tratar Duplicados
    key=existeDoc(titulo,autor,ano,path);
    if (key>0) {
      doUpdate(key);
      return key;
    }
  }
  key = gerachave();
  resultado = insereCache (key,titulo,autor,ano,path);
  return resultado;
}

int removeKey (int key) {
  int resultado;
  resultado = apagaCache (key);
  return resultado;
}

void sendUpdate (int key) {
  int destino;
  destino = open ("ToServer",O_WRONLY);
  if (destino == -1) {
    perror ("Error ao abrir o ToServer para SendUpdate");
  }
  char mensagem[16];
  sprintf(mensagem,"%d -u %d",getpid(),key);
  int tamanho = strlen (mensagem) + 1;
  int escrito = write(destino,mensagem,tamanho);
  if (escrito == -1) {
    perror ("Error ao escrever o ToServer para SendUpdate");
  }
  close (destino);
}

void doUpdate (int key) {
  gestaoConsulta(key);
}

void pedidoAdicionar (char* destino, char* pedido) {
  int resultado;
  resultado = inserir(pedido);
  int correiroResposta = open (destino, O_WRONLY);
  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }
  char resposta[512];
  if (resultado > 0) {
    sprintf(resposta, "Documento %d indexado\n", resultado);
    int total_escrito = write(correiroResposta, resposta, strlen(resposta));
    if (total_escrito < 0) {
      perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
    }
  }
  else {
    int total_escrito = write(correiroResposta, "Para adicionar usar comando -> -a Titulo Autores Ano Path\n", strlen("Para adicionar usar comando -> -a Titulo Autores Ano Path\n")+1);
    if (total_escrito < 0) {
      perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
    }
  }
  close(correiroResposta);
}

void pedidoRemover (char* destino,char* pedido) {
  int key = atoi(pedido);
  char resposta[64];
  int total_escrito=0;

  int correiroResposta = open (destino, O_WRONLY);

  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }
  int resultado = removeKey(key);
  if (resultado == key) {
      sprintf(resposta, "Index %d removido\n", key);
      total_escrito=write(correiroResposta, resposta, strlen(resposta));
      if (total_escrito < 0) {
        perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
      }
  }
  else {
    sprintf(resposta, "Index %d não foi removido\n", key);
    total_escrito = write(correiroResposta, resposta, strlen(resposta));
    if (total_escrito < 0) {
      perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
    }
  }
  close(correiroResposta);
}

void pedidoConsulta (char* destino, char* pedido) {
  int key = atoi(pedido);
  char resposta[1024];
  int total_escrito=0;

  DocInfo* resultado = getDoc (key);

  int correiroResposta = open (destino, O_WRONLY);

  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }

  if (resultado == NULL) {
    sprintf(resposta,"Key %d inválida\n",key);
    total_escrito = write(correiroResposta, resposta, strlen(resposta));
    if (total_escrito < 0) {
      perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
    }
    close(correiroResposta);
    return;
  }
  sendUpdate(key);
  sprintf(resposta, "Titulo: %s\nAutores: %s\nAno: %s\nPath:%s\n",resultado->titulo,resultado->autor,resultado->ano,resultado->path);
  total_escrito = write(correiroResposta, resposta, strlen(resposta));
  if (total_escrito < 0) {
    perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
  }
  close(correiroResposta);
}

void contaLinhas(const char* palavra, int key, int resposta) {
  int pipes[2];
  pipe(pipes);
  DocInfo* resultado = getDoc(key);

  if (resultado == NULL) {
    int escrito = write(resposta, "Id do ficheiro não existe\n", strlen("Id do ficheiro não existe\n"));

    if (escrito < 0) {
      perror("(SERVIDOR)Erro ao escrever ficheiro");
    }
    close (pipes[0]);
    close(pipes[1]);
    close(resposta);
    return;
  }
  else {
    pid_t pid = fork();
    if (pid == 0) {
      close(pipes[0]);
      char fullPath[512];
      dup2(pipes[1], STDOUT_FILENO);
      sprintf(fullPath, "%s/%s", mainPath, resultado->path);
      if (access(fullPath, F_OK) == -1) {
        write (pipes[1], "Path não existe\n", strlen("Path não existe\n"));
        return;
      }
      else {execlp("grep", "grep", "-c", palavra, fullPath, NULL);}
      perror("(SERVIDOR)Erro ao executar grep");
      _exit(1);
    } else {
      close(pipes[1]);
      if (!estaEmCache(key)) {
        free(resultado);
      }
      sendUpdate (key);
      int lidos;
      char buffer[1024];
      int status;

      while ((lidos = read(pipes[0], buffer, 1024)) > 0) {
        if (lidos < 0) {
          perror("(SERVIDOR)Erro ao ler pipeContaLinhas Servidor");
        }
        int escrito = write(resposta, buffer, lidos);
        if (escrito < 0) {
          perror("(SERVIDOR)Erro ao escrever ficheiro");
        }
      }
      waitpid(pid, &status, 0);
      close(resposta);
    }
    close(pipes[0]);
    return;
  }
}


void pedidoContaLinhas (char* destino, char* pedido) {
  int correiroResposta = open (destino, O_WRONLY);
  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }
  char* tokenKey = strtok(pedido, " ");
  int key = atoi(tokenKey);
  char* palavra = strtok(NULL, " ");
  contaLinhas(palavra, key, correiroResposta);
}

void desligaservidor (char* destino) {
  int totalescrito=0;
  int correiroResposta = open (destino, O_WRONLY);
  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }
  totalescrito = write(correiroResposta, "Server vai desligar\n",strlen("Server vai desligar\n"));
  if (totalescrito < 0) {
    perror("(SERVIDOR)Erro ao escrever Pipe Resposta");
  }
  close(correiroResposta);
  power = 0;
}


int palavraExiste (char* palavra, char* path) {
  int lidos, resultado=0;
  char buffer[1024];
  if ( (access(path, F_OK)) !=0 ) {
    return 0;
  }
  int ficheiro = open(path, O_RDONLY);
  if (ficheiro < 0) {
    perror("(SERVIDOR)Erro ao abrir ficheiro (Palavra existe)");
    return -1;
  }
  while ((lidos=read(ficheiro, buffer, 1024))>0) {
    if (strstr(buffer, palavra) != NULL) {
      resultado=1;
      break;
    }
  }
  if (lidos<0) {
    perror("(SERVIDOR)Erro ao ler ficheiro PE - nao leu do path dado");
    close(ficheiro);
    return -1;
  }
  close(ficheiro);
  return resultado;
}


void palavraChave(char* palavra, int n, int total,int resposta) {
  int i,divide,inicio,fim;
  divide = total/n;
  int pipes[2];
  pipe (pipes);
  int escritos;
  char buffer[64];
  for (i = 0; i < n; i++) {
    int pid = fork();
    if (pid < 0) {
      close(pipes[0]);
      close(pipes[1]);
      perror("(SERVIDOR)Erro ao fazer fork");
      return;
    }
    if (pid == 0) {
      close(pipes[0]);
      inicio = i*divide;
      if (i+1!=n) {
        fim = inicio+divide;
      }
      else fim = total;

      for (;inicio<fim;inicio++) {

        char fullPath[128];
        sprintf(fullPath, "%s/%s", mainPath, listaS[inicio].path);

        if (palavraExiste(palavra, fullPath)) {
          sprintf (buffer,"%d, ",listaS[inicio].chave);
          escritos = write(pipes[1], buffer, strlen(buffer));

          if (escritos < 0) {
            perror("(SERVIDOR)Erro filho escrever Pai PalavraChave Servidor");
            close(pipes[1]);
            _exit(0);
          }
        }

      }
      close(pipes[1]);
      _exit(0);
    }

  }
  //pai
  close(pipes[1]);
  size_t bufferSize = 512;
  char* resultado = malloc(bufferSize);
  if (!resultado) {
    perror("(SERVIDOR)Erro malloc");
    close(pipes[0]);
    close(resposta);
    return;
  }
  resultado[0] = '\0';

  int lidos;
  char buffer2[64];

  while ((lidos = read(pipes[0], buffer2, sizeof(buffer2) - 1)) > 0) {
    buffer2[lidos] = '\0';

    if (strlen(resultado) + strlen(buffer2) + 2 >= bufferSize) {
      bufferSize *= 2;
      char* temp = realloc(resultado, bufferSize);
      if (!temp) {
        perror("(SERVIDOR)Erro realloc");
        free(resultado);
        close(pipes[0]);
        close (resposta);
        return;
      }
      resultado = temp;
    }

    strcat(resultado, buffer2);
  }
  for (i = 0; i < n; i++) {
    //printf("Tou a espera de um filho (%d)\n",i);
    wait(NULL);
  }
  //printf("Acabei a espera\n");
  int  len = strlen(resultado);
  if (len > 0 && resultado[len - 2] == ',') {
    resultado[len - 2] = ']';
    resultado[len-1] = '\n';
    resultado[len] = '\0';
  } else {
    strcat(resultado, "]");
  }

  if (write(resposta, "[", 1) < 0) {
    perror("(SERVIDOR)Erro a escrever [");
    close (resposta);
    close(pipes[0]);
    free(resultado);
    return;
  }
  if (write(resposta, resultado, strlen(resultado)) < 0) {
    perror("(SERVIDOR)Erro a escrever resultado");
    close(pipes[0]);
    close (resposta);
    free(resultado);
    return;
  }

  close(pipes[0]);
  free(resultado);
  close (resposta);
  return;
}

void pedidoPalavras (char* destino, char* pedido) {
  int correiroResposta = open (destino, O_WRONLY);
  if (correiroResposta < 0) {
    perror("(SERVIDOR)Erro ao abrir Pipe Resposta");
    return;
  }
  char* palavra = strtok(pedido, " ");
  char* processos = strtok(NULL, " ");
  int processosint = atoi(processos);
  int total = geraListaS();
  palavraChave(palavra, processosint,total, correiroResposta);
  close(correiroResposta);
}

void delcaraZombie () {
  int mypid = getpid();
  char buffer[128];
  sprintf(buffer, "%d -k", mypid);
  int tamanho = strlen (buffer) + 1;
  int comsInternas = open("ToServer", O_WRONLY);
  if (comsInternas < 0) {
    perror("(SERVIDOR)Erro ao abrir ToServer");
  }
  int escrito = write(comsInternas, buffer, tamanho);
  if (escrito < 0) {
    perror("(SERVIDOR)Erro ao escrever ToServer");
  }
  close (comsInternas);
}

void mataZombie (int pid) {
  int status;
  waitpid(pid, &status, 0);
}

void tratamensagem (char* mensagem) {
  int pid;
  char* resposta = strtok(mensagem, " ");
  char* pedido = strtok(NULL, " ");
  if (pedido == NULL) {return;}
  if (strcmp(pedido, "-a") == 0) {
    char* dados = strtok(NULL, "");
    pedidoAdicionar(resposta, dados);
    return;
  }
  if (strcmp(pedido, "-d") == 0) {
    char* dados = strtok(NULL, "");
    pedidoRemover (resposta,dados);
    return;
  }

  if (strcmp(pedido, "-f") == 0) {
    desligaservidor (resposta);
    return;
  }

  if (strcmp(pedido, "-k") == 0) {
    int pidFilho = atoi(resposta);
    mataZombie(pidFilho);
    return;
  }
  if (strcmp(pedido, "-u") == 0) {
    char* dados = strtok(NULL, "");
    int key = atoi(dados);
    doUpdate(key);
    return;
  }
  pid = fork ();
  if (pid == 0) {
    if (strcmp(pedido, "-c") == 0) {
      char* dados = strtok(NULL, "");
      pedidoConsulta (resposta, dados);
      delcaraZombie();
      _exit(0);
    }
    if (strcmp(pedido, "-l") == 0) {
      char* dados = strtok(NULL, "");
      pedidoContaLinhas(resposta, dados);
      delcaraZombie();
      _exit(0);
    }
    if (strcmp(pedido, "-s") == 0) {
      char* dados = strtok(NULL, "");
      pedidoPalavras(resposta, dados);
      delcaraZombie();
      _exit(0);
    }
  }
  else return;
}

void info () {
  printTCache ();
  if (DUPLICADOS) {
    write (1,"CACHE sem dups\n",strlen("CACHE sem dups\n")+1);
  }
  else {
    write (1,"CACHE com dups\n",strlen("CACHE com dups\n")+1);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    int erro= write(1,"Uso: %s <document_folder> <cache_size>\n",strlen("Uso: %s <document_folder> <cache_size>\n"));
    if (erro==-1) {
      perror("(SERVIDOR)Erro ao escrever erro nos comandos inciais");
    }
    exit(1);
  }
  int lidos=0;
  int max;
  info();
  if (mkfifo("ToServer", 0666) == -1) {
    perror("(SERVIDOR)Erro ao criar ToServer");
    return 1;
  }
  //openlog("MeuServidor", LOG_PID | LOG_CONS, LOG_USER);
  char pedido[1024];
  srand(time(NULL));

  max = atoi(argv[2]);
  strcpy(mainPath, argv[1]);
  coldstart(max);
  printf("Iniciei Tudo\n");
  //int i=0;
  int leitura = open("ToServer", O_RDWR);
  if (leitura < 0) {
    perror("(SERVIDOR)Erro ao abrir ToServer");
  }
  /*
  while ((lidos=read(leitura,pedido,1024))>0) {
    if (lidos<0) {
      perror("(SERVIDOR)Erro a ler Pedido");
    }
    else {
      printf("Recebi Pedido -> %s\n",pedido);
      tratamensagem(pedido);
    }
  }
  */
  while (power) {
    lidos=read(leitura,pedido,1024);
    if (lidos<0) {
      perror("(SERVIDOR)Erro a ler Pedido");
    }
    //syslog(LOG_INFO, "Operação recebida: %s", pedido);
    //printf("Pedido: %s\n", pedido);
    tratamensagem(pedido);
    //imprimeCache();
  }
  close(leitura);
  unlink("ToServer");
  //closelog();
  shutdown();
  return 0;
}
