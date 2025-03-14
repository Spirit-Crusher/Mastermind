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

#define SERVNAME "/tmp/GAME_SERVER"


/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair (int argc, char **argv){
  exit(0);
}


/*-------------------------------------------------------------------------+
| Function: cmd_test - apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test (int argc, char** argv){
  int i;

  /* exemplo -- escreve argumentos */
  for (i = 0; i < argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}


/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj (int argc, char** argv){
  char cli_nickn[3];
  
  if(argc != 3){
    printf("Número de argumentos inválido");
    return;
  }
  if(strlen(argv[1]) != 3){
    printf("\n\nNome de jogador inválido! Introduzir 3 caracteres.\n\n");
    return;
  }else{
    if(strlen(argv[2]) != 1){
      printf("Nível de dificuldade inválido! Inserir dificuldade: 1 ou 2.\n\n\n\n");
      return;
    }

    int dif = atoi(argv[2]);
    if((dif == 1) || (dif == 2)){
        
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
  printf("1");
}


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm (int argc, char** argv){
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