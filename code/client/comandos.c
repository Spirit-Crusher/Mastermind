/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Nov 2002
***************************************************************************/
#pragma pack(1)

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define DIMPLAY1 20
#define DIMPLAY2 20
#define CLINAME "/tmp/CLI"
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */


// VARIÁVEIS GLOBAIS
unsigned short int dif;

int sd_stream;                    // file descriptor do socket stream
socklen_t addrlen_s;
struct sockaddr_un srv_addr_s;    

int sd_datagram;                  // file descriptor do socket datagrama
socklen_t tolen_d;
socklen_t addrlen_d;
struct sockaddr_un to_d;
struct sockaddr_un my_addr_d;


typedef enum {
  CNJ, JG, CLM,
  MLM, CER, AER,
  DER, TMM, LTC,
  RTC, TRH
} commands_t;


typedef struct {
  commands_t command;
  
  union {
    char name[4];
    char move[6];
    unsigned int j;
    unsigned short int n;
  } arg1;

  union {
    unsigned short int n;
    time_t t;
  } arg2;
} coms_t;


typedef enum{
    DIFF_ALL,
    DIFF_1,
    DIFF_2,
} game_diff_t;

typedef struct {            /* estrutura de um registo de jogo */
    int nd;                 /* nível de dificuldade do jogo */
    char nj[4];             /* nome do jogador (3 carateres) */
    int nt;                 /* número de tentativas usadas */
    time_t ti;              /* estampilha temporal início do jogo */
    time_t tf;              /* estampilha temporal fim do jogo */
} rjg_t;

typedef struct {
  rjg_t tb[10];
  int tb_n_games;
  int tb_diff;
} log_single_tab_t;

typedef struct {
  char cmd[5];               // tamanho máximo dos comandos é 4
  int arg_n;                 // valor do argumento "n"
} msg_to_JMMlog;


