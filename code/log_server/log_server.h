#ifndef LOG_SERVER_H
#define LOG_SERVER_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>
#include <stdbool.h>


#include "../mastermind.h"  //! tornar o resto assim

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>


/****************** Defines *******************/
#define FILE "FICHEIRO.DAT"
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

/* queue_handler
* @details: responsável pela queue de datos provenientes do JMMserv
*/
void* queue_handler();

/*! open_file
* @return: mfd_p -> ponteiro para o file descritor do ficheiro de dados
* @return: tabel_p -> ponteiro para o ponteiro da estrutura que contém os dados presentes no ficheiro de dados
* @details: abre o ficheiro de dados e projeta este numa estrutura que é devolvida através do argumento tabel_p
*/
//void open_file(int* mfd_p, log_tabs_t** tabel_p);
void open_file();


//TODO -> descrição.. Função para inserir um rjg_t em tb1 ou tb2 ordenado por duração do jogo
void insert_sorted_n(log_tabs_t *log, rjg_t new_game, game_diff_t diff);

/* get_tab_n
* @return: single_tab -> estrutura com uma tabela de dificuldades inteira
* @arg: diff -> dificuldade da estrutura supracitada
* @details: devolve a tabela de jogos da dificuldade especifica em diff
* ps: cada dificuldade tem de ser extraida individualmente, ou seja, DIFF_ALL não é válido
*/
void get_tab_n(log_single_tab_t* single_tab, int diff);

/* del_tab_n
* @arg: diff -> dificuldade da tabela a apagar (diff= DIF_ALL -> todas)
* @details: faz reset das tabelas de dificuldade
*/
void del_tab_n(int diff);

#endif