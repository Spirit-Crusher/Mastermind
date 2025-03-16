#ifndef LOG_SERVER_H
#define LOG_SERVER_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>
#include <stdbool.h>


#include "../include/mastermind.h"  //! tornar o resto assim

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>


/****************** Defines *******************/
#define JMM_LOG_NCOMMANDS (sizeof(commands)/sizeof(struct command_d))
#define MSIZE sizeof(log_tabs_t)    

/****************** Structs *******************/
typedef struct {
    rjg_t tb1[TOPN];
    int tb1_n_games;
    rjg_t tb2[TOPN];
    int tb2_n_games;
} log_tabs_t;


/*************** Function Prototypes *****************/
void* queue_handler(void* pi);
void open_file(int* mfd_p, log_tabs_t** tabel_p);
void get_tab_n(log_single_tab_t* single_tab, int diff);
void del_tab_n(int diff);
void del_tab_n(int diff);

#endif