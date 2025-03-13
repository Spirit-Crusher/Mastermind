#include "core_gameplay.h"
#include <stdio.h>

void analise_move_test(){
    printf("\n\nChanve\n");
    game_t game = {.correct_sequence = "CACB", .player_move = "BCCA", .n_char = 4, .nt_max = 3};
    analise_move(&game);
    putc(game.correct_sequence[0], stdout);
    putc(game.correct_sequence[1], stdout);
    putc(game.correct_sequence[2], stdout);
    putc(game.correct_sequence[3], stdout);
    printf("\nJogador\n");
    putc(game.player_move[0], stdout);
    putc(game.player_move[1], stdout);
    putc(game.player_move[2], stdout);
    putc(game.player_move[3], stdout);
    printf("\nNp = %i      Nb = %i\n", game.np, game.nb);
}

int main(){
    analise_move_test();
    return 0;
}
