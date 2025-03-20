#include "mastermind.h"


// por este linha na main::
//! -> srand(time(NULL));

// Função que gera a chave secreta aleatoriamente, conforme nível
void generate_key(char *key, int level) {
    char *letters = (level == 1) ? "ABCDE" : "ABCDEFGH";
    int length = (level == DIFF_1) ? 3 : 5;
    for (int i = 0; i < length; i++) {
        key[i] = letters[rand() % strlen(letters)];
    }
    key[length] = '\0';
    // DEBUG: Descomente a linha abaixo para ver a chave gerada
    // printf("[DEBUG] Chave gerada: %s\n", key);
}