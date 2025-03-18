/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DIMPLAY1 20
#define DIMPLAY2 20
#define CLINAME "/tmp/CLI"
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */


// VARIÁVEIS GLOBAIS
int dif;

int sd_stream;                    // file descriptor do socket stream
socklen_t addrlen_s;
struct sockaddr_un srv_addr_s;    

int sd_datagram;                  // file descriptor do socket datagrama
socklen_t tolen_d;
socklen_t addrlen_d;
struct sockaddr_un to_d;
struct sockaddr_un my_addr_d;

typedef enum 
{
  CNJ,
  JG,
  CLM,
} commands_t;

typedef struct
{
  commands_t command;
  unsigned short int n;
  char Name[4];
  char move[6];
  unsigned int j;
  time_t t;

} coms;

/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair (int argc, char **argv){        // POSSO FAZER close(..) ou unlink(..) A COISAS QUE NÃO EXISTEM? OU SEJA SD AINDA NÃO TINHA SIDO CRIADO P.E.
  close(sd_stream);
  //unlink(my_addr_d.sun_path)
  close(sd_datagram);
  exit(0);
}


/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj (int argc, char** argv){
  char mypid[6];
  char buf_s[50];
  char buf_d[50];
  char new_game[9];
  
  if(argc != 3){    // comando + nome + dificuldade
    printf("[ERRO] Número de argumentos inválido\n");
    return;     // volta para a "linha de comandos"
  }
  if(strlen(argv[1]) != 3){     // nome com 3 chars
    printf("\n\n[ERRO] Nome inválido! Introduzir no máximo 3 caracteres.\n\n");
    return;     // volta para a "linha de comandos"
  }else{
    dif = atoi(argv[2]);
    if((dif == 1) || (dif == 2)){     // dificuldade: 1 ou 2
    /*-----------------------criar-socket-stream------------------------*/
      if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){  // tenta criar socket
        perror("[ERRO] Criação de socket stream falhou. Tentar novamente:\n"); 
        return;     // volta para a "linha de comandos"
      }

      if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){
        perror("[ERRO] Criação de socket stream falhou. Tentar novamente:\n"); 
        return;     // volta para a "linha de comandos"
      }

      srv_addr_s.sun_family = AF_UNIX;
      memset(srv_addr_s.sun_path, 0, sizeof(srv_addr_s.sun_path));
      strcpy(srv_addr_s.sun_path, JMMSERVSS);      
      addrlen_s = sizeof(srv_addr_s.sun_family) + strlen(srv_addr_s.sun_path);
    
      if(connect(sd_stream, (struct sockaddr *)&srv_addr_s, addrlen_s) < 0){
        perror("[ERRO] Connect. Tentar novamente:\n"); 
        return;     // volta para a "linha de comandos"
      }
      
      sprintf(new_game, "%s %s %s", argv[0], argv[1], argv[2]);
      if((write(sd_stream, new_game, strlen(new_game) + 1) < 0)){
        perror("[ERRO] Write para o servidor. Tentar novamente:\n");
        close(sd_stream);
        return;     // volta para a "linha de comandos"
      }else{
        while(read(sd_stream, buf_s, sizeof(buf_s)) < 0);
        printf("[INFO] Cliente recebeu confirmação: %s\n", buf_s);      
      }

    /*----------------------criar-socket-datagrama----------------------*/
      if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ){     // tenta criar um socket datagrama
        perror("[ERRO] Criação de socket datagrama falhou. Tentar Novamente:\n");
        close(sd_stream);
        return;     // volta para a "linha de comandos"
      }
        
      my_addr_d.sun_family = AF_UNIX;
      memset(my_addr_d.sun_path, 0, sizeof(my_addr_d.sun_path));
      strcpy(my_addr_d.sun_path, CLINAME);
      sprintf(mypid, "%d", getpid());
      strcat(my_addr_d.sun_path, mypid);      // junta o path dos clientes com o pid criando um identificador único
      addrlen_d = sizeof(my_addr_d.sun_family) + strlen(my_addr_d.sun_path);
      
      if(bind(sd_datagram, (struct sockaddr *)&my_addr_d, addrlen_d) < 0 ){
        perror("[ERRO] Bind do socket datagrama. Tentar novamente:\n");
        close(sd_stream); 
        return;     // volta para a "linha de comandos"
      }
  
      to_d.sun_family = AF_UNIX;
      memset(to_d.sun_path, 0, sizeof(to_d.sun_path));
      strcpy(to_d.sun_path, JMMSERVSD);
      tolen_d = sizeof(my_addr_d.sun_family) + strlen(to_d.sun_path);
  
      /*
      if(sendto(sd_datagram, MSG, strlen(MSG) + 1, 0, (struct sockaddr *)&to_d, tolen_d) < 0){
        perror("CLI: Erro no sendto");
      }else{
        if(recvfrom(sd_datagram, buf_d, sizeof(buf_d), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
          perror("CLI: Erro no recvfrom");                                            
        }else{
          printf("CLI: Recebi: %s\n", buf_d);
        }
      }
      */
    }else{
      printf("[ERRO] Nível de dificuldade (%d) inválido! Inserir dificuldade: 1 ou 2.\n\n", dif);
      return;
    }
  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_jg - fazer jogada
