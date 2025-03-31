#include "cliente.h"

extern void monitor();
extern void entrance();
extern void term_handler(int sig);
extern DATAGRAM create_sock(void);
extern void cmd_sair (int, char**);

DATAGRAM datsock;

int main(void){
  entrance();
  signal(SIGTERM, term_handler);
  signal(SIGINT, term_handler);

  datsock = create_sock();

  monitor();

  return 0;
}