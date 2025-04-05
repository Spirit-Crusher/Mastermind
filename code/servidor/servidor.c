#include "servidor.h"

pthread_t thread_acceptgames; //thread que aceita novos jogos
pthread_t thread_gameinstance[NJMAX]; //threads responsáveis por gerir os vários jogos //!acho que isto pode ser local ao acceptgames
game_t* game_instances[NJMAX] = { 0 }; //apontadores para jogos são guardados para poderem ser alterados
struct sockaddr_un client_addr; //endereço do cliente
socklen_t client_addrlen; //não será igual ao server_addrlen? (acho que posso eliminar esta variável e passar o server_addrlen para addrlen)
rules_t global_game_rules = { .maxj = MAXNJ, .maxt = MAXT * 60 }; //criação e iniciaização das regras do jogo
bool ledger_on = true; //usado para verificar se registo de jogos está ou não ativo
char log_server[] = "pwd & ../log_server/JMMlog &"; //caminho relativo para log_server
int sd_datagram; //socket descriptor do servidor para datagrama
int sd_stream; //socket descriptor do servidor para stream
pthread_mutex_t rules_mutex; //mutex para impedir que regras sejam alteradas enquanto está a ser criado novo jogo
pthread_mutex_t save_mutex; //mutex para impedir que 2 threads tentem inicializar log_server simultâneamente


/*####################################################### exit_handler ######################################################*/

