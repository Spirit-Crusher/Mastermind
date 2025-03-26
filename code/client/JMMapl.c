#include "cliente.h"

void monitor(DATAGRAM);
DATAGRAM create_sock(void);
extern void cmd_sair (int, char**);

DATAGRAM datsock;

int main(void){
  signal(SIGTERM, cmd_sair);
  signal(SIGINT, cmd_sair);

  datsock = create_sock();

  monitor(datsock);

  return 0;
}