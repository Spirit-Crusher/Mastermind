#include "servidor.h"

coms_t buffer_dgram; //posso usar só 1 buffer com mutex I think mas acho que ia tornar tudo mais lento sem necessidade e o prof não especifica
pthread_t thread_acceptgames; //thread que aceita novos jogos
pthread_t thread_gameinstance[NJMAX]; //threads responsáveis por gerir os vários jogos //!acho que isto pode ser local ao acceptgames
game_t* game_instances[NJMAX] = { 0 }; //apontadores para jogos são guardados para poderem ser alterados
struct sockaddr_un client_addr; //endereço do cliente
socklen_t client_addrlen; //não será igual ao server_addrlen? (acho que posso eliminar esta variável e passar o server_addrlen para addrlen)
rules_t global_game_rules = { .maxj = MAXNJ, .maxt = MAXT * 60 }; //criação e iniciaização das regras do jogo
bool ledger_on = true; //usado para verificar se registo de jogos está ou não ativo
char log_server[] = "../log_server/JMMlog"; //caminho relativo para log_server
int sd_datagram; //socket descriptor do servidor para datagrama
int sd_stream; //socket descriptor do servidor para stream


/*####################################################### exit_handler ######################################################*/

void exit_handler()
{
    printf("[INFO] Exiting...\n");

    close(sd_datagram);
    close(sd_stream);

    unlink(JMMSERVSD);
    unlink(JMMSERVSS);

    for (int i = 0; i < NJMAX; ++i)
    {
        if (game_instances[i] != NULL)
        {
            close(game_instances[i]->sd);  //!ALGUM DESTES DÁ SEGFAULT
            free(game_instances[i]);
            game_instances[i] = NULL;
        }
    }

    exit(0);
}


/*####################################################### analise_move ######################################################*/

void analise_move(game_t* game_pt)
{
    unsigned short int np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = { false };
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = { false };
    int i;

    if ((game_pt->elapsed_time = difftime(time(NULL), game_pt->log.ti)) >= game_pt->game_rules.maxt)
    {
        // jogador sem mais tempo, perdeu
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_LOST_TIME;
        printf("[INFO] O jogador perdeu por falta de TEMPO\n");
        return;
    }

    // incrementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (i = 0; i < game_pt->n_char; ++i)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            np++;
            used_secret[i] = true;
            used_guess[i] = true;
        }
    }

    // procurar por letras certas no sítio errado
    for (i = 0; i < game_pt->n_char; ++i)
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
    
    //DEBUG 
    printf("[INFO] Time = %ld. Inicial = %ld. Máx = %d\n", time(NULL), game_pt->log.ti, game_pt->game_rules.maxt);
    printf("[INFO] Diff time: %lf\n",difftime(time(NULL), game_pt->log.ti));

    // verificar se o jogo acabou
    if (np == game_pt->n_char)
    {
        // jogador ganhou
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_WIN;
    }
    else if (game_pt->game_rules.maxj <= game_pt->log.nt)
    {
        // jogador sem mais tentativas, perdeu
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_LOST_TRIES;
        printf("[INFO] O jogador perdeu por falta de TENTATIVAS\n");
    }
    else
    {   // jogo continua
        game_pt->game_state = ONGOING;
    }

    return;
}


/*####################################################### create_new_game ######################################################*/

