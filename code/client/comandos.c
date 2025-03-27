#include "cliente.h"

/*---------------------------variávies_globais----------------------------*/
unsigned short int dif;
STREAM strmsock;
extern DATAGRAM datsock;
/*------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj(int argc, char** argv){
  char buf_s[50];
  coms_t coms_msg;                                                            // struct para realizar o envio dos comandos para o servidor
  
  if(argc != 3){                                                              // comando + nome + dificuldade
    printf("[ERRO] Número de argumentos inválido. Tentar novamente.\n");
    return;                                                                   // volta para a "linha de comandos"
  }

  if(strlen(argv[1]) != 3){                                                   // nome com 3 chars
    printf("[ERRO] Nome inválido! Introduzir no máximo 3 caracteres.\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    dif = atoi(argv[2]);
    if((dif == 1) || (dif == 2)){
      if((strmsock.sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ){        // tenta criar socket stream
        perror("[ERRO] Criação de socket stream. Tentar novamente. \a\n"); 
        return;                                                               // volta para a "linha de comandos"
      }

      strmsock.srv_addr_s.sun_family = AF_UNIX;
      memset(strmsock.srv_addr_s.sun_path, 0, sizeof(strmsock.srv_addr_s.sun_path));
      strcpy(strmsock.srv_addr_s.sun_path, JMMSERVSS);      
      strmsock.addrlen_s = sizeof(strmsock.srv_addr_s.sun_family) + strlen(strmsock.srv_addr_s.sun_path);
    
      if(connect(strmsock.sd_stream, (struct sockaddr *)&strmsock.srv_addr_s, strmsock.addrlen_s) < 0){
        perror("[ERRO] Connect. Tentar novamente. \a\n"); 
        return;     // volta para a "linha de comandos"
      }
      
      coms_msg.command = CNJ; coms_msg.arg2.n = atoi(argv[2]);
      strcpy(coms_msg.arg1.name, argv[1]);
      
      if((write(strmsock.sd_stream, &coms_msg, sizeof(coms_msg)) < 0)){
        perror("[ERRO] Write para o servidor. Tentar novamente. \a\n");
        close(strmsock.sd_stream);
        return;     // volta para a "linha de comandos"
      }else{
        if(!(read(strmsock.sd_stream, buf_s, sizeof(buf_s)) < 0)){                // colocar aqui timeout algures
          if(strcmp(buf_s, GAME_DENIED) == 0){   
            printf("%s", GAME_DENIED);
            close(strmsock.sd_stream);                                            // fechar socket stream
            printf("[INFO] Pedido de jogo recusado. Tentar novamente mais tarde. \n");  
          }else{
            printf("[INFO] Cliente recebeu confirmação: %s\n", buf_s);
          }
        }
      }
    }else{
      printf("[ERRO] Nível de dificuldade inválido! Inserir dificuldade: 1 ou 2.\n");
      return;
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_jg - fazer jogada
+--------------------------------------------------------------------------*/
void cmd_jg(int argc, char** argv){
  unsigned short int aux;
  char xua;
  char val_play[DSENDPLAY];
  char rcv_play[DRCVPLAY];
  coms_t cmd_msg;

  if(strmsock.sd_stream > 0){
    if(dif == 1){
      if(strlen(argv[1]) != 3){
        printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE})");
        return;
      }

      strcpy(val_play, argv[1]);
      for(aux = 0; aux < strlen(val_play); aux++){
        xua = val_play[aux];

        if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E'))){
          printf("[ERRO] Jogada inválida! Introduzir 3 letras (de {ABCDE})");
          return;
        }
      }

      cmd_msg.command = JG; strcpy(cmd_msg.arg1.move, argv[1]);               // atribui dados a enviar, na estrutura de dados

      if((write(strmsock.sd_stream, &cmd_msg, sizeof(cmd_msg)) < 0)){
        perror("[ERRO] Envio para o servidor. Tentar novamente. \a\n");
        return;
      }else{
        while(read(strmsock.sd_stream, rcv_play, sizeof(rcv_play)) < 0);      // adicionar timeout aqui algures

        if(strcmp(rcv_play, GAME_WON) != 0){
          printf("[INFO] Jogada: %s", rcv_play);                              // jogada
        }else{
          printf("[INFO] Parabéns: %s \n", GAME_WON);                         // jogador vence
          close(strmsock.sd_stream);                                          // fecha o seu socket                    
        }
      }
    }else if(dif == 2){
      if(strlen(argv[1]) != 5){
        printf("[ERRO] Jogada inválida! Introduzir 5 letras (de {ABCDEFGH})");
        return;
      }

      strcpy(val_play, argv[1]);
      for(aux = 0; aux < strlen(val_play); aux++){
        xua = val_play[aux];
        if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E') || (xua == 'F') || (xua == 'G') || (xua == 'H'))){
          printf("[ERRO] Jogada inválida! Introduzir 5 letras (de {ABCDEFGH})");
          return;
        }
      }

      cmd_msg.command = JG; strcpy(cmd_msg.arg1.move, argv[1]);

      if((write(strmsock.sd_stream, &cmd_msg, sizeof(cmd_msg)) < 0)){
        perror("[ERRO] Envio para o servidor. Tentar novamente. \a\n");
        return;
      }else{
        while(read(strmsock.sd_stream, rcv_play, sizeof(rcv_play)) < 0);      // adicionar timeout aqui algures
        if(strcmp(rcv_play, GAME_WON) != 0){                                  // verifica se o jogador venceu ou não
          printf("[INFO] Jogada: %s", rcv_play);                              // jogada
        }else{
          printf("[INFO] Parabéns: %s \n", rcv_play);                         // jogador vence
          close(strmsock.sd_stream);                                          // fecha o seu socket
          printf("[INFO] Jogador desconectado do seu socket stream. 'cnj' para criar novo jogo. \n");       
        }
      }
    }else{
      printf("[ERRO] Jogo não inicializado. Tentar novamente. \a\n");
      return;
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm(int argc, char** argv){
  char requested_info[50];
  coms_t cmd_msg;

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  cmd_msg.command = CLM;

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(datsock.sd_datagram, requested_info, sizeof(requested_info), 0, (struct sockaddr *)&datsock.to_d, &datsock.tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
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
void cmd_mlm(int argc, char** argv){
  char mlm_msg[50];
  coms_t cmd_msg;

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  if(argc == 3){
    if((atoi(argv[1]) > 0) && ((atoi(argv[2])) > 0)){
      cmd_msg.command = MLM; cmd_msg.arg1.j = atoi(argv[1]); cmd_msg.arg2.t = atoi(argv[2]);
      
      if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
        printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");             // envio falhou
        return;                                                                         // volta para a "linha de comandos"
      }else{                                                                 
        if(recvfrom(datsock.sd_datagram, mlm_msg, sizeof(mlm_msg), 0, (struct sockaddr *)&datsock.to_d, &datsock.tolen_d) < 0){
          printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");     // receção falhou
          return;                                                                       // volta para a "linha de comandos"                                            
        }else{
          printf("[INFO] %s\n", mlm_msg);
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
void cmd_cer(int argc, char** argv){
  coms_t cmd_msg;
  char cer_msg[60];

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path); 

  cmd_msg.command = CER;

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(datsock.sd_datagram, cer_msg, sizeof(cer_msg), 0, (struct sockaddr *)&datsock.to_d, &datsock.tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("%s", cer_msg);
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_aer - activar envio
+--------------------------------------------------------------------------*/
void cmd_aer(int argc, char** argv){
  coms_t cmd_msg;
  char aer_msg[60];

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path); 

  cmd_msg.command = AER;

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(datsock.sd_datagram, aer_msg, sizeof(aer_msg), 0, (struct sockaddr *)&datsock.to_d, &datsock.tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("%s", aer_msg);
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_der - desactivar envio
+--------------------------------------------------------------------------*/
void cmd_der(int argc, char** argv){
  coms_t cmd_msg;
  char der_msg[60];

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path); 

  cmd_msg.command = DER;

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }else{
    if(recvfrom(datsock.sd_datagram, der_msg, sizeof(der_msg), 0, (struct sockaddr *)&datsock.to_d, &datsock.tolen_d) < 0){
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
      return;     // volta para a "linha de comandos"                                            
    }else{
      printf("%s", der_msg);
    }
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_tmm - terminar processo mastermind, matar servidor
+--------------------------------------------------------------------------*/
void cmd_tmm(int argc, char** argv){
  coms_t cmd_msg = {.command = TMM};

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }
  printf("[INFO] Abortar: Terminar servidor de jogo. \n");
}
/*-------------------------------------------------------------------------*/



/*++++++++++++++++++++++++++++++++_JMMlog_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_ltc - listar classificações
+--------------------------------------------------------------------------*/
void cmd_ltc(int argc, char** argv){
  log_single_tab_t msg_tab_recieved;

  if((argc == 2) && (atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2)){
    coms_t cmd_msg = {.command = LTC, .arg1.n = atoi(argv[1])};       // comando =LTC e n=nível=2º argumento

    // definir o destinatário como o JMMlogsd
    datsock.to_d.sun_family = AF_UNIX;
    memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
    strcpy(datsock.to_d.sun_path, JMMLOGSD);
    datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

    if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
      perror("[ERRO] Envio de pedido para o servidor. Tentar novamente. \a\n");
    }else{
      switch(cmd_msg.arg1.n){
        
        case DIFF_ALL:
          if(recvfrom(datsock.sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d) < 0)
            perror("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            printf("[INFO] Dados recebidos: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          
          if(recvfrom(datsock.sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d) < 0)
            perror("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            printf("[INFO] Dados recebidos: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_1:
          if(recvfrom(datsock.sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d) < 0)
            perror("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            printf("[INFO] Dados recebidos: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        case DIFF_2:
          if(recvfrom(datsock.sd_datagram, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d) < 0)
            perror("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            printf("[INFO] Dados recebidos: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
          break;

        default:
          printf("[ERRO] INESPERADO \a\n");
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
  if((argc == 2) && (atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2)){
    coms_t cmd_msg = {.command = RTC, .arg1.n = atoi(argv[1])};

    // definir o destinatário como o JMMlogsd
    datsock.to_d.sun_family = AF_UNIX;
    memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
    strcpy(datsock.to_d.sun_path, JMMLOGSD);
    datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

    if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
      perror("[ERRO] Envio de pedido para o servidor. Tentar novamente. \a\n");
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
void cmd_trh(int argc, char** argv){
  coms_t cmd_msg = {.command = TRH};

  // definir o destinatário como o JMMlogsd
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMLOGSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr *)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;       // volta para a "linha de comandos"
  }
  printf("[INFO] Abortar: Terminar servidor de registo histórico. \n");
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair(int argc, char **argv){        // POSSO FAZER close(..) ou unlink(..) A COISAS QUE NÃO EXISTEM? OU SEJA SD AINDA NÃO TINHA SIDO CRIADO P.E.
  printf("\n[INFO] A sair... \n");
  close(strmsock.sd_stream);
  unlink(datsock.my_addr_d.sun_path);
  close(datsock.sd_datagram);
  printf("[INFO] Saída realizada com sucesso. להתראות!\n");
  
  exit(0);
}
/*-------------------------------------------------------------------------*/




/*+++++++++++++++++++++++++++previously_provided++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_test - função de teste: apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test(int argc, char** argv){
  int i;

  for (i = 0; i < argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}
/*-------------------------------------------------------------------------*/