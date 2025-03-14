#ifndef MASTERMIND_H
#define MASTERMIND_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

/****************** Defines *******************/
#define NJMAX 4                    /* número máximo de jogadores em simultâneo */
#define TOPN 10                    /* número de registos em cada tabela de nível */
#define MAXNJ 10                  /* valor inicial do número máximo de jogadas (tentativas) */
#define MAXT 5                     /* valor inicial do tempo máximo de jogo (minutos) */
#define JMMLOG "JOGOS.LOG"         /* ficheiro com registo histórico */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMLOGQ "/JMMLOGQ"         /* nome do registo histórico (queue) */


/****************** Structs *******************/





#endif