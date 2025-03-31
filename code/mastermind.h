#pragma once

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <mqueue.h>
#include <signal.h> 
#include <ctype.h>

/****************** Defines *******************/
#define NJMAX 4                    /* número máximo de jogadores em simultâneo */
#define MAXNJ 10                  /* valor inicial do número máximo de jogadas (tentativas) */
#define MAXT 5                     /* valor inicial do tempo máximo de jogo (minutos) */
#define TOPN 10                    /* número de jogos a apresentar na tabela de classificação */
#define JMMLOG "../JOGOS.LOG"         /* ficheiro com registo histórico */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMLOGQ "/JMMLOGQ"         /* nome do registo histórico (queue) */
#define MAX_SEQUENCE_SIZE 8
#define NO_TIME_REGISTERED -1
#define GAME_ACCEPTED "{SERVER} [INFO] YOUR GAME WILL START SOON"
#define MOVE_REGISTERED "{SERVER} [INFO] YOUR MOVE HAS BEEN REGISTERED"
#define GAME_DENIED "{SERVER} [INFO] SERVER IS FULL. YOUR GAME HAS BEEN DENIED"
#define GAME_LOST_TIME "{SERVER} [INFO] GAME OVER. TIME RAN OUT"
#define GAME_LOST_TRIES "{SERVER} [INFO] GAME OVER. TOO MANY TRIES"
#define GAME_WON "{SERVER} [INFO] WINNER!"
#define GAME_CRASHED "{SERVER} [ERRO] YOUR GAME HAS CRASHED"
#define GAME_DISCONNECT "{SERVER} [ERRO] GAME DISCONNECTED"

/****************** Enums *******************/
typedef enum
{
  DIFF_ALL,
  DIFF_1,
  DIFF_2,
} game_diff_t;

typedef enum
{
  ONGOING,
  PLAYER_WIN,
  PLAYER_LOST_TIME,
  PLAYER_LOST_TRIES,
  DISCONNECT,
} game_state_t;

typedef enum
{
  CNJ,
  JG,
  CLM,
  MLM,
  CER,
  AER,
  DER,
  TMM,
  LTC,
  RTC,
  TRH,
} commands_t;

/****************** Structs *******************/
typedef struct
{               /* estrutura de um registo de jogo */
  int nd;     /* nível de dificuldade do jogo */
  char nj[4]; /* nome do jogador (3 caracteres) */
  int nt;     /* número de tentativas usadas */
  time_t ti;  /* estampilha temporal início do jogo */
  time_t tf;  /* estampilha temporal fim do jogo */
  
} rjg_t;

typedef struct
{
  int maxj; // número máximo de jogadas
  int maxt; // tempo máximo de jogo (em minutos) //! por em segundos??

} rules_t;

typedef struct // variável de estado do jogo
{
  char correct_sequence[MAX_SEQUENCE_SIZE]; // sequência correta definida no início do jogo
  rjg_t log;                                // log do jogo para ser enviado depois ser armazenado
  unsigned short int n_char;          // número de caracteres na sequência
  char player_move[MAX_SEQUENCE_SIZE];      // sequência enviada pelo utilizador
  unsigned short int np;                    // número de letras certas no sítio certo
  unsigned short int nb;                    // número de letras certas no sítio errado<
  game_state_t game_state; // estado do jogo = {ONGOING,PLAYER_WIN,PLAYER_LOST} //! isto é usado?

  rules_t game_rules; // regras do jogo //! novo

  double elapsed_time;

  int sd; //socket descriptor associado a este jogador
  struct sockaddr_un player_addr; //address do cliente
  socklen_t addr_len; //comprimento do address

} game_t;

typedef struct
{
  commands_t command;

  union
  {
    char name[4];
    char move[6];
    unsigned int j;
    unsigned short int n;
  } arg1;

  union
  {
    unsigned short int n;
    time_t t;
  } arg2;

} coms_t;

typedef struct
{
  int sd;
  int game_number;
  int sock_stream;
  coms_t buffer_s;

} new_game_info;

typedef struct 
{
  rjg_t tb[10];
  int tb_n_games;
  int tb_diff;

} log_single_tab_t;