#ifndef LOG_SERVER_H
#define LOG_SERVER_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>

#include <stdbool.h>
#include "../include/mastermind.h"  //! tornar o resto assim

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>


/****************** Defines *******************/
#define TOPN 10                    /* número de registos em cada tabela de nível */
#define MSIZE sizeof(log_tabs_t)    

/****************** Structs *******************/
typedef struct {
    rjg_t tb1[TOPN];
    unsigned int tb1_n_games;
    rjg_t tb2[TOPN];
    unsigned int tb2_n_games;
} log_tabs_t;



#endif