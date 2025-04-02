/*---------------------------------includes--------------------------------*/
#include "../mastermind.h"
/*------------------------------------------------------------------------*/

/*---------------------------------defines--------------------------------*/
#define ARGVECSIZE   3
#define DSENDPLAY    6
#define MAX_LINE     15
#define DRCVPLAY     70
#define MAX_RCV_SIZE 60
#define CLINAME      "/tmp/CLI"
#define NCOMMANDS    (sizeof(commands)/sizeof(struct command_d))
/*------------------------------------------------------------------------*/


/*-------------------------------variávies--------------------------------*/
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
/*------------------------------------------------------------------------*/


/*---------------------------protótipos_de_funções------------------------*/
// funções para a "linha de comandos"
void cmd_sos  (int, char**);
void cmd_rgr  (int, char**);
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

// outras
void print_log_tabs(log_single_tab_t * log_tab);
/*------------------------------------------------------------------------*/


// fonte: como remover unused variables:      https://cs61.seas.harvard.edu/site/2019/Patterns/