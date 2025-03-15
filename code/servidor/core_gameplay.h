#ifndef CORE_GAMEPLAY_H
#define CORE_GAMEPLAY_H

/****************** Includes *******************/
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "../include/mastermind.h"  //! tornar o resto assim

/****************** Defines *******************/
#define MAX_SEQUENCE_SIZE 8

/****************** Enums *******************/
typedef enum
{
    ONGOING,
    PLAYER_WIN,
    PLAYER_LOST,
} game_state_t;

/****************** Structs *******************/

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


/***************************  Fuctions ***************************/
game_state_t analise_move(game_t *game_pt);

#endif