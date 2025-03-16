#include "core_gameplay.h"


/*void run_the_game()
{

}*/

//inicialização do jogo
game_t create_new_game(char* player_name, int dificulty)
{
    game_t game = { .correct_sequence = "AAAA", 
                    .log.nd = dificulty, 
                    /*.log.nj = , para isto faço strcpy*/ 
                    .log.nt = 0, 
                    .log.ti = time(NULL), 
                    .log.tf = NO_TIME_REGISTERED, //inicializei com alguma coisa para conseguir perceber se der problemas
                    .n_char = 4, 
                    .nt_max = 3, 
                    .player_move = "BCAA", 
                    .np = 0, 
                    .nb = 0, 
                    .game_state = ONGOING,
                };
    
    strcpy(&game.log.nj, player_name); //inicializa o nome do jogador

    return game;
}

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};

    // incrementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (int i = 0; i < game_pt->n_char; ++i)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            (np)++;
            used_secret[i] = true;
            used_guess[i] = true;
        }
    }

    // procurar por letras certas no sítio errado
    for (int i = 0; i < game_pt->n_char; ++i)
    {
        if (!used_guess[i])
        { // se esta posição não teve uma ligação direta
            for (int j = 0; j < game_pt->n_char; j++)
            {
                if (!used_secret[j] && game_pt->correct_sequence[i] == game_pt->player_move[j])
                {
                    nb++;
                    used_secret[j] = true;
                    break;
                }
            }
        }
    }

    // guardar nº de respostas certas no sítio certo/errado
    game_pt->nb = nb;
    game_pt->np = np;

    // decrementar o nº de jogadas que faltam

    // verificar se o jogo acabou
    if (np == game_pt->n_char)
    {
        // jogador ganhou
        return game_pt->game_state = PLAYER_WIN;
    }
    else if (game_pt->nt_max == ++(game_pt->log.nt))
    {
        // jogador sem mais tentativas, perdeu
        return game_pt->game_state = PLAYER_LOST;
    }

    // jogo continua
    return ONGOING;
}