#include "core_gameplay.h"

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;

    // determinar np -> número de letras certas no sítio certo && nb -> número de letras certas no sítio errado
    for (i = 0; i < game_pt->n_char; i++)
    {
        // determinar np -> número de letras certas no sítio certo
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            np++;
            continue; // tenho uma letra da chave a corresponder a uma letra do jogador no sitio certo -> passar a próxima letra da chave
        }

        // nb -> número de letras certas no sítio errado
        for (j = i + 1; j < game_pt->n_char - 1; j++) // se a letra da chave não está na posição na resposta do jogador, verificar se essa letra existe nas posições seguintes (começar na posição a seguir e dar a volta)
        {
            if (game_pt->correct_sequence[i] == game_pt->player_move[j%game_pt->n_char])
            {
                nb++;
                continue;   // encontrei uma letra presente na chave e resposta mas em posições diferentes, passar a outra letra
                //!!! erro aqui -> se a chave for ABCD e a resposta for AAAA ele vai dar nb = 4 em vez de nb = 1 como deveria. Ele precisa de "memória" -> copiar?
            }
        }
    }
    if (np == game_pt->n_char)
    {
        return game_pt->game_state = PLAYER_WIN;
    }
}
