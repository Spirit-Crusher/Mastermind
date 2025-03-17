#include "log_server.h"

// Function to print a single rjg_t entry
void print_rjg(const rjg_t* game) {
    printf("  Difficulty: %d\n", game->nd);
    printf("  Player: %s\n", game->nj);
    printf("  Attempts: %d\n", game->nt);
    printf("  Start Time: %s", ctime(&game->ti));
    printf("  End Time: %s", ctime(&game->tf));
}

// Function to print log_tabs_t
void print_log_tabs(const log_tabs_t* log, int verbose) {
    printf("=== LOG TABS ===\n");

    // Print tb1 games
    printf("TB1 Games (Count: %d)\n", log->tb1_n_games);
    for (int i = 0; i < log->tb1_n_games && i < TOPN; i++) {
        printf("Game %d:\n", i);
        if (verbose) print_rjg(&log->tb1[i]);
    }

    // Print tb2 games
    printf("\nTB2 Games (Count: %d)\n", log->tb2_n_games);
    for (int i = 0; i < log->tb2_n_games && i < TOPN; i++) {
        printf("Game %d:\n", i);
        if (verbose) print_rjg(&log->tb2[i]);
    }

    printf("================\n");
}


/***************************** test_print_memory *******************************/
#define FILE "FICHEIRO.DAT"
void test_print_memory(int verbose) {

    int mfd;
    log_tabs_t* tabel;

    /* abrir / criar ficheiro */
    if ((mfd = open(FILE, O_RDWR | O_CREAT, 0666)) < 0) {
        perror("Erro a criar ficheiro");
        exit(-1);
    }
    else {
        /* definir tamanho do ficheiro */
        if (ftruncate(mfd, MSIZE) < 0) {
            perror("Erro no ftruncate");
            exit(-1);
        }
    }
    /* mapear ficheiro */
    if ((tabel = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0)) < (log_tabs_t*)0) {
        perror("Erro em mmap");
        exit(-1);
    }

    /* mostrar os jogos na memória */
    printf("test_print_memory: mostrar os jogos na memória\n");

    print_log_tabs(tabel, verbose);

    // fechar ficheiro
    munmap(tabel, MSIZE);
    close(mfd);
}

void queue_test_add_game(int verbose) {
    int mqids;
    rjg_t game_save = { .nd = DIFF_2, .nj = "T-2", .nt = 3, .tf = 90, .ti = 1 };

    if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0) {
        perror("Cliente: Erro a associar a queue servidor");
        exit(-1);
    }

    printf("Cliente vai enviar\n");
    if (mq_send(mqids, (char*)&game_save, sizeof(game_save), 0) < 0) {
        perror("Cliente: erro a enviar mensagem");
    }

    for (int i = 0; i < 100000000; i++);
    test_print_memory(verbose);
}



