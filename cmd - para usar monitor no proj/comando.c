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

#define DIMPLAY1 3
#define DIMPLAY2 5
#define CLINAME "/tmp/CLI"
#define SERVERNAME "/tmp/GAME_SERVER"

int sd_stream;
int sd_datagram;
int dif;

/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair (int argc, char **argv){
  // falta fechar os sockets aqui
  exit(0);
}


/*-------------------------------------------------------------------------+
| Function: cmd_test - apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test (int argc, char** argv){
  int i;

  for (i = 0; i < argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}


/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj (int argc, char** argv){
  char mypid[6];
  char buf_s[50];
  char buf_d[50];
  char new_game[9];
  
  struct sockaddr_un srv_addr_s;          // stream
  socklen_t addrlen_s;

  struct sockaddr_un my_addr_d;          // datagram <- p/ não confundir
  socklen_t addrlen_d;
  struct sockaddr_un to_d;
  socklen_t tolen_d;
  
  
  if(argc != 3){    // comando + nome + dificuldade
    printf("Número de argumentos inválido\n");
    return;
  }
  if(strlen(argv[1]) != 3){     // nome com 3 chars
    printf("\n\nNome de jogador inválido! Introduzir 3 caracteres.\n\n");
    return;
  }else{
    dif = atoi(argv[2]);
    if((dif == 1) || (dif == 2)){     // dificuldade: 1 ou 2
    /*-----------------------criar-socket-stream------------------------*/
      if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){  // tenta criar socket
        perror("Erro a criar socket. Tentar novamente"); 
        return;     // retorna para a "linha de comandos"
      }

      if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){
        perror("Erro a criar socket. Tentar novamente:\n"); 
        return;
      }

      srv_addr_s.sun_family = AF_UNIX;
      memset(srv_addr_s.sun_path, 0, sizeof(srv_addr_s.sun_path));
      strcpy(srv_addr_s.sun_path, SERVERNAME);      
      addrlen_s = sizeof(srv_addr_s.sun_family) + strlen(srv_addr_s.sun_path);
    
      if(connect(sd_stream, (struct sockaddr *)&srv_addr_s, addrlen_s) < 0){
        perror("Erro no connect. Tentar novamente"); 
        return;     // retorna para a "linha de comandos"
      }
      
      strcpy(new_game, strcat(strcat(strcat(argv[0], " "), strcat(argv[1], " ")), argv[2]));
      if((write(sd_stream, new_game, strlen(new_game) + 1) < 0)){
        perror("Erro no write para o servidor. Tentar novamente:\n");
      }else{
        while(read(sd_stream, buf_s, sizeof(buf_s)) < 0);
        printf("cliente recebeu confirmação: %s\n", buf_s);      
      }

    /*----------------------criar-socket-datagrama----------------------*/
      if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ){     // tenta criar um socket datagrama
        perror("Erro a criar socket datagrama. Tentar Novamente:\n");
        close(sd_stream);
        return;
      }
        
      my_addr_d.sun_family = AF_UNIX;
      memset(my_addr_d.sun_path, 0, sizeof(my_addr_d.sun_path));
      strcpy(my_addr_d.sun_path, CLINAME);
      sprintf(mypid, "%d", getpid());
      strcat(my_addr_d.sun_path, mypid);
      addrlen_d = sizeof(my_addr_d.sun_family) + strlen(my_addr_d.sun_path);
      
      if(bind(sd_datagram, (struct sockaddr *)&my_addr_d, addrlen_d) < 0 ){          // 
        perror("Erro no bind do socket datagrama. Tentar novamente:\n");
        close(sd_stream); 
        return;
      }
  
      to_d.sun_family = AF_UNIX;
      memset(to_d.sun_path, 0, sizeof(to_d.sun_path));
      strcpy(to_d.sun_path, SERVERNAME);
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
      printf("Nível de dificuldade (%d) inválido! Inserir dificuldade: 1 ou 2.\n\n", dif);
      return;
    }
  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_jg - jogada
+--------------------------------------------------------------------------*/
void cmd_jg (int argc, char** argv){
  int aux;
  char xua;
  char dif1_play[DIMPLAY1];
  char dif2_play[DIMPLAY2];

  if(dif == 1){
    if(strlen(argv[1]) != 3){
      printf("Repetir jogada! Introduzir 3 letras (de {ABCDE})");
      return;
    }

    strcpy(dif1_play, argv[1]);
    for(aux = 0; aux < DIMPLAY1; aux++){
      xua = dif1_play[aux];
      if((xua != 'A') || (xua != 'B') || (xua != 'C') || (xua != 'D') || (xua != 'E')){
        printf("Repetir jogada! Introduzir 3 letras (de {ABCDE})");
        return;
      }
    }

    if((write(sd_stream, argv[1], strlen(argv[1]) + 1) < 0)){
      perror("Erro no write para o servidor. Tentar novamente:\n");
      return;
    }else{
      while(read(sd_stream, dif1_play, sizeof(dif1_play)) < 0);
      printf("jogada: %s\n", dif1_play);      
    }

  }
  if(dif == 2){
    if(strlen(argv[1]) != 5){
      printf("Repetir jogada! Introduzir 3 letras (de {ABCDE})");
      return;
    }

    strcpy(dif2_play, argv[1]);
    for(aux = 0; aux < DIMPLAY1; aux++){
      xua = dif2_play[aux];
      if((xua != 'A') || (xua != 'B') || (xua != 'C') || (xua != 'D') || (xua != 'E') || (xua != 'F') || (xua != 'G') || (xua != 'H')){
        printf("Repetir jogada! Introduzir 5 letras (de {ABCDEFGH})");
        return;
      }
    }

    if((write(sd_stream, argv[1], strlen(argv[1]) + 1) < 0)){
      perror("Erro no write para o servidor. Tentar novamente:\n");
      return;
    }else{
      while(read(sd_stream, dif2_play, sizeof(dif2_play)) < 0);
      printf("jogada: %s\n", dif2_play);      
    }

  }
}


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm (int argc, char** argv){
  // faz pedido por datagrama ao servidor dos limites
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_mlm - mudar limites
+--------------------------------------------------------------------------*/
void cmd_mlm (int, char**){
  printf("1");
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