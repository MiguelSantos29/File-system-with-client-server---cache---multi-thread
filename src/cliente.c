#include "cliente.h"

#include <stdio.h>

void escreve (char* mensagem, int destino) {
  int escrito=0;
  int tamanho = strlen(mensagem) + 1;
  escrito = write(destino, mensagem, tamanho);
  if (escrito == -1 || escrito != tamanho ) {
    perror("Erro ao escrever para o PIPE servidor->Cliente");
    return ;
  }

}

void imprimeResposta (char* pipePessoal) {
  int leitura = open(pipePessoal, O_RDONLY);
  if (leitura == -1) {
    printf("borrei 1\n");
    perror("Erro ao criar mypipe");
  }
  int lidos;
  char resposta[BUFFERSIZE];
  while ((lidos = read(leitura, resposta, 1024))>0) {
    if (lidos <0) {
      perror("nao leu");
      exit(1);
    }
    write(1, resposta,lidos);
  }
  close(leitura);
}


void pedido_adicionar (char* pedido, char* titulo, char* autor, char* ano, char* path, int destino) {
  char resposta[512];
  int eu = getpid();
  sprintf(resposta,"%i %s %s;%s;%s;%s",eu, pedido,titulo,autor,ano,path);
  escreve(resposta,destino);
}

void pedido_key (char* pedido, char* key, int destino) {
  char resposta[512];
  int eu = getpid();
  sprintf(resposta,"%i %s %s",eu, pedido,key);
  escreve(resposta,destino);
}

void pedido_linhas (char* pedido, char* key,char* palavra, int destino) {
  char resposta[512];
  int eu = getpid();
  sprintf(resposta,"%i %s %s %s",eu, pedido,key,palavra);
  escreve(resposta,destino);
}

void pedido_procura (char* pedido, char* key,char* processos, int destino) {
  char resposta[512];
  int eu = getpid();
  sprintf(resposta,"%i %s %s %s",eu, pedido,key,processos);
  escreve(resposta,destino);
}

void pedido_desligar (int destino) {
  char resposta[512];
  int eu = getpid();
  sprintf(resposta,"%i -f",eu);
  escreve(resposta,destino);
}

int enviaPedido(char *argumentos[], int destino,int tamanho) {
  if (strcmp(argumentos[1], "-a") == 0) {
    if (tamanho < 5) {
      perror("Adicionar 4 argumentos-> Titulo,Autor,Ano,path\n");
      return -1;
    }
    pedido_adicionar(argumentos[1], argumentos[2], argumentos[3], argumentos[4], argumentos[5], destino);
  }
  else if (strcmp(argumentos[1], "-c") == 0 || strcmp(argumentos[1], "-d") == 0) {
    if (tamanho < 2) {
      perror("Adicionar 1 argumentos-> key\n");
      return -1;;
    }
    pedido_key(argumentos[1], argumentos[2], destino);
  }
  else if (strcmp(argumentos[1], "-l") == 0) {
    if (tamanho < 3) {
      perror("Adicionar 1 argumentos-> key,path\n");
      return -1;;
    }
    pedido_linhas(argumentos[1], argumentos[2], argumentos[3], destino);
  }
  else if (strcmp(argumentos[1], "-s") == 0) {
    if (tamanho < 3) {
      perror("Adicionar 1 argumentos-> key,palavra,processos\n");
      return -1;;
    }
    pedido_procura(argumentos[1], argumentos[2], argumentos[3], destino);
  }
  else if (strcmp(argumentos[1], "-f") == 0) {
    pedido_desligar(destino);
  }
  else {
    perror("Opção inválida\n");
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int id = getpid();
  int resultado;
  char mypipe[16];
  sprintf(mypipe,"%i",id);
  if (mkfifo(mypipe, 0666) == -1) {
    perror("Erro ao criar Pipe Pessoal");
    return 1;
  }
  int escrever = open("ToServer", O_WRONLY);
  if (escrever == -1) {
    perror("Erro ao abrir ToServer");
    return 1;
  }
  resultado=enviaPedido(argv,escrever,argc);
  if (resultado == -1) {
    perror("Erro: Comando inválido");
    close (escrever);
    return 1;
  }
  close(escrever);
  sprintf(mypipe,"%d",id);
  imprimeResposta(mypipe);
  unlink(mypipe);
  return 0;
}
