/*---------------------------------includes--------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>


/*---------------------------------defines--------------------------------*/
#define DIMPLAY1 30
#define DIMPLAY2 30
#define MAX_LINE 50
#define ARGVECSIZE 3
#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define CLINAME "/tmp/CLI"
#define JMMLOGSD "/tmp/JMMLOGS"     /* nome do registo histórico (socket datagram) */
#define JMMSERVSD "/tmp/JMMSERVSD"  /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS"  /* nome do servidor de jogo (socket stream) */


/*---------------------------variávies_globais----------------------------*/
typedef struct {
  int sd_datagram;                  // file descriptor do socket datagrama
  socklen_t tolen_d;
  socklen_t addrlen_d;
  struct sockaddr_un to_d;
  struct sockaddr_un my_addr_d;
} DATAGRAM;

typedef struct {
  int sd_stream;                    // file descriptor do socket stream
  socklen_t addrlen_s;
  struct sockaddr_un srv_addr_s; 
} STREAM;


/*---------------------------protótipos_de_funções------------------------*/
void cmd_sos  (int, char**);
void cmd_sair (int, char**);
void cmd_test (int, char**);
void cmd_cnj  (int, char**);
void cmd_jg   (int, char**);
void cmd_clm  (int, char**);
void cmd_mlm  (int, char**);
void cmd_cer  (int, char**);
void cmd_aer  (int, char**);
void cmd_der  (int, char**);
void cmd_tmm  (int, char**);
void cmd_ltc  (int, char**);
void cmd_rtc  (int, char**);
void cmd_trh  (int, char**);