void create_new_game(game_t* game, int socket, coms_t buffer_stream)
{
    game->log.nd = buffer_stream.arg2.n; //dificuldade
    strcpy(game->log.nj, buffer_stream.arg1.name); //nome do jogador
    game->log.nt = 0;
    game->log.ti = time(NULL);
    game->log.tf = NO_TIME_REGISTERED;
    game->np = 0;
    game->nb = 0;
    game->game_state = ONGOING;
    game->sd = socket;

    //inicializar regras do jogo
    game->game_rules.maxj = global_game_rules.maxj;
    game->game_rules.maxt = global_game_rules.maxt;

    printf("\ndif: %d\n\n", buffer_stream.arg2.n);
    printf("\nname: %s\n\n", buffer_stream.arg1.name);

    //gerar key com dimensão e letras diferentes conforme nivel de dificuldade
    if (buffer_stream.arg2.n == DIFF_1)
    {
        game->n_char = 3;
        char key[game->n_char + 1];
        generate_key(key, DIFF_1);
        printf("[INFO] Key:%s\n", key);
        if (strcpy(game->correct_sequence, key) != NULL) printf("[INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else if (buffer_stream.arg2.n == DIFF_2)
    {
        game->n_char = 5;
        char key[game->n_char + 1];
        generate_key(key, DIFF_2);
        printf("[INFO] Key: %s\n", key);
        if (strcpy(game->correct_sequence, key) != NULL) printf("[INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else
    {
        printf("[ERRO] Problemas a gerar a key\n");
        exit(-1);
    }

    printf("[INFO] Init do jogo funcionou\n");

    return;
}


/*####################################################### stream_handler ######################################################*/

void stream_handler(int socket, game_t* game, coms_t buffer_stream)
{
    if (buffer_stream.command == JG)
    {
        //analisar jogada
        printf("[INFO] SERVER_STREAM: O jogador com descriptor %d deseja fazer a seguinte jogada: %s\n", socket, buffer_stream.arg1.move);
        strcpy(game->player_move, buffer_stream.arg1.move);
        printf("Move: %s    Correct move: %s\n", game->player_move, game->correct_sequence);
        analise_move(game);
        printf("[INFO] Jogada analisada com sucesso\n");
        printf("Gamestate = %d\n", game->game_state);
    }
    else
    {
        printf("[ERRO] Ops... Isto não era suposto acontecer.\n");
        exit(-1);
    }

    return;
}


/*####################################################### datagram_handler ######################################################*/

void datagram_handler(int sd, struct sockaddr_un client_addr, socklen_t client_addrlen)
{
    char buffer_send[100]; //buffer para mensagem de resposta

    switch (buffer_dgram.command)
    {
        case CLM:
            //enviar regras globais atuais
            sprintf(buffer_send, "%d:%d", global_game_rules.maxj, global_game_rules.maxt);
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }
            break;        
            
        case MLM:
            //alterar as regras
            global_game_rules.maxj = buffer_dgram.arg1.j;
            global_game_rules.maxt = buffer_dgram.arg2.t;

            printf("[AVISO] Regras globais alteradas: jmax=%d tmax=%d\n", global_game_rules.maxj, global_game_rules.maxt);

            sprintf(buffer_send, "Game rules changed.\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }
            break;

        case CER:
            //consultar se registo está ou não ativo
            if (ledger_on)
            {
                sprintf(buffer_send, "[INFO] Registo está ativo\n");
            }
            else sprintf(buffer_send, "[INFO] Registo está inativo\n");

            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }    
            break;

        case AER:
            //ativar registo
            ledger_on = true;
            sprintf(buffer_send, "[INFO] Registo ativado com sucesso\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }    
            break;

        case DER:
            //desativar registo
            ledger_on = false;
            sprintf(buffer_send, "[INFO] Registo desativado com sucesso\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }
            break;

        case TMM:
            //ordem para sair
            exit_handler();
            break;

        default:
            break;
    }
}


/*####################################################### save_game ######################################################*/

void save_game(rjg_t log) 
{
    int mqids;

    if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0) 
    {
        printf("[INFO] Vou fazer o system\n");
        perror("[ERRO] Erro a associar a queue servidor\n");
        system(log_server); //se falhou começar log_server
        sleep(1);
        printf("[INFO] Servidor iniciou log_server\n");

        if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0)
        {
            perror("[ERRO] Servidor não conseguiu abrir queue do log\n\a");
        }

    }

    printf("[INFO] A enviar jogo ao log_server, MQIDS:%d\n", mqids);
    if (mq_send(mqids, (char*)&log, sizeof(log), 0) < 0) 
    {
        perror("[ERRO] Erro a enviar mensagem\n");
    }
}


/*####################################################### thread_func_gameinstance ######################################################*/

void* thread_func_gameinstance(void* game_info)
{
    new_game_info info = (*(new_game_info*)game_info); //TODO: why not just use the struct directly? instead of creating new variables
    coms_t buffer_stream = info.buffer_s;
    int game_number = info.game_number;
    int socket = info.sd;
    //int stream = info.sock_stream;
    char result[100]; //!melhorar isto

    printf("[INFO] Esta é a socket e o game number do novo jogador: %d:%d\n", socket, game_number);

    game_t* current_game = NULL;
    //criar estrutura do novo jogo
    current_game = malloc(sizeof(game_t));
    if (current_game == NULL)
    {
        perror("[ERRO] Falha a alocar estrutura do jogo");
        exit(-1);
    }

    //guardar novo jogo no vetor de jogos
    game_instances[game_number] = current_game;

    //inicializar jogo
    create_new_game(current_game, socket, buffer_stream);
    printf("[INFO] Jogo nº%d foi iniciado\n", game_number);

    do
    {
        //ler jogadas do jogador e processar as mesmas enquanto o jogo estiver a decorrer
        if (read(socket, &buffer_stream, sizeof(buffer_stream)) <= 0)
        {
            perror("[AVISO] Cliente desconectou-se");
            close(socket);
            free(current_game);
            game_instances[game_number] = NULL;
            return NULL;
        }

        printf("STATE1 = %d\n", current_game->game_state);
        stream_handler(socket, current_game, buffer_stream); //processar request
        printf("STATE2 = %d\n", current_game->game_state);

        switch (current_game->game_state)
        {
            case ONGOING:
                //mostrar resultado da jogada
                sprintf(result, "nb:%d np:%d time_left:%.0f \n", current_game->nb, current_game->np, current_game->game_rules.maxt - current_game->elapsed_time);
                printf("Resultado da jogada: %s\n", result);
                if (write(socket, result, strlen(result) + 1) < 0)
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;

            case PLAYER_LOST_TIME:
                //informar jogador da derrota
                printf("[INFO] O jogador perdeu(tempo)\n");
                if (write(socket, GAME_LOST_TIME, strlen(GAME_LOST_TIME) + 1) < 0)
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;

            case PLAYER_LOST_TRIES:
                //informar jogador da derrota
                printf("[INFO] O jogador perdeu(tentativas)\n");
                if (write(socket, GAME_LOST_TRIES, strlen(GAME_LOST_TRIES) + 1) < 0)
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;

            case PLAYER_WIN:
                //informar jogador da vitória
                if (write(socket, GAME_WON, strlen(GAME_WON) + 1) < 0)
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                
                printf("[INFO] O jogador ganhou\n");
                if (ledger_on) {
                    //enviar log para o cliente
                    save_game(current_game->log);
                    printf("[INFO] Ledger ligado. Jogo guardado\n");
                }
                else printf("[INFO] Ledger desligado\n");
                break;

            default:
                if (write(socket, GAME_CRASHED, strlen(GAME_CRASHED) + 1) < 0)
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;
        }

    } while (current_game->game_state == ONGOING);

    //terminar jogo
    close(socket);
    free(current_game);
    game_instances[game_number] = NULL;
    printf("[INFO] GAME N%d TERMINATED\n", game_number);

    return NULL;
}

/*####################################################### thread_func_acceptgames ######################################################*/

void* thread_func_acceptgames()
{
    coms_t buffer_stream;
    int new_sock; //client specific socket descriptor
    struct sockaddr_un streamsv_addr;
    socklen_t streamsv_addrlen;
    /*fd_set rfds; //set dos descriptors das sockets
    int client_sockets_fd[NJMAX] = {0}; //array com socket descriptors de todos os clientes*/

    //criação da socket stream
    if ((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("[ERRO] Não foi possível criar socket stream");
        exit(-1);
    }

    //inicializar socket STREAM
    streamsv_addr.sun_family = AF_UNIX;
    memset(streamsv_addr.sun_path, 0, sizeof(streamsv_addr.sun_path));
    strcpy(streamsv_addr.sun_path, JMMSERVSS); //definir path para a socket (vai ficar no /tmp/...)
    streamsv_addrlen = sizeof(streamsv_addr.sun_family) + strlen(streamsv_addr.sun_path);

    //remover ficheiro socket caso este já tenha sido criado previamente 
    unlink(JMMSERVSS);

    //ligar server à socket (bind)
    if (bind(sd_stream, (struct sockaddr*)&streamsv_addr, streamsv_addrlen) < 0)
    {
        perror("[ERRO] Não consegui dar bind da socket stream");
        exit(-1);
    }


    //socket stream vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if (listen(sd_stream, NJMAX) < 0)
    {
        perror("[ERRO] Listen deu erro");
        exit(-1);
    }

    printf("[INFO] À espera de receber conexões dos clientes...\n");

    while (true)
    {
        //aceitar novos jogadores
        client_addrlen = sizeof(client_addr);
        if ((new_sock = accept(sd_stream, (struct sockaddr*)&client_addr, &client_addrlen)) < 0)
        {
            perror("[ERRO] Accept deu erro");
            exit(-1);
        }

        printf("[AVISO] Novo cliente conectado.\n");

        //ler mensagem (em princípio será um cnj tendo em conta que são novos jogadores)
        if (read(new_sock, &buffer_stream, sizeof(buffer_stream)) <= 0)
        {
            perror("[ERRO] Erro a ler stream");
            exit(-1);
        }

        if (buffer_stream.command == CNJ)
        {
            printf("[INFO] SERVER_STREAM: O jogador %s:%d deseja começar um novo jogo.\n", buffer_stream.arg1.name, new_sock);

            //criação de novas threads de jogo
            for (int i = 0; i < NJMAX; i++)
            {
                if (game_instances[i] == NULL) //verificar onde há uma posição vazia
                {
                    //começar novo jogo
                    new_game_info game_info = { .game_number = i, .sd = new_sock, .sock_stream = sd_stream, .buffer_s = buffer_stream };
                    if (pthread_create(&thread_gameinstance[i], NULL, thread_func_gameinstance, (void*)&game_info) != 0)
                    {
                        printf("[ERRO] Criação da thread de jogo nº%d falhou\n", i);
                    }
                    else
                    {
                        printf("[INFO] Thread de jogo nº%d criada com sucesso\n", i);
                        //enviar mensagem ao jogador para lhe confirmar que request foi aceite
                        if (write(new_sock, GAME_ACCEPTED, strlen(GAME_ACCEPTED) + 1) < 0)
                        {
                            perror("[ERRO] Erro no envio de stream");
                        }
                    }
                    break; //já criámos thread, podemos prosseguir
                }
                else if (game_instances[i] != NULL && i == NJMAX - 1)
                {
                    //informar que servidor está cheio
                    if (write(new_sock, GAME_DENIED, strlen(GAME_DENIED) + 1) < 0)
                    {
                        perror("[ERRO] Erro no envio de stream");
                    }
                    close(new_sock);
                }

            }

        }
        else printf("[AVISO] Lixo no buffer?\n");
    }

    return NULL;
}


/*####################################################### main ######################################################*/

int main()
{
    struct sockaddr_un dgramsv_addr;
    socklen_t dgramsv_addrlen;

    //set signal
    signal(SIGTERM, exit_handler); 
    signal(SIGINT, exit_handler); 

    //set seed
    srand(time(NULL));

    //iniciar thread aceita jogos
    if (pthread_create(&thread_acceptgames, NULL, thread_func_acceptgames, NULL) != 0)
    {
        perror("[ERRO] Criação da thread acceptgames falhou\n");
    }
    else printf("[INFO] ACCEPTGAMES criado\n");

    //criação da socket datagrama
    if ((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("[ERRO] Não foi possível criar socket datagrama");
        exit(-1);
    }

    //inicializar socket datagrama
    dgramsv_addr.sun_family = AF_UNIX;
    memset(dgramsv_addr.sun_path, 0, sizeof(dgramsv_addr.sun_path));
    strcpy(dgramsv_addr.sun_path, JMMSERVSD);
    dgramsv_addrlen = sizeof(dgramsv_addr.sun_family) + strlen(dgramsv_addr.sun_path);

    //remover ficheiro socket caso este já tenha sido criado previamente
    unlink(JMMSERVSD);

    //ligar server à socket (bind)
    if (bind(sd_datagram, (struct sockaddr*)&dgramsv_addr, dgramsv_addrlen) < 0)
    {
        perror("[ERRO] Não consegui dar bind da socket datagrama");
        exit(-1);
    }

    //ler datagramas
    while (true)
    {
        client_addrlen = sizeof(client_addr);
        if (recvfrom(sd_datagram, &buffer_dgram, sizeof(buffer_dgram), 0, (struct sockaddr*)&client_addr, &client_addrlen) < 0)
        {
            perror("[ERRO] Erro na recepção de datagrama");
        }
        else
        {
            datagram_handler(sd_datagram, client_addr, client_addrlen); //processar request
        }
    }

    return 0;
}