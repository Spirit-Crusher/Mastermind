#include "core_gameplay.h"
#include <stdio.h>

int main() {
    
    printf("\n\nChave\n");
    game_t game = {.correct_sequence = "CCCC", .player_move = "CCCA", .n_char = 4, .nt_max = 3};
    analise_move(&game);
    putc(game.player_move[0], stdout);
    putc(game.player_move[1], stdout);
    putc(game.player_move[2], stdout);
    putc(game.player_move[3], stdout);
    printf("\nJogador\n");
    putc(game.correct_sequence[0], stdout);
    putc(game.correct_sequence[1], stdout);
    putc(game.correct_sequence[2], stdout);
    putc(game.correct_sequence[3], stdout);
    printf("\nNp = %i      Nb = %i\n", game.np, game.nb);
    return 0;
}