/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair (int argc, char **argv){        // POSSO FAZER close(..) ou unlink(..) A COISAS QUE NÃO EXISTEM? OU SEJA SD AINDA NÃO TINHA SIDO CRIADO P.E.
  close(sd_stream);
  unlink(my_addr_d.sun_path);
  close(sd_datagram);
  exit(0);
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj (int argc, char** argv){
  char mypid[6];
  char buf_s[50];
  char buf_d[50];
  coms_t coms_msg;         // struct para realizar o envio dos comandos para o servidor
  
  if(argc != 3){    // comando + nome + dificuldade
    printf("[ERRO] Número de argumentos inválido. Tentar novamente. \n\n");
    return;     // volta para a "linha de comandos"
  }

  if(strlen(argv[1]) != 3){     // nome com 3 chars
    printf("\n\n[ERRO] Nome inválido! Introduzir no máximo 3 caracteres.\n\n");
    return;     // volta para a "linha de comandos"
  }else{
    if((atoi(argv[2]) > 0.5) && (atoi(argv[2]) < 2.5)){
      dif = atoi(argv[2]);

      /*-----------------------criar-socket-stream------------------------*/
      if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){  // tenta criar socket
        perror("[ERRO] Criação de socket stream falhou. Tentar novamente. \n"); 
        return;     // volta para a "linha de comandos"
      }

      srv_addr_s.sun_family = AF_UNIX;
      memset(srv_addr_s.sun_path, 0, sizeof(srv_addr_s.sun_path));
      strcpy(srv_addr_s.sun_path, JMMSERVSS);      
      addrlen_s = sizeof(srv_addr_s.sun_family) + strlen(srv_addr_s.sun_path);
    
      if(connect(sd_stream, (struct sockaddr *)&srv_addr_s, addrlen_s) < 0){
        perror("[ERRO] Connect. Tentar novamente. \n"); 
        return;     // volta para a "linha de comandos"
      }
      
      coms_msg.command = CNJ; coms_msg.arg2.n = dif;
      strcpy(coms_msg.arg1.name, argv[1]);
      
      if((write(sd_stream, &coms_msg, sizeof(coms_msg)) < 0)){
        perror("[ERRO] Write para o servidor. Tentar novamente. \n");
        close(sd_stream);
        return;     // volta para a "linha de comandos"
      }else{
        while(read(sd_stream, buf_s, sizeof(buf_s)) < 0);
        printf("[INFO] Cliente recebeu confirmação: %s\n", buf_s);      
      }

    /*----------------------criar-socket-datagrama----------------------*/
      if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ){     // tenta criar um socket datagrama
        perror("[ERRO] Criação de socket datagrama falhou. Tentar Novamente. \n");
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
        perror("[ERRO] Bind do socket datagrama. Tentar novamente. \n");
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
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_jg - fazer jogada
+--------------------------------------------------------------------------*/
void cmd_jg (int argc, char** argv){
  unsigned short int aux;
  char xua;
  char val1_play[DIMPLAY1];
  char val2_play[DIMPLAY2];
  coms_t cmd_msg;

  if(dif == 1){
    if(strlen(argv[1]) != 3){
      printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
      return;
    }

    strcpy(val1_play, argv[1]);
    for(aux = 0; aux < strlen(val1_play); aux++){
      xua = val1_play[aux];
      if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E'))){
        // printf("OLA!!!");
        printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
        return;
      }
    }

    cmd_msg.command = JG; strcpy(cmd_msg.arg1.move, argv[1]);         // atribui dados a enviar, na estrutura de dados

    if((write(sd_stream, &cmd_msg, sizeof(cmd_msg)) < 0)){
      perror("[ERRO] Write para o servidor. Tentar novamente. \n");
      return;
    }else{
      while(read(sd_stream, val1_play, sizeof(val1_play)) < 0);       // espera até receber
      printf("[INFO] Jogada: %s\n", val1_play);                       // jogada
    }
  }

  if(dif == 2){
    if(strlen(argv[1]) != 5){
      printf("[ERRO] Repetir jogada! Introduzir 5 letras (de {ABCDEFGH})");
      return;
    }

    strcpy(val2_play, argv[1]);
    for(aux = 0; aux < strlen(val2_play); aux++){
      xua = val2_play[aux];
      if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E') || (xua == 'F') || (xua == 'G') || (xua == 'H'))){
        printf("[ERRO] Repetir jogada! Introduzir 5 letras (de {ABCDEFGH})");
        return;
      }
    }

    cmd_msg.command = JG; strcpy(cmd_msg.arg1.move, argv[1]);

    if((write(sd_stream, &cmd_msg, sizeof(cmd_msg)) < 0)){
      perror("[ERRO] Write para o servidor. Tentar novamente.\n");
      return;
    }else{
      while(read(sd_stream, val2_play, sizeof(val2_play)) < 0);
      printf("[INFO] Jogada: %s\n", val2_play);      
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm (int argc, char** argv){
  char requested_info[15];
  coms_t cmd_msg;

  cmd_msg.command = CLM;

  if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(sd_datagram, requested_info, sizeof(requested_info), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("[INFO] Informação recebida: %s\n", requested_info);
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_mlm - mudar limites
+--------------------------------------------------------------------------*/
void cmd_mlm (int argc, char** argv){
  bool xflag;
  coms_t cmd_msg;

  if(argc == 3){
    if((atoi(argv[1]) > 0) && ((atoi(argv[2])) > 0)){
      cmd_msg.command = MLM; cmd_msg.arg1.j = atoi(argv[1]); cmd_msg.arg2.t = atoi(argv[2]);
      
      if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0){
        printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");   // envio falhou
        return;     // volta para a "linha de comandos"
      }else{                                                                          /* ALGO DO GÉNERO "PEDIDO CONCEBIDO. NOVOS LIMITES: ..." */
        if(recvfrom(sd_datagram, &xflag, sizeof(xflag), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
          printf("[ERRO] Receção de informação do servidor. Tentar novamente. \n");     // receção falhou
          return;     // volta para a "linha de comandos"                                            
        }else{
          printf("[INFO] Envio de registos para o histórico: %s", xflag ? "true" : "false");
        }
      }
    }else{
      printf("[ERRO] Argumento(s) fora dos limites válidos. Tentar novamente. \n");
    }
  }else{
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_cer - consultar estado
+--------------------------------------------------------------------------*/
void cmd_cer (int argc, char** argv){
  bool xflag;
  coms_t cmd_msg;

  cmd_msg.command = CER;

  if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(sd_datagram, &xflag, sizeof(xflag), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("[INFO] Envio de registos para o histórico: %s", xflag ? "true" : "false");
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_aer - activar envio
+--------------------------------------------------------------------------*/
void cmd_aer (int argc, char** argv){
  bool xflag;
  coms_t cmd_msg;

  cmd_msg.command = DER;

  if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(sd_datagram, &xflag, sizeof(xflag), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      if(xflag)
        printf("[INFO] Informação recebida: REGISTO HISTÓRICO FOI ATIVADO \n");
      else
        printf("[INFO] Informação recebida: REGISTO HISTÓRICO NÃO FOI ATIVADO \n");
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_der - desactivar envio
+--------------------------------------------------------------------------*/
void cmd_der (int argc, char** argv){
  bool xflag;
  coms_t cmd_msg;

  cmd_msg.command = DER;

  if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(sd_datagram, &xflag, sizeof(xflag), 0, (struct sockaddr *)&to_d, &tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      if(xflag)
        printf("[INFO] Informação recebida: REGISTO HISTÓRICO FOI DESATIVADO \n");
      else
        printf("[INFO] Informação recebida: REGISTO HISTÓRICO NÃO FOI DESATIVADO \n");
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_tmm - terminar processo mastermind, matar servidor
+--------------------------------------------------------------------------*/
void cmd_tmm (int argc, char** argv){
  coms_t cmd_msg = {.command = TMM};
  if(sendto(sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&to_d, tolen_d) < 0)
  {
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \n");
    return;       // volta para a "linha de comandos"
  }
  printf("[INFO] ABORT SENT\n");
}
/*-------------------------------------------------------------------------*/



/*++++++++++++++++++++++++++++++++_JMMlog_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_ltc - listar classificações
+--------------------------------------------------------------------------*/
void cmd_ltc (int argc, char** argv){
  log_single_tab_t msg_tab_recieved;
  msg_to_JMMlog msg_sent;

  struct sockaddr_un my_addr;
  struct sockaddr_un to;
  socklen_t tolen;

  if((argc == 2) && (atoi(argv[1]) > 0) && (atoi(argv[1]) <= 2)){
    to.sun_family = AF_UNIX;
    memset(to.sun_path, 0, sizeof(to.sun_path));
    strcpy(to.sun_path, JMMLOGSD);
    tolen = sizeof(my_addr.sun_family) + strlen(to.sun_path);

    strcpy(msg_sent.cmd, "ltc");

    if(atoi(argv[1]) == 0)
      msg_sent.arg_n = DIFF_ALL;
    else if(atoi(argv[1]) == 1)
      msg_sent.arg_n = DIFF_1;
    else if(atoi(argv[1]) == 2)
      msg_sent.arg_n = DIFF_2;
    else
      printf("[ERRO INESPERADO]");

    if (sendto(sd_datagram, &msg_sent, sizeof(msg_sent), 0, (struct sockaddr*)&to, tolen) < 0) {
      perror("teste: Erro no sendto");
    }else{
      switch (msg_sent.arg_n){
        
        case DIFF_ALL:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_1:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_2:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        default:
          printf("[ERRO] INESPERADO");
          return;

      }
    }
  }else{
    printf("[ERRO] Número inválido de argumentos. Tentar Novamente. \n");
    return;
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_rtc - reiniciar tabelas de classificação
+--------------------------------------------------------------------------*/
void cmd_rtc(int argc, char** argv){
  log_single_tab_t msg_tab_recieved;
  msg_to_JMMlog msg_sent;

  struct sockaddr_un my_addr;
  struct sockaddr_un to;
  socklen_t tolen;

  if((argc == 2) && (atoi(argv[1]) > 0) && (atoi(argv[1]) <= 2)){
    to.sun_family = AF_UNIX;
    memset(to.sun_path, 0, sizeof(to.sun_path));
    strcpy(to.sun_path, JMMLOGSD);
    tolen = sizeof(my_addr.sun_family) + strlen(to.sun_path);

    strcpy(msg_sent.cmd, "ltc");

    if(atoi(argv[1]) == 0)
      msg_sent.arg_n = DIFF_ALL;
    else if(atoi(argv[1]) == 1)
      msg_sent.arg_n = DIFF_1;
    else if(atoi(argv[1]) == 2)
      msg_sent.arg_n = DIFF_2;
    else
      printf("[ERRO INESPERADO]");

    if(sendto(sd_datagram, &msg_sent, sizeof(msg_sent), 0, (struct sockaddr*)&to, tolen) < 0) {
      perror("teste: Erro no sendto");
    }else{
      switch (msg_sent.arg_n){
        
        case DIFF_ALL:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_1:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_2:
          if(recvfrom(sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0)
            perror("teste: Erro no recvfrom");
          else
            printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        default:
          printf("[ERRO] INESPERADO");
          return;

      }
    }
  }else{
    printf("[ERRO] Número inválido de argumentos. Tentar Novamente. \n");
    return;
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_trh - terminar processo de registo histórico
+--------------------------------------------------------------------------*/
void cmd_trh (int argc, char** argv){
  printf("1");
}
/*-------------------------------------------------------------------------*/





/*+++++++++++++++++++++++++++previously_provided++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_test - função de teste: apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test (int argc, char** argv){
  int i;

  for (i = 0; i < argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}
/*-------------------------------------------------------------------------*/