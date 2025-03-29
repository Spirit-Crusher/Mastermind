#include "core_gameplay.h"

// Função que gera a chave secreta aleatoriamente, conforme nível
void generate_key(char *key, game_diff_t level) 
{
    char *letters = (level == DIFF_1) ? "ABCDE" : "ABCDEFGH";
    int length = (level == DIFF_1) ? 3 : 5;
    
    for (int i = 0; i < length; i++) {
        key[i] = letters[rand() % strlen(letters)];
    }
    key[length] = '\0';
}

