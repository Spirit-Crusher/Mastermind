#include "cliente.h"

void monitor(DATAGRAM);
DATAGRAM create_sock(void);

DATAGRAM datsock;

int main(void){
  datsock = create_sock();

  monitor(datsock);

  return 0;
}