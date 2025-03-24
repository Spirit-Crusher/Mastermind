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

#define DIMPLAY1 30
#define DIMPLAY2 30
#define CLINAME "/tmp/CLI"
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */

typedef struct {
    int sd_datagram;                  // file descriptor do socket datagrama
    socklen_t tolen_d;
    socklen_t addrlen_d;
    struct sockaddr_un to_d;
    struct sockaddr_un my_addr_d;
  } DATAGRAM;