void exit_handler()
{
    printf("{SERVER} [INFO] Exiting...\n");

    close(sd_datagram);
    close(sd_stream);

    unlink(JMMSERVSD);
    unlink(JMMSERVSS);

    //libertar memória de todos os jogos alocados dinamicamente
    for (int i = 0; i < NJMAX; ++i)
    {
        if (game_instances[i] != NULL)
        {
            close(game_instances[i]->sd);
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
        printf("{SERVER} [INFO] O jogador perdeu por falta de TEMPO\n");
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
    
    // verificar se o jogo acabou
    if (np == game_pt->n_char)
    {
        // jogador ganhou
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_WIN;
        printf("{SERVER} [INFO] O jogador ganhou\n");
    }
    else if (game_pt->game_rules.maxj <= game_pt->log.nt)
    {
        // jogador sem mais tentativas, perdeu
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_LOST_TRIES;
        printf("{SERVER} [INFO] O jogador perdeu por falta de TENTATIVAS\n");
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
    pthread_mutex_lock(&rules_mutex);
    game->game_rules.maxj = global_game_rules.maxj;
    game->game_rules.maxt = global_game_rules.maxt;
    pthread_mutex_unlock(&rules_mutex);

    //gerar key com dimensão e letras diferentes conforme nivel de dificuldade
    if (buffer_stream.arg2.n == DIFF_1)
    {
        game->n_char = 3;
        char key[game->n_char + 1];
        generate_key(key, DIFF_1);
        printf("{SERVER} [INFO] Key:%s\n", key);
        if (strcpy(game->correct_sequence, key) != NULL) printf("{SERVER} [INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else if (buffer_stream.arg2.n == DIFF_2)
    {
        game->n_char = 5;
        char key[game->n_char + 1];
        generate_key(key, DIFF_2);
        printf("{SERVER} [INFO] Key: %s\n", key);
        if (strcpy(game->correct_sequence, key) != NULL) printf("{SERVER} [INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else
    {
        printf("{SERVER} [ERRO] Problemas a gerar a key\n");
        exit_handler();
    }

    printf("{SERVER} [INFO] Init do jogo funcionou\n");

    return;
}


/*####################################################### stream_handler ######################################################*/

void stream_handler(int socket, game_t* game, coms_t buffer_stream)
{
    if (buffer_stream.command == JG)
    {
        //analisar jogada
        printf("{SERVER} [INFO] SERVER_STREAM: O jogador com descriptor %d deseja fazer a seguinte jogada: %s\n", socket, buffer_stream.arg1.move);
        strcpy(game->player_move, buffer_stream.arg1.move);
        printf("{SERVER} Move: %s    Correct move: %s\n", game->player_move, game->correct_sequence);
        analise_move(game);
        printf("{SERVER} [INFO] Jogada analisada com sucesso\n");
        printf("{SERVER} Gamestate = %d\n", game->game_state);
    }
    else
    {
        printf("{SERVER} [ERRO] Ops... Isto não era suposto acontecer.\n");
        exit_handler();
    }

    return;
}


/*####################################################### datagram_handler ######################################################*/

void datagram_handler(int sd, struct sockaddr_un client_addr, socklen_t client_addrlen, coms_t buffer_dgram)
{
    char buffer_send[MAX_BUFFER_SIZE]; //buffer para mensagem de resposta
    switch (buffer_dgram.command)
    {
        case CLM:
            //enviar regras globais atuais
            sprintf(buffer_send, "Jogadas máximas:%d    Tempo máximo:%d sec\n", global_game_rules.maxj, global_game_rules.maxt);
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
            }
            break;        
            
        case MLM:
            //alterar as regras
            pthread_mutex_lock(&rules_mutex);
            global_game_rules.maxj = buffer_dgram.arg1.j;
            global_game_rules.maxt = buffer_dgram.arg2.t;
            pthread_mutex_unlock(&rules_mutex);

            printf("{SERVER} [AVISO] Regras globais alteradas: jmax=%d tmax=%d sec\n", global_game_rules.maxj, global_game_rules.maxt);

            sprintf(buffer_send, "[INFO] Regras alteradas com sucesso.\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
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
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
            }    
            break;

        case AER:
            //ativar registo
            ledger_on = true;
            sprintf(buffer_send, "[INFO] Registo ativado com sucesso\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
            }    
            break;

        case DER:
            //desativar registo
            ledger_on = false;
            sprintf(buffer_send, "[INFO] Registo desativado com sucesso\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
            }
            break;

        case TMM:
            //ordem para sair
            sprintf(buffer_send, "[INFO] Servidor vai terminar\n");
            if (sendto(sd, buffer_send, strlen(buffer_send)+1, 0, (struct sockaddr*)&client_addr, client_addrlen) < 0)
            {
                perror("{SERVER} [ERRO] Erro no envio de datagrama\n\a");
            }
            exit_handler();
            break;

        default:
            printf("{SERVER} [ERRO] Comando datagrama desconhecido.\n");
            break;
    }
}


/*####################################################### save_game ######################################################*/

void save_game(rjg_t log) 
{
    int mqids;

    //tentar abrir queue
    pthread_mutex_lock(&save_mutex);
    if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0) 
    {
        perror("{SERVER} [ERRO] Erro a associar a queue servidor\n\a");
        printf("{SERVER} [INFO] Vou fazer o system\n");
        system(log_server); //se falhou começar log_server
        sleep(1); //esperar inicialização do servidor
        printf("{SERVER} [INFO] Servidor iniciou log_server\n");

        //tentar novamente
        if ((mqids = mq_open(JMMLOGQ, O_RDWR)) < 0)
        {
            perror("{SERVER} [ERRO] Servidor não conseguiu abrir queue do log\n\a");
            return; //se falhou novamente, desistir
        }

    }
    pthread_mutex_unlock(&save_mutex);

    //enviar registo ao log_server
    printf("{SERVER} [INFO] A enviar jogo ao log_server, MQIDS:%d\n", mqids);
    if (mq_send(mqids, (char*)&log, sizeof(log), 0) < 0) 
    {
        perror("{SERVER} [ERRO] Erro a enviar mensagem\n\a");
    }
}


/*####################################################### thread_func_gameinstance ######################################################*/

void* thread_func_gameinstance(void* game_info)
{
    new_game_info info = (*(new_game_info*)game_info);
    coms_t buffer_stream = info.buffer_s;
    int game_number = info.game_number;
    int socket = info.sd;
    char result[MAX_BUFFER_SIZE];
    int bytes;

    printf("{SERVER} [INFO] Esta é a socket e o game number do novo jogador: %d:%d\n", socket, game_number);

    game_t* current_game = NULL;
    //criar estrutura do novo jogo
    current_game = malloc(sizeof(game_t));
    if (current_game == NULL)
    {
        perror("{SERVER} [ERRO] Falha a alocar estrutura do jogo\n\a");
        exit_handler();
    }

    //guardar novo jogo no vetor de jogos
    game_instances[game_number] = current_game;

    //inicializar jogo
    create_new_game(current_game, socket, buffer_stream);
    printf("{SERVER} [INFO] Jogo nº%d foi iniciado\n", game_number);

    //definir timeouts
    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    do
    {
        while ((time(NULL)-current_game->log.ti) < current_game->game_rules.maxt) //verificar se tempo máximo foi atingido
        {
            //ler jogadas do jogador e processar as mesmas enquanto o jogo estiver a decorrer
            bytes = read(socket, &buffer_stream, sizeof(buffer_stream));
            printf("{SERVER} [INFO] Bytes: %d\n", bytes);
            if (bytes <= 0 && errno != EWOULDBLOCK)
            {
                //jogador disconectou-se
                current_game->game_state = DISCONNECT;
                break;
            }
            else if ((bytes <= 0) && (errno == EWOULDBLOCK))
            {
                //vamos tentar ler novamente
                errno = -1;
                current_game->game_state = DISCONNECT;
                continue;
            }
            else
            {
                //processar request
                stream_handler(socket, current_game, buffer_stream);
                break;
            } 
        }

        switch (current_game->game_state)
        {
            case ONGOING:
                //mostrar resultado da jogada
                sprintf(result, "[INFO] nb:%d np:%d time_left:%.0f sec\n", current_game->nb, current_game->np, current_game->game_rules.maxt - current_game->elapsed_time);
                printf("{SERVER} [INFO] Resultado da jogada: %s\n", result);
                if (write(socket, result, strlen(result) + 1) < 0)
                {
                    perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                }
                break;

            case PLAYER_LOST_TIME:
                //informar jogador da derrota
                printf("{SERVER} [INFO] O jogador perdeu(tempo)\n");
                if (write(socket, GAME_LOST_TIME, strlen(GAME_LOST_TIME) + 1) < 0)
                {
                    perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                }
                break;

            case PLAYER_LOST_TRIES:
                //informar jogador da derrota
                printf("{SERVER} [INFO] O jogador perdeu(tentativas)\n");
                if (write(socket, GAME_LOST_TRIES, strlen(GAME_LOST_TRIES) + 1) < 0)
                {
                    perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                }
                break;

            case PLAYER_WIN:
                //informar jogador da vitória
                if (write(socket, GAME_WON, strlen(GAME_WON) + 1) < 0)
                {
                    perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                }
                
                printf("{SERVER} [INFO] O jogador ganhou\n");
                if (ledger_on) {
                    //enviar log para o cliente
                    save_game(current_game->log);
                    printf("{SERVER} [INFO] Ledger ligado. Jogo guardado\n");
                }
                else printf("{SERVER} [INFO] Ledger desligado\n");
                break;

            case DISCONNECT:
                //fazer disconexão do cliente
                printf("{SERVER} [AVISO] Cliente desconectou-se\n\a");
                break;

            default:
                if (write(socket, GAME_CRASHED, strlen(GAME_CRASHED) + 1) < 0)
                {
                    perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                }
                break;
        }

    } while (current_game->game_state == ONGOING);

    //terminar jogo
    close(socket);
    free(current_game);
    game_instances[game_number] = NULL;
    printf("{SERVER} [INFO] GAME N%d TERMINATED\n", game_number);

    return NULL;
}

/*####################################################### thread_func_acceptgames ######################################################*/

void* thread_func_acceptgames()
{
    coms_t buffer_stream;
    int new_sock;
    struct sockaddr_un streamsv_addr;
    socklen_t streamsv_addrlen;

    //criação da socket stream
    if ((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("{SERVER} [ERRO] Não foi possível criar socket stream\n\a");
        exit_handler();
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
        perror("{SERVER} [ERRO] Não consegui dar bind da socket stream\n\a");
        exit_handler();
    }

    //socket stream vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if (listen(sd_stream, NJMAX) < 0)
    {
        perror("{SERVER} [ERRO] Listen deu erro\n\a");
        exit_handler();
    }

    printf("{SERVER} [INFO] À espera de receber conexões dos clientes...\n");

    while (true)
    {
        //aceitar novos jogadores
        client_addrlen = sizeof(client_addr);
        if ((new_sock = accept(sd_stream, (struct sockaddr*)&client_addr, &client_addrlen)) < 0)
        {
            perror("{SERVER} [ERRO] Accept deu erro\n\a");
            exit_handler();
        }

        printf("{SERVER} [AVISO] Novo cliente conectado.\n");

        //ler mensagem (em princípio será um cnj tendo em conta que são novos jogadores)
        if (read(new_sock, &buffer_stream, sizeof(buffer_stream)) <= 0)
        {
            perror("{SERVER} [ERRO] Erro a ler stream\n\a");
            exit_handler();
        }

        if (buffer_stream.command == CNJ)
        {
            printf("{SERVER} [INFO] SERVER_STREAM: O jogador %s:%d deseja começar um novo jogo.\n", buffer_stream.arg1.name, new_sock);

            //criação de novas threads de jogo
            for (int i = 0; i < NJMAX; i++)
            {
                if (game_instances[i] == NULL) //verificar onde há uma posição vazia
                {
                    //começar novo jogo
                    new_game_info game_info = { .game_number = i, .sd = new_sock, .sock_stream = sd_stream, .buffer_s = buffer_stream };
                    if (pthread_create(&thread_gameinstance[i], NULL, thread_func_gameinstance, (void*)&game_info) != 0)
                    {
                        printf("{SERVER} [ERRO] Criação da thread de jogo nº%d falhou\n", i);
                    }
                    else
                    {
                        printf("{SERVER} [INFO] Thread de jogo nº%d criada com sucesso\n", i);
                        //enviar mensagem ao jogador para lhe confirmar que request foi aceite
                        if (write(new_sock, GAME_ACCEPTED, strlen(GAME_ACCEPTED) + 1) < 0)
                        {
                            perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                        }
                    }
                    break; //já criámos thread, podemos prosseguir
                }
                else if (game_instances[i] != NULL && i == NJMAX - 1)
                {
                    //informar que servidor está cheio
                    printf("{SERVER} [AVISO] Servidor cheio, a recusar pedido...\n");
                    if (write(new_sock, GAME_DENIED, strlen(GAME_DENIED) + 1) < 0)
                    {
                        perror("{SERVER} [ERRO] Erro no envio de stream\n\a");
                    }
                    close(new_sock);
                }

            }

        }
        else printf("{SERVER} [AVISO] Lixo no buffer?\n");
    }

    return NULL;
}


/*####################################################### main ######################################################*/

int main()
{
    coms_t buffer_dgram;
    struct sockaddr_un dgramsv_addr;
    socklen_t dgramsv_addrlen;

    //definir exit_handler signal
    signal(SIGTERM, exit_handler); 
    signal(SIGINT, exit_handler); 

    //definir seed
    srand(time(NULL));

    //iniciar mutexes
    if (pthread_mutex_init(&rules_mutex, NULL) != 0)
    {
        printf("{SERVER} [ERRO] Erro a inicializar rules_mutex\n");
    }
    if (pthread_mutex_init(&save_mutex, NULL) != 0)
    {
        printf("{SERVER} [ERRO] Erro a inicializar save_mutex\n");
    }
    
    //iniciar thread aceita jogos
    if (pthread_create(&thread_acceptgames, NULL, thread_func_acceptgames, NULL) != 0)
    {
        perror("{SERVER} [ERRO] Criação da thread acceptgames falhou\n\a");
    }
    else printf("{SERVER} [INFO] ACCEPTGAMES criado\n");

    //criação da socket datagrama
    if ((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("{SERVER} [ERRO] Não foi possível criar socket datagrama\n\a");
        exit_handler();
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
        perror("{SERVER} [ERRO] Não consegui dar bind da socket datagrama\n\a");
        exit_handler();
    }

    //ler datagramas
    while (true)
    {
        client_addrlen = sizeof(client_addr);
        if (recvfrom(sd_datagram, &buffer_dgram, sizeof(buffer_dgram), 0, (struct sockaddr*)&client_addr, &client_addrlen) <= 0)
        {
            perror("{SERVER} [ERRO] Erro na recepção de datagrama\n\a");
        }
        else
        {
            datagram_handler(sd_datagram, client_addr, client_addrlen, buffer_dgram); //processar request
        }
    }

    return 0;
}