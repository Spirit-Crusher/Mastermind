#include "cliente.h"

extern void monitor();
extern void entrance();
extern void term_handler(int sig);
extern DATAGRAM create_sock(void);
extern void cmd_sair (int, char**);

DATAGRAM datsock;                      // contém os dados do socket datagrama

int main(void){
  entrance();                          // dar print ao monitor de entrada do cliente
  signal(SIGPIPE, SIG_IGN);            // comentando esta linha, o programa crasha quando jogador tenta fazer write com socket fechado
  signal(SIGTERM, term_handler);       // comando kill
  signal(SIGINT, term_handler);        // Ctrl+C

  datsock = create_sock();             // criar socket datagrama

  monitor();                           // aceitar comandos de jogo

  return 0;
}
