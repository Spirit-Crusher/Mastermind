#ifndef CORE_GAMEPLAY_H
#define CORE_GAMEPLAY_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

/****************** Defines *******************/
#define MAX_SEQUENCE_SIZE 8

/****************** Structs *******************/
typedef enum
{
    ONGOING,
    PLAYER_WIN,
    PLAYER_LOST,
} game_state_t;

typedef struct
{               /* estrutura de um registo de jogo */
    int nd;     /* nível de dificuldade do jogo */
    char nj[4]; /* nome do jogador (3 caracteres) */
    int nt;     /* número de tentativas usadas */
    time_t ti;  /* estampilha temporal início do jogo */
    time_t tf;  /* estampilha temporal fim do jogo */
} rjg_t;

typedef struct // variável de estado do jogo
{
    char correct_sequence[MAX_SEQUENCE_SIZE]; // sequência correta definida no início do jogo
    rjg_t log;                                // log do jogo para ser enviado depois ser armazenado
    const unsigned short int n_char;          // número de caracteres na sequência
    char player_move[MAX_SEQUENCE_SIZE];      // sequência enviada pelo utilizador
    unsigned short int np;                    // número de letras certas no sítio certo
    unsigned short int nb;                    // número de letras certas no sítio errado
    unsigned short int nt_left;               // número de tentativas que restam ao jogador //! adicionei este
    game_state_t game_state;                  // estado do jogo = {ONGOING,PLAYER_WIN,PLAYER_LOST}
} game_t;

#endif