#define CLINAME "/tmp/CLI"
void test_datagram_ltc() {
    char cliname[20];

    int socket_d;
    struct sockaddr_un my_addr;
    socklen_t addrlen;
    struct sockaddr_un to;
    socklen_t tolen;

    //rjg_t game_save;
    log_single_tab_t msg_tab_recieved;;

    if ((socket_d = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("Erro a criar socket"); exit(-1);
    }

    my_addr.sun_family = AF_UNIX;
    memset(my_addr.sun_path, 0, sizeof(my_addr.sun_path));

    pid_t pid = getpid();
    snprintf(cliname, sizeof(cliname), "%s_%05d", CLINAME, pid);
    strcpy(my_addr.sun_path, cliname);
    addrlen = sizeof(my_addr.sun_family) + strlen(my_addr.sun_path);

    if (bind(socket_d, (struct sockaddr*)&my_addr, addrlen) < 0) {
        perror("Erro no bind"); exit(-1);
    }

    to.sun_family = AF_UNIX;
    memset(to.sun_path, 0, sizeof(to.sun_path));
    strcpy(to.sun_path, JMMLOGSD);
    tolen = sizeof(my_addr.sun_family) + strlen(to.sun_path);
    msg_to_JMMlog msg_sent = { .cmd = "ltc", .arg_n = DIFF_ALL }; ////////////////////! aqui
    if (sendto(socket_d, &msg_sent, sizeof(msg_sent), 0, (struct sockaddr*)&to, tolen) < 0) {
        perror("teste: Erro no sendto");
    }
    else {
        switch (msg_sent.arg_n)
        {
        case DIFF_ALL:
            if (recvfrom(socket_d, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0) perror("teste: Erro no recvfrom");
            else {
                printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
            }
            if (recvfrom(socket_d, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0) perror("teste: Erro no recvfrom");
            else {
                printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
            }
            break;
        default:
            if (recvfrom(socket_d, &msg_tab_recieved, sizeof(msg_tab_recieved), 0, (struct sockaddr*)&to, &tolen) < 0) perror("teste: Erro no recvfrom");
            else {
                printf("teste: Recebi: diff=%i n_games=%i\n", msg_tab_recieved.tb_diff, msg_tab_recieved.tb_n_games);
            }
            break;
        }

    }
    unlink("CLINAME");
}

void test_datagram_rtc() {
    char cliname[20];

    int socket_d;
    struct sockaddr_un my_addr;
    socklen_t addrlen;
    struct sockaddr_un to;
    socklen_t tolen;

    //rjg_t game_save;
    //log_single_tab_t msg_tab_recieved;;

    if ((socket_d = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("Erro a criar socket"); exit(-1);
    }

    my_addr.sun_family = AF_UNIX;
    memset(my_addr.sun_path, 0, sizeof(my_addr.sun_path));

    pid_t pid = getpid();
    snprintf(cliname, sizeof(cliname), "%s_%05d", CLINAME, pid);
    strcpy(my_addr.sun_path, cliname);
    addrlen = sizeof(my_addr.sun_family) + strlen(my_addr.sun_path);

    if (bind(socket_d, (struct sockaddr*)&my_addr, addrlen) < 0) {
        perror("Erro no bind"); exit(-1);
    }

    to.sun_family = AF_UNIX;
    memset(to.sun_path, 0, sizeof(to.sun_path));
    strcpy(to.sun_path, JMMLOGSD);
    tolen = sizeof(my_addr.sun_family) + strlen(to.sun_path);
    msg_to_JMMlog msg_sent = { .cmd = "rtc", .arg_n = DIFF_ALL }; ////////////////////! aqui
    if (sendto(socket_d, &msg_sent, sizeof(msg_sent), 0, (struct sockaddr*)&to, tolen) < 0) {
        perror("teste: Erro no sendto");
    }
    close(socket_d);
    unlink("CLINAME");
}

void test_datagram_trh() {
    char cliname[20];

    int socket_d;
    struct sockaddr_un my_addr;
    socklen_t addrlen;
    struct sockaddr_un to;
    socklen_t tolen;

    //rjg_t game_save;
    //log_single_tab_t msg_tab_recieved;;

    if ((socket_d = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("Erro a criar socket"); exit(-1);
    }

    my_addr.sun_family = AF_UNIX;
    memset(my_addr.sun_path, 0, sizeof(my_addr.sun_path));

    pid_t pid = getpid();
    snprintf(cliname, sizeof(cliname), "%s_%05d", CLINAME, pid);
    strcpy(my_addr.sun_path, cliname);
    addrlen = sizeof(my_addr.sun_family) + strlen(my_addr.sun_path);

    if (bind(socket_d, (struct sockaddr*)&my_addr, addrlen) < 0) {
        perror("Erro no bind"); exit(-1);
    }

    to.sun_family = AF_UNIX;
    memset(to.sun_path, 0, sizeof(to.sun_path));
    strcpy(to.sun_path, JMMLOGSD);
    tolen = sizeof(my_addr.sun_family) + strlen(to.sun_path);
    msg_to_JMMlog msg_sent = { .cmd = "trh" }; ////////////////////! aqui
    if (sendto(socket_d, &msg_sent, sizeof(msg_sent), 0, (struct sockaddr*)&to, tolen) < 0) {
        perror("teste: Erro no sendto");
    }
    close(socket_d);
    unlink("CLINAME");
}


int main() {
    //test_datagram_ltc();
    //test_datagram_rtc();
    //test_datagram_trh();
    int verbose = 0; queue_test_add_game(verbose);
    //int verbose = 1; test_print_memory(verbose);


    return 0;

}