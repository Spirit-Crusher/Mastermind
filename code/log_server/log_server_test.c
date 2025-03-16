#include "log_server.h"

int main() {
    int mqids;
    rjg_t game_save = { .nd = DIFF_1, .nj = "T-1", .nt = 3, .tf = 90, .ti = 1 };

    if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0) {
        perror("Cliente: Erro a associar a queue servidor");
        exit(-1);
    }

    printf("Cliente vai enviar\n");
    if (mq_send(mqids, (char*)&game_save, sizeof(game_save), 0) < 0) {
        perror("Cliente: erro a enviar mensagem");
    }

}