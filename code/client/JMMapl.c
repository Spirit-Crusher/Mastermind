#include "cliente.h"

extern void monitor();
extern void entrance();
extern DATAGRAM create_sock(void);
extern void cmd_sair (int, char**);

DATAGRAM datsock;

int main(void){
  entrance();
  signal(SIGTERM, cmd_sair);
  signal(SIGINT, cmd_sair);

  datsock = create_sock();

  monitor();

  return 0;
}