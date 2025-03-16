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

/****************** Defines *******************/
#define NJMAX 4                    /* número máximo de jogadores em simultâneo */
#define MAXNJ 10                  /* valor inicial do número máximo de jogadas (tentativas) */
#define MAXT 5                     /* valor inicial do tempo máximo de jogo (minutos) */
#define TOPN 10                    /* número de registos em cada tabela de nível */
#define JMMLOG "JOGOS.LOG"         /* ficheiro com registo histórico */
#define JMMSERVSD "/tmp/JMMSERVSD" /* nome do servidor de jogo (socket datagram) */
#define JMMSERVSS "/tmp/JMMSERVSS" /* nome do servidor de jogo (socket stream) */
#define JMMLOGSD "/tmp/JMMLOGS"    /* nome do registo histórico (socket datagram) */
#define JMMLOGQ "/JMMLOGQ"         /* nome do registo histórico (queue) */
#define MAX_SEQUENCE_SIZE 8
#define NO_TIME_REGISTERED -1

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
    PLAYER_LOST,
} game_state_t;


/****************** Structs *******************/
typedef struct
{               /* estrutura de um registo de jogo */
    int nd;     /* nível de dificuldade do jogo */
    char nj[4]; /* nome do jogador (3 carateres) */
    int nt;     /* número de tentativas usadas */
    time_t ti;  /* estampilha temporal início do jogo */
    time_t tf;  /* estampilha temporal fim do jogo */
} rjg_t;

typedef struct // variável de estado do jogo
{
    char correct_sequence[MAX_SEQUENCE_SIZE]; // sequência correta definida no início do jogo
    rjg_t log;                                // log do jogo para ser enviado depois ser armazenado
    const unsigned short int n_char;          // número de caracteres na sequência
    unsigned short int nt_max;                // número de tentativas total do jogador
    char player_move[MAX_SEQUENCE_SIZE];      // sequência enviada pelo utilizador
    unsigned short int np;                    // número de letras certas no sítio certo
    unsigned short int nb;                    // número de letras certas no sítio errado
    // unsigned short int nt_left;               // número de tentativas que restam ao jogador //! adicionei este
    game_state_t game_state; // estado do jogo = {ONGOING,PLAYER_WIN,PLAYER_LOST}
} game_t;

typedef struct {
    rjg_t tb[TOPN];
    int tb_n_games;
    int tb_diff;
} log_single_tab_t;

typedef struct {
    char cmd[5];             //tamanho máximo dos comandos é 4
    int arg_n;                 // valor do argumento "n"
} msg_to_JMMlog;




/***************************  Fuctions ***************************/
game_state_t analise_move(game_t *game_pt);