+--------------------------------------------------------------------------*/
void cmd_jg (int argc, char** argv){
  int aux;
  char xua;
  char dif1_play[DIMPLAY1];
  char dif2_play[DIMPLAY2];
  char move[50];

  if(dif == 1){
    if(strlen(argv[1]) != 3){
      printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
      return;
    }

    strcpy(dif1_play, argv[1]);
    for(aux = 0; aux < strlen(dif1_play); aux++){
      xua = dif1_play[aux];
      if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E'))){
        printf("OLA!!!");
        printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
        return;
      }
    }

    sprintf(move, "%s %s", argv[0], argv[1]);
    if((write(sd_stream, move, strlen(move) + 1) < 0)){
      perror("[ERRO] Write para o servidor. Tentar novamente:\n");
      return;
    }else{
      while(read(sd_stream, dif1_play, sizeof(dif1_play)) < 0);
      printf("[INFO] Jogada: %s\n", dif1_play);      
    }
  }
  if(dif == 2){
    if(strlen(argv[1]) != 5){
      printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
      return;
    }

    strcpy(dif2_play, argv[1]);
    for(aux = 0; aux < strlen(dif2_play); aux++){
      xua = dif2_play[aux];
      if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E') || (xua == 'F') || (xua == 'G') || (xua == 'H'))){
        printf("[ERRO] Repetir jogada! Introduzir 5 letras (de {ABCDEFGH})");
        return;
      }
    }

    sprintf(move, "%s %s", argv[0], argv[1]);
    if((write(sd_stream, move, strlen(move) + 1) < 0)){
      perror("[ERRO] Write para o servidor. Tentar novamente:\n");
      return;
    }else{
      while(read(sd_stream, dif2_play, sizeof(dif2_play)) < 0);
      printf("[INFO] Jogada: %s\n", dif2_play);      
    }
  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm (int argc, char** argv){
  char requested_info[15];

  if(sendto(sd_datagram, argv[0], strlen(argv[0]) + 1, 0, (struct sockaddr *)&to_d, tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor");
    return;     // volta para a "linha de comandos"
  }else{
    if(recvfrom(sd_datagram, requested_info, sizeof(requested_info), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("[INFO] Informação recebida: %s\n", requested_info);
    }
  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_mlm - mudar limites
+--------------------------------------------------------------------------*/
void cmd_mlm (int argc, char** argv){
  char request[50];
  char from_serv[50];
  int jogadas, tempo;

  if(argc == 3){
    jogadas = atoi(argv[1]);
    tempo = atoi(argv[2]);

    if((jogadas > 0) || (tempo > 0)){
      sprintf(request, "%s %s %s", argv[0], argv[1], argv[2]);
      
      if(sendto(sd_datagram, request, strlen(request) + 1, 0, (struct sockaddr *)&to_d, tolen_d) < 0){
        printf("[ERRO] Envio de pedido ao servidor");
        return;     // volta para a "linha de comandos"
      }else{                                                                          /* ALGO DO GÉNERO "PEDIDO CONCEBIDO. NOVOS LIMITES: ..." */
        if(recvfrom(sd_datagram, from_serv, sizeof(from_serv), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
          printf("[ERRO] Receção de informação do servidor");
          return;     // volta para a "linha de comandos"                                            
        }else{
          printf("[INFO] Informação recebida: %s\n", from_serv);
        }
      }

    }else{
      printf("[ERRO] Argumento(s) fora dos limites válidos.");
    }
  }else{
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_cer - consultar estado
+--------------------------------------------------------------------------*/
void cmd_cer (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_aer - activar envio
+--------------------------------------------------------------------------*/
void cmd_aer (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_der - desactivar envio
+--------------------------------------------------------------------------*/
void cmd_der (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_tmm - terminar processo mastermind
+--------------------------------------------------------------------------*/
void cmd_tmm (int argc, char** argv){
  printf("1");
}



/*++++++++++++++++++++++++++++++++_JMMlog_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_ltc - listar classificações
+--------------------------------------------------------------------------*/
void cmd_ltc (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_rtc - reiniciar tabelas de classificação
+--------------------------------------------------------------------------*/
void cmd_rtc (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_trh - terminar processo de registo histórico
+--------------------------------------------------------------------------*/
void cmd_trh (int argc, char** argv){
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_test - função de teste: apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test (int argc, char** argv){
  int i;

  for (i = 0; i < argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}