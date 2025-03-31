/*--------------------------------includes--------------------------------*/
#include "cliente.h"
/*------------------------------------------------------------------------*/


/*---------------------------variávies_globais----------------------------*/
unsigned short int dif;
STREAM strmsock;
extern DATAGRAM datsock;
/*------------------------------------------------------------------------*/



/*++++++++++++++++++++++++++++++++_JMMserv_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_cnj - criar novo jogo
+--------------------------------------------------------------------------*/
void cmd_cnj(int argc, char** argv){
  char buf_s[MAX_RCV_SIZE];                                                   // mensagem a receber
  coms_t coms_msg;                                                            // comandos a enviar 
  short int bytes;                                                            // variável auxiliar para timeouts

  if(argc != 3){                                                              // comando + nome + dificuldade
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;                                                                   // volta para a "linha de comandos"
  }

  if(strlen(argv[1]) != 3){                                                   // nome com 3 chars
    printf("[ERRO] Nome inválido! Introduzir no máximo 3 caracteres.\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    dif = atoi(argv[2]);
    if((dif == 1) || (dif == 2)){
      if((strmsock.sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){         // tenta criar socket stream
        perror("[ERRO] Criação de socket stream. Tentar novamente. \a\n");
        return;                                                               // volta para a "linha de comandos"
      }

      strmsock.srv_addr_s.sun_family = AF_UNIX;
      memset(strmsock.srv_addr_s.sun_path, 0, sizeof(strmsock.srv_addr_s.sun_path));
      strcpy(strmsock.srv_addr_s.sun_path, JMMSERVSS);
      strmsock.addrlen_s = sizeof(strmsock.srv_addr_s.sun_family) + strlen(strmsock.srv_addr_s.sun_path);

      if(connect(strmsock.sd_stream, (struct sockaddr*)&strmsock.srv_addr_s, strmsock.addrlen_s) < 0){
        perror("[ERRO] Connect. Tentar novamente. \a\n");
        return;                                                               // volta para a "linha de comandos"
      }

      coms_msg.command = CNJ; coms_msg.arg2.n = atoi(argv[2]);                // comando para o JMMserv
      strcpy(coms_msg.arg1.name, argv[1]);                                    // definir nome do jogador

      struct timeval timeout;                                                 // estrutura para definir timeout
      timeout.tv_sec = 3;                                                     // definir tempo timeout: 3seg
      timeout.tv_usec = 0;                                                    // definir tempo timeout: 0useg
      
      setsockopt(strmsock.sd_stream, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));     // definir timeout

      if((write(strmsock.sd_stream, &coms_msg, sizeof(coms_msg)) < 0)){       // faz write (envio) para o JMMserv
        perror("[ERRO] Write para o servidor. Tentar novamente. \a\n");       // erro se falhar
        close(strmsock.sd_stream);                                            // fecha o socket, criar novo jogada again
        return;                                                               // volta para a "linha de comandos"
      }else{                                                                  // enviou
        bytes = read(strmsock.sd_stream, buf_s, sizeof(buf_s)); 
        if(bytes == EWOULDBLOCK){                                             // tempo expirou
          printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
        }else if(bytes <= 0 && errno != EWOULDBLOCK){                         // leitura falhou
          printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
        }else{
          if(strcmp(buf_s, GAME_DENIED) == 0){
            printf("%s", GAME_DENIED);                                        // limite máximo de jogadores
            close(strmsock.sd_stream);                                        // fechar socket stream
            printf("[INFO] Pedido de jogo recusado. Tentar novamente mais tarde. \n");
          }else{
            printf("%s\n", buf_s);                                            // jogo aceite pelo JMMserv
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
  short int bytes;                                                            // variável auxiliar para timeouts
  unsigned short int aux;
  char xua;
  char val_play[DSENDPLAY];                                                   // jogada a enviar
  char rcv_play[DRCVPLAY];                                                    // jogada a receber
  coms_t cmd_msg;                                                             // comandos a enviar 

  if((strmsock.sd_stream > 0) && (argc == 2)){                                // se socket criado e #argumentos correto
    struct timeval timeout;                                                   // estrutura para definir timeout
    timeout.tv_sec = 3;                                                       // definir tempo timeout: 3seg
    timeout.tv_usec = 0;                                                      // definir tempo timeout: 0useg
    
    setsockopt(strmsock.sd_stream, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));       // definir timeout

    if(dif == 1){                                                             // dificuldade for igual a 1
      if(strlen(argv[1]) != 3){
        printf("[ERRO] Repetir jogada! Introduzir 3 letras (de {ABCDE}) \n");
        return;
      }

      if(!(strmsock.sd_stream > 0)){                                          // socket permanece aberto
        printf("[ERRO]  Jogo não inicializado. Tempo pode ter terminado... Tentar novamente. \n"); 
        return;
      }

      strcpy(val_play, argv[1]);
      for(aux = 0; aux < strlen(val_play); aux++){                            // verificar se jogada tem só cars. válidos
        xua = val_play[aux];
        if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E'))){
          printf("[ERRO] Jogada inválida! Introduzir 3 letras (de {ABCDE}) \n");
          return;
        }
      }
    }else if(dif == 2){                                                       // dificuldade igual a 2
      if(strlen(argv[1]) != 5){
        printf("[ERRO] Jogada inválida! Introduzir 5 letras (de {ABCDEFGH})\n");
        return;
      }

      if(!(strmsock.sd_stream > 0)){                                          // socket permanece aberto
        printf("[ERRO] Jogo não inicializado. Tempo pode ter terminado... Tentar novamente. \n"); 
        return;
      }

      strcpy(val_play, argv[1]);
      for(aux = 0; aux < strlen(val_play); aux++){                            // verificar se jogada tem só cars. válidos
        xua = val_play[aux];
        if(!((xua == 'A') || (xua == 'B') || (xua == 'C') || (xua == 'D') || (xua == 'E') || (xua == 'F') || (xua == 'G') || (xua == 'H'))){
          printf("[ERRO] Jogada inválida! Introduzir 5 letras (de {ABCDEFGH})\n");
          return;
        }
      }
    }else{                                                                    // algum erro estranho
      printf("[ERRO] Jogo não inicializado. Tentar novamente. \a\n");
      return;
    }

    cmd_msg.command = JG; strcpy(cmd_msg.arg1.move, argv[1]);               // jogada a enviar ao JMMserv

    if((write(strmsock.sd_stream, &cmd_msg, sizeof(cmd_msg)) <= 0)){         // tentar enviar
      perror("[ERRO] Envio para o servidor. Tentar novamente. \a\n");
      return;
    }else{                                                                  // enviou
      bytes = read(strmsock.sd_stream, rcv_play, sizeof(rcv_play));         // adicionar timeout aqui algures
      if((bytes <= 0) && (errno == EWOULDBLOCK)){                           // tempo expirou
        printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
      }else if(bytes <= 0 && errno != EWOULDBLOCK){                         // leitura falhou
        printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
      }else{
        if(strcmp(rcv_play, GAME_WON) != 0){                                // verifica se o jogador venceu ou não
          printf("%s", rcv_play);                                           // jogada
        }else{
          printf("[INFO] Parabéns: %s \n", rcv_play);                       // jogador vence
          close(strmsock.sd_stream);                                        // fecha o seu socket
          printf("[INFO] Jogador desconectado do seu socket stream. 'cnj' para criar novo jogo. \n");
        }
      }
    }
  }else{                                                                      // argumentos errados ou socket fechado somehow
    printf("[ERRO] Jogada não processada. Tentar novamente. \n");
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_clm - consultar limites
+--------------------------------------------------------------------------*/
void cmd_clm(int argc, char** argv){
  coms_t cmd_msg;                                                             // comandos a enviar 
  short int bytes;                                                            // variável auxiliar para timeouts
  char requested_info[MAX_RCV_SIZE];                                          // mensagem a receber
  
  if(argc != 1){                                                              // validar #argumentos
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda. \n");
    return;
  }

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  cmd_msg.command = CLM;                                                      // comando para o JMMserv

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));        // definir timeout

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    bytes = recvfrom(datsock.sd_datagram, requested_info, sizeof(requested_info), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if((bytes == EWOULDBLOCK))                                                // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                                           
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");                                                  
    else                                                                      // recebeu a informação do JMMserv
      printf("[INFO] Informação recebida: %s", requested_info);               // mostra mensagem do JMMserv
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_mlm - mudar limites
+--------------------------------------------------------------------------*/
void cmd_mlm(int argc, char** argv){
  coms_t cmd_msg;                                                             // comandos a enviar 
  short int bytes;                                                            // variável auxiliar para timeouts
  char mlm_msg[MAX_RCV_SIZE];                                                 // mensagem a receber

  if(argc == 3){                                                              // #argumentos correto
    // definir o destinatário como o JMMserv
    datsock.to_d.sun_family = AF_UNIX;
    memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
    strcpy(datsock.to_d.sun_path, JMMSERVSD);
    datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

    if((atoi(argv[1]) > 0) && ((atoi(argv[2])) > 0)){                         // #jogadas e tempo válidos
      cmd_msg.command = MLM; cmd_msg.arg1.j = atoi(argv[1]); cmd_msg.arg2.t = atoi(argv[2]);  // comando para o JMMserv

      struct timeval timeout;                                                 // estrutura para definir timeout
      timeout.tv_sec = 3;                                                     // definir tempo timeout: 3seg
      timeout.tv_usec = 0;                                                    // definir tempo timeout: 0useg
      
      setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));    // definir timeout

      if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
        printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");                 // envio falhou
        return;                                                                               // volta para a "linha de comandos"
      }else{
        bytes = recvfrom(datsock.sd_datagram, mlm_msg, sizeof(mlm_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
        if(bytes == EWOULDBLOCK)                                              // tempo expirou
          printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                                         
        else if(bytes <= 0 && errno != EWOULDBLOCK)                           // leitura falhou
          printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
        else
          printf("%s", mlm_msg);                                              // mostra mensagem do JMMserv
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
| Function: cmd_cer - consultar estado de envio para o JMMlog
+--------------------------------------------------------------------------*/
void cmd_cer(int argc, char** argv){
  char cer_msg[MAX_RCV_SIZE];                                                 // mensagem a receber
  coms_t cmd_msg;                                                             // comandos a enviar 
  short int bytes;                                                            // variável auxiliar para timeouts

  if(argc != 1){                                                              // #argumentos inválidos
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  cmd_msg.command = CER;                                                      // comando para o JMMserv

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));        // definir timeout

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    bytes = recvfrom(datsock.sd_datagram, cer_msg, sizeof(cer_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if(bytes == EWOULDBLOCK)                                                  // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
    else
      printf("%s", cer_msg);                                                  // mostra mensagem do JMMserv
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_aer - activar envio para o JMMlog
+--------------------------------------------------------------------------*/
void cmd_aer(int argc, char** argv){
  coms_t cmd_msg;                                                             // comandos a enviar   
  short int bytes;                                                            // variável auxiliar para timeouts
  char aer_msg[MAX_RCV_SIZE];                                                 // mensagem a receber

  if(argc != 1){                                                              // #argumentos inválido
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  cmd_msg.command = AER;                                                      // comando para o JMMserv

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    bytes = recvfrom(datsock.sd_datagram, aer_msg, sizeof(aer_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if(bytes == EWOULDBLOCK)                                                  // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                                 
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
    else
      printf("%s", aer_msg);                                                  // mostra mensagem do JMMserv
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_der - desactivar envio para o JMMlog
+--------------------------------------------------------------------------*/
void cmd_der(int argc, char** argv){
  coms_t cmd_msg;                                                             // comandos a enviar 
  char der_msg[MAX_RCV_SIZE];                                                 // mensagem a receber
  short int bytes;                                                            // variável auxiliar para timeouts

  if(argc != 1){                                                              // #argumentos inválido
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  cmd_msg.command = DER;                                                      // comando para o JMMserv

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    bytes = recvfrom(datsock.sd_datagram, der_msg, sizeof(der_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if(bytes == EWOULDBLOCK)                                                  // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                                    
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n"); 
    else
      printf("%s", der_msg);                                                  // mostra mensagem do JMMserv
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_tmm - terminar processo mastermind, matar servidor
+--------------------------------------------------------------------------*/
void cmd_tmm(int argc, char** argv){
  char tmm_msg[MAX_RCV_SIZE];                                                 // mensagem a receber
  coms_t cmd_msg = { .command = TMM };                                        // comando para o JMMserv
  short int bytes;                                                            // variável auxiliar para timeouts

  if(argc != 1){                                                              // #argumentos inválido
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  // definir o destinatário como o JMMserv
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMSERVSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
    return;                                                                   // volta para a "linha de comandos"
  }else{
    bytes = recvfrom(datsock.sd_datagram, tmm_msg, sizeof(tmm_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if(bytes == EWOULDBLOCK)                                                  // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                              
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
    else
      printf("%s", tmm_msg);                                                  // mostra mensagem do JMMlog
  }
  printf("[INFO] Abortar: Terminar servidor de jogo. \n");
}
/*-------------------------------------------------------------------------*/



/*++++++++++++++++++++++++++++++++_JMMlog_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_ltc - listar classificações
+--------------------------------------------------------------------------*/
void cmd_ltc(int argc, char** argv) {
  short int bytes1;                                                           // variável auxiliar1 para timeouts
  short int bytes2;                                                           // variável auxiliar2 para timeouts
  log_single_tab_t msg_tab_recieved1;                                         // tabelas dif1
  log_single_tab_t msg_tab_recieved2;                                         // tabelas dif2


  if((argc == 2) && (atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2)){
    coms_t cmd_msg = { .command = LTC, .arg1.n = atoi(argv[1]) };             // comando para o JMMlog

    // definir o destinatário como o JMMlogsd
    datsock.to_d.sun_family = AF_UNIX;
    memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
    strcpy(datsock.to_d.sun_path, JMMLOGSD);
    datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

    struct timeval timeout;                                                   // estrutura para definir timeout
    timeout.tv_sec = 3;                                                       // definir tempo timeout: 3seg
    timeout.tv_usec = 0;                                                      // definir tempo timeout: 0useg
    
    setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
      perror("[ERRO] Envio de pedido para o servidor. Tentar novamente. \a\n");
    }else{
      switch(cmd_msg.arg1.n){                                                 // dificuldades
        case DIFF_ALL:                                                        // tabelas dificulades 1 e 2
          bytes1 = recvfrom(datsock.sd_datagram, &msg_tab_recieved1, sizeof(msg_tab_recieved1), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
          if(bytes1 == EWOULDBLOCK){                                          // tempo expirou
            printf("[ERRO] Tempo Expirou. Tentar novamente. \a\n");
          }else if(bytes1 <= 0 && errno != EWOULDBLOCK){                      // leitura falhou
            printf("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          }else{
            bytes2 = recvfrom(datsock.sd_datagram, &msg_tab_recieved2, sizeof(msg_tab_recieved2), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
            if(bytes2 == EWOULDBLOCK){                                        // tempo expirou
              printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
              break;
            }else if(bytes2 <= 0 && errno != EWOULDBLOCK){                    // leitura falhou
              printf("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");             
            }else{
              print_log_tabs(&msg_tab_recieved1);                             // mostra mensagem do JMMlog
              print_log_tabs(&msg_tab_recieved2);                             // mostra mensagem do JMMlog
            }
          }
          break;

        case DIFF_1:                                                          // tabelas dificuldade 1
          bytes1 = recvfrom(datsock.sd_datagram, &msg_tab_recieved1, sizeof(msg_tab_recieved1), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
          if(bytes1 == EWOULDBLOCK)                                           // tempo expirou
            printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
          else if(bytes1 <= 0 && errno != EWOULDBLOCK)                        // leitura falhou
            printf("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            print_log_tabs(&msg_tab_recieved1);                               // mostra mensagem do JMMlog
          break;

        case DIFF_2:                                                          // tabelas dificuldade 2
          bytes2 = recvfrom(datsock.sd_datagram, &msg_tab_recieved2, sizeof(msg_tab_recieved2), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
          if(bytes2 == EWOULDBLOCK)                                           // tempo expirou
            printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");
          else if(bytes2 <= 0 && errno != EWOULDBLOCK)                        // leitura falhou
            perror("[ERRO] Receção de dados do servidor. Tentar novamente. \a\n");
          else
            print_log_tabs(&msg_tab_recieved2);                               // mostra mensagem do JMMlog
          break;

        default:
          printf("[ERRO] INESPERADO \a\n");
          return;

      }
    }
  }else{
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_rtc - reiniciar tabelas de classificação
+--------------------------------------------------------------------------*/
void cmd_rtc(int argc, char** argv){
  char rtc_msg[MAX_RCV_SIZE];                                                 // mensagem a receber
  short int bytes;                                                            // variável auxiliar para timeouts

  if((argc == 2) && (atoi(argv[1]) >= 0) && (atoi(argv[1]) <= 2)){            // dificuldade e #argumentos válidos
    coms_t cmd_msg = { .command = RTC, .arg1.n = atoi(argv[1]) };             // comando para o JMMlog

    // definir o destinatário como o JMMlogsd
    datsock.to_d.sun_family = AF_UNIX;
    memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
    strcpy(datsock.to_d.sun_path, JMMLOGSD);
    datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

    struct timeval timeout;                                                   // estrutura para definir timeout
    timeout.tv_sec = 3;                                                       // definir tempo timeout: 3seg
    timeout.tv_usec = 0;                                                      // definir tempo timeout: 0useg
    
    setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
      perror("[ERRO] Envio de pedido para o servidor. Tentar novamente. \a\n");
    }else{                                                                    // envio bem-sucedido
      bytes = recvfrom(datsock.sd_datagram, rtc_msg, sizeof(rtc_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
      if(bytes == EWOULDBLOCK)                                                // tempo expirou
        printf("[ERRO] Tempo expirou. Tentar novamente");                                 
      else if(bytes <= 0 && errno != EWOULDBLOCK)                             // leitura falhou
        printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n");
      else
        printf("%s", rtc_msg);                                                // mostra mensagem do JMMlog
    }
  }else{                                                                      // #argumentos inválido
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_trh - terminar processo de registo histórico
+--------------------------------------------------------------------------*/
void cmd_trh(int argc, char** argv){
  char trh_msg[MAX_RCV_SIZE];                                                 // mensagem a receber
  coms_t cmd_msg = { .command = TRH};                                         // comando para o JMMlog
  short int bytes;                                                            // variável auxiliar para timeouts

  if(argc != 1){                                                              // #argumentos inválido
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  // definir o destinatário como o JMMlogsd
  datsock.to_d.sun_family = AF_UNIX;
  memset(datsock.to_d.sun_path, 0, sizeof(datsock.to_d.sun_path));
  strcpy(datsock.to_d.sun_path, JMMLOGSD);
  datsock.tolen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.to_d.sun_path);

  struct timeval timeout;                                                     // estrutura para definir timeout
  timeout.tv_sec = 3;                                                         // definir tempo timeout: 3seg
  timeout.tv_usec = 0;                                                        // definir tempo timeout: 0useg
  
  setsockopt(datsock.sd_datagram, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));        // definir timeout

  if(sendto(datsock.sd_datagram, &cmd_msg, sizeof(cmd_msg), 0, (struct sockaddr*)&datsock.to_d, datsock.tolen_d) < 0){
    printf("[ERRO] Envio de pedido ao servidor. Tentar novamente. \a\n");
  }else{                                                                      // envio bem-sucedido
    bytes = recvfrom(datsock.sd_datagram, trh_msg, sizeof(trh_msg), 0, (struct sockaddr*)&datsock.to_d, &datsock.tolen_d);
    if(bytes == EWOULDBLOCK)                                                  // tempo expirou
      printf("[ERRO] Tempo expirou. Tentar novamente. \a\n");                              
    else if(bytes <= 0 && errno != EWOULDBLOCK)                               // leitura falhou
      printf("[ERRO] Receção de informação do servidor. Tentar novamente. \a\n"); 
    else
      printf("%s", trh_msg);                                                  // JMMlog manda um 'ACK', cliente mostra
  }
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: print_log_tabs - função para imprimir as tabelas de classificação
+--------------------------------------------------------------------------*/
void print_log_tabs(log_single_tab_t* log_tab){
  printf("=== LOG TABELA %d  (nº jogos: %d)===\n", log_tab->tb_diff, log_tab->tb_n_games);    // formatação

  // mostrar cada registo de histórico de jogo
  for(int i = 0; i < log_tab->tb_n_games && i < TOPN; i++){
    printf("Game %d:\n", i);
    printf("  Difficulty: %d\n", log_tab->tb[i].nd);
    printf("  Player: %s\n", log_tab->tb[i].nj);
    printf("  Attempts: %d\n", log_tab->tb[i].nt);
    printf("  Start Time: %s", ctime(&log_tab->tb[i].ti));
    printf("  End Time: %s", ctime(&log_tab->tb[i].tf));
  }
  printf("================\n");
}
/*-------------------------------------------------------------------------*/



/*++++++++++++++++++++++++++++++++_JMMapl_++++++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/
void cmd_sair(int argc, char** argv){
  printf("\n[INFO] A sair... \n");                                            // avisa o cliente de que está a sair
  close(strmsock.sd_stream);                                                  // fecha socket stream
  unlink(datsock.my_addr_d.sun_path);                                         // faz unlink do socket
  close(datsock.sd_datagram);                                                 // fecha socket datagram
  printf("[INFO] Saída realizada com sucesso. להתראות!\n");                  // bye-bye

  exit(0);                                                                    // termina processo do JMMapl
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: term_handler - sinais de término
+--------------------------------------------------------------------------*/
void term_handler(int sig){
  printf("\n[INFO] A sair... \n");                                            // avisa o cliente de que está a sair
  close(strmsock.sd_stream);                                                  // fecha socket stream
  unlink(datsock.my_addr_d.sun_path);                                         // faz unlink do socket
  close(datsock.sd_datagram);                                                 // fecha socket datagram
  printf("[INFO] Saída realizada com sucesso. להתראות!\n");                  // bye-bye

  exit(0);                                                                    // termina processo do JMMapl
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------+
| Function: cmd_rgr - listar regras do jogo
+--------------------------------------------------------------------------*/
void cmd_rgr(int argc, char** argv){
  if(argc != 1){
    printf("[ERRO] Número de argumentos inválido. Escrever 'sos' para consultar ajuda.\n");
    return;
  }

  printf("\n______________________________________REGRAS_DO_JOGO:_MASTERMIND!______________________________________ \n");
  printf("- Com o início do jogo, uma chave aleatória será gerada; \n");
  printf("- Essa chave pode ser de 3 letras de entre {ABCDE}, para a dificuldade 1; \n");
  printf("- Ou ter 5 letras de entre {ABCDEFGH}, para a dificuldade 2; \n");
  printf("- O jogador deve tentar adivinhar, qual a chave secreta; \n");
  printf("- A cada jogada, o servidor responde com duas grandes; \n");
  printf("- 'np' = número de pinos pretos: número de letras na posição correta da chave, \n");
  printf("- 'nb' = número de pinos brancas: número de letras certas no sítio errado; \n");
  printf("- Os restantes comandos de jogo e interação poderão ser consultados ao introduzir 'sos' ou 'help' no cmd>  \n");
  printf("______________________________________________BOA_SORTE!!!______________________________________________ \n");
}



/*+++++++++++++++++++++++++++previously_provided++++++++++++++++++++++++++*/

/*-------------------------------------------------------------------------+
| Function: cmd_test - função de teste: apenas como exemplo
+--------------------------------------------------------------------------*/
void cmd_test(int argc, char** argv){
  int i;

  for (i = 0; i < argc; i++)
    printf("\nargv[%d] = %s", i, argv[i]);
}
/*-------------------------------------------------------------------------*/