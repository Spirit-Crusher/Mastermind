#ifndef MASTERMIND_H
#define MASTERMIND_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>

/****************** Defines *******************/
#define NJMAX 4                    /* número máximo de jogadores em simultâneo */
#define MAXNJ 10                  /* valor inicial do número máximo de jogadas (tentativas) */
#define MAXT 5                     /* valor inicial do tempo máximo de jogo (minutos) */
#define JMMLOG "JOGOS.LOG"         /* ficheiro com registo histórico */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMLOGQ "/JMMLOGQ"         /* nome do registo histórico (queue) */


/****************** Structs *******************/
typedef struct
{               /* estrutura de um registo de jogo */
    int nd;     /* nível de dificuldade do jogo */
    char nj[4]; /* nome do jogador (3 caracteres) */
    int nt;     /* número de tentativas usadas */
    time_t ti;  /* estampilha temporal início do jogo */
    time_t tf;  /* estampilha temporal fim do jogo */
} rjg_t;




#endif