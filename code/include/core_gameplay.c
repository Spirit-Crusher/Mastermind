#include "core_gameplay.h"

//game_t game = {.correct_sequence = "AAAA", .player_move = "BCAA", .n_char = 4, .nt_left = 3};

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;
    // unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};

    // determinar np -> número de letras certas no sítio certo && nb -> número de letras certas no sítio errado
    for (i = 0; i < game_pt->n_char; i++)
    {
        // determinar np -> número de letras certas no sítio certo
        if ((game_pt->correct_sequence[i] == game_pt->player_move[i]) && (used_guess[i] == false))
        {
            np++;
            // used_secret[i] = true;
            used_guess[i] = true;
            continue; // tenho uma letra da chave a corresponder a uma letra do jogador no sitio certo -> passar a próxima letra da chave
        }

        // nb -> número de letras certas no sítio errado
        for (j = 1; j < game_pt->n_char; j++) // se a letra da chave não está na posição na resposta do jogador, verificar se essa letra existe nas posições seguintes (começar na posição a seguir e dar a volta)
        {
            if (used_guess[j + i] == true)
            {
                continue; // passar à proxima letra pois esta já foi contabilizada
            }

            if (game_pt->correct_sequence[i] == game_pt->player_move[(j + i) % game_pt->n_char])
            {
                nb++;
                used_guess[(j + i) % game_pt->n_char] = true;
                break; // encontrei uma letra presente na chave e resposta mas em posições diferentes, passar a outra letra da chave
            }
        }
    }

    game_pt->nb = nb;
    game_pt->np = np;

    if (np == game_pt->n_char)
    {
        return game_pt->game_state = PLAYER_WIN;
    }
}
